#pragma once

#include "pch.h"

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"

namespace winrt::WebRTCModule {

    class PeerConnectionObserver : public webrtc::PeerConnectionObserver {
        int id_;
        WebRTCModule* module_;
        rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
        std::list<rtc::scoped_refptr<webrtc::MediaStreamInterface>> local_streams_;
        std::map<std::string, rtc::scoped_refptr<webrtc::MediaStreamInterface>> remote_streams_;
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
            webrtc::PeerConnectionInterface::SignalingState new_state) override;
        void OnAddTrack(
            rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
            const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
            streams) override;
        void OnRemoveTrack(
            rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;
        void OnDataChannel(
            rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override {}
        void OnRenegotiationNeeded() override {}
        void OnIceConnectionChange(
            webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
        void OnIceGatheringChange(
            webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
        void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
        void OnIceConnectionReceivingChange(bool receiving) override {}
    };

}

namespace winrt::WebRTCModule {

    void PeerConnectionObserver::OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state)  {
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
void PeerConnectionObserver::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>&
        streams) {
    module_->
}

void PeerConnectionObserver::OnRemoveTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  RTC_LOG(INFO) << __FUNCTION__ << " " << receiver->id();
  main_wnd_->QueueUIThreadCallback(TRACK_REMOVED, receiver->track().release());
}

void PeerConnectionObserver::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  RTC_LOG(INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();
  // For loopback test. To save some connecting delay.
  if (loopback_) {
    if (!peer_connection_->AddIceCandidate(candidate)) {
      RTC_LOG(WARNING) << "Failed to apply the received candidate";
    }
    return;
  }

  Json::StyledWriter writer;
  Json::Value jmessage;

  jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
  jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
  std::string sdp;
  if (!candidate->ToString(&sdp)) {
    RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  jmessage[kCandidateSdpName] = sdp;
  SendMessage(writer.write(jmessage));
}

//
// PeerConnectionClientObserver implementation.
//

void PeerConnectionObserver::OnSignedIn() {
  RTC_LOG(INFO) << __FUNCTION__;
  main_wnd_->SwitchToPeerList(client_->peers());
}

void PeerConnectionObserver::OnDisconnected() {
  RTC_LOG(INFO) << __FUNCTION__;

  DeletePeerConnection();

  if (main_wnd_->IsWindow())
    main_wnd_->SwitchToConnectUI();
}

void PeerConnectionObserver::OnPeerConnected(int id, const std::string& name) {
  RTC_LOG(INFO) << __FUNCTION__;
  // Refresh the list if we're showing it.
  if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
    main_wnd_->SwitchToPeerList(client_->peers());
}

void PeerConnectionObserver::OnPeerDisconnected(int id) {
  RTC_LOG(INFO) << __FUNCTION__;
  if (id == peer_id_) {
    RTC_LOG(INFO) << "Our peer disconnected";
    main_wnd_->QueueUIThreadCallback(PEER_CONNECTION_CLOSED, NULL);
  } else {
    // Refresh the list if we're showing it.
    if (main_wnd_->current_ui() == MainWindow::LIST_PEERS)
      main_wnd_->SwitchToPeerList(client_->peers());
  }
}

void PeerConnectionObserver::OnMessageFromPeer(int peer_id, const std::string& message) {
  RTC_DCHECK(peer_id_ == peer_id || peer_id_ == -1);
  RTC_DCHECK(!message.empty());

  if (!peer_connection_.get()) {
    RTC_DCHECK(peer_id_ == -1);
    peer_id_ = peer_id;

    if (!InitializePeerConnection()) {
      RTC_LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
      client_->SignOut();
      return;
    }
  } else if (peer_id != peer_id_) {
    RTC_DCHECK(peer_id_ != -1);
    RTC_LOG(WARNING)
        << "Received a message from unknown peer while already in a "
           "conversation with a different peer.";
    return;
  }

  Json::Reader reader;
  Json::Value jmessage;
  if (!reader.parse(message, jmessage)) {
    RTC_LOG(WARNING) << "Received unknown message. " << message;
    return;
  }
  std::string type_str;
  std::string json_object;

  rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName,
                               &type_str);
  if (!type_str.empty()) {
    if (type_str == "offer-loopback") {
      // This is a loopback call.
      // Recreate the peerconnection with DTLS disabled.
      if (!ReinitializePeerConnectionForLoopback()) {
        RTC_LOG(LS_ERROR) << "Failed to initialize our PeerConnection instance";
        DeletePeerConnection();
        client_->SignOut();
      }
      return;
    }
    absl::optional<webrtc::SdpType> type_maybe =
        webrtc::SdpTypeFromString(type_str);
    if (!type_maybe) {
      RTC_LOG(LS_ERROR) << "Unknown SDP type: " << type_str;
      return;
    }
    webrtc::SdpType type = *type_maybe;
    std::string sdp;
    if (!rtc::GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName,
                                      &sdp)) {
      RTC_LOG(WARNING) << "Can't parse received session description message.";
      return;
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
        webrtc::CreateSessionDescription(type, sdp, &error);
    if (!session_description) {
      RTC_LOG(WARNING) << "Can't parse received session description message. "
                          "SdpParseError was: "
                       << error.description;
      return;
    }
    RTC_LOG(INFO) << " Received session description :" << message;
    peer_connection_->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(),
        session_description.release());
    if (type == webrtc::SdpType::kOffer) {
      peer_connection_->CreateAnswer(
          this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
  } else {
    std::string sdp_mid;
    int sdp_mlineindex = 0;
    std::string sdp;
    if (!rtc::GetStringFromJsonObject(jmessage, kCandidateSdpMidName,
                                      &sdp_mid) ||
        !rtc::GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                                   &sdp_mlineindex) ||
        !rtc::GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
      RTC_LOG(WARNING) << "Can't parse received message.";
      return;
    }
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
    if (!candidate.get()) {
      RTC_LOG(WARNING) << "Can't parse received candidate message. "
                          "SdpParseError was: "
                       << error.description;
      return;
    }
    if (!peer_connection_->AddIceCandidate(candidate.get())) {
      RTC_LOG(WARNING) << "Failed to apply the received candidate";
      return;
    }
    RTC_LOG(INFO) << " Received candidate :" << message;
  }
}

void PeerConnectionObserver::OnMessageSent(int err) {
  // Process the next pending message if any.
  main_wnd_->QueueUIThreadCallback(SEND_MESSAGE_TO_PEER, NULL);
}

void PeerConnectionObserver::OnServerConnectionFailure() {
  main_wnd_->MessageBox("Error", ("Failed to connect to " + server_).c_str(),
                        true);
}

}