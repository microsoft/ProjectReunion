Write-Host "TestPass-OneTimeMachineSetup.ps1"

# This script is run once on a test machine.
# It is a good place to add any logic to install test dependencies, etc.

# This script is called by TestPass-OneTimeMachineSetupCore.ps1 which is a part of the WinUI.Helix package.

$packagesToFind = @(
    "Microsoft.VCLibs.*.appx",
    "Microsoft.NET.CoreRuntime.*.appx",
    "Microsoft.NET.CoreFramework.*.appx",
    "Microsoft.NET.Native.Framework.*.appx",
    "Microsoft.NET.Native.Runtime.*.appx"
)

foreach($pattern in $packagesToFind)
{
    foreach($package in (Get-ChildItem $pattern))
    {
        Write-Host "Installing $package"
        Add-AppxPackage $package -ErrorVariable appxerror -ErrorAction SilentlyContinue 
        if($appxerror)
        {
            foreach($error in $appxerror)
            {
                # In the case where the package does not install becasuse a higher version is already installed
                # we don't want to print an error message, since that is just noise. Filter out such errors.
                if(($error.Exception.Message -match "0x80073D06") -or ($error.Exception.Message -match "0x80073CFB"))
                {
                    Write-Host "The same or higher version of this package is already installed."
                }
                else
                {
                    Write-Error $error
                }
            }
        }
    }
}

# Install any .cer files that were included in the payload:
foreach($cerFile in (Get-ChildItem "*.cer"))
{
    Write-Host "Adding cert '$cerFile'"
    certutil -addstore TrustedPeople "$cerFile"
}

if(Test-Path .\dotnet-windowsdesktop-runtime-installer.exe)
{
    Write-Host "Install dotnet runtime"
    .\dotnet-windowsdesktop-runtime-installer.exe /quiet /install /norestart /log dotnetinstalllog.txt |Out-Null
    Get-Content .\dotnetinstalllog.txt
    if($env:HELIX_WORKITEM_UPLOAD_ROOT)
    {
        Copy-Item .\dotnetinstalllog.txt $env:HELIX_WORKITEM_UPLOAD_ROOT -Force
    }
}


# If we set the registry from a 32-bit process on a 64-bit machine, we will set the "virtualized" syswow registry. 
# For crash dump collection we always want to set the "native" registry, so we make sure to invoke the native cmd.exe
$nativeCmdPath = "$env:SystemRoot\system32\cmd.exe"
if([Environment]::Is64BitOperatingSystem -and ![Environment]::Is64BitProcess)
{
    # The "sysnative" path is a 'magic' path that allows a 32-bit process to invoke the native 64-bit cmd.exe.
    $nativeCmdPath = "$env:SystemRoot\sysnative\cmd.exe"
}

$dumpFolder = $env:HELIX_DUMP_FOLDER
if(!$dumpFolder)
{
    $dumpFolder = "C:\dumps"
}

function Enable-CrashDumpsForProcesses {
    Param([string[]]$namesOfProcessesForDumpCollection)

    foreach($procName in $namesOfProcessesForDumpCollection )
    {
        Write-Host "Enabling local crash dumps for $procName"
        & $nativeCmdPath /c reg add "HKLM\Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\$procName" /v DumpFolder /t REG_EXPAND_SZ /d $dumpFolder /f
        & $nativeCmdPath /c reg add "HKLM\Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\$procName" /v DumpType /t REG_DWORD /d 2 /f
        & $nativeCmdPath /c reg add "HKLM\Software\Microsoft\Windows\Windows Error Reporting\LocalDumps\$procName" /v DumpCount /t REG_DWORD /d 3 /f
    }
}

# Note: te.exe and te.processhost.exe are already enabled for dump collection by TestPass-OneTimeMachineSetupCore.ps1

# Enable dump collection for any additional apps/processes below:
$namesOfProcessesForDumpCollection = @(
)

Enable-CrashDumpsForProcesses $namesOfProcessesForDumpCollection
