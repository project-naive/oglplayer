# oglplayer

A simple framework for playing video in opengl environment.

## Plans

Currrently there's only the main playback function and still working on wiring things together.
The plan is to expose an interface where caller can acquire a texture handle filled with the
frame based on the current audio progress and estimated latencies. The codec and context, etc 
can be hidden away from user as long as the user provides a callback for reading input and
regularly call the audio write function.

Also, there is hardware acceleration of decoding common codecs on most platforms available in
dxva (d3d11va, etc), vaapi, vdpau, qsv, etc. Most of these support directly rendering of decoded
frame onto the display directly. Tackling the platform details of interoperation with opengl
will be the other major thing and is what inspired this project.(For example, nv_dx_interop2
can share a ID3D11Texture2D with opengl TEXTURE_2D). FFmpeg by default transfers the decoded
frame back to the CPU memory, which takes up bandwidth (typically a few MB per frame, 30 frames
per second at least, which amounts to somewhere near 1GB/s with upload speed), and rendering
back to the screen would again take so much bandwidth, thus only better than using software
decoder when the codec is extremely computation-bound and paralled, 

## Limitations

This project has limited resources, and only plan to support 64 bit little endian cpu, and testing
is only done in one specific machine. This can mean wrong usage of APIs that "just work" for my
device, but not so in the general sense (which is really common when using opengl, which I test only
for my integrated intel and discrete nv gpus with driver of my installed version).

## Libraries used

Currently this project uses ffmpeg to try and support a wide range of codecs. This may be decoupled
in the future and allow for cases where lgpl is not an option. This repo also contains a modified
version of libsoundio (only the wasapi part was modified). This may be changed to use other other
options like RtAudio in the future. The window and events are currently handled by SDL2, which is
under zlib license. The spsc queue implementation is taken from moodycammel and rigtorp implementations
respectively. The queue libraries are header-only and under a permissive license. There may be extra
work rolling out if need is encountered, but I'll just stick with the tested and proven for now.

Many thanks for all the developers that have worked on all the projects that the current project
is dependent on.

## License

This my code in this project is under MIT license. You can emit a license notice if you like when
using it in your code, but I'm really interested in feedback and improvement suggestions.
