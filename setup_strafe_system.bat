@echo off
echo ========================================
echo НАСТРОЙКА СИСТЕМЫ СТРЕЙФА
echo ========================================

echo Создание папок...

:: Создаем папку для Input Actions
if not exist "Content\BackToZaraysk\Core\Input\Actions\Strafe" (
    mkdir "Content\BackToZaraysk\Core\Input\Actions\Strafe"
    echo ✓ Папка Strafe создана
)

:: Создаем папку для Curves
if not exist "Content\BackToZaraysk\Core\Data\Curves" (
    mkdir "Content\BackToZaraysk\Core\Data\Curves"
    echo ✓ Папка Curves создана
)

echo.
echo ========================================
echo ИНСТРУКЦИИ ДЛЯ UE5:
echo ========================================
echo.
echo 1. СОЗДАНИЕ INPUT ACTIONS:
echo    Откройте UE5 Editor
echo    Создайте в папке Strafe:
echo    - IA_A (Boolean Input Action)
echo    - IA_D (Boolean Input Action)  
echo    - IA_Space (Boolean Input Action)
echo.
echo 2. НАСТРОЙКА IMC_DEFAULT:
echo    Откройте IMC_Default
echo    Добавьте маппинги:
echo    - IA_A → Key: A
echo    - IA_D → Key: D
echo    - IA_Space → Key: Space Bar
echo.
echo 3. СОЗДАНИЕ КРИВОЙ:
echo    Создайте Curve Float: CF_StrafeMovement
echo    В папке Curves
echo    Настройте точки: (0,0) → (0.5,1) → (1,0)
echo.
echo 4. НАСТРОЙКА BP_PLAYER:
echo    Откройте BP_Player
echo    В Input Events:
echo    - IA_A (Pressed/Released) → Handle A Input
echo    - IA_D (Pressed/Released) → Handle D Input
echo    - IA_Space (Pressed) → Handle Space Input
echo.
echo    В функциях Handle A/D/Space Input:
echo    - Input: bPressed (Boolean)
echo    - Call Parent Function: Handle A/D/Space Input
echo.
echo 5. ИМПОРТ АНИМАЦИИ:
echo    Найдите strafing.fbx в папке Strafing
echo    Импортируйте с скелетом SK_Mannequin
echo.
echo ========================================
echo ГОТОВО! Следуйте инструкциям выше.
echo ========================================

pause


