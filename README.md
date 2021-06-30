# Windows App SDK - Calling all Windows developers!

*Previously known as Project Reunion*.

| Delight users with modern UI | Access new platform features | Backwards compatible | 
|:--:|:--:|:--:|
| <img src="https://docs.microsoft.com/media/illustrations/biztalk-get-started-get-started.svg" width=250 alt="Modern navigation"/><br>Powerful WinUI 3 UI | <img src="https://docs.microsoft.com/media/illustrations/biztalk-get-started-scenarios.svg" width=250 alt="Platform logo"/><br>Localization + power status + more<br> | <img src="https://docs.microsoft.com/media/illustrations/biztalk-host-integration-install-configure.svg" width=250 alt="Down-level logo"/><br>Down to Windows 10 1809 |

...and more, **without having to rewrite your app!**

* Use your current installer *(no requirement to use MSIX, but there are [reliability/security benefits to using MSIX](https://docs.microsoft.com/windows/msix/overview#key-features)!)*
* 100% additive *(only add what you need, leave the rest of your app as-is)*
* Works with Win32, WPF, WinForms, and more apps

Plus, **create beautiful, modern apps for Windows 11 that are backwards compatible** to Windows 10 October 2018 Update (build 17763 / version 1809)!


### Use it now!
[Packaged MSIX desktop apps](https://docs.microsoft.com/windows/msix/overview) can [use the 0.8 release](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/) now!

Some notes about 0.8...

* Requires using MSIX *(1.0 won't require MSIX)*
* Only includes WinUI 3 / Text rendering / Localization
* To use WinUI 3, it's best to start a new app project and, if useful, copy any existing code/business logic from past projects into the new project *(in the future, XAML Islands will enable incrementally adopting WinUI 3)*

### Roadmap

Here's a snippet of what we intend to deliver in 2021 *(times/features/releases are tentative and subject to change)*: 

App type |       Windows App SDK 0.8    |       Windows App SDK 1.0 (Q4 2021)     | 
-- | ------------------------------- | ------------------------------- |
[Packaged MSIX](https://docs.microsoft.com/windows/msix/overview) desktop apps<br>(WPF, Win32, WinForms, console) |      ✅ [WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md#winui-3)<br>✅ [Text rendering](https://docs.microsoft.com/windows/apps/project-reunion/dwritecore)<br>✅ [Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview)<br>🔄 [App lifecycle](https://github.com/microsoft/WindowsAppSDK/issues/111)  | ✅ [WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md#winui-3)<br>✅ [Text rendering](https://github.com/microsoft/WindowsAppSDK/issues/112)<br>✅ [Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview)<br>✅ [App lifecycle](https://github.com/microsoft/WindowsAppSDK/issues/111)  <br>🔄 [Push Notifications](https://github.com/microsoft/WindowsAppSDK/issues/334)  | 
Unpackaged desktop apps<br>(WPF, Win32, WinForms, console) |    🔄 [Text rendering](https://docs.microsoft.com/windows/apps/project-reunion/dwritecore)<br>🔄 [Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview)<br>🔄 [App lifecycle](https://github.com/microsoft/WindowsAppSDK/issues/111)  |  ✅ [WinUI 3](https://github.com/microsoft/microsoft-ui-xaml/blob/master/docs/roadmap.md#winui-3)<br>✅ [Text rendering](https://docs.microsoft.com/windows/apps/project-reunion/dwritecore)<br>✅ [Localization](https://docs.microsoft.com/windows/apps/project-reunion/mrtcore/mrtcore-overview)<br>✅ [App lifecycle](https://github.com/microsoft/WindowsAppSDK/issues/111) <br>🔄 [Push Notifications](https://github.com/microsoft/WindowsAppSDK/issues/334) | 

✅ Supported | 🔄 Experimental


See our [full roadmap](docs/roadmap.md) for more details.



### Version support

The Windows App SDK aims to work on supported versions of Windows.
Right now we support down to build 17763 (version 1809/October 2018 Update) of Windows 10.
Wherever possible, we'll use techniques like '_polyfills_' 
and custom implementations to do the
heavy lifting for you and make sure your app works across Windows versions and devices.
In the future, there may be a couple APIs that are dependent on new OS features
(like new Action Center functionality),
however we will do our best to ensure
this is the exception and not the norm, and provide reasonable fallbacks when possible.


### Documentation

More on what the project is and how we're approaching the work.
* [Video on Windows App SDK](https://aka.ms/project_reunion_ignite) - Learn about the Windows App SDK in a 22 minute video!
* [Roadmap](docs/roadmap.md) - Where we're going!
* [Contributor guide](docs/contributor-guide.md) - How to contribute to the Windows App SDK.
* [FAQ](docs/faq.md) - Frequently asked questions about the Windows App SDK.


### Contributing

[![Feature Proposals](https://img.shields.io/github/issues/microsoft/WindowsAppSDK/feature%20proposal)](https://github.com/microsoft/WindowsAppSDK/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+proposal%22)
[![Bugs](https://img.shields.io/github/issues/microsoft/WindowsAppSDK/bug)](https://github.com/microsoft/WindowsAppSDK/issues?q=is%3Aissue+is%3Aopen+label%3Abug)

_We want to hear from you!_

File a [new issue!](https://github.com/microsoft/WindowsAppSDK/issues/new/choose) Tell us what problem you're
trying to solve, how you've tried to solve it so far, and what would be the ideal solution for your app. Bonus
points if there's a gist or existing repo we can look at with you.


1. **[Ask a question](https://github.com/microsoft/WindowsAppSDK/discussions/categories/q-a)**<br>
    Also, have a look at our [frequently asked questions](docs/faq.md) page which we will update periodically.

2. **[Start a discussion](https://github.com/microsoft/WindowsAppSDK/discussions)**<br>
   Discussions can be about any topics or ideas related to the Windows App SDK. For example,
   you might start a Discussion issue to see if others are interested in a cool new idea
   you've been thinking about that isn't quite ready to be filed as a more formal Feature Proposal.

3. **[Make a feature proposal](https://github.com/microsoft/WindowsAppSDK/issues/new?assignees=&labels=feature+proposal&template=feature-proposal.md&title=)**<br>
   What components do you want to see in the Windows App SDK? For example you can propose a new API on an existing type, or an idea for a new API.
   It's fine if you don't have all the details: you can start with a summary and rationale.

**Over time, we will add Windows App SDK code and samples into this repo and provide code contribution/developer guidelines.**

Read more about the [contribution guide here](docs/contributor-guide.md).

## Data collection

The software may collect information about you and your use of the software and send it
to Microsoft. Microsoft may use this information to provide services and improve our
products and services. You may turn off the telemetry as described in the repository.
There are also some features in the software that may enable you and Microsoft to collect
data from users of your applications. If you use these features, you must comply with
applicable law, including providing appropriate notices to users of your applications
together with a copy of Microsoft's privacy statement. Our privacy statement is located
at https://go.microsoft.com/fwlink/?LinkID=824704. You can learn more about data collection
and use in the help documentation and our privacy statement. Your use of the software
operates as your consent to these practices.

## Legal notices

Microsoft and any contributors grant you a license to the Microsoft documentation and other content
in this repository under the [Creative Commons Attribution 4.0 International Public License](https://creativecommons.org/licenses/by/4.0/legalcode),
see the [LICENSE](LICENSE) file, and grant you a license to any code in the repository under the [MIT License](https://opensource.org/licenses/MIT), see the
[LICENSE-CODE](LICENSE-CODE) file.

Microsoft, Windows, Microsoft Azure and/or other Microsoft products and services referenced in the documentation
may be either trademarks or registered trademarks of Microsoft in the United States and/or other countries.
The licenses for this project do not grant you rights to use any Microsoft names, logos, or trademarks.
Microsoft's general trademark guidelines can be found at http://go.microsoft.com/fwlink/?LinkID=254653.

Privacy information can be found at https://privacy.microsoft.com/

Microsoft and any contributors reserve all other rights, whether under their respective copyrights, patents,
or trademarks, whether by implication, estoppel or otherwise.
