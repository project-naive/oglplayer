#pragma once

#include "Objects.h"

struct rect_size {
	union {
		atomic<uint64_t> dimension;
		//width, height
		int32_t data[2];
	};
};

struct RenderThread {
	RenderThread(video_state& state, sync_state& in_state):frame_state(state), state(in_state){};
	render_objects glo{};
	SDL_Window* window{nullptr};
	SDL_GLContext glcontext;
	video_state& frame_state;
	sync_state& state;
	thread handle;
	condition cond;
	mutex mtx;
	int dec_width, dec_height;
	rect_size cur_size;
	atomic<bool> size_changed{false};
	atomic<bool> gl_inited{false};
	int prev_increment{-1};
	void join()
	{
		handle.join();
	}
	void RenderProc();
	void start()
	{
		handle = std::move(thread(&RenderThread::RenderProc, this));
	}
	int GoToCurrentTexture(double last_update_time, render_objects::Frame& frame, bool& dropped);
};

