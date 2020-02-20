#include "Render.h"

#include <algorithm>
#include <iostream>
#include <chrono>

void CreateGLTexture(render_objects& objects, int width, int height)
{
/*	GLuint pbos[FRAME_BUFF_NUM];
	GLuint textures[FRAME_BUFF_NUM];
	glCreateBuffers(FRAME_BUFF_NUM, pbos);
	glCreateTextures(GL_TEXTURE_2D, FRAME_BUFF_NUM, textures);
	glActiveTexture(GL_TEXTURE0);
	float border_color[] = {0.0, 0.0, 0.0, 1.0};
	for (int i = 0; i < FRAME_BUFF_NUM; ++i) {
//		glNamedBufferStorage(pbos[i], int64_t(4) * width * height,nullptr, GL_MAP_WRITE_BIT);
		glTextureParameterfv(textures[i], GL_TEXTURE_BORDER_COLOR, border_color);
		glTextureParameteri(textures[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTextureParameteri(textures[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTextureParameteri(textures[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(textures[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//		glTextureParameterf(objects.texture[i],GL_TEXTURE_MAX_ANISOTROPY, max_anisotropy);
		glTextureStorage2D(textures[i], 1, GL_RGBA8, width, height);
		objects.frames[i].pbo = pbos[i];
		objects.frames[i].texture = textures[i];
	}
//	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, objects.pbo);
	
//	glBufferStorage(GL_PIXEL_UNPACK_BUFFER, int64_t(4) * width * height * FRAME_BUFF_NUM, nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
//	uint8_t* buffer = (uint8_t*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, int64_t(4) * width * height * FRAME_BUFF_NUM,
//			GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
	for (int i = 0; i < FRAME_BUFF_NUM; ++i) {
	//	objects.buffer_maps[i] = buffer + int64_t(4) * i * width * height;
	}*/
}

void DeleteGLTexture(render_objects& objects)
{
/*	GLuint pbos[FRAME_BUFF_NUM];
	GLuint textures[FRAME_BUFF_NUM];
	for (int i = 0; i < FRAME_BUFF_NUM; ++i) {
	//	pbos[i]= objects.frames[i].pbo;
	//	textures[i] = objects.frames[i].texture;
	}
	glDeleteTextures(FRAME_BUFF_NUM, textures);
	glDeleteBuffers(FRAME_BUFF_NUM, pbos);*/
}

void CreateGLProgram(render_objects& objects)
{
	const char* vsource =
		"#version 450 compatibility\n"
		"in vec2 vertex;\n"
		"out vec2 TexCoords;\n"
		"void main(void){\n"
		"TexCoords = vec2((vertex.x+1.0)/2.0,-(vertex.y-1.0)/2.0);\n"
		"gl_Position = vec4(vertex.xy, 1.0, 1.0);}";
	objects.vshader = glCreateShader(GL_VERTEX_SHADER);
	GLint length = strlen(vsource);
	glShaderSource(objects.vshader, 1, &vsource, &length);
	glCompileShader(objects.vshader);
	const char* fsource =
		"#version 450 compatibility\n"
		"out vec4 out_color;\n"
		"in vec2 TexCoords;\n"
//		"uniform float index;\n"
		"uniform sampler2D Texture;\n"
		"void main(void){\n"
		"out_color = texture(Texture, TexCoords);}";
	objects.fshader = glCreateShader(GL_FRAGMENT_SHADER);
	length = strlen(fsource);
	glShaderSource(objects.fshader, 1, &fsource, &length);
	glCompileShader(objects.fshader);
	objects.program = glCreateProgram();
	glAttachShader(objects.program, objects.vshader);
	glAttachShader(objects.program, objects.fshader);
	glLinkProgram(objects.program);
	glUseProgram(objects.program);
	objects.texture_location = glGetUniformLocation(objects.program, "Texture");
	objects.index_location = glGetUniformLocation(objects.program, "index");
	glUniform1i(objects.texture_location, 0);
}

void DeleteProgram(render_objects& objects)
{
	glUseProgram(0);
	glDeleteProgram(objects.program);
	glDeleteShader(objects.fshader);
	glDeleteShader(objects.vshader);
}

void CreateVertexBuffer(render_objects& objects)
{
	float vertices[] = {
		-1.0, 1.0,
		-1.0, -1.0,
		1.0, 1.0,
		1.0, -1.0
	};
	glGenVertexArrays(1, &objects.vao);
	glBindVertexArray(objects.vao);
	glGenBuffers(1, &objects.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, objects.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

void DeleteVertexBuffer(render_objects& objects)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &objects.vbo);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &objects.vao);
}

void InitGraphics(int width, int height, render_objects& objects)
{
	CreateGLProgram(objects);
	CreateGLTexture(objects, width, height);
	CreateVertexBuffer(objects);
}

void DeInitGraphics(render_objects& objects)
{
	DeleteGLTexture(objects);
	DeleteVertexBuffer(objects);
	DeleteProgram(objects);
}

void RenderFrame(SDL_Window* window, render_objects& glo, int index, int width, int height)
{
	glClear(GL_COLOR_BUFFER_BIT);
	//Use bindless texture here to improve performance
//	glBindTexture(GL_TEXTURE_2D, glo.frames[index].texture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void RenderThread::RenderProc()
{
	SDL_GL_MakeCurrent(window, glcontext);
	initContext();
	InitGraphics(dec_width, dec_height, glo);
	gl_inited = true;
	cond.notify_one();
	double time_till_next_update = 0;
	double last_frame_time=0;
	int pts_increment = 0;
	int64_t last_pts=0;
	int64_t last_last_pts = 0;
	timer timer;
	timer.reset();
	render_objects::Frame frame{};
	while (running && (decoding || glo.frame_in_queue.peek())) {
		{
			lock_guard<> lck(state.render_mutex);
			time_till_next_update -= 0.1 * last_frame_time;
			if(time_till_next_update<0.003)
				time_till_next_update = 0;
			state.vframe_cond.wait_for(lck, time_till_next_update);
		}
		bool resized = true;
		if (size_changed.compare_exchange_weak(resized, false, std::memory_order_acquire, std::memory_order_relaxed)) {
			//use atomic operation to load packed two 32 bit data
			rect_size size;
			size.dimension.store(this->cur_size.dimension.load(std::memory_order_relaxed), std::memory_order_relaxed);
			int width = size.data[0];
			int height = size.data[1];
			int viewportx, viewporty, view_width, view_height;
			view_width = (width * dec_height <= height * dec_width) ? width : dec_width * height / dec_height;
			view_height = (height * dec_width <= width * dec_height) ? height : dec_height * width / dec_width;
			viewportx = (width - view_width) / 2;
			viewporty = (height - view_height) / 2;
			glViewport(viewportx, viewporty, view_width, view_height);
		}
		last_last_pts = last_pts;
		last_pts = frame.pts;
		bool dropped=false;
		pts_increment = GoToCurrentTexture(state.v_state.multiple_buffer*last_frame_time*1.5,frame, dropped);
		if (pts_increment) {
			RenderFrame(window, glo, pts_increment, dec_width, dec_height);
			SDL_GL_SwapWindow(window);
			prev_increment = pts_increment;
		}
		last_frame_time = timer.time();
		if(glo.frame_in_queue.peek() && audio_running)
			time_till_next_update = (double(glo.frame_in_queue.peek()->pts-frame.pts)*(1+bool(pts_increment))/2*state.v_state.time_base.num/ state.v_state.time_base.den)
									- last_frame_time-last_frame_time*dropped;
		else		
			time_till_next_update = time_till_next_update/2;
		timer.reset();
	}
	running = false;
	//deinit graphics
	DeInitGraphics(glo);
	SDL_GL_MakeCurrent(window, nullptr);
	SDL_GL_DeleteContext(glcontext);
	//post empty event to wake main thread
	SDL_Event empty{};
	SDL_PushEvent(&empty);
}

bool GoToNextFrame(double prev_time, double next_time, double cur_time, bool alreay_skipped) {
//	assert(next_pts>=cur_pts);
//	assert(cur_pts>=prev_pts);
	assert(next_time>=prev_time);
	if(!alreay_skipped)
		return next_time -cur_time<cur_time-prev_time;
	return cur_time > next_time;
}

int RenderThread::GoToCurrentTexture(double last_update_time, render_objects::Frame& prev_frame, bool& dropped) {
	while (!glo.frame_in_queue.peek() && (running && decoding)) {
		lock_guard<> lck(state.render_mutex);
		state.decode_cond.wait(lck);
	};
/*	while (glo.ptss[frame_state.render_pos % FRAME_BUFF_NUM] - state.s_state.cur_pts > 800 && running && audio_running) {
		lock_guard<> lck(state.wait_audio_mtx);
		state.wait_audio_cond.wait(lck);
	}*/
	bool already_skipped = false;
	int increment_pts = 0;
	//See if going on is needed here
	while (running) {
		if (!glo.frame_in_queue.peek()) {
			glBindTexture(GL_TEXTURE_2D,prev_frame.texture);
			return increment_pts;
		}
		int64_t video_start_pts = state.v_state.start_pts;
		int64_t audio_start_pts = state.s_state.start_pts;
		if (!GoToNextFrame(double(prev_frame.pts- video_start_pts)*state.v_state.time_base.num/state.v_state.time_base.den, 
			double(glo.frame_in_queue.peek()->pts - video_start_pts) * state.v_state.time_base.num / state.v_state.time_base.den,
			double(state.s_state.cur_pts- audio_start_pts) * state.s_state.time_base.num / state.s_state.time_base.den
			+ (state.v_state.latency + state.v_state.multiple_buffer * last_update_time), already_skipped)) {
			glBindTexture(GL_TEXTURE_2D, prev_frame.texture);
			return increment_pts;
		}
		else {
			if(prev_frame.pbo)
				glo.frame_out_queue.enqueue(prev_frame);
			if (increment_pts) {
				dropped = true;
				fprintf(stderr, "Video frame dropped!\n");
			}
			increment_pts += glo.frame_in_queue.peek()->pts-prev_frame.pts;
			glo.frame_in_queue.try_dequeue(prev_frame);
			if (prev_frame.sync) {
				glWaitSync(prev_frame.sync, 0, GL_TIMEOUT_IGNORED);
				glDeleteSync(prev_frame.sync);
				prev_frame.sync = 0;
			}
			state.render_cond.notify_one();
			continue;
		}
	}
	/*}
		if (!glo.frame_in_queue.peek()) {
			if (frame.sync) {
				glWaitSync(frame.sync,GL_SYNC_GPU_COMMANDS_COMPLETE,GL_TIMEOUT_IGNORED);
				glDeleteSync(frame.sync);
				frame.sync = 0;
			}
			return frame.texture;
		}
		if (GoToNextFrame(glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].pts, 
				glo.frames[(frame_state.render_pos + 1) % FRAME_BUFF_NUM].pts,
				state.s_state.cur_pts + (state.v_state.latency+state.v_state.multiple_buffer*last_update_time), already_skipped)) {
			if (glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync) {
	//			glBindTexture(GL_TEXTURE_2D, glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].texture);
	//			glWaitSync(glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync, 0, GL_TIMEOUT_IGNORED);
				glDeleteSync(glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync);
				glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync = 0;
				std::cout << "Video Frame Dropped!\n";
			}
			already_skipped = true;
			++frame_state.render_pos;
			state.render_cond.notify_one();
		}
		else {
			if (glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync) {
				glBindTexture(GL_TEXTURE_2D, glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].texture);
				glWaitSync(glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync, 0, GL_TIMEOUT_IGNORED);
				glDeleteSync(glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync);
				glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].sync = 0;
			}
			break;
		}
	}
	frame_state.render_pts = glo.frames[frame_state.render_pos % FRAME_BUFF_NUM].pts;
	return frame_state.render_pos.load() % FRAME_BUFF_NUM;*/
}
