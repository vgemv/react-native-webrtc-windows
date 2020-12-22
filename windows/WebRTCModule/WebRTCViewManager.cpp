#include "pch.h"
#include "WebRTCViewManager.h"

#include "JSValueReader.h"
#include "NativeModules.h"

using namespace winrt;
using namespace Microsoft::ReactNative;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Controls;

namespace winrt::WebRTCModule::implementation {

    // IViewManager
    hstring WebRTCViewManager::Name() noexcept {
        return L"CustomUserControl";
    }

    FrameworkElement WebRTCViewManager::CreateView() noexcept {
        return winrt::WebRTCModule::RTCView();
    }

    // IViewManagerWithNativeProperties
    IMapView<hstring, ViewManagerPropertyType> WebRTCViewManager::NativeProps() noexcept {
        auto nativeProps = winrt::single_threaded_map<hstring, ViewManagerPropertyType>();

        nativeProps.Insert(L"label", ViewManagerPropertyType::String);
        nativeProps.Insert(L"color", ViewManagerPropertyType::Color);
        nativeProps.Insert(L"backgroundColor", ViewManagerPropertyType::Color);

        return nativeProps.GetView();
    }

    void WebRTCViewManager::UpdateProperties(
        FrameworkElement const& view,
        IJSValueReader const& propertyMapReader) noexcept {
        if (auto control = view.try_as<winrt::WebRTCModule::RTCView>()) {

            const JSValueObject& propertyMap = JSValue::ReadObjectFrom(propertyMapReader);

            for (auto const& pair : propertyMap) {
                auto const& propertyName = pair.first;
                auto const& propertyValue = pair.second;

                if (propertyName == "label") {
                    if (propertyValue != nullptr) {
                        auto const& value = winrt::box_value(winrt::to_hstring(propertyValue.String()));
                        control.SetValue(winrt::WebRTCModule::CustomUserControl::LabelProperty(), propertyValue);
                    }
                    else {
                        control.ClearValue(winrt::WebRTCModule::CustomUserControl::LabelProperty());
                    }
                }
                else if (propertyName == "color") {
                    if (auto value = propertyValue.To<Brush>()) {
                        control.SetValue(Control::ForegroundProperty(), value);
                    }
                    else {
                        control.ClearValue(Control::ForegroundProperty());
                    }
                }
                else if (propertyName == "backgroundColor") {
                    if (auto value = propertyValue.To<Brush>()) {
                        control.SetValue(Control::BackgroundProperty(), value);
                    }
                    else {
                        control.ClearValue(Control::BackgroundProperty());
                    }
                }
            }
        }
    }

    // IViewManagerWithCommands
    IVectorView<hstring> WebRTCViewManager::Commands() noexcept {
        auto commands = winrt::single_threaded_vector<hstring>();
        commands.Append(L"CustomCommand");
        return commands.GetView();
    }

    void WebRTCViewManager::DispatchCommand(
        FrameworkElement const& view,
        winrt::hstring const& commandId,
        winrt::Microsoft::ReactNative::IJSValueReader const& commandArgsReader) noexcept {
        //if (auto control = view.try_as<winrt::SampleLibraryCPP::CustomUserControlCPP>()) {
        //    if (commandId == L"CustomCommand") {
        //        const JSValueArray& commandArgs = JSValue::ReadArrayFrom(commandArgsReader);
        //        // Execute command
        //    }
        //}
    }

}