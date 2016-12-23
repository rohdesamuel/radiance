/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef TABLE__H
#define TABLE__H

#ifdef __COMPILE_AS_WINDOWS__
#define _ENABLE_ATOMIC_ALIGNMENT_FIX
#endif

#include <boost/container/map.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/container/vector.hpp>
#include <iostream>

#include "common.h"
#include "component.h"
#include "system.h"
#include "operators.h"

namespace radiance
{

enum class IndexedBy {
  UNKNOWN = 0,
  OFFSET,
  HANDLE,
  KEY
};

enum class MutateBy {
  UNKNOWN = 0,
  INSERT,
  REMOVE,
  WRITE
};

template<class Key, class Component>
struct BaseElement {
  IndexedBy indexed_by;
  union {
    Offset offset;
    Handle handle;
    Key key;
  };
  Component component;
};

template <class K, class C, class Allocator = std::allocator<C>>
class Table {
public:
  typedef K Key;
  typedef C Component;

  typedef BaseElement<Key, Component> Element;

  typedef ::boost::container::vector<Key> Keys;
  typedef ::boost::container::vector<Component> Components;

  // For fast lookup if you have the handle to an entity.
  typedef ::boost::container::vector<uint64_t> Handles;
  typedef ::boost::container::vector<Handle> FreeHandles;

  // For fast lookup by Entity Id.
  typedef ::boost::container::map<Key, Handle> Index;

  Table() {}

  Table(boost::container::vector<std::tuple<Key, Component>>&& init_data) {
    for (auto& t : init_data) {
      insert(std::move(std::get<0>(t)), std::move(std::get<1>(t)));
    }
  }

  struct Mutation {
    MutateBy mutate_by;
    BaseElement<Key, Component> el;
  };

  class Reader {
  private:
    IndexedBy indexed_by_;

    static void iterate_table(Table* table, SystemExecutor& system,
                              std::function<Element(Table*, int64_t)>&& reader) {
#if 1
#pragma omp parallel for
      for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
        thread_local static Frame frame;
        frame.clear();
        frame.result(reader(table, i));
        system(&frame);
      }
#else
#pragma omp parallel
      {
        auto it = system.begin();
        auto end = system.end();
        thread_local static std::vector<Frame> frames;
        if (table->size() > frames.size()) {
          frames.resize(table->size());
        }

        do {
          System& sys = it->system;
#pragma omp parallel for
          for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
            Frame& frame = frames[i];
            if (it == system.begin()) {
              frame.clear();
              frame.result(reader(table, i));
            }
            sys(&frame);
          }
          ++it;
        } while(it != end);

#pragma omp parallel for
        for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
          Frame& frame = frames[i];
          (system.back())(&frame);
        }
      }
#endif
    }

    inline static Element get_by_offset(Table* table, int64_t index) {
      return {
        IndexedBy::OFFSET,
        (Offset)index,
        table->component(index)
      };
    }

    inline static Element get_by_handle(Table* table, int64_t index) {
      return {
        IndexedBy::HANDLE,
        (Handle)table->find(table->key(index)),
        table->component(index)
      };
    }

    inline static Element get_by_key(Table* table, int64_t index) {
      return {
        IndexedBy::KEY,
        (Key)table->key(index),
        table->component(index)
      };
    }

  public:
    Reader(): indexed_by_(IndexedBy::UNKNOWN) {};
    Reader(IndexedBy indexed_by): indexed_by_(indexed_by) {}

    constexpr bool is_thread_safe() {
      return true;
    }

    void operator()(Table* table, SystemExecutor& system) const {
      read(table, system, indexed_by_);
    }

    static void read(Table* table, SystemExecutor& system,
                     IndexedBy indexed_by) {
      switch (indexed_by) {
        case IndexedBy::OFFSET: 
          iterate_table(table, system, get_by_offset);
          break;
        case IndexedBy::HANDLE:
          iterate_table(table, system, get_by_handle);
          break;
        case IndexedBy::KEY:
          iterate_table(table, system, get_by_key);
          break;
        case IndexedBy::UNKNOWN:
          break;
      }
    }
  };

  class Writer {
  public:
    Writer() {}
    
    constexpr bool is_thread_safe() {
      return false;
    }

    static void write(Table* table, Frame* frame) {
      Element* el = frame->result<Element>();
      switch (el->indexed_by) {
        case IndexedBy::OFFSET:
          table->components[el->offset] = std::move(el->component);
          break;
        case IndexedBy::HANDLE:
          (*table)[el->handle] = std::move(el->component);
          break;
        case IndexedBy::KEY:
          table->components[table->find(el->key)] = std::move(el->component);
          break;
        case IndexedBy::UNKNOWN:
          break;
      }
    }

    void operator()(Table* table, Frame* frame) const {
      write(table, frame);
    }
  };

  Handle insert(Key&& key, Component&& component) {
    Handle handle = make_handle();

    index_[key] = handle;

    components.push_back(std::move(component));
    keys.push_back(key);
    return handle;
  }

  Handle insert(const Key& key, Component&& component) {
    Handle handle = make_handle();

    index_[key] = handle;

    components.push_back(std::move(component));
    keys.push_back(key);
    return handle;
  }

  Component& operator[](Handle handle) {
    return components[handles_[handle]];
  }

  const Component& operator[](Handle handle) const {
    return components[handles_[handle]];
  }

  Handle find(Key&& key) const {
    typename Index::const_iterator it = index_.find(key);
    if (it != index_.end()) {
      return it->second;
    }
    return -1;
  }

  Handle find(const Key& key) const {
    typename Index::const_iterator it = index_.find(key);
    if (it != index_.end()) {
      return it->second;
    }
    return -1;
  }

  inline Key& key(uint64_t index) {
    return keys[index];
  }

  inline const Key& key(uint64_t index) const {
    return keys[index];
  }

  inline Component& component(uint64_t index) {
    return components[index];
  }

  inline const Component& component(uint64_t index) const {
    return components[index];
  }

  uint64_t size() const {
    return components.size();
  }

  int64_t remove(Handle handle) {
    Handle h_from = handle;
    uint64_t& i_from = handles_[h_from];
    Key& k_from = keys[i_from];

    Key& k_to = keys.back();
    Handle h_to = index_[k_to];
    uint64_t& i_to = handles_[h_to];

    Component& c_from = components[i_from];
    Component& c_to = components[i_to];

    release_handle(h_from);

    index_.erase(k_from);
    std::swap(i_from, i_to);
    std::swap(k_from, k_to); keys.pop_back();
    std::swap(c_from, c_to); components.pop_back();

    return 0;
  }

  Keys keys;
  Components components;

private:
  Handle make_handle() {
    if (free_handles_.size()) {
      Handle h = free_handles_.back();
      free_handles_.pop_back();
      return h;
    }
    handles_.push_back(handles_.size());
    return handles_.back();
  }

  void release_handle(Handle h) {
    free_handles_.push_back(h);
  }

  Handles handles_;
  FreeHandles free_handles_;
  Index index_;
};

template <class T>
class View {
public:
  typedef T Table;
  typedef BaseElement<typename Table::Key, const typename Table::Component&> Element;

  View(Table* table) : table_(table) {}

  class Reader {
  private:
    IndexedBy indexed_by_;

    static void iterate_view(View* view, SystemExecutor& system,
                             std::function<Element(View*, int64_t)> reader) {
#if 0
#pragma omp parallel for
      for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
        thread_local static Frame frame;
        frame.clear();
        frame.result(reader(view, i));
        system(&frame);
      }
#else
      {
        auto it = system.begin();
        auto end = system.end();
        static std::vector<Frame> frames;
        if (view->size() > frames.size()) {
          frames.resize(view->size());
        }

        do {
          System& sys = it->system;
#pragma omp parallel for
          for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
            Frame& frame = frames[i];
            if (it == system.begin()) {
              frame.clear();
              frame.result(reader(view, i));
            }
            sys(&frame);
          }
          ++it;
        } while(it != end);

        for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
          Frame& frame = frames[i];
          (system.back())(&frame);
        }
      }
#endif
/*      auto it = system.begin();
      do {
//#pragma omp parallel for
        for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
          thread_local static std::vector<Frame> frames;
          if (view->size() > frames.size()) {
            frames.resize(view->size());
          }
          Frame& frame = frames[i];
          if (i == 0) {
            frame.clear();
          }

          frame.result(reader(view, i));
          (it->system)(&frame);
        }
        ++it;
      }
      while (it != system.end());*/
    }

    inline static Element get_by_offset(View* view, int64_t index) {
      return {
        IndexedBy::OFFSET,
        (Offset)index,
        view->component(index)
      };
    }

    inline static Element get_by_handle(View* view, int64_t index) {
      return {
        IndexedBy::HANDLE,
        (Handle)view->find(view->key(index)),
        view->component(index)
      };
    }

    inline static Element get_by_key(View* view, int64_t index) {
      return {
        IndexedBy::KEY,
        (typename Table::Key)view->key(index),
        view->component(index)
      };
    }

  public:
    Reader(): indexed_by_(IndexedBy::UNKNOWN) {}
    Reader(IndexedBy indexed_by): indexed_by_(indexed_by) {}

    constexpr bool is_thread_safe() {
      return true;
    }

    void operator()(View* view, SystemExecutor& system) const {
      read(view, system, indexed_by_);
    }

    static void read(View* view, SystemExecutor& system, IndexedBy indexed_by) {
      switch (indexed_by) {
        case IndexedBy::OFFSET:
          iterate_view(view, system, &get_by_offset);
          break;
        case IndexedBy::HANDLE:
          iterate_view(view, system, &get_by_handle);
          break;
        case IndexedBy::KEY:
          iterate_view(view, system, &get_by_key);
          break;
        case IndexedBy::UNKNOWN:
          break;
      }
    }
  };

  inline const typename Table::Component& operator[](Handle handle) const {
    return table_->operator[](handle);
  }

  inline Handle find(typename Table::Key&& key) const {
    return table_->find(std::move(key));
  }

  inline Handle find(const typename Table::Key& key) const {
    return table_->find(key);
  }

  inline const typename Table::Key& key(uint64_t index) const {
    return table_->keys[index];
  }

  inline const typename Table::Component& component(uint64_t index) const {
    return table_->components[index];
  }

  inline uint64_t size() const {
    return table_->components.size();
  }
private:
  Table* table_;
};

template<class Table>
class MutationBuffer {
public:
  typedef typename Table::Mutation Mutation;
  typedef Mutation Element;

  const uint64_t INITIAL_SIZE = 1 << 10;

  MutationBuffer() : mutations_(INITIAL_SIZE) {}

  const std::function<void(Table*, Mutation&&)> default_resolver = 
    [=](Table* table, Mutation&& m) {
        switch (m.mutate_by) {
          case MutateBy::INSERT:
            table->insert(std::move(m.el.key), std::move(m.el.component));
            break;
          case MutateBy::REMOVE:
            table->remove(table->find(m.el.key));
            break;
          case MutateBy::WRITE:
            switch (m.el.indexed_by) {
              case IndexedBy::HANDLE:
                (*table)[m.el.handle] = std::move(m.el.component);
                break;
              case IndexedBy::KEY:
                table->components[table->find(m.el.key)] = std::move(m.el.component);
                break;
              case IndexedBy::OFFSET:
                table->components[m.el.offset] = std::move(m.el.component);
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }
      };

  bool push(Mutation&& m) {
    return mutations_.push(m);
  }

  template<MutateBy mutate_by, IndexedBy indexed_by, class IndexType>
  void emplace(IndexType&& index, typename Table::Component&& component) {
    push(Mutation { 
           mutate_by, {
             indexed_by,
             std::move(index),
             std::move(component) 
           }
         });
  }

  uint64_t flush(Table* table) {
    return mutations_.consume_all([&](Mutation& m) {
      default_resolver(table, std::move(m));
    });
  }

  template<class Resolver>
  uint64_t flush(Table* table, Resolver r) {
    return mutations_.consume_all([=](Mutation m) {
      r(table, std::move(m));
    });
  }

  class Writer {
  public:
    constexpr bool is_thread_safe() {
      return true;
    }

    static void write(MutationBuffer* queue, Frame* frame) {
      Element* el = frame->result<Element>();
      queue->push(std::move(*el));
    }

    void operator()(MutationBuffer* queue, Frame* frame) const {
      write(queue, frame);
    }
  };

private:
  ::boost::lockfree::queue<Mutation> mutations_;
};

}  // namespace radiance

#endif
