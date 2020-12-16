#pragma once
#ifndef JSVALUEREADER
#define JSVALUEREADER
#include "JSValue.h"
#include "JSValueTreeReader.h"
#include "StructInfo.h"

#include "winrt/Microsoft.ReactNative.h"
#include "api/peer_connection_interface.h"

namespace winrt::Microsoft::ReactNative {

	void ReadValue(IJSValueReader const& reader, /*out*/ webrtc::PeerConnectionInterface::RTCConfiguration& value) noexcept;

	// implementation
	inline void ReadValue(IJSValueReader const& reader, /*out*/ webrtc::PeerConnectionInterface::RTCConfiguration& value) noexcept {

	}
}

#endif