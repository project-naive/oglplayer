extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/error.h>
}

#include <iostream>
#include <string>
#include <thread>
#include <algorithm>
#include "ThreadDefine.h"

#include <GL/glew.h>
#include <SDL/SDL_video.h>
#include <SDL/SDL_events.h>
#include <soundio/soundio.h>

#include <rigtorp/SPSCQueue.h>

#include "Decode.h"
#include "Render.h"
#include "Audio.h"

int num_recieved=0;
int num_sent = 0;
int num_alloc = 0;
int num_free = 0;
volatile bool want_pasue = 0;
atomic<bool> running = true;
atomic<bool> decoding = true;
volatile bool audio_running = true;
volatile bool video_running = true;
int num = 0;

//init the hdc, needs to init opengl
//because it needs a framebuffer to share
SDL_Window* initWindow(int width, int height, const char* title) {
	SDL_VideoInit(nullptr);
//	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window =
		SDL_CreateWindow(title,SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,width,height, 
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetSwapInterval(-1);
	return window;
}

void initThreads(RenderThread& rend, DecoderThread& dec, char* uri) {
	{
		dec.start(uri);
		lock_guard<mutex>lck(dec.mtx);
		while (!dec.av_inited.load(std::memory_order_relaxed)) {
			dec.cond.wait(lck);
		}
	}
	SDL_Window* window = initWindow(dec.avo.vcodec_ctx->width, dec.avo.vcodec_ctx->height, "new");
	rend.window = window;
	dec.window = window;
	SDL_DisplayMode mode;
	SDL_GetWindowDisplayMode(window,&mode);
	rend.state.v_state.latency += 1.5 / mode.refresh_rate;
	rend.state.v_state.refresh_rate = mode.refresh_rate;
	rend.glcontext = SDL_GL_CreateContext(window);
	dec.glcontext = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, nullptr);
	rend.dec_width = dec.avo.vcodec_ctx->width;
	rend.dec_height = dec.avo.vcodec_ctx->height;
	//no need to be atomic
	rend.cur_size.data[0] = dec.avo.vcodec_ctx->width;
	rend.cur_size.data[1] = dec.avo.vcodec_ctx->height;
	{
		rend.start();
		lock_guard<> lck(rend.mtx);
		while (!rend.gl_inited) {
			rend.cond.wait(lck);
		}
	}
	dec.gl_inited.store(true, std::memory_order_relaxed);
	dec.cond.notify_one();
	{
		lock_guard<mutex>lck(dec.mtx);
		while (!dec.dc_started.load(std::memory_order_relaxed)) {
			dec.cond.wait(lck);
		}
	}
}

void WindowEvent(const SDL_WindowEvent& event,RenderThread& rend) {
	switch (event.event) {
		case SDL_WINDOWEVENT_CLOSE:
			running.store(false, std::memory_order_relaxed);
			break;
		case SDL_WINDOWEVENT_RESIZED:
			//use a union to make things atomic
			rect_size temp;
			temp.data[0] = event.data1;
			temp.data[1] = event.data2;
			rend.cur_size.dimension.store(temp.dimension.load(std::memory_order_relaxed),std::memory_order_relaxed);
			rend.size_changed.store(true, std::memory_order_release);
			break;
		case SDL_WINDOW_SHOWN:
			break;
		case SDL_WINDOWEVENT_EXPOSED:
			break;
		default:
			break;
	}
}

void ProcessEvent(const SDL_Event& event, RenderThread& rend) {
	switch (event.type) {
		case SDL_WINDOWEVENT:
			WindowEvent(event.window,rend);
			break;
		default:
			break;
	}
}


void my_log_callback(void* ptr, int level, const char* fmt, va_list vargs) {
//	if(level<= AV_LOG_DEBUG)
//	std::cout<<vprintf(fmt, vargs);
}

#include <Audioclient.h>

int main(int argc, char* argv[]) {
//while(true){
	GUID test = __uuidof(IAudioClock);
	running = true;
	decoding = true;
	sound_state s_state;
	video_state frame_state;
	sync_state state(s_state,frame_state);
	s_state.samples_read=0;
	s_state.audio_ready = false;
	audio_running = true;
//	if(argc<2) return 0;
	argv[1]=const_cast<char*>("D:\\GamePlatforms\\Steam\\steamapps\\common\\Neptunia Rebirth1\\data\\MOVIE00000\\movie\\game_op.mkv");
//	argv[1]= const_cast<char*>("C:\\Users\\shang\\Videos\\Captures\\BlueStacks App Player 2020-01-12 19-36-46.mp4");
	av_log_set_level(AV_LOG_ERROR);
	av_log_set_callback(my_log_callback);
	audio_context actx;
	actx.begin();
	audio_out_device adev(actx);
	audio_ostream aost(adev,state);

	RenderThread rend(frame_state,state);
	DecoderThread dec(rend.glo,frame_state,state);
	initThreads(rend,dec,argv[1]);
//	soundio_outstream_set_volume(aost.ost, 0.1);
	aost.start();
	SDL_Event Event;
	while (running) {
		SDL_WaitEvent(&Event);
		ProcessEvent(Event,rend);
	}
	SDL_DisplayMode();
	audio_running = false;
	aost.stop();
	actx.end();
	rend.cond.notify_all();
	dec.cond.notify_all();
	state.render_cond.notify_all();
	state.decode_cond.notify_all();
	state.vframe_cond.notify_all();
	dec.join();
	rend.join();

//}
	return 0;
}




