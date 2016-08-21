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

template<class Key, class Component, class Allocator = std::allocator<Component>>
struct Schema {
	typedef radiance::Table<Key, Component, Allocator> Table;
	typedef radiance::View<Table> View;
	typedef typename Table::Mutation Mutation;
	typedef radiance::MutationQueue<Table> MutationQueue;

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