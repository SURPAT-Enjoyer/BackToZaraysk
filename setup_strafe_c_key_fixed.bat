@echo off
echo ========================================
echo    НАСТРОЙКА СТРЕЙФА С КЛАВИШЕЙ C
echo ========================================
echo.

echo Запуск исправленного скрипта настройки...
echo.

REM Запускаем исправленный скрипт настройки
python Content/Python/SetupStrafeCKey_Fixed.py

echo.
echo ========================================
echo    НАСТРОЙКА ЗАВЕРШЕНА
echo ========================================
echo.

echo Следующие шаги:
echo 1. Создайте IA_C вручную в Content Browser:
echo    - Правый клик в папке Strafe
echo    - Input → Input Action
echo    - Назовите IA_C
echo    - Value Type: Digital (bool)
echo 2. Откройте BP_Player в редакторе
echo 3. Проверьте Event Graph на наличие Input Event:
echo    - IA_C Started → Handle C Input
echo 4. Убедитесь, что функция Handle C Input вызывает Parent Function
echo 5. Соберите проект: .\build_ue53.bat
echo 6. Протестируйте: Нажмите C для стрейфа вправо
echo.

echo Управление:
echo - C → Стрейф вправо (одиночное нажатие)
echo.

echo Дополнительная помощь:
echo - Ручная настройка: MANUAL_STRAFE_C_SETUP.md
echo - Детальные инструкции: STRAFE_C_KEY_SETUP.md
echo.

pause


