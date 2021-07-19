[CmdLetBinding()]
Param(
    [string]$BuildOutputDir,
    [string]$OverrideDir,
    [string]$PublishDir,
    [string]$NugetDir,
    [string]$Platform,
    [string]$Configuration,
    [switch]$PublishAppxFiles=$false
)

$FullBuildOutput = "$($BuildOutputDir)\$($Configuration)\$($Platform)"
$FullPublishDir = "$($PublishDir)\$($Configuration)\$($Platform)"

if (!(Test-Path $FullPublishDir)) { mkdir $FullPublishDir }


function PublishFile {
    Param($source, $destinationDir, [switch]$IfExists = $false)

    if ((-not $IfExists) -or (Test-Path $source))
    {
        Write-Host "Copy from '$source' to '$destinationDir'"
        if (-not (Test-Path $destinationDir))
        {
            $ignore = New-Item -ItemType Directory $destinationDir
        }
        Copy-Item -Recurse -Force $source $destinationDir
    }
    else
    {
        Write-Host "Not copying '$source' to $destinationDir because it did not exist"
    }
}

PublishFile $OverrideDir\DynamicDependency-Override.json $FullPublishDir\

PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.dll $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.lib $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.Windows.AppLifecycle.winmd $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsApp.PushNotifications.winmd $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.Windows.ApplicationModel.DynamicDependency.winmd $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\MsixDynamicDependency.h $FullPublishDir\Microsoft.WindowsAppSDK\
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\wil_msixdynamicdependency.h $FullPublishDir\Microsoft.WindowsAppSDK\
#
PublishFile $FullBuildOutput\DynamicDependency.DataStore\DynamicDependency.DataStore.exe $FullPublishDir\DynamicDependency.DataStore\
PublishFile $FullBuildOutput\DynamicDependency.DataStore.ProxyStub\DynamicDependency.DataStore.ProxyStub.dll $FullPublishDir\DynamicDependency.DataStore\
#
PublishFile $FullBuildOutput\PushNotificationsLongRunningTask\PushNotificationsLongRunningTask.exe $FullPublishDir\PushNotificationsLongRunningTask\
#
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager\DynamicDependencyLifetimeManager.exe $FullPublishDir\DynamicDependencyLifetimeManager\
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager.ProxyStub\DynamicDependencyLifetimeManager.ProxyStub.dll $FullPublishDir\DynamicDependencyLifetimeManager\
#
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.dll $FullPublishDir\Microsoft.WindowsAppSDK.Bootstrap\
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.lib $FullPublishDir\Microsoft.WindowsAppSDK.Bootstrap\
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\MddBootstrap.h $FullPublishDir\Microsoft.WindowsAppSDK.Bootstrap\

#PublishFile -IfExists $FullBuildOutput\WindowsAppSDK_DLL\Generic.xaml $FullPublishDir\Microsoft.WindowsAppSDK\
#PublishFile -IfExists $FullBuildOutput\Microsoft.WindowsAppSDK.Design\Microsoft.WindowsAppSDK.Design.dll $FullPublishDir\Microsoft.WindowsAppSDK.Design\
#PublishFile -IfExists $BuildOutputDir\$Configuration\AnyCPU\Microsoft.WindowsAppSDK.FrameworkPackagePRI\Microsoft.WindowsAppSDK.pri $FullPublishDir\Microsoft.WindowsAppSDK.FrameworkPackagePRI\

#UNDONE - PGO etc. later PR
# pgosweep and vcruntime are required to run pgo instrumented test run. They are placed from the
# cx test app instead of releasetest.dll since these are architecture specific and the ReleaseTest assembly is AnyCPU.
#PublishFile -IfExists $FullBuildOutput\NugetPackageTestAppCX\pgosweep.exe $FullPublishDir\Test\
#PublishFile -IfExists $FullBuildOutput\NugetPackageTestAppCX\vcruntime140.dll $FullPublishDir\Test\

PublishFile -IfExists $FullBuildOutput\FrameworkPackage\*.* $FullPublishDir\FrameworkPackage

if($PublishAppxFiles)
{
    $AppxPackagesDir = "$FullPublishDir\AppxPackages"

#    PublishFile -IfExists $FullBuildOutput\MUXControlsTestApp.TAEF\AppPackages\MUXControlsTestApp_Test\ $AppxPackagesDir
#    PublishFile -IfExists $FullBuildOutput\IXMPTestApp.TAEF\AppPackages\IXMPTestApp_Test\ $AppxPackagesDir
#    PublishFile -IfExists $FullBuildOutput\MUXControlsTestAppWPFPackage\AppPackages\MUXControlsTestAppWPFPackage_Test\ $AppxPackagesDir

#    PublishFile -IfExists $FullBuildOutput\NugetPackageTestApp\AppPackages\NugetPackageTestApp_Test\ $AppxPackagesDir
#    PublishFile -IfExists $FullBuildOutput\NugetPackageTestAppCX\AppPackages\NugetPackageTestAppCX_Test\ $AppxPackagesDir
#    PublishFile -IfExists $FullBuildOutput\AppThatUsesMUXIndirectly\AppPackages\AppThatUsesMUXIndirectly_Test\ $AppxPackagesDir
}

# Publish pdbs:
$symbolsOutputDir = "$($FullPublishDir)\Symbols\"
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.pdb $symbolsOutputDir
PublishFile $FullBuildOutput\DynamicDependency.DataStore\DynamicDependency.DataStore.pdb $symbolsOutputDir
PublishFile $FullBuildOutput\PushNotificationsLongRunningTask\PushNotificationsLongRunningTask.pdb $symbolsOutputDir
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager\DynamicDependencyLifetimeManager.pdb $symbolsOutputDir
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.pdb $symbolsOutputDir
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.pdb $symbolsOutputDir

# Copy files to Full Nuget package
#
# Includes (*.h)
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\MddBootstrap.h $NugetDir\include
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\MsixDynamicDependency.h $NugetDir\include
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\wil_msixdynamicdependency.h $NugetDir\include
#
# Libraries (*.lib)
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.lib $NugetDir\lib\win10-$Platform
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.lib $NugetDir\lib\win10-$Platform
#
# MSIX Framework package - DLLs
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.dll $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsAppSDK.pdb $NugetDir\runtimes\win10-$Platform\native
#
# MSIX Main package
PublishFile $FullBuildOutput\DynamicDependency.DataStore\DynamicDependency.DataStore.exe $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependency.DataStore\DynamicDependency.DataStore.pdb $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\PushNotificationsLongRunningTask\PushNotificationsLongRunningTask.exe $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\PushNotificationsLongRunningTask\PushNotificationsLongRunningTask.pdb $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependency.DataStore.ProxyStub\DynamicDependency.DataStore.ProxyStub.dll $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependency.DataStore.ProxyStub\DynamicDependency.DataStore.ProxyStub.pdb $NugetDir\runtimes\win10-$Platform\native
#
# MSIX DDLM package
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager\DynamicDependencyLifetimeManager.exe $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager\DynamicDependencyLifetimeManager.pdb $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager.ProxyStub\DynamicDependencyLifetimeManager.ProxyStub.dll $NugetDir\runtimes\win10-$Platform\native
PublishFile $FullBuildOutput\DynamicDependencyLifetimeManager.ProxyStub\DynamicDependencyLifetimeManager.ProxyStub.pdb $NugetDir\runtimes\win10-$Platform\native
#
# WinMD for UWP apps
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.Windows.AppLifecycle.winmd $NugetDir\lib\uap10.0
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsApp.PushNotifications.winmd $NugetDir\lib\uap10.0
#
# Native (not managed, no MSIX)
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.dll $NugetDir\runtimes\lib\native\$Platform
PublishFile $FullBuildOutput\WindowsAppSDK_BootstrapDLL\Microsoft.WindowsAppSDK.Bootstrap.pdb $NugetDir\runtimes\lib\native\$Platform
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.Windows.AppLifecycle.winmd $NugetDir\lib\native
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.Windows.ApplicationModel.DynamicDependency.winmd $NugetDir\lib\native
PublishFile $FullBuildOutput\WindowsAppSDK_DLL\Microsoft.WindowsApp.PushNotifications.winmd $NugetDir\lib\native
#
# C#/WinRT Projections
#
# Dynamic Dependency build overrides
PublishFile $OverrideDir\DynamicDependency-Override.json $NugetDir\runtimes\win10-$Platform\native
