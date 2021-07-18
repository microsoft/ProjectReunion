# AppLifecycle - State Notifications

Windows App SDK introduces a new AppLifecycle component that provides a core set of functionality related to
app activation and lifecycle. Much of this functionality is directly equivalent - or very similar -
to existing functionality within the platform. However, there is value in abstracting this
functionality so that all app types can use it, delivering it out-of-band from the OS, and making it
available as open-source. This spec addresses the System State and Power Notifications APIs in 
the component.

## Background

There are several existing APIs which apps can use to detect changes in system state, in order to
participate in improved power usage and battery life. These APIs cover most if not all of the
battery/power state change scenarios that apps would care about - but there's no single API that is
easily consumable by all app types. There's also no single API that's undocked from the OS.

This spec describes new APIs that address these issues by incorporating a comprehensive set of the
[Windows.System.Power.PowerManager](https://docs.microsoft.com/uwp/api/Windows.System.Power.PowerManager)
WinRT API in the Windows App SDK AppLifecycle component, and include additional low-level notifications
based on the Win32
[PowerSettingRegisterNotification](https://docs.microsoft.com/windows/win32/api/powersetting/nf-powersetting-powersettingregisternotification)
and matching
[PowerSettingUnregisterNotification](https://docs.microsoft.com/windows/win32/api/powersetting/nf-powersetting-powersettingunregisternotification)
APIs.


## Description

Providing a Windows App SDK version of PowerManager is relatively straightforward, in that we aim to bring
across the entire existing API in its current form. For the Win32 APIs, the situation is a little
murkier. It boils down to a choice between 2 usage patterns:

-   [RegisterPowerSettingNotification](https://docs.microsoft.com/windows/win32/api/winuser/nf-winuser-registerpowersettingnotification)
    uses a Windows message-based model. When the app registers for notifications, the system will
    send out WM_POWERBROADCAST messages.
-   [PowerSettingRegisterNotification](https://docs.microsoft.com/windows/win32/api/powersetting/nf-powersetting-powersettingregisternotification)
    uses a callback model. When the app registers for notifications, it supplies a pointer to a
    callback function, and the system will call back to this function instead of sending messages.

Classic Win32 apps can use either the WM model and/or the callback model. The callback model is more
consistent with other app types - including not only modern UWP apps, but also traditional MFC,
Winforms and WPF apps. Also, the callback model does not prescribe a windowing or threading model,
which is better for headless/background components. Given this, since all app types can work with
the callback model, but only classic Win32 apps use the WM model, the Windows App SDK API will use the
callback model for Cobalt. Additional WM-based behavior can be added in a later release, if customer
demand indicates it would provide significant benefit.

The net is that the Windows App SDK PowerManager API will be a combination of WinRT PowerManager and Win32
PowerSettingRegisterNotification. We will define new WinRT types (in
Microsoft.ApplicationModel.PowerManager) that provide event handlers for apps to use.

## Examples

### Subscribing and responding to events

The power notifications are relatively simple. The app provides a callback function for each
notification it wants to receive, and when the event is raised the system invokes that callback. The
app could start listening for the notifications at any time; it would most often do this early in
its life, perhaps in WinMain - but the AppLifecycle mechanism does not impose any constraints on
this, and the app is free to subscribe or unsubscribe at any time.

```c++
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize COM.
    winrt::init_apartment();

    // Optionally, register callbacks for power/battery state changes.
    PowerManager::BatteryStatusChanged([](auto&&...)
        { OnBatteryStatusChanged(); });
    PowerManager::PowerSupplyStatusChanged([](auto&&...)
        { OnPowerStatusChanged(); });
    PowerManager::EnergySaverStatusChanged([](auto&&...)
        { OnEnergySaverStatusChanged(); });
    PowerManager::RemainingChargePercentChanged([](auto&&...)
        { OnRemainingChargePercentChanged(); });
    PowerManager::RemainingDischargeTimeChanged([](auto&&...)
        { OnRemainingDischargeTimeChanged(); });

    ///////////////////////////////////////////////////////////////////////////
    // Standard Win32 window configuration/creation and message pump:
    // ie, whatever the app would normally do - nothing new here.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CLASSIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
```

In the app-defined callback, the app can do whatever it likes. Typically, it would fetch the current
status of the particular feature (that is, the battery status, power supply status, energy saver
status, and so on), and use that information to form its decision. For example, if battery is at a
low power state and discharging, the app might choose to defer any non-critical background work, in
the interests of being a good power/battery citizen.

Note that these are regular WinRT event delegate invocations. See also: https://github.com/microsoft/xlang/issues/673

```c++
void OnBatteryStatusChanged()
{
    BatteryStatus batteryStatus = PowerManager::BatteryStatus();
    int remainingCharge = PowerManager::RemainingChargePercent();

    if (batteryStatus == BatteryStatus::Discharging && remainingCharge < 25)
    {
        // We're in a bad battery state, we should pause any non-critical work.
        PauseNonCriticalWork();
    }
    else if (batteryStatus == BatteryStatus::Charging && remainingCharge > 75)
    {
        // Battery is in great shape, let's kick of some high-power work.
        StartPowerIntensiveWork();
    }
}
```

### App logic based on multiple status values

The individual notifications are relatively low-level, and in some scenarios each notification in
itself might not provide enough information for the app to make sensible decisions. More likely, the
app will use multiple PowerManager properties and methods to factor into its decision. For example,
when it receives a power supply state change notification, it might also want to check the battery
status before proceeding.

```c++
void OnPowerSupplyStatusChanged()
{
    PowerSupplyStatus powerStatus = PowerManager::PowerSupplyStatus();
    BatteryStatus batteryStatus = PowerManager::BatteryStatus();
    int remainingCharge = PowerManager::RemainingChargePercent();

    // Note if the BatteryStatus is BatteryStatus::NotPresent,
    // then RemainingChargePercent is 100.

    if (batteryStatus == BatteryStatus::Discharging
        && remainingCharge < 25
        && powerStatus != PowerSupplyStatus::Adequate)
    {
        // We're in a bad power/battery state: let's pause any non-critical work.
        PauseNonCriticalWork();
    }
    else if (powerStatus == PowerSupplyStatus::Adequate ||
        (batteryStatus == BatteryStatus::Charging && remainingCharge > 75))
    {
        // Power/battery is in great shape, let's kick of some high-power work.
        StartPowerIntensiveWork();
    }
}
```

### New API combines multiple old APIs

The additional enums, properties and events brought forward for the PowerSettingRegisterNotification
functionality should be completely seamless and indistinguishable from the existing PowerManager
members, such that the app can consume these in exactly the same way.

```c++
void OnDisplayStatusChanged([](auto&&...)
{
    DisplayStatus displayStatus = PowerManager::DisplayStatus();
    if (displayStatus == DisplayStatus::Off)
    {
        // The screen is off, let's stop rendering foreground graphics,
        // and instead kick off some background work now.
        StopRenderingGraphics();
        StartDoingBackgroundWork();
    }
});
```

### Unsubscribing

As with all WinRT APIs, apps can register and deregister for notifications during their lifecycle.
Use your language's preferred event-registration management system if your app doesn't need to
recieve power status notifications during its entire lifecycle.

```c++
winrt::event_token s_batteryToken;
winrt::event_token s_powerToken;

void MyRegisterPowerManagerCallbacks()
{
    s_batteryToken = PowerManager::BatteryStatusChanged([](IInspectable sender, auto args)
        { OnBatteryStatusChanged(sender, args); });
    s_powerToken = PowerManager::PowerSupplyStatusChanged([](IInspectable sender, auto args)
        { OnPowerSupplyStatusChanged(sender, args); });
}

void MyUnregisterPowerManagerCallbacks()
{
    PowerManager::BatteryStatusChanged(s_batteryToken);
    PowerManager::PowerSupplyStatusChanged(s_powerToken);
}
```

## API Details

### Existing APIs

This set of APIs is a subset of the union of the existing WinRT PowerManager class and the existing
PowerSettingRegisterNotification API:

-   [Windows.System.Power.PowerManager](https://docs.microsoft.com/uwp/api/Windows.System.Power.PowerManager)
-   [PowerSettingRegisterNotification](https://docs.microsoft.com/windows/win32/api/powersetting/nf-powersetting-powersettingregisternotification)

The existing PowerSettingRegisterNotification API takes a number of parameters with many different
possible behaviors and events supported. Windows App SDK wraps these into a WinRT type accessible to
all languages and runtimes.

-   Many of the common power registration settings will be available as WinRT events
-   Initially, Windows App SDK only supports apps, so the "services or functions" flag is not
    configurable
-   The PowerManager implementation eliminates the need for apps to manage notification handles
    directly

As with many static WinRT `-Changed` events, the application is expected to query the current state of the
system when an event is raised; PowerManager events do not have `-EventArgs` payloads, and the type
of all event payloads is `Object`.

The well-known
[power-setting GUIDs](https://docs.microsoft.com/windows/win32/power/power-setting-guids) are
listed below, along with the proposed mappings to new PowerManager events and enum-based properties.

| Existing GUID                     | Description                                                  | New property and event                                    |
| --------------------------------- | ------------------------------------------------------------ | --------------------------------------------------------- |
| GUID_ACDC_POWER_SOURCE            | The system power source has changed.                         | `PowerSourceKind, PowerSourceKindChanged`                 |
| GUID_BATTERY_PERCENTAGE_REMAINING | The remaining battery capacity has changed.                  | `RemainingChargePercent`, `RemainingChargePercentChanged` |
| GUID_CONSOLE_DISPLAY_STATE        | The current monitor's display state has change. Apps should stop rendering graphics content when the monitor is off to reduce system power consumption. | _Not implemented_                                         |
| GUID_GLOBAL_USER_PRESENCE         | The user status associated with any session has changed. This notification is only to services and other programs running in session 0. User-mode applications should register for GUID_SESSION_USER_PRESENCE instead. | _Not implemented_                                         |
| GUID_IDLE_BACKGROUND_TASK         | The system is busy: now is a good time for apps to perform background or idle tasks that would otherwise prevent the computer from entering an idle state. | `SystemIdleStatusChanged` (no property)               |
| GUID_MONITOR_POWER_ON             | The monitor has been powered on or off. From Windows 8 and Windows Server 2012, apps should use GUID_CONSOLE_DISPLAY_STATE instead. | _Not implemented_                                         |
| GUID_POWER_SAVING_STATUS          | Battery saver has been turned on or off.                     | `EnergySaverStatus, EnergySaverStatusChanged`             |
| GUID_POWERSCHEME_PERSONALITY      | The active power scheme personality has changed. All power schemes map to one of these personalities. | _Not implemented_                                         |
| GUID_SESSION_DISPLAY_STATUS       | The display associated with the application's session has been powered on or off. | `DisplayStatus, DisplayStatusChanged`                     |
| GUID_SESSION_USER_PRESENCE        | The user status associated with the application's session has changed. | `UserPresenceStatus`                                      |
| GUID_SYSTEM_AWAYMODE              | The system is entering or exiting away mode.                 | _Not implemented_                                         |

In addition, the PowerRegisterForEffectivePowerModeNotifications API is brought forward with a new property and event:

| Existing API                                             | Description                             | New property and event                          |
| -------------------------------------------------------- | --------------------------------------- | ----------------------------------------------- |
| PowerRegisterForEffectivePowerModeNotifications function | The effective power mode of the system. | `EffectivePowerMode, EffectivePowerModeChanged` |

The existing PowerManager class exposes only static properties and events: all members are valid to
be brought over to the Windows App SDK version. Many existing enumerated values are brought over as-is as
well.

```idl
namespace Microsoft.Windows.System.Power
{
    // Enums duplicated from the existing WinRT PowerManager API.
    enum EnergySaverStatus
    {
        Uninitialized = 0,
        Disabled,
        Off,
        On
    };

    enum BatteryStatus
    {
        NotPresent = 0,
        Discharging,
        Idle,
        Charging
    };

    enum PowerSupplyStatus
    {
        NotPresent = 0,
        Inadequate,
        Adequate
    };

    // Enums based on the Win32 PowerSettingRegisterNotification API.
    enum PowerSourceKind
    {
        AC = 0,
        DC
    };

    enum DisplayStatus
    {
        Off = 0,
        On,
        Dimmed
    };

    enum EffectivePowerMode
    {
        // EFFECTIVE_POWER_MODE_V1 values
        BatterySaver,
        BetterBattery,
        Balanced,
        HighPerformance,
        MaxPerformance, 
        // EFFECTIVE_POWER_MODE_V2 additional values
        GameMode,
        MixedReality
    };

    enum UserPresenceStatus
    {
        Present = 0,
        Absent
    };

    enum SystemSuspendStatus
    {
        Uninitialized = 0,
        Entering,
        AutoResume,
        ManualResume
    };

    static runtimeclass PowerManager
    {
        static EnergySaverStatus EnergySaverStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> EnergySaverStatusChanged;

        static BatteryStatus BatteryStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> BatteryStatusChanged;

        static PowerSupplyStatus PowerSupplyStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> PowerSupplyStatusChanged;

        static Int32 RemainingChargePercent{ get; };
        static event Windows.Foundation.EventHandler<Object> RemainingChargePercentChanged;

        static Windows.Foundation.TimeSpan RemainingDischargeTime{ get; };
        static event Windows.Foundation.EventHandler<Object> RemainingDischargeTimeChanged;

        static PowerSourceKind PowerSourceKind{ get; };
        static event Windows.Foundation.EventHandler<Object> PowerSourceKindChanged;

        static DisplayStatus DisplayStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> DisplayStatusChanged;

        static event Windows.Foundation.EventHandler<Object> SystemIdleStatusChanged;

        static Windows.Foundation.IAsyncOperation<EffectivePowerMode> EffectivePowerMode{ get; };
        static event Windows.Foundation.EventHandler<Object> EffectivePowerModeChanged;

        static UserPresenceStatus UserPresenceStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> UserPresenceStatusChanged;

        static SystemSuspendStatus SystemSuspendStatus{ get; };
        static event Windows.Foundation.EventHandler<Object> SystemSuspendStatusChanged;
    };
}
```

## Details

### Mapping platform power registration to Windows App SDK

| PowerSettingRegisterNotification parameter | Description                                                  |
| ------------------------------------------ | ------------------------------------------------------------ |
| LPCGUID SettingGuid                        | A GUID that represents the specific power setting in which the app is interested. |
| DWORD Flags                                | This is either DEVICE_NOTIFY_SERVICE_HANDLE if the Recipient is a service, or DEVICE_NOTIFY_CALLBACK if the Recipient is a callback function. |
| HANDLE Recipient                           | A handle to the recipient of the notifications.              |
| PHPOWERNOTIFY RegistrationHandle           | Out parameter: a handle to the registration, which the app can later use to unregister for notifications, via the PowerSettingUnregisterNotification API. |

Windows App SDK AppLifecycle will not bring this function forward in its current form. Instead, AppLifecycle
will bring the functionality forward using the PowerManager pattern for the additional
PowerSettingRegisterNotification notifications:

-   Instead of a single register function that takes a GUID, the new API will expose multiple new
    events, mapping to each of the GUIDs that are brought forward. Not all GUIDs will be brought
    forward, as some of them are already covered by PowerManager events.
-   For Windows App SDK v1, the target consumers of the new APIs are scoped to apps not services - this
    eliminates the need for the Flags parameter.
-   The new API will adopt the PowerManager pattern for registering and unregistering callbacks,
    which eliminates the need for the register/unregister handles.

The notification pattern for PowerSettingRegisterNotification is to send additional data on the
updated state along with the notification. Conversely, the pattern for PowerManager is to send no
additional data, and the app is expected to query the current state via the enum-based PowerManager
properties instead. This means that along with the new events, the new API will also expose new
properties where appropriate.
