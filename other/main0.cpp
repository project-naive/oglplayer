#include <initguid.h>
#include <d3d11.h>
//#include <dxvahd.h>
extern "C" {
	#include <libavcodec/d3d11va.h>
	#include <libavcodec/avcodec.h>
	#include <libavutil/hwcontext_d3d11va.h>
	#include <libavutil/hwcontext.h>
}
#include <Windows.h>

#include <iostream>

bool print_profile(const GUID& guid)
{
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG2_MOCOMP)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG2_MOCOMP" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG2_IDCT)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG2_IDCT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG2_VLD)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG2_VLD" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG1_VLD)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG1_VLD" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG2and1_VLD)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG2and1_VLD" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_MOCOMP_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_MOCOMP_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_MOCOMP_FGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_MOCOMP_FGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_IDCT_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_IDCT_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_IDCT_FGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_IDCT_FGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_FGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_FGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_WITHFMOASO_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_WITHFMOASO_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_STEREO_PROGRESSIVE_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_STEREO_PROGRESSIVE_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_STEREO_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_STEREO_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_H264_VLD_MULTIVIEW_NOFGT)) {
		std::cout << "D3D11_DECODER_PROFILE_H264_VLD_MULTIVIEW_NOFGT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_WMV8_POSTPROC)) {
		std::cout << "D3D11_DECODER_PROFILE_WMV8_POSTPROC" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_WMV8_MOCOMP)) {
		std::cout << "D3D11_DECODER_PROFILE_WMV8_MOCOMP" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_WMV9_POSTPROC)) {
		std::cout << "D3D11_DECODER_PROFILE_WMV9_POSTPROC" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_WMV9_MOCOMP)) {
		std::cout << "D3D11_DECODER_PROFILE_WMV9_MOCOMP" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_WMV9_IDCT)) {
		std::cout << "D3D11_DECODER_PROFILE_WMV9_IDCT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VC1_POSTPROC)) {
		std::cout << "D3D11_DECODER_PROFILE_VC1_POSTPROC" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VC1_MOCOMP)) {
		std::cout << "D3D11_DECODER_PROFILE_VC1_MOCOMP" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VC1_IDCT)) {
		std::cout << "D3D11_DECODER_PROFILE_VC1_IDCT" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VC1_VLD)) {
		std::cout << "D3D11_DECODER_PROFILE_VC1_VLD" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VC1_D2010)) {
		std::cout << "D3D11_DECODER_PROFILE_VC1_D2010" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG4PT2_VLD_SIMPLE)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG4PT2_VLD_SIMPLE" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG4PT2_VLD_ADVSIMPLE_NOGMC)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG4PT2_VLD_ADVSIMPLE_NOGMC" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_MPEG4PT2_VLD_ADVSIMPLE_GMC)) {
		std::cout << "D3D11_DECODER_PROFILE_MPEG4PT2_VLD_ADVSIMPLE_GMC" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_HEVC_VLD_MAIN)) {
		std::cout << "D3D11_DECODER_PROFILE_HEVC_VLD_MAIN" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10)) {
		std::cout << "D3D11_DECODER_PROFILE_HEVC_VLD_MAIN10" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0)) {
		std::cout << "D3D11_DECODER_PROFILE_VP9_VLD_PROFILE0" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2)) {
		std::cout << "D3D11_DECODER_PROFILE_VP9_VLD_10BIT_PROFILE2" << '\n';
		return false;
	}
	if (IsEqualGUID(guid, D3D11_DECODER_PROFILE_VP8_VLD)) {
		std::cout << "D3D11_DECODER_PROFILE_VP8_VLD" << '\n';
		return false;
	}
	std::cout << "UNKNOWN!" << '\n';
	return true;
}

void print_format(DXGI_FORMAT fmt)
{
	switch (fmt) {
		case DXGI_FORMAT_NV12:
			std::cout << "DXGI_FORMAT_NV12" << '\t';
			break;
		case DXGI_FORMAT_P010:
			std::cout << "DXGI_FORMAT_P010" << '\t';
			break;
		case DXGI_FORMAT_420_OPAQUE:
			std::cout << "DXGI_FORMAT_420_OPAQUE" << '\t';
			break;
		case DXGI_FORMAT_YUY2:
			std::cout << "DXGI_FORMAT_YUY2" << '\t';
			break;
		default:
			std::cout << fmt << '\t';
	}
}

void check_formats(ID3D11VideoDevice* vdev, const GUID& profile) {
	for (UINT i = 0; i < DXGI_FORMAT_V408; ++i) {
		BOOL supported;
		vdev->CheckVideoDecoderFormat(&profile,(DXGI_FORMAT)i,&supported);
		if (supported) {
			print_format((DXGI_FORMAT)i);
		}
	}
	std::cout << "\n\n";
}

int main()
{
/*
	D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	D3D_FEATURE_LEVEL level;
	HRESULT hr=D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_VIDEO_SUPPORT| D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		levels,2,D3D11_SDK_VERSION,&device,&level,&context);
	std::cout<<hr<<'\n';
	*/
	D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
	ID3D11Device* device;
	ID3D11DeviceContext* context;
	D3D_FEATURE_LEVEL level;
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_VIDEO_SUPPORT | D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		levels, 2, D3D11_SDK_VERSION, &device, &level, &context);
	std::cout << hr << '\n';
	AVHWDeviceType type = AV_HWDEVICE_TYPE_NONE;
	while(true) {
		type = av_hwdevice_iterate_types(type);
		if(type == AV_HWDEVICE_TYPE_NONE) break;
		std::cout<<av_hwdevice_get_type_name(type)<<'\n';
	}
	AVBufferRef* ref = av_hwdevice_ctx_alloc(AV_HWDEVICE_TYPE_D3D11VA);
	AVHWDeviceContext* device_ctx= (AVHWDeviceContext*)ref->data;
	AVD3D11VADeviceContext* hwctx = (AVD3D11VADeviceContext*)device_ctx->hwctx;
	hwctx->device=device;
	hwctx->device_context=context;
	std::cout<<av_hwdevice_ctx_init(ref)<<'\n';
	AVBufferRef* frame_ref=av_hwframe_ctx_alloc(ref);
	AVHWFramesContext* frame_ctx=(AVHWFramesContext*)frame_ref->data;
	AVD3D11VAFramesContext* va_frame=(AVD3D11VAFramesContext*)frame_ctx->hwctx;
	frame_ctx->format = AV_PIX_FMT_D3D11;
	frame_ctx->sw_format = AV_PIX_FMT_YUV420P;
	frame_ctx->width=1920;
	frame_ctx->height=1080;
	//frame_ctx->pool;
	//frame_ctx->initial_pool_size=4;
	std::cout << av_hwframe_ctx_init(frame_ref)<<'\n';
	AVD3D11VAContext* va_ctx = av_d3d11va_alloc_context();
	{
		ID3D11VideoContext* vctx = hwctx->video_context;
		ID3D11VideoDevice* vdev=hwctx->video_device;
		{
			UINT profile_count=vdev->GetVideoDecoderProfileCount();
			for (UINT i = 0; i < profile_count; ++i) {
				GUID profile_guid;
				hr = vdev->GetVideoDecoderProfile(i,&profile_guid);
				if (print_profile(profile_guid)) {
					GUID temp = profile_guid;
				}
				D3D11_VIDEO_DECODER_DESC desc{};
				desc.Guid=profile_guid;
				UINT decoder_count;
				check_formats(vdev,profile_guid);
			}
		}
		
	}
	return 0;
}










