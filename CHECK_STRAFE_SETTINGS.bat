@echo off
echo ============================================
echo   ПРОВЕРКА НАСТРОЕК СИСТЕМЫ СТРЕЙФА
echo ============================================
echo.
echo Запустите этот скрипт в консоли Python редактора:
echo.
echo 1. Откройте редактор Unreal Engine
echo 2. Нажмите Ctrl+Shift+P (консоль Python)
echo 3. Выполните команду:
echo.
echo    exec(open('Content/Python/CheckStrafeSettings.py').read())
echo.
echo ============================================
echo.
echo Также проверьте вручную:
echo.
echo 1. Edit ^> Project Settings ^> Input ^> Bindings
echo    - StrafeLeft (A)
echo    - StrafeRight (D)
echo    - StrafeSpace (Space)
echo.
echo 2. BP_Player ^> Components
echo    - StrafeComponent
echo    - InventoryComponent
echo    - ObstacleClimbingComponent
echo.
echo 3. ABP_Player ^> Variables
echo    - LeftFootEffectorLocation (Vector)
echo    - RightFootEffectorLocation (Vector)
echo    - LeftFootIKAlpha (Float)
echo    - RightFootIKAlpha (Float)
echo.
echo 4. GameMode Settings
echo    - Default Pawn Class: BP_Player
echo    - Player Controller Class: BP_PlayerController
echo.
echo ============================================
pause



