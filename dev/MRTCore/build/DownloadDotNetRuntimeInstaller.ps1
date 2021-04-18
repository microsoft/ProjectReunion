$ProgressPreference = "SilentlyContinue"

$arch = $env:_BuildArch;
$buildFavor = "$($env:_BuildArch)$($env:_BuildType)"
$binRoot = $env:BinRoot

$outputDir = Join-Path (Join-Path $binRoot $buildFavor) "TestDependencies"
$outputPath = Join-Path $outputDir "dotnet-windowsdesktop-runtime-installer.exe"

if(!(Test-Path $outputDir))
{
    New-Item -ItemType Directory -Path $outputDir | Out-Null
}

# Find direct download links at https://dotnet.microsoft.com/download/dotnet/5.0
$downloadurlx86 = "https://download.visualstudio.microsoft.com/download/pr/c089205d-4f58-4f8d-ad84-c92eaf2f3411/5cd3f9b3bd089c09df14dbbfb64124a4/windowsdesktop-runtime-5.0.5-win-x86.exe"
$downloadurlx64 = "https://download.visualstudio.microsoft.com/download/pr/c1ef0b3f-9663-4fc5-85eb-4a9cadacdb87/52b890f91e6bd4350d29d2482038df1c/windowsdesktop-runtime-5.0.5-win-x64.exe"

if($arch -eq "x86")
{
    $downloadurl = $downloadurlx86
}
elseif($arch -eq "amd64")
{
    $downloadurl = $downloadurlx64
}
else
{
    Write-Host "dotnet Desktop runtime not available for arch $arch"
    return
}


if(!(Test-Path $outputPath))
{
    Write-Host "Downloading $downloadurl to $outputPath"
    Invoke-WebRequest $downloadurl -OutFile $outputPath
}