#pragma once

#include "pch.h"

#include <functional>
#define _USE_MATH_DEFINES
#include <math.h>

#define WEBRTC_WIN 1

#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
//#include "modules/audio_device/include/audio_device.h"
//#include "modules/audio_processing/include/audio_processing.h"
//#include "modules/video_capture/video_capture.h"
//#include "modules/video_capture/video_capture_factory.h"
//#include "p2p/base/port_allocator.h"
//#include "pc/video_track_source.h"
//#include "rtc_base/checks.h"
//#include "rtc_base/logging.h"
//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/rtc_certificate_generator.h"
//#include "rtc_base/strings/json.h"

#include "NativeModules.h"
#include "JSValue.h"
#include "JSValueReader.h"

using winrt::Microsoft::ReactNative::JSValue;
using winrt::Microsoft::ReactNative::JSValueArray;
using winrt::Microsoft::ReactNative::JSValueType;
using winrt::Microsoft::ReactNative::JSValueObject;

#include "PeerConnectionObserver.h"

namespace winrt::WebRTCModule
{
	class PeerConnectionObserver;

	REACT_MODULE(WebRTCModule);
	class WebRTCModule
	{
	private:
		rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory_;
		std::map<int, rtc::scoped_refptr<PeerConnectionObserver>> pc_observers_;
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> local_streams_;
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> local_tracks_;
		std::unique_ptr<rtc::Thread> worker_thread_;

		rtc::scoped_refptr<webrtc::PeerConnectionInterface>(getPeerConnection)(int id) noexcept;
	protected:
		void sendEvent(std::string name, JSValue& params) noexcept;

	public:
		WebRTCModule();
		REACT_METHOD(peerConnectionInit);
		void(peerConnectionInit)(webrtc::PeerConnectionInterface::RTCConfiguration configuration, int id) noexcept;

		//REACT_METHOD(getDisplayMedia);
		//void(getDisplayMedia)(React::ReactPromise< promise) noexcept;

		REACT_METHOD(getUserMedia);
		void(getUserMedia)(JSValueObject constraints,
			const std::function<void(std::string, JSValueArray&)> successCallback,
			const std::function<void(std::string, std::string)> errorCallback) noexcept;

		//REACT_METHOD(enumerateDevices);
		//void(enumerateDevices)(const std::function<void(std::string, std::string)> callback) noexcept;

		REACT_METHOD(mediaStreamCreate);
		void(mediaStreamCreate)(std::string id) noexcept;

		REACT_METHOD(mediaStreamAddTrack);
		void(mediaStreamAddTrack)(std::string streamId, std::string trackId) noexcept;

		REACT_METHOD(mediaStreamRemoveTrack);
		void(mediaStreamRemoveTrack)(std::string streamId, std::string trackId) noexcept;

		REACT_METHOD(mediaStreamRelease);
		void(mediaStreamRelease)(std::string id) noexcept;

		REACT_METHOD(mediaStreamTrackRelease);
		void(mediaStreamTrackRelease)(std::string id) noexcept;

		REACT_METHOD(mediaStreamTrackSetEnabled);
		void(mediaStreamTrackSetEnabled)(std::string id, boolean enabled) noexcept;

		REACT_METHOD(mediaStreamTrackSwitchCamera);
		void(mediaStreamTrackSwitchCamera)(std::string id) noexcept;

		REACT_METHOD(peerConnectionSetConfiguration);
		void(peerConnectionSetConfiguration)(webrtc::PeerConnectionInterface::RTCConfiguration configuration,
			int id) noexcept;

		REACT_METHOD(peerConnectionAddStream);
		void(peerConnectionAddStream)(std::string streamId, int id) noexcept;

		REACT_METHOD(peerConnectionRemoveStream);
		void(peerConnectionRemoveStream)(std::string streamId, int id) noexcept;

		REACT_METHOD(peerConnectionCreateOffer);
		void(peerConnectionCreateOffer)(int id,
			JSValueObject&& options,
			const std::function<void(bool, JSValue&)>  callback) noexcept;

		REACT_METHOD(peerConnectionCreateAnswer);
		void(peerConnectionCreateAnswer)(int id,
			JSValueObject&& options,
			const std::function<void(bool, JSValue&)>  callback) noexcept;

		REACT_METHOD(peerConnectionSetLocalDescription);
		void(peerConnectionSetLocalDescription)(JSValueObject&& sdpMap,
			int id,
			const std::function<void(bool, JSValue&)> callback) noexcept;

		REACT_METHOD(peerConnectionSetRemoteDescription);
		void(peerConnectionSetRemoteDescription)(JSValueObject&& sdpMap,
			int id,
			const std::function<void(bool, JSValue&)> callback) noexcept;

		REACT_METHOD(peerConnectionAddICECandidate);
		void(peerConnectionAddICECandidate)(JSValueObject&& candidateMap,
			int id,
			const std::function<void(bool)> callback) noexcept;

		REACT_METHOD(peerConnectionGetStats);
		void(peerConnectionGetStats)(int peerConnectionId, React::ReactPromise<JSValue> promise) noexcept;

		REACT_METHOD(peerConnectionClose);
		void(peerConnectionClose)(int id) noexcept;

		REACT_METHOD(createDataChannel);
		void(createDataChannel)(int peerConnectionId,
			std::string label,
			JSValueObject&& config) noexcept;

		REACT_METHOD(dataChannelClose);
		void(dataChannelClose)(int peerConnectionId, int dataChannelId) noexcept;

		REACT_METHOD(dataChannelSend);
		void(dataChannelSend)(int peerConnectionId,
			int dataChannelId,
			std::string data,
			std::string type) noexcept;

		REACT_EVENT(peerConnectionSignalingStateChanged);
		std::function<void(JSValue)> peerConnectionSignalingStateChanged;
		REACT_EVENT(peerConnectionStateChanged);
		std::function<void(JSValue)> peerConnectionStateChanged;
		REACT_EVENT(peerConnectionAddedStream);
		std::function<void(JSValue)> peerConnectionAddedStream;
		REACT_EVENT(peerConnectionRemovedStream);
		std::function<void(JSValue)> peerConnectionRemovedStream;
		REACT_EVENT(peerConnectionOnRenegotiationNeeded);
		std::function<void(JSValue)> peerConnectionOnRenegotiationNeeded;
		REACT_EVENT(peerConnectionIceConnectionChanged);
		std::function<void(JSValue)> peerConnectionIceConnectionChanged;
		REACT_EVENT(peerConnectionIceGatheringChanged);
		std::function<void(JSValue)> peerConnectionIceGatheringChanged;
		REACT_EVENT(peerConnectionGotICECandidate);
		std::function<void(JSValue)> peerConnectionGotICECandidate;
		REACT_EVENT(peerConnectionDidOpenDataChannel);
		std::function<void(JSValue)> peerConnectionDidOpenDataChannel;
		REACT_EVENT(kEventDataChannelStateChanged);
		std::function<void(JSValue)> kEventDataChannelStateChanged;
		REACT_EVENT(kEventDataChannelReceiveMessage);
		std::function<void(JSValue)> kEventDataChannelReceiveMessage;
		REACT_EVENT(kEventMediaStreamTrackMuteChanged);
		std::function<void(JSValue)> kEventMediaStreamTrackMuteChanged;
	};
} // namespace winrt::WebRTCModule