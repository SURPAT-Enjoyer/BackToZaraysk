@echo off
echo ========================================
echo СОЗДАНИЕ INPUT ACTIONS ДЛЯ СТРЕЙФА
echo ========================================

echo Создание Input Actions...

:: Создаем папку для Input Actions
if not exist "Content\BackToZaraysk\Core\Input\Actions\Strafe" (
    mkdir "Content\BackToZaraysk\Core\Input\Actions\Strafe"
    echo Папка Strafe создана
)

:: Создаем папку для Curves
if not exist "Content\BackToZaraysk\Core\Data\Curves" (
    mkdir "Content\BackToZaraysk\Core\Data\Curves"
    echo Папка Curves создана
)

echo.
echo ========================================
echo РУЧНАЯ НАСТРОЙКА В UE5:
echo ========================================
echo.
echo 1. Откройте UE5 Editor
echo 2. Создайте Input Actions:
echo    - IA_A (Boolean) в папке Strafe
echo    - IA_D (Boolean) в папке Strafe  
echo    - IA_Space (Boolean) в папке Strafe
echo.
echo 3. Создайте Curve Float:
echo    - CF_StrafeMovement в папке Curves
echo.
echo 4. Настройте IMC_Default:
echo    - Добавьте маппинги для A, D, Space
echo.
echo 5. Настройте BP_Player:
echo    - Подключите Input Events к функциям
echo    - Назначьте CF_StrafeMovement в Strafe Curve
echo.
echo 6. Импортируйте strafing.fbx
echo.
echo ========================================
echo ГОТОВО! Следуйте инструкциям выше.
echo ========================================

pause


