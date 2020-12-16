#include "pch.h"
#include "ReactPackageProvider.h"
#include "NativeModules.h"

#include "WebRTCModule.h" 

using namespace winrt::Microsoft::ReactNative;

namespace winrt::WebRTCModule::implementation
{

void ReactPackageProvider::CreatePackage(IReactPackageBuilder const &packageBuilder) noexcept
{
    AddAttributedModules(packageBuilder);
}

} // namespace winrt::WebRTCModule::implementation


