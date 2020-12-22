#pragma once

#include "pch.h"

#include "WebRTCModule.h"
#include "PeerConnectionObserver.h"

namespace winrt::WebRTCModule {

	std::string new_uuid();

	void PeerConnectionObserver::OnSignalingChange(
		webrtc::PeerConnectionInterface::SignalingState new_state) {

		std::string signalingState;
		switch (new_state) {
		case webrtc::PeerConnectionInterface::SignalingState::kClosed:
			signalingState = "closed"; break;
		case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalOffer:
			signalingState = "have-local-offer"; break;
		case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer:
			signalingState = "have-local-pranswer"; break;
		case webrtc::PeerConnectionInterface::SignalingState::kHaveRemoteOffer:
			signalingState = "have-remote-offer"; break;
		case webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer:
			signalingState = "have-remote-pranswer"; break;
		case webrtc::PeerConnectionInterface::SignalingState::kStable:
			signalingState = "stable"; break;
		}
		module_->peerConnectionSignalingStateChanged(JSValue{ std::move(JSValueObject{
			{"id", id_},
			{"signalingState", signalingState}
			}) });
	}

	void PeerConnectionObserver::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
		std::string uuid = new_uuid();
		remote_streams_[uuid] = stream;

		JSValueArray tracks;
		for (auto track : stream->GetVideoTracks()) {
			remote_tracks_[track->id()] = track;
			tracks.push_back(JSValue{ std::move(JSValueObject{
				{"id", track->id()},
				{"kind", track->kind()},
				{"label", track->id()},
				{"enabled", track->enabled()},
				{"remote", true},
				{"readyState", "live"}
			}) });
		}
		for (auto track : stream->GetAudioTracks()) {
			remote_tracks_[track->id()] = track;
			tracks.push_back(JSValue{ std::move(JSValueObject{
				{"id", track->id()},
				{"kind", track->kind()},
				{"label", track->id()},
				{"enabled", track->enabled()},
				{"remote", true},
				{"readyState", "live"}
			}) });
		}

		module_->peerConnectionAddedStream(JSValue{ std::move(JSValueObject{
			{"id", id_},
			{"streamId", stream->id()},
			{"streamReactTag", uuid},
			{"tracks", JSValue{ std::move(tracks)}}
			}) });

	}

	void PeerConnectionObserver::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {

		std::string streamReactTag;
		for (auto s : remote_streams_) {
			if (s.second == stream) {
				streamReactTag = s.first;
				remote_streams_.erase(s.first);
			}
		}
		if (streamReactTag.empty()) {
			RTC_LOG(WARNING) << "stream for id: " << stream->id() << "not found";
			return;
		}
		for (auto t : stream->GetAudioTracks()) {
			remote_tracks_.erase(t->id());
		}
		for (auto t : stream->GetVideoTracks()) {
			remote_tracks_.erase(t->id());
		}
		module_->peerConnectionRemovedStream(JSValue{ std::move(JSValueObject{
			{"id",id_},
			{"streamId",streamReactTag}
			}) });
	}

	void PeerConnectionObserver::OnRenegotiationNeeded() {
		module_->peerConnectionOnRenegotiationNeeded(JSValue{ std::move(JSValueObject{
			{"id",id_}
			}) });
	}

	void PeerConnectionObserver::OnIceConnectionChange(
		webrtc::PeerConnectionInterface::IceConnectionState new_state) {
		std::string state;
		switch (new_state) {
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionChecking:
			state = "checking"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed:
			state = "closed"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted:
			state = "completed"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionConnected:
			state = "connected"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionDisconnected:
			state = "disconnected"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionFailed:
			state = "failed"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionMax:
			state = "max"; break;
		case webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionNew:
			state = "new"; break;
		}
		module_->peerConnectionIceConnectionChanged(JSValue{ std::move(JSValueObject{
			{"id",id_},
			{"iceConnectionState",state}
			}) });

	}

	void PeerConnectionObserver::OnConnectionChange(
		webrtc::PeerConnectionInterface::PeerConnectionState new_state) {
		std::string state;
		switch (new_state) {
		case webrtc::PeerConnectionInterface::PeerConnectionState::kClosed:
			state = "closed"; break;
		case webrtc::PeerConnectionInterface::PeerConnectionState::kConnected:
			state = "connected"; break;
		case webrtc::PeerConnectionInterface::PeerConnectionState::kConnecting:
			state = "connecting"; break;
		case webrtc::PeerConnectionInterface::PeerConnectionState::kDisconnected:
			state = "disconnected"; break;
		case webrtc::PeerConnectionInterface::PeerConnectionState::kFailed:
			state = "failed"; break;
		case webrtc::PeerConnectionInterface::PeerConnectionState::kNew:
			state = "new"; break;
		}
		module_->peerConnectionStateChanged(JSValue{ std::move(JSValueObject{
			{"id",id_},
			{"connectionState",state}
			}) });
	}

	void PeerConnectionObserver::OnIceGatheringChange(
		webrtc::PeerConnectionInterface::IceGatheringState new_state) {
		std::string state;
		switch (new_state) {
		case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete:
			state = "complete"; break;
		case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringGathering:
			state = "gathering"; break;
		case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringNew:
			state = "new"; break;
		}

		module_->peerConnectionIceGatheringChanged(JSValue{ std::move(JSValueObject{
			{"id",id_},
			{"iceGatheringState",state}
			}) });
	}

	void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
		std::string sdp;
		candidate->ToString(&sdp);

		module_->peerConnectionGotICECandidate(JSValue{ std::move(JSValueObject{

			{"id", id_},
			{"candidate", JSValueObject{
				{"candidate", sdp},
				{"sdpMLineIndex", candidate->sdp_mline_index()},
				{"sdpMid", candidate->sdp_mid()}
			}}
			})
			});
	}

}

