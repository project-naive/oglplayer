#include "Decode.h"
#include <iostream>

static inline void init_av_codec(decode_objects& objects, sync_state& state, const char* uri)
{
	std::cout << avformat_open_input(&objects.format_ctx, uri, nullptr, nullptr) << '\n';
	std::cout << avformat_find_stream_info(objects.format_ctx, nullptr) << '\n';
	av_dump_format(objects.format_ctx, 0, uri, false);
	objects.video_stream = av_find_best_stream(objects.format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &objects.vcodec, 0);
	objects.audio_stream = av_find_best_stream(objects.format_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &objects.acodec, 0);
	state.s_state.time_base = objects.format_ctx->streams[objects.audio_stream]->time_base;
	state.v_state.time_base = objects.format_ctx->streams[objects.video_stream]->time_base;
	if (objects.format_ctx->streams[objects.audio_stream]->start_time != AV_NOPTS_VALUE)
		state.s_state.start_pts = objects.format_ctx->streams[objects.audio_stream]->start_time;
	if (objects.format_ctx->streams[objects.video_stream]->start_time != AV_NOPTS_VALUE)
		state.v_state.start_pts = objects.format_ctx->streams[objects.video_stream]->start_time;
	objects.vcodec_ctx = avcodec_alloc_context3(objects.vcodec);
	objects.acodec_ctx = avcodec_alloc_context3(objects.acodec);
	std::cout << avcodec_parameters_to_context(objects.vcodec_ctx, objects.format_ctx->streams[objects.video_stream]->codecpar) << '\n';
	std::cout << avcodec_parameters_to_context(objects.acodec_ctx, objects.format_ctx->streams[objects.audio_stream]->codecpar) << '\n';
	objects.acodec_ctx->request_channel_layout = AV_CH_LAYOUT_STEREO;
	objects.acodec_ctx->request_sample_fmt = AV_SAMPLE_FMT_FLTP;
	std::cout << avcodec_open2(objects.vcodec_ctx, objects.vcodec, nullptr) << '\n';
	std::cout << avcodec_open2(objects.acodec_ctx, objects.acodec, nullptr) << '\n';
	if (!objects.acodec_ctx->channel_layout) {
		//force a channel layout
		switch (objects.acodec_ctx->channels) {
			case 1:
				objects.acodec_ctx->channel_layout = AV_CH_LAYOUT_MONO;
				break;
			case 2:
				objects.acodec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
				break;
			default:
				printf("Unkown Channel layout!");
				exit(1);
		}
	}

}

static inline void deinit_av_codec(decode_objects& avo)
{
	avcodec_close(avo.acodec_ctx);
	avcodec_close(avo.vcodec_ctx);
	avcodec_free_context(&avo.acodec_ctx);
	avcodec_free_context(&avo.vcodec_ctx);
	avformat_close_input(&avo.format_ctx);
}

static inline void init_swscale_swresample(decode_objects& avo)
{
	avo.sws_ctx = sws_getContext(   // [13]
		avo.vcodec_ctx->width,
		avo.vcodec_ctx->height,
		avo.vcodec_ctx->pix_fmt,
		avo.vcodec_ctx->width,
		avo.vcodec_ctx->height,
		AV_PIX_FMT_RGBA,	// sws_scale destination color scheme
		SWS_POINT,			// sws_scale size scaling algo, since same size, use simplest
		NULL,
		NULL,
		NULL
	);

	avo.swr_ctx = swr_alloc_set_opts(
		nullptr,
		AV_CH_LAYOUT_STEREO,
		AV_SAMPLE_FMT_FLTP,
		48000,
		avo.acodec_ctx->channel_layout,
		avo.acodec_ctx->sample_fmt,
		avo.acodec_ctx->sample_rate,
		0,
		0
	);
}

static inline void deinit_sws_swr(decode_objects& avo)
{
	sws_freeContext(avo.sws_ctx);
	swr_free(&avo.swr_ctx);
}

static inline int DecodeFrameToPBO(AVCodecContext* codec_ctx, SwsContext* sws_ctx,
		AVFrame* frame, render_objects& glo)
{
	int err = avcodec_receive_frame(codec_ctx, frame);
	if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
		return err;
	}
	if (err < 0) {
		std::cout << "Error!\n";
		return err;
	}
//	printf("Video frame pts: %d\tBest effort:%d\tpkt_dts:%d\n",frame->pts,frame->best_effort_timestamp,frame->pkt_dts);
	render_objects::Frame glframe;
	int width = codec_ctx->width;
	int height = codec_ctx->height;
	if (!glo.frame_out_queue.try_dequeue(glframe)) {
		glCreateBuffers(1, &glframe.pbo);
		glCreateTextures(GL_TEXTURE_2D, 1, &glframe.texture);
		float border_color[] = {0.0, 0.0, 0.0, 1.0};
		glTextureParameterfv(glframe.texture, GL_TEXTURE_BORDER_COLOR, border_color);
		glTextureParameteri(glframe.texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(glframe.texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTextureParameteri(glframe.texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(glframe.texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureStorage2D(glframe.texture, 1, GL_RGBA8, width, height);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glframe.pbo);
		glBufferStorage(GL_PIXEL_UNPACK_BUFFER, int64_t(4) * width * height,0,GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		glframe.map = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, int64_t(4) * width * height,  GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_PERSISTENT_BIT);
	}
	else {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glframe.pbo);
	}
	int linesize[] = {width * 4};
	
//	glBufferData(GL_PIXEL_UNPACK_BUFFER, int64_t(4) * width * height,nullptr,GL_STREAM_DRAW);
	uint8_t* ptrs[] = {glframe.map};
	sws_scale(  // [16]
		sws_ctx,
		frame->data,
		frame->linesize,
		0,
		height,
		//rgb has only one plane
		ptrs,
		//rgb has only one plane
		linesize
	);
	glFlushMappedBufferRange(GL_PIXEL_UNPACK_BUFFER,0, int64_t(4) * width * height);
	glBindTexture(GL_TEXTURE_2D,glframe.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)(0));
	glframe.pts = frame->pts;
	glframe.sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	//intel without this will cause server hang
	//nvidia with this will not use copy engine (1-2% gpu usage up as tested)
//	glFlush();
//	glFlush();
	if (glo.upload_need_flush) {
		glFlush();
	}
	glo.frame_in_queue.enqueue(glframe);
	return err;
}


static inline int DecodeVideoPacket(AVCodecContext* vcodec_ctx, SwsContext* sws_ctx, AVPacket* packet, AVFrame* vframe, render_objects& glo, video_state& frame_state, sync_state& state)
{
	int err = avcodec_send_packet(vcodec_ctx, packet);
	if (err < 0) running = false;
	//each packet has only one frame
	while (err >= 0) {
		while (glo.frame_in_queue.size_approx()>4 && running && decoding) {
			lock_guard<> lck(state.decode_mutex);
			state.render_cond.wait(lck);
		}
		err = DecodeFrameToPBO(vcodec_ctx, sws_ctx, vframe, glo);
		if (!err) {
			frame_state.decode_pts = vframe->pts;
			++frame_state.decode_pos;
			state.decode_cond.notify_one();
			state.vframe_cond.notify_one();
		}
	}
	return err;
	return 0;
}

static inline int send_audio_frame(AVFrame* aframe, AVFrame*& asend, SwrContext* swr_ctx, sync_state& state)
{
	if (!asend) {
		if (!state.s_state.recycle_queque.empty()) {
			asend = *state.s_state.recycle_queque.front();
			state.s_state.recycle_queque.pop();
		}
		else {
			asend = av_frame_alloc();
			asend->format = AV_SAMPLE_FMT_FLTP;
			asend->channel_layout = AV_CH_LAYOUT_STEREO;
			asend->sample_rate = 48000;
		}
	}
	int64_t delay = swr_get_delay(swr_ctx, 48000);
	int err = swr_convert_frame(swr_ctx, asend, aframe);
	if(aframe)
		asend->pts = aframe->pts + double(delay* state.s_state.time_base.den)/(48000i64*state.s_state.time_base.num);
	if (asend->nb_samples) {
		while (!state.s_state.in_queque.try_emplace(asend)) {
			lock_guard<> lck(state.s_state.in_queue_mtx);
			state.s_state.in_queue_cond.wait(lck);
		}
//		printf("Audio frame pts: %d\tBest effort:%d\tpkt_dts:%d\n", asend->pts, asend->best_effort_timestamp,asend->pkt_dts);
		asend = nullptr;
		state.s_state.audio_ready = true;
	}
	return err;
}

static inline int DecodeAudioPacket(AVCodecContext* acodec_ctx, SwrContext* swr_ctx, AVPacket* packet, AVFrame* aframe, AVFrame*& asend, sync_state& s_state)
{
	int err = avcodec_send_packet(acodec_ctx, packet);
	if (err < 0) running = false;
	//each packet has only one frame
	while (err >= 0) {
		err = avcodec_receive_frame(acodec_ctx, aframe);
		if (!err && aframe->nb_samples) {
			aframe->sample_rate = acodec_ctx->sample_rate;
			aframe->format = acodec_ctx->sample_fmt;
			err = send_audio_frame(aframe, asend, swr_ctx, s_state);
		}
		if (err < 0 && !(err == AVERROR_EOF || err == AVERROR(EAGAIN))) {
			char str[AV_ERROR_MAX_STRING_SIZE]{0};
			std::cout << "Error!: " << av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err) << '\n';
		}
	}
	return err;
}


void DecoderThread::DecoderProc(char* uri)
{
	init_av_codec(avo,state, uri);
	init_swscale_swresample(avo);
	av_inited.store(true, std::memory_order_relaxed);
	cond.notify_one();
	{
		lock_guard<mutex> lck(mtx);
		while (!gl_inited.load(std::memory_order_relaxed)) {
			cond.wait(lck);
		}
	}
	SDL_GL_MakeCurrent(window, glcontext);
	initContext();
	AVFrame* vframe = av_frame_alloc();
	AVFrame* aframe = av_frame_alloc();
	AVFrame* asend{};
	//part of abi
	AVPacket packet;
	dc_started = true;
	cond.notify_one();
	AVSampleFormat;
	int err;
	while (running && (audio_running || video_running) && av_read_frame(avo.format_ctx, &packet) >= 0) {
		if (packet.stream_index == avo.video_stream) {
			err = DecodeVideoPacket(avo.vcodec_ctx, avo.sws_ctx, &packet, vframe, glo, frame_state,state);
			if (err < 0 && !(err == AVERROR_EOF || err == AVERROR(EAGAIN))) {
				char str[AV_ERROR_MAX_STRING_SIZE]{0};
				std::cout << "Error!: " << av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err) << '\n';
			}
		}
		else if (packet.stream_index == avo.audio_stream) {
			err = DecodeAudioPacket(avo.acodec_ctx, avo.swr_ctx, &packet, aframe, asend,state);
			if (err < 0 && !(err == AVERROR_EOF || err == AVERROR(EAGAIN))) {
				char str[AV_ERROR_MAX_STRING_SIZE]{0};
				std::cout << "Error!: " << av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err) << '\n';
			}
		}
		av_packet_unref(&packet);
	}
	err = DecodeAudioPacket(avo.acodec_ctx, avo.swr_ctx, nullptr, aframe, asend, state);
	if (err < 0 && !(err == AVERROR_EOF || err == AVERROR(EAGAIN))) {
		char str[AV_ERROR_MAX_STRING_SIZE]{0};
		std::cout << "Error!: " << av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err) << '\n';
	}
	err = DecodeVideoPacket(avo.vcodec_ctx, avo.sws_ctx, nullptr, vframe, glo, frame_state, state);
	if (err < 0 && !(err == AVERROR_EOF || err == AVERROR(EAGAIN))) {
		char str[AV_ERROR_MAX_STRING_SIZE]{0};
		std::cout << "Error!: " << av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err) << '\n';
	}
	send_audio_frame(nullptr, asend, avo.swr_ctx, state);
	while (!state.s_state.recycle_queque.empty() || !state.s_state.in_queque.empty()) {
		while (!state.s_state.recycle_queque.empty()) {
			av_frame_free(state.s_state.recycle_queque.front());
			num_free++;
			state.s_state.recycle_queque.pop();
		}
		if(audio_running && !state.s_state.in_queque.empty()){
			lock_guard<> lck(state.s_state.in_queue_mtx);
			state.s_state.in_queue_cond.wait(lck);
		}
		else if (!audio_running) {
			av_frame_free(state.s_state.in_queque.front());
			num_free++;
			state.s_state.in_queque.pop();
		}
	}
	SDL_GL_MakeCurrent(window, nullptr);
	SDL_GL_DeleteContext(glcontext);
	decoding = false;
	state.render_cond.notify_all();
	state.decode_cond.notify_all();
	deinit_sws_swr(avo);
	av_frame_free(&vframe);
	av_frame_free(&aframe);
	if (asend)
		av_frame_free(&asend);
	deinit_av_codec(avo);
	//post empty event to wake main thread
	SDL_Event empty{};
	SDL_PushEvent(&empty);
}
