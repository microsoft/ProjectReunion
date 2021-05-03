# AppLifecycle - Single/Multi-instancing

Reunion introduces a new AppLifecycle component that provides a core set of functionality related to
app activation and lifecycle. Much of this functionality is directly equivalent - or very similar -
to existing functionality within the platform. However, there is value in abstracting this
functionality so that all app types can use it, delivering it out-of-band from the OS, and making it
available as open-source. The totality of AppLifecycle features in v1 is as follows (additional
features may be added in later releases):

-   Rich activation objects
-   Selective single/multi-instancing
-   System state and power notifications.

This spec addresses the Single/Multi-instancing APIs in the component.

## Background

Traditional Win32 apps are multi-instanced by default, and they have to do work to achieve
single-instancing if they need that behavior. For example, If you open a docx file in Word, and then
attempt to open the same file in another instance of Word, the app detects that you're doing this,
redirects you to the existing instance, and terminates the new instance. There is no
platform-defined formal mechanism or pattern for achieving single-instancing: apps tend to use named
mutexes. If an app's instances need to communicate with each other - even if only to gather
information about each other to make a decision about single-instancing, they often use named pipes.

Conversely, UWP apps are single-instanced by default. Windows 10 1803 introduced support for UWP
apps to opt in to be multi-instanced. This also allowed for the possibility that even though an app
has chosen to be multi-instanced, there might be some scenario where it wants to redirect a fresh
activation request to an existing instance instead.

## Differences between Windows and Reunion Instancing

The Reunion behavior is based on the existing
[Windows.ApplicationModel.AppInstance](https://docs.microsoft.com/en-us/uwp/api/windows.applicationmodel.appinstance)
class, but with some key differences:

### AppInstance class

-   **Platform**: The AppInstance class is focused purely on instance redirection scenarios.
-   **Reunion**: The AppInstance class includes instance redirection scenarios, and is additionally
    intended to form the basis of other instance-related functionality in later releases.

### List of Instances

-   **Platform**: GetInstances returns only the instances that the app explicitly registered for
    potential redirection.
-   **Reunion**: GetInstances returns all running instances of the app, including the current instance. 
    There is no guaranteed ordering of instances in the list. In the scenario where the user has multiple
    versions of the same app running, separate lists are maintained for each version. In the scenario
    where there are multiple user sessions active, separate lists are maintained per user also.

### Registering Keys

Each instance of the app can register a key, and this key is based on arbitrary app-defined logic.
The app is allowed to set its key as many times as it likes; and each time we will overwrite the
data for that instance in the cache. A given key can only be registered for one instance. If an app
attempts to register a key that has already been registered for a different instance, the API will
return the registered instance, and the app can check whether it is the current instance.

-   **Platform**: An instance must register a key in order to be included in the list returned from
    GetInstances.
-   **Reunion**: Registering a key is decoupled from the list of instances. An instance does not
    need to register a key in order to be included in the list.

### Instance Redirection Targets

An instance of an app can take part in "activation redirection". This means that this instance of
the process can handle multiple Activated events, and so it could be a target for selective
activation redirection. When an activation is redirected to another instance, it arrives at that
instance's Activated callback, thus hooking into the normal activation workflow that apps already
expect. An app can have multiple instances available as redirection targets.

-   **Platform**: Only instances that have registered a key can be a target for redirection.
-   **Reunion**: Any instance can be a redirection target, whether or not it has a registered key.

### Post-Redirection Behavior

-   **Platform**: In the UWP implementation, redirection is a terminal operation: after calling the 
    method to redirect, there is nothing the app can do - regardless of whether the redirect 
    succeeded or failed. The platform therefore explicitly terminates an instance that has chosen
    to redirect. If redirection fails, the activation request fails. The UX is the same as an 
    activation failure today.

-   **Reunion**: The Reunion implementation is more flexible: redirection is not a terminal operation. 
    Partly this is because it is a lot more problematic to terminate a traditional unpackaged app than
    a UWP app. Partly it is because the app may wish to redirect a given activation request but then 
    to continue activating the current instance anyway. This supports the case where an activation 
    request is redirected to an instance that was already running: that instance could already be 
    doing work, and may choose to  redirect the new request elsewhere. Even in the case where an 
    instance was not already running prior to receiving the activation request, it might still want 
    the option to choose for itself what happens next.

    An activation request can be redirected multiple times - this leaves it open for the app to decide
    what makes sense for that app. That is, instance A could redirect to instance B, which in turn 
    could redirect to instance C, and so on. This also allows for a circular redirection - and again, 
    it is left open for the app to resolve circular redirections in whatever way it chooses. For 
    example, to  resolve a circular redirection, an app instance could take note of the activation 
    payload and match it against future activation requests. Or it could modify the payload when it 
    redirects a request,  such that it can easily determine that this is a request it has already seen.

### Unregistering

At any time, an instance of the app can unregister its key. At any time, an app instance can
re-register itself (again with any arbitrary key, not necessarily the same key each time), even
after it has unregistered itself.

-   **Platform**: When an instance unregisters itself, it is no longer available for activation
    redirection. An app that has unregistered itself is not included in the list of instance that is
    made available to the app.
-   **Reunion**: Unregistering a key does not prevent an instance from being a target for
    redirection. Nor does it exclude the instance from the list.

### Activation Events

In order to handle reactivation, the app can register for an Activated event.

-   **Platform**: The event passes a `Windows.ApplicationModel.Activation.IActivatedEventArgs` to
    the app.
-   **Reunion**: The event passes a `Microsoft.Windows.AppLifecycle.AppActivationArguments` instance to
    the app, which contains one of the `-ActivatedEventArgs` instances.

## Examples

### Redirection logic based on activation kind

In this example, the app calls GetActivatedEventArgs, and if it has been activated as a result of a
file-type association, it checks first to see if there is any other instance already running that
has this file open. The app can tell whether the instance has the file open because it includes the
filename as part of the key that it registers with the system.

If it finds another instance with the file open, the app redirects this activation to that instance,
and then does nothing. After redirecting the activation to the other instance, the system will then
terminate this instance.

If it doesn't find another instance with the file open, it registers itself as the instance that has
this file open, and continues with normal app initialization (creating its main window, starting its
message pump, and so on).

```c++
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize COM.
    winrt::init_apartment();

    // First, we'll get our rich activation event args.
    AppActivationArguments activationArgs =
        AppInstance::GetCurrent().GetActivatedEventArgs();

    // An app might want to set itself up for possible redirection in
    // the case where it opens files - for example, to prevent multiple
    // instances from working on the same file.
    ExtendedActivationKind kind = activationArgs.Kind;
    if (kind == ExtendedActivationKind::File)
    {
        auto fileArgs = activationArgs.Data.as<FileActivatedEventArgs>();
        IStorageItem file = fileArgs.Files().GetAt(0);

        // Let's try to register this instance for this file.
        AppInstance instance =
            AppInstance::FindOrRegisterForKey(file.Name());
        if (instance.IsCurrent)
        {
            // If we successfully registered this instance, we can now just
            // go ahead and do normal initialization.
            RegisterClassAndStartMessagePump(hInstance, nCmdShow);
        }
        else
        {
            // Some other instance has already registered for this file,
            // so we'll redirect this activation to that instance instead.
            instance.RedirectActivationTo(activationArgs);
        }
    }
    return 1;
}
```

### Arbitrary redirection

In the previous example, the app only redirects for file activations. In the simplest case, it might
just redirect on every activation if there is already an instance running. The logic for deciding
when to redirect is entirely up to the app - it might be dependent on activation kind, or on some
other logic.

In the following example, the app might have checked some specific activation kind first, or it
might not. Regardless, the app then checks all other running instances to see if any of them has
registered to be a "reusable" instance. What constitutes "reusable" is also entirely up to the app
to decide. It might be any instance that doesn't have a file open, or doesn't have an editing
session active, or has some specific database connection, and so on. If it finds a reusable
instance, it redirects activation to that instance; if not, it goes ahead with this activation and
normal app initialization. In this example, the app signals that an instance is reusable by
specifying the "reusable" text as the key it registers with the system. Of course, exactly what text
to use for this purpose is entirely up to the app to decide. For more complex scenarios, if the app
wanted to allow for multiple reusable instances, it could specify "reusable" plus some unique
identifier for each instance, and so on.

```c++
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize COM.
    winrt::init_apartment();

    AppActivationArguments activationArgs =
        AppInstance::GetCurrent().GetActivatedEventArgs();

    // As above, check for any specific activation kind we care about.
    ExtendedActivationKind kind = activationArgs.Kind;
    if (kind == ExtendedActivationKind::File)
    {
        // etc... as in previous scenario.
    }
    else
    {
        // For other activation kinds, we'll trawl all instances to see if
        // any are suitable for redirecting this request. First, get a list
        // of all running instances of this app.
        auto instances = AppInstance::GetInstances();

        // In the simple case, we'll redirect to any other instance.
        AppInstance instance = instances.GetAt(0);

        // If the app re-registers re-usable instances, we can filter for these instead.
        // In this example, the app uses the string "REUSABLE" to indicate to itself
        // that it can redirect to a particular instance.
        bool isFound = false;
        for (AppInstance instance : instances)
        {
            if (instance.Key == L"REUSABLE")
            {
                isFound = true;
                instance.RedirectActivationTo(activationArgs);
                break;
            }
        }
        if (!isFound)
        {
            // We'll register this as a reusable instance, and then
            // go ahead and do normal initialization.
            winrt::hstring szKey = L"REUSABLE";
            AppInstance::FindOrRegisterForKey(szKey);
            RegisterClassAndStartMessagePump(hInstance, nCmdShow);
        }
    }
    return 1;
}
```

### Handling redirection

The first 2 scenarios illustrate how an app would redirect to another running instance. This
scenario shows how an app would handle the case where it is already running, and then a fresh
activation is redirected to this instance. The AppInstance class will expose an Activated event, for
which the app can register. In its Activated event handler, the app would examine the type of
activation, retrieve the rich ActivationArguments object, and then do whatever app-specific work it
wishes to do based on those arguments.

```c++
int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize COM.
    winrt::init_apartment();

    // First, hook up the Activated event, to allow for this instance of the app
    // getting reactivated as a result of multi-instance redirection.
    Microsoft::ProjectReunion::AppInstance::GetCurrent().Activated([](
        AppActivationArguments const& args)
        { OnActivated(args); });

    //...etc - the rest of WinMain as normal.
}

void OnActivated(AppActivationArguments const& args)
{
    ExtendedActivationKind kind = args.Kind;
    if (kind == ExtendedActivationKind::Launch)
    {
        auto launchArgs = args.Data().as<LaunchActivatedEventArgs>();
        DoSomethingWithLaunchArgs(launchArgs.Arguments());
    }
    else if (kind == ExtendedActivationKind::File)
    {
        auto fileArgs = args.Data().as<FileActivatedEventArgs>();
        DoSomethingWithFileArgs(fileArgs.Files());
    }
    else if (kind == ExtendedActivationKind::Protocol)
    {
        auto protocolArgs = args.Data().as<ProtocolActivatedEventArgs>();
        DoSomethingWithProtocolArgs(protocolArgs.Uri());
    }
    else if (kind == ExtendedActivationKind::StartupTask)
    {
        auto startupArgs = args.Data().as<StartupTaskActivatedEventArgs>();
        DoSomethingWithStartupArgs(startupArgs.TaskId());
    }
}
```

### Redirection orchestration

We can also imagine a more advanced scenario, where the app registers an instance for redirection,
but when it receives a redirected activation, it may apply further app logic to decide whether to
handle this new activation in the current instance or redirect again, effectively chaining
redirections.

```c++
void OnActivated(AppActivationArguments const& args)
{
    const ExtendedActivationKind kind = args.Kind;

    // For example, we might want to redirect protocol activations.
    if (kind == ExtendedActivationKind::Protocol)
    {
        auto protocolArgs = args.Data().as<ProtocolActivatedEventArgs>();
        Uri uri = protocolArgs.Uri();

        // We'll try to find the instance that handles protocol activations.
        // If there isn't one, then this instance will take over that duty.
        auto instance = AppInstance::FindOrRegisterForKey(uri.AbsoluteUri());
        if (!instance.IsCurrent)
        {
            instance.RedirectActivationTo(args);
        }
        else
        {
            DoSomethingWithProtocolArgs(uri);
        }
    }
    else
    {
        // In this example, this instance of the app handles all other
        // activation kinds.
        DoSomethingWithNewActivationArgs(args);
    }
}
```

In the existing UWP API, RedirectActivationTo does not take any parameters. However, in the UWP
case, the activation contract is tightly controlled, and we could ensure that the arguments passed
on to the app were the arguments for the redirected instance. In bringing this feature to Win32, we
can't control activation as tightly. Also, the ActivationArguments object returned by
GetActivatedEventArgs is not the same object as the IActivatedEventArgs object that is passed in to
the Activated event callback. The new API gives the app the opportunity to modify or replace the
payload to be passed to the target instance.

### Unregister for redirection

Just as an app can define its own logic for how and when it registers an instance for redirection,
so too will it be able to define its own logic for when a given instance no longer wants to be a
target for redirection.

Following the previous example of an app that supports filetype activation, and redirects to the
instance that already has the selected file open - when that file is closed, the app could
unregister itself.

```c++
void CALLBACK OnFileClosed(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    AppInstance::GetCurrent().UnregisterKey();
}
```

The app is free to choose when to unregister itself. When an instance terminates, it is removed
from the list that the API maintains of running instances. However, it is possible that a
redirection could be in-flight just at the point where an app instances terminates. To
mitigate this, an app could choose to unregister itself just prior to termination (for example,
in its WM_CLOSE handler).

### Instance information

The AppInstance class is intended to represent an instance of the app. In the initial release, the
API is focused on instance redirection, and therefore exposes methods and properties relevant to
that operation. However, the intention is that the class will expand in later releases to include
other methods and properties that would be relevant for an app instance. In the first release, we
also include the process ID, which the app may use for any purpose it sees fit, including telemetry
or debugging.

```c++
void DumpExistingInstances()
{
    for (AppInstance const& instance : AppInstance::GetInstances())
    {
        std::wostringstream sStream;
        sStream << L"Instance: ProcessId = " << instance.ProcessId
            << L", Key = " << instance.Key().c_str() << std::endl;
        ::OutputDebugString(sStream.str().c_str());
    }
}
```

## API Details

This API extends the [AppLifecycle AppInstance](../Activation/AppLifecycle%20Activation.md) type
with some additional methods for activation redirection. Specifically, `AppInstance` is expanded
to include:

```idl
namespace Microsoft.Windows.AppLifecycle
{
    runtimeclass AppInstance
    {
        // Existing activation functionality like GetCurrent and GetActivatedEventArgs

        static event EventHandler<AppActivationArguments> Activated;

        static Windows.Foundation.IVector<AppInstance> GetInstances();

        static AppInstance FindOrRegisterForKey(String key);
        void UnregisterKey();

        void RedirectActivationTo(AppActivationArguments args);

        String Key { get; };
        bool IsCurrent { get; };
        UInt32 ProcessId { get; };
    }
}
```

**Activated** events are raised when using single/multi-instancing, or when redirecting an
activation from another instance to this app.

**GetInstances** returns a collection of all running instances of the app.

> Note: the existing
> [AppInstance.GetInstances](https://docs.microsoft.com/en-us/uwp/api/windows.applicationmodel.appinstance.getinstances)
> only returns instances that have explicitly registered for multi-instance redirection. However,
> the new AppInstance Reunion class will provide an API surface for all manner of
> app-instance-related behaviors, not restricted to instance redirection. For this reason, Reunion
> will maintain a list of all running instances and will not require explicit registration by the
> app.

**FindOrRegisterForKey** enables an app to register an app-defined key for the current instance, or
if another instance has already registered that key, then return that other instance instead. This
is similar to the platform
[AppInstance.FindOrRegisterInstanceForKey](https://docs.microsoft.com/en-us/uwp/api/windows.applicationmodel.appinstance.findorregisterinstanceforkey)
except that that implementation is specific to instance redirection, whereas the Reunion design
allows for the app to register a key for any reason.

**UnregisterKey** unregisters the key for this instance. The existing platform behavior is
specific to instance redirection, and unregistering the key unregisters that instance for
redirection (and removes it from the platform's collection of registered instances). In the Reunion
design, unregistering a key simply removes the key for this instance; it does not have any effect on
instance redirection, nor does it remove this instance from the collection that Reunion is
maintaining of all running instances.

**RedirectActivationTo** enables an instance of the app to redirect the current activation request to another
instance. This is very similar to the existing platform
[AppInstance.RedirectActivationTo](https://docs.microsoft.com/en-us/uwp/api/windows.applicationmodel.appinstance.redirectactivationto)
method, except that the Reunion implementation allows the app to pass an ActivationArguments
payload, thus opening the scope to allow the app to modify or replace the activation arguments that
the target instance will receive.

**Key** an app-defined string value that can be used to identify an instance for redirection
purposes.

**IsCurrent** indicates whether this instance object represents the current instance of the app or a
different instance.

**ProcessId** the process ID of the instance.
