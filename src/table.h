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

#include "common.h"
#include "system.h"

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
  public:
    template<class System>
    static void read(Table* table, System system, IndexedBy indexed_by) {
      switch (indexed_by) {
        case IndexedBy::OFFSET:
          for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(Element{ indexed_by, (Offset)i, table->component(i) });
            system(&frame);
          }
          break;
        case IndexedBy::HANDLE:
          for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(
                Element{
                  indexed_by,
                  (Handle)table->find(table->key(i)),
                  table->component(i)
                });
            system(&frame);
          }
          break;
        case IndexedBy::KEY:
          for (int64_t i = 0; i < (int64_t)table->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(Element{ indexed_by, (Key)table->key(i), table->component(i) });
            system(&frame);
          }
          break;
        case IndexedBy::UNKNOWN:
          break;
      }
    }
  };

  class Writer {
  public:
    static void write(Table* table, Frame* frame) {
      Element* el = frame->result<Element>();
      switch (el->indexed_by) {
        case IndexedBy::OFFSET:
          table->component(el->offset) = std::move(el->component);
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
  public:
    template<class System>
    static void read(View* view, System system, IndexedBy indexed_by) {
      switch (indexed_by) {
        case IndexedBy::OFFSET:
          for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(Element{ indexed_by, (Offset)i, view->component(i) });
            system(&frame);
          }
          break;
        case IndexedBy::HANDLE:
          for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(Element{ indexed_by, (Handle)view->find(view->key(i)), view->component(i) });
            system(&frame);
          }
          break;
        case IndexedBy::KEY:
          for (int64_t i = 0; i < (int64_t)view->size(); ++i) {
            thread_local static Frame frame;
            frame.clear();
            frame.result(Element{ indexed_by, (typename Table::Key)view->key(i), view->component(i) });
            system(&frame);
          }
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
class MutationQueue {
public:
  typedef typename Table::Mutation Mutation;

  const uint64_t INITIAL_SIZE = 4096;

  MutationQueue() : mutations_(INITIAL_SIZE) {}

  const std::function<void(Table*, Mutation&&)> default_resolver = [=](Table* table, Mutation m) {
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
    push(Mutation{ mutate_by,{ indexed_by, std::move(index), std::move(component) } });
  }

  uint64_t flush(Table* table) {
    return mutations_.consume_all([=](Mutation m) {
      default_resolver(table, std::move(m));
    });
  }

  template<class Resolver>
  uint64_t flush(Table* table, Resolver r) {
    return mutations_.consume_all([=](Mutation m) {
      r(table, std::move(m));
    });
  }

private:
  ::boost::lockfree::queue<Mutation> mutations_;
};

}

#endif
