# 🎭 КОМАНДЫ ДЛЯ ИСПРАВЛЕНИЯ КЛАССА ANIMINSTANCE

## Проблема
Ошибка: `FileNotFoundError: [Errno 2] No such file or directory: 'Content/Python/SetCorrectAnimInstanceClass.py'`

## Решение

### Команда 1: Быстрая проверка и инструкции
Выполните в консоли Python редактора (Ctrl+Shift+P):
```python
exec(open('Content/Python/QuickFixAnimInstance.py').read())
```

### Команда 2: Подробная диагностика и исправление
```python
exec(open('Content/Python/FixAnimInstanceClass.py').read())
```

### Команда 3: Ручные инструкции
```python
exec(open('Content/Python/ManualAnimInstanceFix.py').read())
```

## Ручное исправление (если команды не работают)

### Шаг 1: Откройте ABP_Player
1. В Content Browser найдите файл `ABP_Player`
2. Дважды кликните для открытия

### Шаг 2: Откройте Class Settings
1. Нажмите `F4` или
2. Найдите кнопку "Class Settings" в верхней панели

### Шаг 3: Установите правильный Parent Class
1. В разделе "Parent Class" найдите выпадающий список
2. Выберите `UBTZBaseCharacterAnimInstance`
3. Если не найдено, ищите `BTZBaseCharacterAnimInstance`

### Шаг 4: Сохраните изменения
1. Нажмите `Ctrl+S`
2. Или File → Save

## Проверка результата
После исправления в Class Settings должно отображаться:
**Parent Class: BTZBaseCharacterAnimInstance**

## Если класс не найден
1. Убедитесь, что проект собран без ошибок
2. Перезапустите редактор
3. Проверьте, что класс `UBTZBaseCharacterAnimInstance` существует в коде

## Результат
После исправления:
- ✅ ИК система будет работать правильно
- ✅ Ноги персонажа будут адаптироваться к неровностям
- ✅ Анимация будет плавной и естественной
