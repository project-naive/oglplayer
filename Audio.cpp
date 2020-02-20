#include "Audio.h"

#define min(x,y) x<y?x:y
#define max(x,y) x>y?x:y

void audio_ostream::drop_out(SoundIoOutStream* stream)
{
	printf("Audio Dropout!\n");
}

void audio_ostream::sound_write(SoundIoOutStream* stream, int min_frames, int max_frames) noexcept
{
	if (!max_frames) return;
	double float_sample_rate = stream->sample_rate;
	double seconds_per_frame = 1.0 / float_sample_rate;
	struct SoundIoChannelArea* areas;
	const struct SoundIoChannelLayout* layout = &stream->layout;
	int err;
	audio_ostream& ost = *(audio_ostream*)stream->userdata;
	int frame_count = min_frames ? min_frames : max_frames;
	frame_count = 481;
	int frame_left = frame_count;
	bool rendered{false};
	bool reset{false};
	int64_t pts{};
	int64_t nb_samples{};
	int64_t cur_samples{};
	if ((err = soundio_outstream_begin_write(stream, &areas, &frame_count))) {
		fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
		exit(1);
	}
	while (frame_left > 0 && !ost.state.s_state.in_queque.empty()) {
		AVFrame* avframe = *(ost.state.s_state.in_queque.front());
		pts = avframe->pts;
		nb_samples = avframe->nb_samples;
		rendered = true;
		reset = false;
		//Update current pts in state and notify
		//the presentation controll thread. Syncing video
		//to audio is simpler on the audio side with
		//no need for resampling on time change, etc.
		//Also get smoother audio when playing since
		//audio is a strict realtime constraint
		//while video is not. Dropping audio currently
		//fills the buffer with 0, but may employ a
		//swr_context to drop the time, and compensate.
		//Note: allocate the buffer before doing anything
		//		else, and use the direct swr function.
		//      Not using the direct conversion to write
		//      to the buffer here is becase while on
		//      most backends the buffer is packed,
		//      jack goes with planar, and alsa has both.
		//      Allowing to specify a stride on swr_convert
		//      may work around this, but will mean work
		//      on the ffmpeg side and a separate code path
		//      for preparing such data.
		int64_t cur_write = min(avframe->nb_samples - ost.state.s_state.samples_read, frame_left);
		for (int64_t frame = 0; frame < cur_write; frame += 1) {
			for (int channel = 0; channel < layout->channel_count; channel += 1) {
				if (avframe->data[channel]) {
					memcpy(areas[channel].ptr, avframe->data[channel] + size_t(4) * (frame + ost.state.s_state.samples_read), 4);
					areas[channel].ptr += areas[channel].step;
				}
			}
		}
		if (avframe->nb_samples - ost.state.s_state.samples_read == cur_write) {
			cur_samples = (ost.state.s_state.samples_read);
			ost.state.s_state.samples_read = 0;
			ost.state.s_state.recycle_queque.push(avframe);
			ost.state.s_state.in_queque.pop();
			++num_recieved;
			reset = true;
			ost.state.s_state.in_queue_cond.notify_one();
		}
		else {
			cur_samples = (ost.state.s_state.samples_read + cur_write);
			ost.state.s_state.samples_read += frame_left;
		}
		frame_left -= cur_write;
	}
	if (frame_left) {
		if (ost.state.s_state.audio_ready) {
			fprintf(stderr, "No audio frame recieved!\n");
		}
		for (int frame = 0; frame < frame_left; frame += 1) {
			for (int channel = 0; channel < layout->channel_count; channel += 1) {
				memset(areas[channel].ptr, 0, 4);
				areas[channel].ptr += areas[channel].step;
			}
		}
	}
	if ((err = soundio_outstream_end_write(stream))) {
		if (err == SoundIoErrorUnderflow)
			return;
		fprintf(stderr, "unrecoverable stream error: %s\n", soundio_strerror(err));
		exit(1);
	}
	double latency;
	soundio_outstream_get_latency(ost.ost,&latency);
	if (rendered) {
		ost.state.s_state.prev_nb_samples=nb_samples;
		ost.state.s_state.prev_pts = pts;
		ost.state.s_state.cur_pts = pts + ((cur_samples * ost.state.s_state.time_base.den) / (float_sample_rate * ost.state.s_state.time_base.num)) - ((latency * ost.state.s_state.time_base.den) / ost.state.s_state.time_base.num);
		ost.state.s_state.time_since_epoch=timer::since_epoch()-latency*1000000000;
	}
	soundio_outstream_pause(stream, want_pasue);
	ost.state.wait_audio_cond.notify_one();
}