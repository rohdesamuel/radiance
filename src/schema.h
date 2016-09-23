/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#ifndef SCHEMA__H
#define SCHEMA__H

#include "common.h"
#include "pipeline.h"
#include "system.h"
#include "table.h"

namespace radiance
{

template<class Key_, class Component_,
         class Allocator_ = std::allocator<Component_>>
struct Schema {
  typedef radiance::Table<Key_, Component_, Allocator_> Table;
  typedef radiance::View<Table> View;
  typedef typename Table::Mutation Mutation;
  typedef typename Table::Element Element;
  typedef radiance::MutationQueue<Table> MutationQueue;
  typedef Key_ Key;
  typedef Component_ Component;

  static std::function<void(SystemQueue&&)> make_system_queue() {
    return [](SystemQueue&& queue) {
      for (auto& system : queue) {
        system();
      }
    };
  }
};

}

#endif
