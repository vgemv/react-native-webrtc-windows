/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#pragma once

#include <memory>
#include <vector>

#define WEBRTC_WIN 1

#include "api/scoped_refptr.h"
#include "modules/video_capture/video_capture.h"
#include "CameraVideoCapturer.h"

namespace winrt::WebRTCModule {

    class VcmCapturer : public CameraVideoCapturer,
        public rtc::VideoSinkInterface<webrtc::VideoFrame> {
    public:
        static VcmCapturer* Create();
        static VcmCapturer* Create(size_t width,
            size_t height,
            size_t target_fps,
            const char* unique_name);
        virtual ~VcmCapturer();

        void OnFrame(const webrtc::VideoFrame& frame) override;

        void StartCapturer();

        void StopCapturer();

    private:
        VcmCapturer();
        bool Init();
        //use device_idx when unique_name is empty, or ignore.
        bool Init(size_t width,
            size_t height,
            size_t target_fps,
            const char* unique_name,
            size_t device_idx);
        void Destroy();

        rtc::scoped_refptr<webrtc::VideoCaptureModule> vcm_;
        webrtc::VideoCaptureCapability capability_;
    };

}

