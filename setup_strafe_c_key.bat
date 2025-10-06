@echo off
echo ========================================
echo    НАСТРОЙКА СТРЕЙФА С КЛАВИШЕЙ C
echo ========================================
echo.

echo Запуск настройки стрейфа с клавишей C...
echo.

REM Запускаем скрипт настройки
python Content/Python/SetupStrafeCKey.py

echo.
echo ========================================
echo    НАСТРОЙКА ЗАВЕРШЕНА
echo ========================================
echo.

echo Следующие шаги:
echo 1. Откройте BP_Player в редакторе
echo 2. Проверьте Event Graph на наличие Input Event:
echo    - IA_C Started → Handle C Input
echo 3. Убедитесь, что функция Handle C Input вызывает Parent Function
echo 4. Соберите проект: .\build_ue53.bat
echo 5. Протестируйте: Нажмите C для стрейфа вправо
echo.

echo Управление:
echo - C → Стрейф вправо (одиночное нажатие)
echo.

echo Дополнительная помощь:
echo - Детальные инструкции: STRAFE_C_KEY_SETUP.md
echo.

pause


