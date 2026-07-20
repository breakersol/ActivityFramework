param(
    [ValidateSet("configure", "build", "test")]
    [string]$Action = "build",

    [ValidateSet("Debug", "Release")]
    [string]$Config = "Debug"
)

$ErrorActionPreference = "Stop"

$workspace = Resolve-Path (Join-Path $PSScriptRoot "..\..")
$buildDir = Join-Path $workspace "build\windows-msvc\$($Config.ToLowerInvariant())"
$testExe = Join-Path $buildDir "Test\ActivityFrameworkTest.exe"

$vswhere = Get-Command "vswhere.exe" -ErrorAction SilentlyContinue
if (-not $vswhere) {
    $candidate = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $candidate) {
        $vswherePath = $candidate
    }
} else {
    $vswherePath = $vswhere.Source
}

if (-not $vswherePath) {
    throw "Could not find vswhere.exe. Install Visual Studio Build Tools or add vswhere.exe to PATH."
}

$vsInstall = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsInstall) {
    throw "Could not find a Visual Studio installation with the MSVC x64 tools."
}

$devCmd = Join-Path $vsInstall "Common7\Tools\VsDevCmd.bat"
if (-not (Test-Path $devCmd)) {
    throw "Could not find VsDevCmd.bat under '$vsInstall'."
}

$cmake = Get-Command "cmake.exe" -ErrorAction SilentlyContinue
if ($cmake) {
    $cmakePath = $cmake.Source
} else {
    $candidate = Join-Path $vsInstall "Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    if (Test-Path $candidate) {
        $cmakePath = $candidate
    }
}

if (-not $cmakePath) {
    throw "Could not find cmake.exe on PATH or under the Visual Studio installation."
}

$ninja = Get-Command "ninja.exe" -ErrorAction SilentlyContinue
if ($ninja) {
    $ninjaPath = $ninja.Source
} else {
    $candidate = Join-Path $vsInstall "Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    if (Test-Path $candidate) {
        $ninjaPath = $candidate
    }
}

if (-not $ninjaPath) {
    throw "Could not find ninja.exe on PATH or under the Visual Studio installation."
}

function Invoke-InVsDevCmd {
    param([string]$CommandLine)

    $cmd = "call `"$devCmd`" -arch=x64 -host_arch=x64 >nul && $CommandLine"
    & cmd.exe /d /s /c "`"$cmd`""
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

switch ($Action) {
    "configure" {
        Invoke-InVsDevCmd "`"$cmakePath`" -S `"$workspace`" -B `"$buildDir`" -G Ninja -D CMAKE_MAKE_PROGRAM=`"$ninjaPath`" -D CMAKE_CXX_COMPILER=cl.exe -D CMAKE_BUILD_TYPE=$Config -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D ACTIVITYFRAMEWORK_BUILD_TESTS=ON"
    }
    "build" {
        Invoke-InVsDevCmd "`"$cmakePath`" --build `"$buildDir`""
    }
    "test" {
        if (-not (Test-Path $testExe)) {
            throw "Test executable does not exist: $testExe"
        }
        Invoke-InVsDevCmd "`"$testExe`""
    }
}
