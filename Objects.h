#pragma once

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/error.h>
}

#include <cstdint>
#include <thread>
#include <limits>
#include <chrono>
#include <queue>

#include <GL/glew.h>


#include <SDL/SDL_video.h>
#include <SDL/SDL_events.h>
#include <soundio/soundio.h>

#include <rigtorp/SPSCQueue.h>
#include <moodycamel/readerwriterqueue.h>

#include "ThreadDefine.h"

using std::thread;
//#define FRAME_BUFF_NUM 5
//#define FRAME_BUFF_NUM 10

extern atomic<bool> running;
extern atomic<bool> decoding;

extern int num;

extern int num_alloc;
extern int num_free;
extern int num_recieved;
extern int num_sent;
extern volatile bool audio_running;
extern volatile bool video_running;
extern volatile bool want_pasue;

struct timer {
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	void reset()
	{
		start = std::chrono::high_resolution_clock::now();
	}
	double time()
	{
		return std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count();
	}
	static uint64_t since_epoch()
	{
		return std::chrono::duration_cast<std::chrono::duration<uint64_t, std::ratio<1, 1000000000>>>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
	}
};


struct sound_state {
	sound_state():in_queque(1000), recycle_queque(1000) {}//, recycle_queue(10){}
	rigtorp::SPSCQueue<AVFrame*> in_queque;
	rigtorp::SPSCQueue<AVFrame*> recycle_queque;
	int samples_read = 0;
	volatile bool audio_ready = false;
	int64_t prev_nb_samples{};
	int64_t prev_pts{};
	atomic<int64_t> cur_pts{};
	atomic<uint64_t> time_since_epoch{};
	mutex in_queue_mtx;
	condition in_queue_cond;
	AVRational time_base;
	int64_t start_pts;
};

struct video_state {
	video_state():decode_pts(-1), render_pts(-1), decode_pos(0), render_pos(0){}
	atomic<int64_t> decode_pts;
	atomic<int64_t> render_pts;
	atomic<int64_t> decode_pos;
	atomic<int64_t> render_pos;
	double refresh_rate = 60;
	double latency = 0;
	bool multiple_buffer =true;
	AVRational time_base;
	int64_t start_pts;
};

struct sync_state {
	sync_state(sound_state&sound, video_state&video):s_state(sound),v_state(video){}
	sound_state& s_state;
	video_state& v_state;
	mutex render_mutex;
	mutex decode_mutex;
	condition render_cond;
	condition decode_cond;
	condition vframe_cond;
	mutex wait_audio_mtx;
	condition wait_audio_cond;
};

struct decode_objects {
	decode_objects(sync_state& in_state):state(in_state){}
	AVFormatContext* format_ctx{};
	AVCodec* vcodec{}, * acodec{};
	AVCodecContext* vcodec_ctx{}, * acodec_ctx{};
	int video_stream{-1}, audio_stream{-1};
	SwsContext* sws_ctx{};
	SwrContext* swr_ctx{};
	sync_state& state;
};

struct render_objects {
	struct Frame {
		GLuint pbo;
		GLuint texture;
		GLsync sync;
		int64_t pts;
		uint8_t* map;
	};
//	Frame frames[FRAME_BUFF_NUM];
	moodycamel::ReaderWriterQueue<Frame> frame_in_queue;
	moodycamel::ReaderWriterQueue<Frame> frame_out_queue;
//	GLuint pbo;
//	GLuint textures[FRAME_BUFF_NUM];
//	volatile GLsync syncs[FRAME_BUFF_NUM];
//	volatile int64_t ptss[FRAME_BUFF_NUM];
//	uint8_t* buffer_maps[FRAME_BUFF_NUM];
	GLuint program;
	GLuint vshader, fshader;
	GLint index_location, texture_location;
	GLuint vao, vbo;
	//NV does not need flush
	//and will give 20% less utilization
	bool upload_need_flush;
};


void initContext();
