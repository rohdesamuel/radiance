#ifndef COMPONENT__H
#define COMPONENT__H

#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>

#include "common.h"
#include "system.h"

/*
Invariants: 
 * Component data dependencies
 * System data dependencies

Variances:
 * If and when a system is run
 * What components attach to which entity
 * # of entities
 * pipeline systems
*/

namespace radiance
{
struct FamilyBase {
 protected:
  static Id counter_;
};

template<typename Type_>
struct Family : FamilyBase {
  static Id id() {
    static Id id = counter_++;
    return id;
  }
};

struct Data {
  Id family;
  Id id;
  void* ptr;

  Data(): family(-1), id(-1), ptr(nullptr) {}
  Data(Id family, Id id, void* ptr): family(family), id(id), ptr(ptr) {}
  ~Data() {
    family = -1;
    id = -1;
    ptr = nullptr;
  }

  Data(const Data& other) {
    family = other.family;
    id = other.id;
    ptr = other.ptr;
  }

  Data(Data&& other) {
    family = other.family;
    id = other.id;
    ptr = other.ptr;
    other.family = -1;
    other.id = -1;
    other.ptr = nullptr;
  }

  template<typename Ptr_>
  operator Ptr_*() {
    DEBUG_ASSERT(family == Counter<Ptr>::family(),
                 Status::Code::INCOMPATIBLE_DATA_TYPES);
    return (Ptr_*)ptr;
  }

  Data& operator=(const Data& other) {
    DEBUG_ASSERT(family == other.family,
                 Status::Code::INCOMPATIBLE_DATA_TYPES);
    if (this != &other) {
      id = other.id;
      ptr = other.ptr;
    }
    return *this;
  }

  Data& operator=(Data&& other) {
    DEBUG_ASSERT(family == other.family,
                 Status::Code::INCOMPATIBLE_DATA_TYPES);
    if (this != &other) {
      id = other.id;
      ptr = other.ptr;
      other.id = -1;
      other.ptr = nullptr;
    }
    return *this;
  }
};

struct ComponentMetadata {
  Id id;
  std::vector<Data> dependencies;
};

struct SystemMetadata {
  Id id;
  std::vector<Id> families;
  System system;
};

class EntityManager {
 private:
  typedef std::unordered_map<Id, std::vector<Id>> Entities;
  Entities entities_;
  Id id_counter_ = 0;

 public:
  Id add(std::vector<Id> components) {
    Id ret = id_counter_++;
    entities_[ret] = components;
    return ret;
  }

  Id instantiate(Id) {
    /*
    std::vector<Id> components = entities_[entity];
    for (Id id : components) {
      auto dependencies = universe->component_manager.dependencies(id);
    }*/
    return -1;
  }
};

class SystemManager {
 private:
  std::unordered_map<Id, SystemMetadata> system_metadata_;
  Id id_counter_ = 0;

 public:
  template<typename... Components_>
  Id add(System system) {
    Id ret = id_counter_++;
    system_metadata_[ret] =
        { ret, { Family<Components_>::id()... }, std::move(system) };
    return ret;
  }

  void remove(Id) {
  }
};

class ComponentManager {
 private:
  std::vector<ComponentMetadata> component_metadata_;

 public:
  template<typename... Data_>
  Id add(std::vector<Data> data) {
    DEBUG_OP(
        std::vector<Id> template_families = { Family<Data_>::id()... };
        for (size_t i = 0; i < data.size(); ++i) {
          DEBUG_ASSERT(data[i].family == template_families[i],
                       Status::Code::INCOMPATIBLE_DATA_TYPES);
        });
    Id ret = component_metadata_.size();
    component_metadata_.push_back({ ret, data });
    return ret;
  }

  const std::vector<Data>& dependencies(Id id) const {
    return component_metadata_[id].dependencies;
  }
};

class DataManager {
 private:
  std::vector<Data> data_types_;

 public:
  template<typename Collection_>
  Data add(Collection_* collection) {
    Id id = (Id)data_types_.size();
    Data ret = {Family<Collection_>::id(), id, (void*)collection };
    data_types_.push_back(ret);
    return ret;
  };

  template<typename Collection_, typename... Args_>
  Data emplace(Args_... args) {
    Collection_* ptr = new Collection_(std::forward<Args_>(args)...);
    Data ret = {Family<Collection_>::id(), (Id)data_types_.size(), ptr};
    data_types_.push_back(ret);
    return ret;
  }

  const Data& get(Id id) const {
    return data_types_[id];
  }
};

}  // namespace radiance

#endif
