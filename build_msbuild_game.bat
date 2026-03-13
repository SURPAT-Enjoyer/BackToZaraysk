@echo off
setlocal

rem Build only the game project via MSBuild (avoids C# engine tools).
set PROJECT_DIR=%~dp0
set MSBUILD_EXE="C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

if not exist %MSBUILD_EXE% (
    echo MSBuild not found at %MSBUILD_EXE%
    echo Please check your Visual Studio 2022 Community installation path.
    exit /b 1
)

rem Use the generated vcxproj instead of the solution to avoid AutomationTool/BuildGraph C# errors.
rem Clean + Build Development_Editor for x64 so game modules are fully rebuilt.
%MSBUILD_EXE% "%PROJECT_DIR%Intermediate\ProjectFiles\BackToZaraysk.vcxproj" /m ^
    /t:Clean;Build ^
    /p:Configuration=Development_Editor ^
    /p:Platform=x64 ^
    /p:SolutionDir=%PROJECT_DIR%

endlocal

