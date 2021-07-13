[CmdLetBinding()]
Param(
    [string]$Platform = "x86",
    [string]$OutputDirectory = "$PSScriptRoot\..\BuildOutput\Debug\x86\TestDependencies"
)

$ProgressPreference = "SilentlyContinue"
$ErrorActionPreference = "Stop"

$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
$outputPath = Join-Path $OutputDirectory "dotnet-windowsdesktop-runtime-installer.exe"

if(!(Test-Path $OutputDirectory))
{
    New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
}

# Find direct download links at https://dotnet.microsoft.com/download/dotnet/5.0
$downloadurlx86 = "https://download.visualstudio.microsoft.com/download/pr/c089205d-4f58-4f8d-ad84-c92eaf2f3411/5cd3f9b3bd089c09df14dbbfb64124a4/windowsdesktop-runtime-5.0.5-win-x86.exe"
$downloadurlx64 = "https://download.visualstudio.microsoft.com/download/pr/c1ef0b3f-9663-4fc5-85eb-4a9cadacdb87/52b890f91e6bd4350d29d2482038df1c/windowsdesktop-runtime-5.0.5-win-x64.exe"

if($Platform -eq "x86")
{
    $downloadurl = $downloadurlx86
}
elseif($Platform -eq "x64")
{
    $downloadurl = $downloadurlx64
}
else
{
    Write-Host "dotnet Desktop runtime not available for arch $Platform"
    return
}


if(!(Test-Path $outputPath))
{
    Write-Host "Downloading $downloadurl to $outputPath"
    Invoke-WebRequest $downloadurl -OutFile $outputPath
}
else
{
    Write-Host "The file '$outputPath' already exists. Skipping download."
}
