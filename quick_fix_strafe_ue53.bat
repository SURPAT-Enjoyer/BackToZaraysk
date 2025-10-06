@echo off
echo ========================================
echo    БЫСТРОЕ ИСПРАВЛЕНИЕ СТРЕЙФА UE 5.3
echo ========================================
echo.

echo Запуск быстрого исправления...
echo.

REM Запускаем быстрый скрипт исправления
python Content/Python/QuickFixStrafeUE53.py

echo.
echo ========================================
echo    БЫСТРОЕ ИСПРАВЛЕНИЕ ЗАВЕРШЕНО
echo ========================================
echo.

echo Следующие шаги:
echo 1. Откройте BP_Player в редакторе
echo 2. Проверьте Event Graph на наличие Input Events
echo 3. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function
echo 4. Соберите проект: .\build_ue53.bat
echo 5. Протестируйте: A/D + Space(2x)
echo.

echo Если проблема не решена:
echo - Запустите полное исправление: run_strafe_fix_ue53.bat
echo - Используйте ручную настройку: FIX_JUMP_INSTEAD_OF_STRAFE.md
echo.

pause


