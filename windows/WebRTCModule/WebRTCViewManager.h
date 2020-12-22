#pragma once

#include "pch.h"

#include "winrt/Microsoft.ReactNative.h"
//#include <windows.ui.xaml.h>
#include <winrt/windows.ui.xaml.controls.h>
using namespace Windows::UI::Xaml::Controls;

namespace winrt::WebRTCModule::implementation {
    struct RTCView : winrt::implements < Control > {

    };

    struct WebRTCViewManager : winrt::implements<
        WebRTCViewManager,
        winrt::Microsoft::ReactNative::IViewManager,
        winrt::Microsoft::ReactNative::IViewManagerWithNativeProperties,
        winrt::Microsoft::ReactNative::IViewManagerWithCommands> {
    public:
        WebRTCViewManager() = default;

        // IViewManager
        winrt::hstring Name() noexcept;

        winrt::Windows::UI::Xaml::FrameworkElement CreateView() noexcept;

        // IViewManagerWithNativeProperties
        winrt::Windows::Foundation::Collections::
            IMapView<winrt::hstring, winrt::Microsoft::ReactNative::ViewManagerPropertyType>
            NativeProps() noexcept;

        void UpdateProperties(
            winrt::Windows::UI::Xaml::FrameworkElement const& view,
            winrt::Microsoft::ReactNative::IJSValueReader const& propertyMapReader) noexcept;

        // IViewManagerWithCommands
        winrt::Windows::Foundation::Collections::IVectorView<winrt::hstring> Commands() noexcept;

        void DispatchCommand(
            winrt::Windows::UI::Xaml::FrameworkElement const& view,
            winrt::hstring const& commandId,
            winrt::Microsoft::ReactNative::IJSValueReader const& commandArgsReader) noexcept;
    };

}