/**
* Author: Samuel Rohde (rohde.samuel@gmail.com)
*
* This file is subject to the terms and conditions defined in
* file 'LICENSE.txt', which is part of this source code package.
*/

#include <cstdint>

#include <iostream>
#include <sstream>

#include <string.h>
#include <time.h>

#include "radiance.h"

#include "console_draw.h"
#include "timer.h"
#include "vector_math.h"

using namespace radiance;

struct Transformation {
	Vec3 p;
	Vec3 v;
};

typedef ::boost::container::vector<std::tuple<Handle, float>> Spring;

typedef Schema<Handle, Spring> SpringsSchema;
typedef Schema<Entity, Transformation> TransformationSchema;
typedef SpringsSchema::Table Springs;
typedef TransformationSchema::Table Transformations;

template<class Source, class Sink>
class Compiler {
public:
	virtual Pipeline<Source, Sink> compile(const std::string& name, Source* source, Sink* sink) = 0;
};
/*
template<class Source, class Sink>
class PhysicsModule : public Compiler<Source, Sink> {
public:
	void move(Frame* frame) {
		auto* el = frame->result<Transformations::Element>();

		float l = el->component.v.length();
		el->component.v.normalize();
		el->component.v *= min(l, 10.0f);
		el->component.p += el->component.v;
	}

	void bound_position(Frame* frame) {
		auto* el = frame->result<Transformations::Element>();

		if (el->component.p.x <= 0 || el->component.p.x >= MAX_WIDTH - 1) {
			el->component.p.x = min(max(el->component.p.x, 0), MAX_WIDTH - 1);
			el->component.v.x *= -0.9f;
		}

		if (el->component.p.y <= 0 || el->component.p.y >= MAX_HEIGHT - 1) {
			el->component.p.y = min(max(el->component.p.y, 0), MAX_HEIGHT - 1);
			el->component.v.y *= -0.9f;
		}
	}

	Pipeline<Source, Sink> compile(const std::string& name, Source* source, Sink* sink) override {
		return Pipeline<Source, Sink>(source, sink, System(&PhysicsModule::bound_position) * System(&PhysicsModule::move), IndexedBy::OFFSET);
	}
};*/

int main() {
	const int MAX_HEIGHT = 640 / 8;
	const int MAX_WIDTH = 640 / 8;

	int32_t life_grid_buffer[2][MAX_HEIGHT][MAX_WIDTH];
	int32_t(*read)[80][80] = &life_grid_buffer[0];
	int32_t(*write)[80][80] = &life_grid_buffer[1];

	HANDLE hCD = CD_CreateWindow(0, 0, MAX_WIDTH, MAX_HEIGHT);

	Springs springs;
	Transformations transformations;
	//PhysicsModule<Transformations, Springs> physics_mod;
	
	TransformationSchema::MutationQueue transformations_queue;

	uint64_t count = 1000;

	for (uint64_t i = 0; i < count; ++i) {
		Vec3 p = { (float)(rand() % MAX_WIDTH), (float)(rand() % MAX_HEIGHT), 0 };
		Vec3 v;
		v.x = (((float)(rand() % 10000)) - 5000.0f) / 50000.0f;
		v.y = (((float)(rand() % 10000)) - 5000.0f) / 50000.0f;
		transformations_queue.emplace<MutateBy::INSERT, IndexedBy::OFFSET>((int64_t)i, { p, v });
	}
	transformations_queue.flush(&transformations);
	srand((uint32_t)time(NULL));
	for (uint64_t i = 0; i < count; ++i) {
		for (uint64_t j = 0; j < count; ++j) {
			springs.insert(i, Springs::Component({ { j, 0.0000001 } }));
		}
	}

	TransformationSchema::View transformations_view(&transformations);
	SpringsSchema::View springs_view(&springs);

	auto spring = System([=](Frame* frame) {
		auto* el = frame->result<SpringsSchema::View::Element>();

		Handle handle = el->key;
		Transformation accum = transformations_view[handle];

		for (const auto& connection : el->component) {
			const Handle& other = std::get<0>(connection);
			const float& spring_k = std::get<1>(connection);
			Vec3 p = transformations_view[other].p - transformations_view[handle].p;
			//if (p.length() < 10) {
			accum.v += p * spring_k;
			//}
		}
		
		frame->result(Transformations::Element{ IndexedBy::HANDLE , handle, accum});
	});

	auto move = System([=](Frame* frame) {
		auto* el = frame->result<Transformations::Element>();

		float l = el->component.v.length();
		el->component.v.normalize();
		el->component.v *= min(l, 10.0f);
		el->component.p += el->component.v;
	});

	auto bound_position = System([=](Frame* frame) {
		auto* el = frame->result<Transformations::Element>();

		if (el->component.p.x <= 0 || el->component.p.x >= MAX_WIDTH - 1) {
			el->component.p.x = min(max(el->component.p.x, 0), MAX_WIDTH - 1);
			el->component.v.x *= -0.9f;
		}

		if (el->component.p.y <= 0 || el->component.p.y >= MAX_HEIGHT - 1) {
			el->component.p.y = min(max(el->component.p.y, 0), MAX_HEIGHT - 1);
			el->component.v.y *= -0.9f;
		}
	});

	auto draw_system = System([&](Frame* frame) {
		auto* el = frame->result<TransformationSchema::View::Element>();
		auto& c = el->component;
		CD_SetBufferChar(hCD, (int)c.p.x, (int)c.p.y, ' ', CD_BG_BLUE | CD_BG_INTENSE);
		/*CD_SetBufferChar(hCD, (int)c.p.x+1, (int)c.p.y, ' ', CD_BG_BLUE | CD_BG_INTENSE);
		CD_SetBufferChar(hCD, (int)c.p.x, (int)c.p.y+1, ' ', CD_BG_BLUE | CD_BG_INTENSE);
		CD_SetBufferChar(hCD, (int)c.p.x+1, (int)c.p.y+1, ' ', CD_BG_BLUE | CD_BG_INTENSE);*/
	});

	float delta_time = 1.0;

	auto physics_pipeline = TransformationSchema::make_system_queue();

	Pipeline<SpringsSchema::View, Transformations> springs_pipeline(&springs_view, &transformations, spring, IndexedBy::KEY);
	Pipeline<Transformations, Transformations> transformations_pipeline(&transformations, &transformations, bound_position * move, IndexedBy::OFFSET);
	Pipeline<TransformationSchema::View, void> draw_pipeline(&transformations_view, draw_system, IndexedBy::OFFSET);

	WindowTimer fps(60);
	uint64_t frame = 0;
	while (1) {
		CD_Refresh(hCD);

		fps.start();
		if (frame % 1 == 0) {
			physics_pipeline({ springs_pipeline, transformations_pipeline, draw_pipeline });
		} else {
			physics_pipeline({ transformations_pipeline, draw_pipeline });
		}
		fps.stop();
		fps.step();


		char buf[64] = { 0 };
		sprintf_s(buf, "%.3fms", fps.get_avg_elapsed_ns() / 1e6);
		std::string fps_str(buf);

		while (fps_str.size() < 10) {
			fps_str = " " + fps_str;
		}

		WORD color = (fps.get_avg_elapsed_ns() / 1e6) > 10 ? CD_FG_RED : CD_FG_GREEN;
		for (int i = 0; i < fps_str.size(); ++i) {
			CD_SetBufferChar(hCD, i, 0, fps_str[i], color | CD_FG_INTENSE);
		}

		CD_DrawScreen(hCD);

		++frame;
	}
	while (1);
}