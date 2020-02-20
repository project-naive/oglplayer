#pragma once

#include "Objects.h"

enum class backend_type {
	None       = SoundIoBackendNone,
	Jack       = SoundIoBackendJack,
	PulseAudio = SoundIoBackendPulseAudio,
	Alsa       = SoundIoBackendAlsa,
	CoreAudio  = SoundIoBackendCoreAudio,
	WASAPI     = SoundIoBackendWasapi,
	Dummy      = SoundIoBackendDummy,
	Default,
	NotFound
};

struct audio_context {
	SoundIo* ctx;
	backend_type type = backend_type::None;
	audio_context(const char* name = "My Playback App"):ctx(soundio_create()){
		if (ctx) {
			ctx->userdata = this;
			ctx->app_name = name;
			ctx->on_devices_change = audio_context::on_devices_change;
			ctx->on_events_signal = audio_context::on_events_signal;
			ctx->on_backend_disconnect = audio_context::on_backend_disconnect;
			ctx->emit_rtprio_warning = audio_context::emit_rtprio_warning;
			ctx->jack_error_callback = audio_context::jack_error;
			ctx->jack_info_callback = audio_context::jack_info;
		}
	}
	~audio_context(){
		soundio_destroy(ctx);
	}
	int begin(backend_type in_type = backend_type::None) {
		if (!ctx) {
			type = backend_type::NotFound;
			return SoundIoErrorNoMem;
		}
		label:
		int err;
		if (in_type == backend_type::None) {
			err = soundio_connect(ctx);
			if (!err)
				type = backend_type::Default;
			else {
				type = backend_type::NotFound;
			}
		}
		else {
			err = soundio_connect_backend(ctx, SoundIoBackend(in_type));
			if (!err)
				type = in_type;
			else {
				in_type = backend_type::None;
				goto label;
			}
		}
		if (!err) {
			soundio_flush_events(ctx);
		}
		return err;
	}
	void end() {
		soundio_disconnect(ctx);
		type = backend_type::None;
	}
	void flush_events() {
		soundio_flush_events(ctx);
	}
	void wait_events() {
		soundio_wait_events(ctx);
	}
	static const char* get_backend_name(backend_type in_type) {
		return soundio_backend_name(SoundIoBackend(in_type));
	}
	static void on_devices_change(SoundIo* soundio) {
		audio_context* __this = (audio_context*)soundio->userdata;
		//do whatever is needed, no realtime constraint.
	}
	static void on_events_signal(SoundIo* soundio){} //do nothing
	static void on_backend_disconnect(SoundIo* soundio, int err) {} //do nothing
	static void emit_rtprio_warning() {
		printf("Setting audio realtime failed!\n");
	}
	static void jack_info(const char* msg) {printf(msg);}
	static void jack_error(const char* msg) {printf(msg);}
};

struct audio_out_device {
	audio_out_device(audio_context& in_ctx, int index = -1, backend_type type = backend_type::None):
		ctx(in_ctx.ctx?in_ctx:*new(&in_ctx)audio_context()),device(nullptr) {
		if (ctx.type == backend_type::NotFound) {
			return;
		}
		if (ctx.type==backend_type::None) {
			int err = ctx.begin(type);
			if(err)
				return;
		}
		label:
		if (index > soundio_output_device_count(ctx.ctx) || index < 0) {
			index = soundio_default_output_device_index(ctx.ctx);
			if (index < 0) 
				index = 0;
			device = soundio_get_output_device(ctx.ctx, index);
		}
		else {
			device = soundio_get_output_device(ctx.ctx, index);
			if (!device) {
				index = -1;
				goto label;
			}
		}
	};
	~audio_out_device()
	{
		soundio_device_unref(device);
	}
	//Do not expose the ref count etc. Copying will always invalidate the osts
	//management unless accessed through userdata to get the actuall owning object
	//which complicates the design and brings overhead
	audio_out_device(const audio_out_device& dev) = delete;
	audio_out_device(audio_out_device&& dev) = delete;
	audio_out_device& operator=(const audio_out_device& dev)=delete;
	audio_out_device& operator=(audio_out_device&& dev) = delete;
	static int get_count(audio_context& ctx) {
		return soundio_output_device_count(ctx.ctx);
	}
	audio_context& ctx;
	SoundIoDevice* device;
	//for managing multiple streams in the future
	SoundIoOutStream** osts=nullptr;
};

struct audio_ostream {
	static void sound_write(SoundIoOutStream* stream, int min_frames, int max_frames) noexcept;
	static void drop_out(SoundIoOutStream* stream);
	static void error(SoundIoOutStream* stream, int err){
		printf(soundio_strerror(err));
	};
	//layout needs special care
	audio_ostream(audio_out_device& dev, sync_state& in_state,int sample_rate = 48000, SoundIoFormat format = SoundIoFormatFloat32NE,
		const char* name = "Audio Stream")
			:device(dev),ost(soundio_outstream_create(dev.device)), state(in_state)
	{
		if(!ost)
			return;
		ost->userdata = this;
		ost->sample_rate = sample_rate;
		ost->format = format;
		ost->layout = *soundio_channel_layout_get_default(2);
		ost->write_callback = audio_ostream::sound_write;
		ost->error_callback = audio_ostream::error;
		ost->underflow_callback = audio_ostream::drop_out;
		ost->name = name;
		int err  = soundio_outstream_open(ost);
		if (err) {
			soundio_outstream_destroy(ost);
			ost = nullptr;
		}
	}
	~audio_ostream() {
		if(ost)
			soundio_outstream_destroy(ost);
	}
	int start() {
		int err = soundio_outstream_start(ost);
		if (err) {
			soundio_outstream_destroy(ost);
			ost=nullptr;
		}
		return err;
	}
	int pause(bool pasue) {
		return soundio_outstream_pause(ost,pasue);
	}
	void stop() {
		soundio_outstream_destroy(ost);
		ost = nullptr;
	}
	int clear_buffer() {
		return soundio_outstream_clear_buffer(ost);
	}
	audio_out_device& device;
	SoundIoOutStream* ost;
	sync_state& state;
};

