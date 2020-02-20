#pragma once

#include "Objects.h"

struct DecoderThread {
	DecoderThread(render_objects& pglo, video_state& pframe_state, sync_state& in_state):
		avo(in_state), glo(pglo), frame_state(pframe_state), state(in_state)
	{};
	void start(char* uri)
	{
		handle = std::move(std::thread(&DecoderThread::DecoderProc, this, uri));
	}
	void DecoderProc(char* uri);
	void join()
	{
		handle.join();
	}
	decode_objects avo;
	render_objects& glo;
	video_state& frame_state;
	sync_state& state;
	thread handle;
	condition cond;
	mutex mtx;
	SDL_GLContext glcontext{nullptr};
	SDL_Window* window{nullptr};
	atomic<bool> av_inited{false};
	atomic<bool> gl_inited{false};
	atomic<bool> dc_started{false};
};
