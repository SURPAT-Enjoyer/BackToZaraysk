@echo off
setlocal
set PROJECT_DIR=%~dp0
"C:\Program Files\Epic Games\UE_5.3\Engine\Build\BatchFiles\Build.bat" UnrealEditor Win64 Development -Project="%PROJECT_DIR%BackToZaraysk.uproject" -WaitMutex -NoHotReloadFromIDE
endlocal


