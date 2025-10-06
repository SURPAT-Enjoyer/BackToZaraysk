@echo off
echo ========================================
echo    НАСТРОЙКА СТРЕЙФА SHIFT + A/D
echo    БЕЗ КОНФЛИКТОВ С ПРЫЖКОМ!
echo ========================================
echo.

echo Запуск скрипта настройки...
echo.

REM Запускаем скрипт настройки
python Content/Python/SetupStrafeShiftAD.py

echo.
echo ========================================
echo    НАСТРОЙКА ЗАВЕРШЕНА
echo ========================================
echo.

echo Следующие шаги:
echo 1. Создайте Input Actions вручную в Content Browser:
echo    - IA_A (Value Type: Digital bool)
echo    - IA_D (Value Type: Digital bool)
echo    - IA_Shift (Value Type: Digital bool)
echo 2. Откройте BP_Player в редакторе
echo 3. Проверьте Event Graph на наличие Input Events:
echo    - IA_A Started/Completed → Handle A Input
echo    - IA_D Started/Completed → Handle D Input
echo    - IA_Shift Started/Completed → Handle Shift Input
echo 4. Убедитесь, что функции Handle A/D/Shift Input вызывают Parent Function
echo 5. Соберите проект: .\build_ue53.bat
echo 6. Протестируйте:
echo    - Shift + A → Стрейф влево
echo    - Shift + D → Стрейф вправо
echo    - Space → Прыжок (должен работать без конфликтов)
echo.

echo Управление:
echo - Shift + A → Стрейф влево (без конфликтов)
echo - Shift + D → Стрейф вправо (без конфликтов)
echo - Space → Прыжок (без конфликтов)
echo - Space → Лазание (без конфликтов)
echo.

echo Дополнительная помощь:
echo - Ручная настройка: MANUAL_STRAFE_SHIFT_AD_SETUP.md
echo.

pause


