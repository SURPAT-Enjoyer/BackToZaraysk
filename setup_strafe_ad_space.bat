@echo off
echo ========================================
echo    НАСТРОЙКА СТРЕЙФА A/D + SPACE
echo ========================================
echo.

echo Запуск скрипта настройки...
echo.

REM Запускаем скрипт настройки
python Content/Python/SetupStrafeADSpace.py

echo.
echo ========================================
echo    НАСТРОЙКА ЗАВЕРШЕНА
echo ========================================
echo.

echo Следующие шаги:
echo 1. Создайте Input Actions вручную в Content Browser:
echo    - IA_A (Value Type: Digital bool)
echo    - IA_D (Value Type: Digital bool)
echo    - IA_Space (Value Type: Digital bool)
echo 2. Откройте BP_Player в редакторе
echo 3. Проверьте Event Graph на наличие Input Events:
echo    - IA_A Started/Completed → Handle A Input
echo    - IA_D Started/Completed → Handle D Input
echo    - IA_Space Started → Handle Space Input
echo 4. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function
echo 5. Соберите проект: .\build_ue53.bat
echo 6. Протестируйте:
echo    - A + Space → Стрейф влево
echo    - D + Space → Стрейф вправо
echo.

echo Управление:
echo - A + Space → Стрейф влево (одно нажатие)
echo - D + Space → Стрейф вправо (одно нажатие)
echo.

echo Дополнительная помощь:
echo - Ручная настройка: MANUAL_STRAFE_AD_SPACE_SETUP.md
echo.

pause


