@echo off
echo.
echo 🎭 УСТАНОВКА ПРАВИЛЬНОГО КЛАССА ANIMINSTANCE
echo ===========================================
echo.
echo Команды для выполнения в Unreal Editor (Ctrl+Shift+P):
echo.
echo 1. АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ:
echo exec(open('Content/Python/SetCorrectAnimInstanceClass.py').read())
echo.
echo 2. ПРОВЕРКА И РУЧНЫЕ ИНСТРУКЦИИ:
echo exec(open('Content/Python/ManualAnimInstanceFix.py').read())
echo.
echo ИНСТРУКЦИЯ:
echo 1. Откройте Unreal Editor
echo 2. Нажмите Ctrl + Shift + P для консоли Python
echo 3. Выполните команду 1 для автоматического исправления
echo 4. Если не сработает, выполните команду 2 для ручных инструкций
echo.
echo РУЧНОЕ ИСПРАВЛЕНИЕ (если автоматическое не работает):
echo 1. Откройте ABP_Player в Content Browser
echo 2. Дважды кликните для открытия
echo 3. Нажмите F4 или кнопку "Class Settings"
echo 4. В "Parent Class" выберите "UBTZBaseCharacterAnimInstance"
echo 5. Сохраните файл (Ctrl+S)
echo.
echo РЕЗУЛЬТАТ:
echo - ИК система будет работать правильно
echo - Ноги персонажа будут адаптироваться к неровностям
echo - Анимация будет плавной и естественной
echo.
pause
