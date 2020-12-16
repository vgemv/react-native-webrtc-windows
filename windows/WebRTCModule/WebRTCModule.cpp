#include "pch.h"
#include "WebRTCModule.h"
#include "PeerConnectionObserver.h"
#include "VcmCapturer.h"


#include "pc/video_track_source.h"
#include "modules/video_capture/video_capture_factory.h"

using winrt::Microsoft::ReactNative::JSValue;

namespace winrt::WebRTCModule
{

	class CapturerTrackSource : public webrtc::VideoTrackSource {
	public:
		static rtc::scoped_refptr<CapturerTrackSource> Create() {
			return Create("", 0, 0, 0);
		}
		static rtc::scoped_refptr<CapturerTrackSource> Create(std::string device_id, size_t width, size_t height, size_t fps) {
			std::unique_ptr<VcmCapturer> capturer;

			capturer = absl::WrapUnique(
				VcmCapturer::Create(width, height, fps, device_id.c_str()));
			if (capturer) {
				return new rtc::RefCountedObject<CapturerTrackSource>(
					std::move(capturer));
			}

			return nullptr;
		}
		void StartCapturer() {
			capturer_->StartCapturer();
		}
		void StopCapturer() {
			capturer_->StopCapturer();
		}
	protected:
		explicit CapturerTrackSource(
			std::unique_ptr<VcmCapturer> capturer)
			: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

	private:
		rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
			return capturer_.get();
		}
		std::unique_ptr<VcmCapturer> capturer_;
	};

	std::string new_uuid() {
		UUID uuid;
		RPC_STATUS ret;
		if ((ret = UuidCreate(&uuid)) != 0) {
			//TODO: error
			return "";
		}
		RPC_CSTR uuid_str;

		if ((ret = UuidToStringA(&uuid, &uuid_str)) != 0) {
			//TODO: error
			return "";
		}

		std::string result = (char*)uuid_str;

		if ((ret = RpcStringFreeA(&uuid_str)) != 0) {
			//TODO: error
			return "";
		}

		return result;
	}


	void(WebRTCModule::sendEvent)(std::string name, JSValue& params) noexcept {
		
	}

	void(WebRTCModule::peerConnectionInit)(webrtc::PeerConnectionInterface::RTCConfiguration configuration, int id) noexcept
	{
		if (!peer_connection_factory_) {
			peer_connection_factory_ = webrtc::CreatePeerConnectionFactory(
				nullptr /* network_thread */, nullptr /* worker_thread */,
				nullptr /* signaling_thread */, nullptr /* default_adm */,
				webrtc::CreateBuiltinAudioEncoderFactory(),
				webrtc::CreateBuiltinAudioDecoderFactory(),
				webrtc::CreateBuiltinVideoEncoderFactory(),
				webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
				nullptr /* audio_processing */);

			if (!peer_connection_factory_) {
				//TODO: error
				return;
			}
		}

		rtc::scoped_refptr<PeerConnectionObserver> observer(
			new rtc::RefCountedObject<PeerConnectionObserver>(this, id));

		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection = peer_connection_factory_->CreatePeerConnection(
			configuration, nullptr, nullptr, observer.get());

		observer->SetPeerConnection(peer_connection);
		pc_observers_[id] = observer;

	}

	//void(WebRTCModule::getDisplayMedia)(React::ReactPromise< promise) noexcept;

	void(WebRTCModule::getUserMedia)(JSValueObject&& constraints,
		const std::function<void(std::string, JSValueArray&)> success_callback,
		const std::function<void(std::string, std::string)> error_callback) noexcept
	{
		rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
		rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;

		std::string uuid;

		JSValue& audio_constaints = constraints["audio"];
		JSValue& video_constaints = constraints["video"];

		//deviceId
		//	A ConstrainDOMString object specifying a device ID or an array of device IDs which are acceptable and /or required.
		//groupId
		//	A ConstrainDOMString object specifying a group ID or an array of group IDs which are acceptable and /or required.

		if (audio_constaints) {
			cricket::AudioOptions audio_options;
			if (audio_constaints.Type() == JSValueType::Object) {
				//autoGainControl
				//A ConstrainBoolean object which specifies whether automatic gain control is preferredand /or required.
				if (auto val = audio_constaints["autoGainControl"].TryGetBoolean())
					audio_options.auto_gain_control = *val;
				//channelCount
				//A ConstrainLong specifying the channel count or range of channel counts which are acceptable and /or required.
				//echoCancellation
				//A ConstrainBoolean object specifying whether or not echo cancellation is preferred and /or required.
				if (auto val = audio_constaints["echoCancellation"].TryGetBoolean())
					audio_options.echo_cancellation = *val;
				//latency
				//A ConstrainDouble specifying the latency or range of latencies which are acceptable and /or required.
				//noiseSuppression
				//A ConstrainBoolean which specifies whether noise suppression is preferred and /or required.
				if (auto val = audio_constaints["noiseSuppression"].TryGetBoolean())
					audio_options.noise_suppression = *val;
				//sampleRate
				//A ConstrainLong specifying the sample rate or range of sample rates which are acceptable and /or required.
				//sampleSize
				//A ConstrainLong specifying the sample size or range of sample sizes which are acceptable and /or required.
				//volume
				//A ConstrainDouble specifying the volume or range of volumes which are acceptable and /or required.
			}
			uuid = new_uuid();
			audio_track = peer_connection_factory_->CreateAudioTrack(uuid,
				peer_connection_factory_->CreateAudioSource(audio_options));
			local_tracks_[uuid] = audio_track;
		}


		if (video_constaints) {
			rtc::scoped_refptr<CapturerTrackSource> video_device;
			if (video_constaints.Type() == JSValueType::Object) {

				std::string device_id = video_constaints["deviceId"].AsString();
				//aspectRatio
				//A ConstrainDouble specifying the video aspect ratio or range of aspect ratios which are acceptable and /or required.
				//facingMode
				//A ConstrainDOMString object specifying a facing or an array of facings which are acceptable and /or required.
				//frameRate
				//A ConstrainDouble specifying the frame rate or range of frame rates which are acceptable and /or required.
				size_t fps = video_constaints["frameRate"].AsDouble();
				//height
				//A ConstrainLong specifying the video height or range of heights which are acceptable and /or required.
				size_t height = video_constaints["height"].AsUInt32();
				//width
				//A ConstrainLong specifying the video width or range of widths which are acceptable and /or required.
				size_t width = video_constaints["width"].AsUInt32();
				//resizeMode
				//A ConstrainDOMString object specifying a mode or an array of modes the UA can use to derive the resolution of a video track.Allowed values are none and crop - and -scale.none means that the user agent uses the resolution provided by the camera, its driver or the OS.crop - and -scale means that the user agent can use cropping and downscaling on the camera output  in order to satisfy other constraints that affect the resolution.


				video_device = CapturerTrackSource::Create(device_id, width, height, fps);
			}
			else {
				video_device = CapturerTrackSource::Create();
			}
			if (video_device) {
				uuid = new_uuid();
				video_track = peer_connection_factory_->CreateVideoTrack(uuid, video_device);
				local_tracks_[uuid] = video_track;
			}
		}

		if (!audio_track && !video_track) {
			RTC_LOG(LS_ERROR) << "audio_track == null, video_track == null";
			error_callback("DOMException", "AbortError");
			return;
		}

		uuid = new_uuid();
		auto media_stream = peer_connection_factory_->CreateLocalMediaStream(uuid);
		if (!media_stream) {
			RTC_LOG(LS_ERROR) << "media_stream == null";
			error_callback("DOMException", "AbortError");
			return;
		}

		JSValueArray tracks;
		if (audio_track) {
			media_stream->AddTrack(audio_track);
			tracks.push_back(JSValueObject{
				{"enabled", audio_track->enabled()},
				{"id", audio_track->id()},
				{"kind", audio_track->kind()},
				{"label", audio_track->id()},
				{"readyState", "live"},
				{"remote", false}
				});
		}
		if (video_track) {
			media_stream->AddTrack(video_track);
			tracks.push_back(JSValueObject{
				{"enabled", video_track->enabled()},
				{"id", video_track->id()},
				{"kind", video_track->kind()},
				{"label", video_track->id()},
				{"readyState", "live"},
				{"remote", false}
				});
		}

		local_streams_[uuid] = media_stream;

		success_callback(media_stream->id(), tracks);
	}


	//void(WebRTCModule::enumerateDevices)(const std::function<void(std::string, std::string)> callback) noexcept;

	void(WebRTCModule::mediaStreamCreate)(std::string id) noexcept {
		rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = peer_connection_factory_->CreateLocalMediaStream(id);
		local_streams_[id] = mediaStream;
	}

	void(WebRTCModule::mediaStreamAddTrack)(std::string stream_id, std::string track_id) noexcept {
		if (local_streams_.find(stream_id) == local_streams_.end()) {
			RTC_LOG(LS_ERROR) << "stream " << stream_id << " not found";
			return;
		}

		rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track;//TODO:

		if (!track.get()) {
			RTC_LOG(LS_ERROR) << "track " << track_id << " not found";
			return;
		}

		rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = local_streams_[stream_id];

		std::string kind = track->kind();
		if (kind == "video")
			mediaStream->AddTrack((webrtc::VideoTrackInterface*)track.get());
		else if (kind == "audio")
			mediaStream->AddTrack((webrtc::AudioTrackInterface*)track.get());

	}

	void(WebRTCModule::mediaStreamRemoveTrack)(std::string stream_id, std::string track_id) noexcept {

		if (local_streams_.find(stream_id) == local_streams_.end()) {
			RTC_LOG(LS_ERROR) << "stream " << stream_id << " not found";
			return;
		}

		rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track;//TODO:

		if (!track.get()) {
			RTC_LOG(LS_ERROR) << "track " << track_id << " not found";
			return;
		}

		rtc::scoped_refptr<webrtc::MediaStreamInterface> mediaStream = local_streams_[stream_id];

		std::string kind = track->kind();
		if (kind == "video")
			mediaStream->RemoveTrack((webrtc::VideoTrackInterface*)track.get());
		else if (kind == "audio")
			mediaStream->RemoveTrack((webrtc::AudioTrackInterface*)track.get());
	}

	void(WebRTCModule::mediaStreamRelease)(std::string id) noexcept {

		if (local_streams_.find(id) == local_streams_.end()) {
			RTC_LOG(LS_ERROR) << "stream " << id << " not found";
			return;
		}

		rtc::scoped_refptr<webrtc::MediaStreamInterface> media_stream = local_streams_[id];

		local_streams_.erase(id);

		std::for_each(pc_observers_.begin(), pc_observers_.end(), [media_stream](rtc::scoped_refptr<PeerConnectionObserver> ob) {
			ob->RemoveStream(media_stream);
			});

		media_stream->Release();

	}

	void(WebRTCModule::mediaStreamTrackRelease)(std::string id) noexcept {
		if (auto track = local_tracks_[id]) {
			track->set_enabled(false);
			local_tracks_.erase(id);
			track->Release();
		}
		else
		{
			RTC_LOG(LS_ERROR) << "track " << id << " not found";
		}
	}

	void(WebRTCModule::mediaStreamTrackSetEnabled)(std::string id, boolean enabled) noexcept {
		if (auto track = local_tracks_[id]) {
			if (track->enabled() != enabled) {
				track->set_enabled(enabled);
				if (track->kind() == "video") {
					rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track = std::move(track);
					if (CapturerTrackSource* source = dynamic_cast<CapturerTrackSource*>(video_track->GetSource())) {
						enabled ? source->StartCapturer() : source->StopCapturer();
					}
					else {
						RTC_LOG(LS_ERROR) << "track " << id << " video source type unknown(not CapturerTrackSource)";
					}
				}
			}
		}
		else
		{
			RTC_LOG(LS_ERROR) << "track " << id << " not found";
		}
	}

	void(WebRTCModule::mediaStreamTrackSwitchCamera)(std::string id) noexcept {
		RTC_LOG(LS_ERROR) << "mediaStreamTrackSwitchCamera not implemented";
	}

	void(WebRTCModule::peerConnectionSetConfiguration)(webrtc::PeerConnectionInterface::RTCConfiguration configuration,
		int id) noexcept {
		if (auto pc = getPeerConnection()) {
			pc->SetConfiguration(configuration);
		}
	}

	void(WebRTCModule::peerConnectionAddStream)(std::string stream_id, int id) noexcept {
		if (auto pc = getPeerConnection()) {
			if (auto media_stream = local_streams_[stream_id]) {
				pc->AddStream(media_stream);
			}
			else {
				RTC_LOG(LS_ERROR) << "stream " << stream_id << " not found";
			}
		}
	}

	void(WebRTCModule::peerConnectionRemoveStream)(std::string stream_id, int id) noexcept {
		if (auto pc = getPeerConnection()) {
			if (auto media_stream = local_streams_[stream_id]) {
				pc->RemoveStream(media_stream);
			}
			else {
				RTC_LOG(LS_ERROR) << "stream " << stream_id << " not found";
			}
		}
	}

	class SDPCallbackHelper :
		public webrtc::CreateSessionDescriptionObserver,
		public webrtc::SetSessionDescriptionObserver,
		public webrtc::SetLocalDescriptionObserverInterface,
		public webrtc::SetRemoteDescriptionObserverInterface {
		const std::function<void(bool, JSValue&)> callback_;
	public:
		SDPCallbackHelper(const std::function<void(bool, JSValue&)> callback) :callback_(callback) {
		}
		SDPCallbackHelper(const std::function<void(bool, JSValue&)> callback) :callback_(callback) {
		}
		void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
			JSValueObject params;
			std::string sdp;
			desc->ToString(&sdp);
			params["sdp"] = sdp;
			params["type"] = desc->type();
			callback_(true, JSValue(std::move(params)));
		}
		void OnFailure(webrtc::RTCError error)override {
			callback_(false, JSValue{ error.message() });
		}
		void OnSuccess() override {
			callback_(true, JSValue());
		}
		void OnFailure(webrtc::RTCError error)override {
			callback_(false, JSValue{ error.message() });
		}
		void OnSetLocalDescriptionComplete(webrtc::RTCError error)override {
			callback_(error.ok(), JSValue{ error.message() });
		}
		void OnSetRemoteDescriptionComplete(webrtc::RTCError error)override {
			callback_(error.ok(), JSValue{ error.message() });
		}
	};

	rtc::scoped_refptr<webrtc::PeerConnectionInterface>(WebRTCModule::getPeerConnection)(int id) noexcept {

		if (pc_observers_.find(id) == pc_observers_.end()) {
			RTC_LOG(LS_ERROR) << "peerconnection " << id << " not found";
			return nullptr;
		}
		auto pc = pc_observers_[id]->GetPeerConnection();
		if (!pc) {
			RTC_LOG(LS_ERROR) << "peerconnection " << id << " broken";
			return nullptr;
		}

		return pc;
	}

	void(WebRTCModule::peerConnectionCreateOffer)(int id,
		JSValueObject&& options,
		const std::function<void(bool, JSValueObject&)> callback) noexcept {

		if (auto pc = getPeerConnection())
			pc->CreateOffer(new rtc::RefCountedObject<SDPCallbackHelper>(callback));
	}

	void(WebRTCModule::peerConnectionCreateAnswer)(int id,
		JSValueObject&& options,
		const std::function<void(bool, JSValueObject&)> callback) noexcept {

		if (auto pc = getPeerConnection())
			pc->CreateAnswer(new rtc::RefCountedObject<SDPCallbackHelper>(callback));
	}

	void(WebRTCModule::peerConnectionSetLocalDescription)(JSValueObject&& sdp_map,
		int id,
		const std::function<void(bool, JSValueObject&)> callback) noexcept {

		if (auto pc = getPeerConnection()) {
			std::string type = sdp_map["type"].AsString();
			std::string sdp = sdp_map["sdp"].AsString();
			webrtc::SdpType sdp_type;
			if (auto sdp_type_opt = webrtc::SdpTypeFromString(sdp)) {
				sdp_type = sdp_type_opt.value();
			}

			pc->SetLocalDescription(webrtc::CreateSessionDescription(sdp_type, sdp), new rtc::RefCountedObject<SDPCallbackHelper>(callback));
		}
	}

	void(WebRTCModule::peerConnectionSetRemoteDescription)(JSValueObject&& sdp_map,
		int id,
		const std::function<void(bool, JSValueObject&)> callback) noexcept {
		if (auto pc = getPeerConnection()) {
			std::string type = sdp_map["type"].AsString();
			std::string sdp = sdp_map["sdp"].AsString();
			webrtc::SdpType sdp_type;
			if (auto sdp_type_opt = webrtc::SdpTypeFromString(sdp)) {
				sdp_type = sdp_type_opt.value();
			}

			pc->SetRemoteDescription(webrtc::CreateSessionDescription(sdp_type, sdp), new rtc::RefCountedObject<SDPCallbackHelper>(callback));
		}
	}

	void(WebRTCModule::peerConnectionAddICECandidate)(JSValueObject&& candidate_map,
		int id,
		const std::function<void(bool)> callback) noexcept {
		bool result = false;

		if (auto pc = getPeerConnection()) {
			webrtc::SdpParseError error;
			auto candidate = webrtc::CreateIceCandidate(
				candidate_map["sdpMid"].AsString(),
				(int)candidate_map["sdpMLineIndex"].AsInt32(),
				candidate_map["candidate"].AsString(), &error);

			if (!candidate) {
				RTC_LOG(LS_ERROR) << "parse candidate error: " << error.description;
				callback(false);
				return;
			}
			if (!pc->AddIceCandidate(candidate)) {
				RTC_LOG(LS_ERROR) << "add candidate failed";
				callback(false);
				return;
			}

			result = true;
		}

		callback(result);
	}

	class GetStatsCallbackHelper : public webrtc::RTCStatsCollectorCallback {
		const React::ReactPromise<JSValue> promise_;
	public:
		GetStatsCallbackHelper(const React::ReactPromise<JSValue>& promise) :promise_(promise) {}
		void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) override {
			promise_.Resolve(JSValue{ report->ToJson() });
		}
	};

	void(WebRTCModule::peerConnectionGetStats)(int id, React::ReactPromise<JSValue> promise) noexcept {
		if (auto pc = getPeerConnection(id)) {
			pc->GetStats(new rtc::RefCountedObject<GetStatsCallbackHelper>(promise));
		}
		else {
			promise.Reject("peerconnection not found");
		}
	}

	void(WebRTCModule::peerConnectionClose)(int id) noexcept {
		if (auto pco = pc_observers_[id]) {
			pco->Close();
			pc_observers_.erase(id);
		}
	}

	void(WebRTCModule::createDataChannel)(int peerConnectionId,
		std::string label,
		JSValueObject&& config) noexcept {

	}

	void(WebRTCModule::dataChannelClose)(int peerConnectionId, int dataChannelId) noexcept {

	}

	void(WebRTCModule::dataChannelSend)(int peerConnectionId,
		int dataChannelId,
		std::string data,
		std::string type) noexcept {

	}
} // namespace winrt::WebRTCModule
