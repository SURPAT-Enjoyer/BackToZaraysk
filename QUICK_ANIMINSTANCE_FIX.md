# 🎭 БЫСТРОЕ ИСПРАВЛЕНИЕ КЛАССА ANIMINSTANCE

## Проблема
Animation Blueprint ABP_Player использует неправильный класс AnimInstance, из-за чего ИК система не работает.

## Решение

### Способ 1: Автоматическое исправление (РЕКОМЕНДУЕТСЯ)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Выполните команду:
```python
exec(open('Content/Python/SetCorrectAnimInstanceClass.py').read())
```

### Способ 2: Ручное исправление
1. **Откройте ABP_Player:**
   - Найдите файл в Content Browser
   - Дважды кликните для открытия

2. **Откройте Class Settings:**
   - Нажмите `F4` или кнопку "Class Settings"

3. **Установите правильный Parent Class:**
   - В разделе "Parent Class" найдите выпадающий список
   - Выберите `UBTZBaseCharacterAnimInstance`
   - Если не найдено, ищите `BTZBaseCharacterAnimInstance`

4. **Сохраните изменения:**
   - Нажмите `Ctrl+S`

### Способ 3: Проверка и инструкции
Если нужно проверить текущее состояние:
```python
exec(open('Content/Python/ManualAnimInstanceFix.py').read())
```

## Результат
После исправления:
- ✅ **ИК система будет работать правильно**
- ✅ **Ноги персонажа будут адаптироваться к неровностям**
- ✅ **Анимация будет плавной и естественной**
- ✅ **В консоли появятся отладочные сообщения ИК**

## Проверка
После исправления в Class Settings должно отображаться:
**Parent Class: BTZBaseCharacterAnimInstance**

## Если проблемы остаются
1. Убедитесь, что класс `UBTZBaseCharacterAnimInstance` существует
2. Проверьте, что проект собран без ошибок
3. Перезапустите редактор
4. Проверьте, что Animation Blueprint сохранен
