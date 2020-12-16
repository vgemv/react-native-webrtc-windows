/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pch.h"
#include "CameraVideoCapturer.h"

#include <algorithm>

#include "api/scoped_refptr.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"

namespace winrt::WebRTCModule {
	CameraVideoCapturer::~CameraVideoCapturer() = default;

	void CameraVideoCapturer::OnFrame(const webrtc::VideoFrame& original_frame) {
		int cropped_width = 0;
		int cropped_height = 0;
		int out_width = 0;
		int out_height = 0;

		webrtc::VideoFrame frame = MaybePreprocess(original_frame);

		if (!video_adapter_.AdaptFrameResolution(
			frame.width(), frame.height(), frame.timestamp_us() * 1000,
			&cropped_width, &cropped_height, &out_width, &out_height)) {
			// Drop frame in order to respect frame rate constraint.
			return;
		}

		if (out_height != frame.height() || out_width != frame.width()) {
			// Video adapter has requested a down-scale. Allocate a new buffer and
			// return scaled version.
			// For simplicity, only scale here without cropping.
			rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer =
				webrtc::I420Buffer::Create(out_width, out_height);
			scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
			webrtc::VideoFrame::Builder new_frame_builder =
				webrtc::VideoFrame::Builder()
				.set_video_frame_buffer(scaled_buffer)
				.set_rotation(webrtc::kVideoRotation_0)
				.set_timestamp_us(frame.timestamp_us())
				.set_id(frame.id());
			if (frame.has_update_rect()) {
				webrtc::VideoFrame::UpdateRect new_rect = frame.update_rect().ScaleWithFrame(
					frame.width(), frame.height(), 0, 0, frame.width(), frame.height(),
					out_width, out_height);
				new_frame_builder.set_update_rect(new_rect);
			}
			broadcaster_.OnFrame(new_frame_builder.build());

		}
		else {
			// No adaptations needed, just return the frame as is.
			broadcaster_.OnFrame(frame);
		}
	}

	rtc::VideoSinkWants CameraVideoCapturer::GetSinkWants() {
		return broadcaster_.wants();
	}

	void CameraVideoCapturer::AddOrUpdateSink(
		rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
		const rtc::VideoSinkWants& wants) {
		broadcaster_.AddOrUpdateSink(sink, wants);
		UpdateVideoAdapter();
	}

	void CameraVideoCapturer::RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) {
		broadcaster_.RemoveSink(sink);
		UpdateVideoAdapter();
	}

	void CameraVideoCapturer::UpdateVideoAdapter() {
		video_adapter_.OnSinkWants(broadcaster_.wants());
	}

	webrtc::VideoFrame CameraVideoCapturer::MaybePreprocess(const webrtc::VideoFrame& frame) {
		webrtc::MutexLock lock(&lock_);
		if (preprocessor_ != nullptr) {
			return preprocessor_->Preprocess(frame);
		}
		else {
			return frame;
		}
	}

}
