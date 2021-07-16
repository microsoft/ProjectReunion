# Roadmap

The roadmap for the Windows App SDK supports the primary goals of breaking down the barriers between Win32 and UWP and making the Windows developer platform more agile, compatible, modern and open.

For a general overview, see the [readme](https://github.com/microsoft/ProjectReunion/tree/master/docs) and [FAQ](https://github.com/microsoft/ProjectReunion/blob/master/docs/faq.md).

## 2021 Roadmap

Here's how we intend to roll out the Windows App SDK. To subscribe to updates, subscribe to [this discussion](https://github.com/microsoft/ProjectReunion/discussions/507). *(all times/features/releases are tentative and subject to change)*:

### Packaged Desktop Apps
As a [packaged MSIX desktop app](https://docs.microsoft.com/windows/msix/overview) (WPF, WinForms, Win32, console), here's what and when you (tentatively) can expect from the Windows App SDK!

🌟 |      Windows App SDK 0.5      |         Windows App SDK 0.8    |       Windows App SDK 1.0 (Q4 2021)     |
-- | ------------------------------- | --------------------------------- | ------------------------------- |
[WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md#winui-3) | ✅ Supported  | ✅ Supported | ✅ Supported |
[Text rendering](https://docs.microsoft.com/en-us/windows/apps/project-reunion/dwritecore) | ✅ Supported | ✅ Supported | ✅ Supported |
[Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview) | ✅ Supported | ✅ Supported | ✅ Supported |
[App lifecycle](https://github.com/microsoft/ProjectReunion/issues/111) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
[Power state notifications](https://github.com/microsoft/ProjectReunion/issues/111) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
Windowing | ❌ Not Supported | ❌ Not Supported | ✅ Supported |
[Push Notifications](https://github.com/microsoft/ProjectReunion/blob/main/specs/PushNotifications/PushNotifications-spec.md) | ❌ Not Supported | ❌ Not Supported | 🔄 Experimental |
Deploy Options: |
• In-app | ✅ Supported | ✅ Supported | ✅ Supported |
• Fx Package | ✅ Supported | ✅ Supported | ✅ Supported |

### Unpackaged Desktop Apps

As an unpackaged desktop app (WPF, WinForms, Win32, or console, not using [MSIX](https://docs.microsoft.com/windows/msix/overview)), here's what and when you (tentatively) can expect from the Windows App SDK!

 🌟 |       Windows App SDK 0.5      |         Windows App SDK 0.8   |       Windows App SDK 1.0 (Q4 2021)    |
-- | ---------------------------------- | ------------------------------ | ------------------------------- |
[WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md#winui-3) | ❌ Not Supported | ❌ Not Supported | ✅ Supported |
[Text rendering](https://docs.microsoft.com/en-us/windows/apps/project-reunion/dwritecore) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
[Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview) | ❌ Not Supported| 🔄 Experimental | ✅ Supported |
[Dynamic Dependencies](https://github.com/microsoft/WindowsAppSDK/blob/main/specs/dynamicdependencies/DynamicDependencies.md) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
[App lifecycle](https://github.com/microsoft/ProjectReunion/issues/111) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
[Power state notifications](https://github.com/microsoft/ProjectReunion/issues/111) | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
Windowing | ❌ Not Supported | ❌ Not Supported | ✅ Supported |
[Push Notifications](https://github.com/microsoft/ProjectReunion/blob/main/specs/PushNotifications/PushNotifications-spec.md) | ❌ Not Supported | ❌ Not Supported | 🔄 Experimental |
Deploy Options:  |
• In-app | ❌ Not Supported | 🔄 Experimental | ✅ Supported |
• Fx Package | ❌ Not Supported | 🔄 Experimental | ✅ Supported |

## 2021 Focus Areas

The Windows platform team is currently focused on the four primary areas below for the Windows App SDK.

This isn't an exhaustive list: it's a sampling of some of the key infrastructure work we're doing to break down the barriers between Win32 and UWP and decouple the platform from the OS, plus some of the new functionality we're adding to enable new app capabilities and address top developer issues.

### 1. Coherent, modern interactions and UX design

* [WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md) - the Windows 10 native UI platform for Win32 and UWP
* [WebView2](https://docs.microsoft.com/microsoft-edge/webview2/) - embedding web content in Windows apps using the new Edge (Chromium) engine
* [React Native Windows](https://github.com/microsoft/react-native-windows/projects/30) - now targeting WinUI
* [Modern Windowing](https://github.com/microsoft/ProjectReunion/discussions/370)

### 2. Optimized for the device hardware

* Touch, inking, display improvements
* ARM64 support
* Input

### 3. Great system performance and battery life

* [Better options for app lifecycle management and power usage](https://github.com/microsoft/ProjectReunion/issues/111)
* [DirectWrite text rendering platform](https://github.com/microsoft/ProjectReunion/issues/112)
* [Local and Push Notifications](https://github.com/microsoft/ProjectReunion/discussions/371)
* Background Tasks

### 4. Hassle-free app discovery and management

* Enhanced app packaging
* Framework package deployment
* Auto update for all app types

### 5. Platform unification and deployment

* Decoupling the Windows platform from the OS
  * faster updates that you can start using on day 1
* Ensuring features work on all supported Windows versions
  * initial min OS version = 1809
  * polyfilling features as needed
  * supporting both Win32 and UWP

* Moving engineering to GitHub
  * [Proposals](https://github.com/microsoft/ProjectReunion/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+proposal%22) -> [Specs](https://github.com/microsoft/ProjectReunion/tree/master/specs) -> [Code](https://github.com/microsoft/ProjectReunion/tree/master/dev)

## 2022 and Beyond

In the future, you can expect multiple releases per year from the Windows App SDK team. We will continue to improve app compatibility and deliver new capabilities.
