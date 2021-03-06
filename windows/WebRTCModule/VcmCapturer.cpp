/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "pch.h"
#include "VcmCapturer.h"

#include <stdint.h>

#include <memory>

#include "modules/video_capture/video_capture_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace winrt::WebRTCModule {

	VcmCapturer::VcmCapturer() : vcm_(nullptr) {}

	bool VcmCapturer::Init() {
		return Init(0, 0, 0, NULL, 0);
	}
	bool VcmCapturer::Init(size_t width,
		size_t height,
		size_t target_fps,
		const char* unique_name,
		size_t device_idx) {
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());

		char unique_name_tmp[256];

		if (!unique_name || strlen(unique_name) == 0) {
			char device_name[256];
			if (device_info->GetDeviceName(static_cast<uint32_t>(device_idx),
				device_name, sizeof(device_name), unique_name_tmp,
				sizeof(unique_name_tmp)) != 0) {
				Destroy();
				return false;
			}
			unique_name = unique_name_tmp;
		}

		vcm_ = webrtc::VideoCaptureFactory::Create(unique_name);
		if (!vcm_) {
			return false;
		}
		vcm_->RegisterCaptureDataCallback(this);

		device_info->GetCapability(vcm_->CurrentDeviceName(), 0, capability_);

		capability_.width = static_cast<int32_t>(width);
		capability_.height = static_cast<int32_t>(height);
		capability_.maxFPS = static_cast<int32_t>(target_fps);
		capability_.videoType = webrtc::VideoType::kI420;

		if (vcm_->StartCapture(capability_) != 0) {
			Destroy();
			return false;
		}

		RTC_CHECK(vcm_->CaptureStarted());

		return true;
	}

	VcmCapturer* VcmCapturer::Create() {
		return Create(0, 0, 0, NULL);
	}
	VcmCapturer* VcmCapturer::Create(size_t width,
		size_t height,
		size_t target_fps,
		const char* unique_name) {
		std::unique_ptr<VcmCapturer> vcm_capturer(new VcmCapturer());

		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(
			webrtc::VideoCaptureFactory::CreateDeviceInfo());

		for (int i = 0; i < device_info->NumberOfDevices(); i++) {
			if (vcm_capturer->Init(width, height, target_fps, unique_name, i)) {
				return vcm_capturer.release();
			}
		}

		RTC_LOG(LS_WARNING) << "Failed to create VcmCapturer(w = " << width
			<< ", h = " << height << ", fps = " << target_fps
			<< ")";
		return nullptr;

	}

	void VcmCapturer::Destroy() {
		if (!vcm_)
			return;

		vcm_->StopCapture();
		vcm_->DeRegisterCaptureDataCallback();
		// Release reference to VCM.
		vcm_ = nullptr;
	}

	VcmCapturer::~VcmCapturer() {
		Destroy();
	}

	void VcmCapturer::OnFrame(const webrtc::VideoFrame& frame) {
		CameraVideoCapturer::OnFrame(frame);
	}

	void VcmCapturer::StartCapturer() {

		if (!vcm_)
			return;

		if (vcm_->StartCapture(capability_)) {

			RTC_LOG(LS_WARNING) << "Failed to start VcmCapturer(w = " << capability_.width
				<< ", h = " << capability_.height << ", fps = " << capability_.maxFPS
				<< ")";
			return;
		}

		RTC_CHECK(vcm_->CaptureStarted());
	}

	void VcmCapturer::StopCapturer() {

		if (!vcm_)
			return;

		if (vcm_->StopCapture()) {

			RTC_LOG(LS_WARNING) << "Failed to stop VcmCapturer(w = " << capability_.width
				<< ", h = " << capability_.height << ", fps = " << capability_.maxFPS
				<< ")";
			return;
		}
	}

}
