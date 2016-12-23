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
struct CounterBase {
 protected:
  static Id counter_;
};

template<class Type_>
struct Counter : CounterBase {
  static Id id() {
    static Id id = counter_++;
    return id;
  }
};

template<class... Data_>
struct Component {
};

template<class Type_>
struct Data {
  static Id id() {
    return Counter<Component<Type_>>::id();
  }
};

struct ComponentMetadata {
  Id id;
  std::vector<Id> dependencies;
};

struct SystemMetadata {
  Id id;
  std::vector<Id> families;
  System system;
};

struct DataType {
  Id id;
  void* ptr;
};

class EntityManager {};

class SystemManager {
 private:
  std::vector<SystemMetadata> system_metadata_;

 public:
  template<typename... Components_>
  Id add(System system) {
    Id ret = system_metadata_.size();
    system_metadata_.push_back(
        { ret, { Components_::id()... }, std::move(system) });
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
  Id add() {
    Id ret = component_metadata_.size();
    component_metadata_.push_back(
        { ret, { Data_::id()... } });
    return ret;
  }

  const std::vector<Id>& dependencies(Id id) const {
    return component_metadata_[id].dependencies;
  }
};

class DataManager {
 private:
  std::vector<DataType> data_types_;

 public:
  template<typename Collection_>
  Id add(Collection_* collection) {
    Id ret = data_types_.size();
    data_types_.push_back({ ret, (void*)collection });
    return ret;
  };

  DataType get(Id id) {
    return data_types_[id];
  }
};

}

#endif
