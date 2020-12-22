#pragma once

#include "pch.h"

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#define WEBRTC_WIN 1

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"

namespace winrt::WebRTCModule {

	class WebRTCModule;

	class PeerConnectionObserver : public webrtc::PeerConnectionObserver, public rtc::RefCountInterface {
		int id_;
		WebRTCModule* module_;
		rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
		std::list<rtc::scoped_refptr<webrtc::MediaStreamInterface>> local_streams_;
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> remote_streams_;
		std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> remote_tracks_;
	public:

		PeerConnectionObserver(WebRTCModule* module, int id) :
			module_(module),
			id_(id) {
		}

		void Close() {
			peer_connection_->Close();
			for (rtc::scoped_refptr<webrtc::MediaStreamInterface> s : local_streams_) {
				peer_connection_->RemoveStream(s.get());
			}
			local_streams_.clear();

			for (auto s : remote_streams_) {
				for (auto track : s.second->GetVideoTracks()) {
					//TODO: adapter?
				}
			}

			peer_connection_->Release();

			remote_streams_.clear();
		}

		void SetPeerConnection(rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection) {
			peer_connection_ = peer_connection;
		}

		rtc::scoped_refptr<webrtc::PeerConnectionInterface> GetPeerConnection() {
			return peer_connection_;
		}

		bool AddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> local_stream) {
			if (peer_connection_ && peer_connection_->AddStream(local_stream.get())) {
				local_streams_.push_back(local_stream);
				return true;
			}

			return false;
		}

		bool RemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> local_stream) {
			if (peer_connection_) {
				peer_connection_->RemoveStream(local_stream.get());
			}

			auto found = std::find(local_streams_.begin(), local_streams_.end(), local_stream);
			if (found != local_streams_.end())
				local_streams_.erase(found);

			return true;
		}

	protected:

		void OnSignalingChange(
			webrtc::PeerConnectionInterface::SignalingState new_state);

		void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);

		void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream);
		void OnDataChannel(
			rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) {}

		void OnRenegotiationNeeded();

		void OnNegotiationNeededEvent(uint32_t event_id) {
		}

		void OnIceConnectionChange(
			webrtc::PeerConnectionInterface::IceConnectionState new_state);

		void OnStandardizedIceConnectionChange(
			webrtc::PeerConnectionInterface::IceConnectionState new_state) {}

		void OnConnectionChange(
			webrtc::PeerConnectionInterface::PeerConnectionState new_state);

		void OnIceGatheringChange(
			webrtc::PeerConnectionInterface::IceGatheringState new_state);

		void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
		void OnIceCandidateError(const std::string& host_candidate,
			const std::string& url,
			int error_code,
			const std::string& error_text) {}

		void OnIceCandidateError(const std::string& address,
			int port,
			const std::string& url,
			int error_code,
			const std::string& error_text) {}

		void OnIceCandidatesRemoved(
			const std::vector<cricket::Candidate>& candidates) {}

		void OnIceConnectionReceivingChange(bool receiving) {}

		void OnIceSelectedCandidatePairChanged(
			const cricket::CandidatePairChangeEvent& event) {}

		void OnAddTrack(
			rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
			const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {}

		void OnTrack(
			rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {}

		void OnRemoveTrack(
			rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {}

		void OnInterestingUsage(int usage_pattern) {}
	};

}