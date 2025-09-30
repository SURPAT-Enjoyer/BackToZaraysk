# 🤖 ПРЯМОЕ ИЗМЕНЕНИЕ BLUEPRINT'ОВ ДЛЯ IK СИСТЕМЫ

## Быстрое прямое изменение Blueprint'ов

### Способ 1: В консоли Python (РЕКОМЕНДУЕТСЯ)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/DirectEditBlueprints.py').read())
```

### Способ 2: Через консоль игры
1. Запустите игру в редакторе
2. Нажмите `~` для консоли
3. Выполните команду:
```
py Content/Python/DirectEditBlueprints.py
```

### Способ 3: Через Blueprint
1. Создайте Blueprint Actor
2. Добавьте Event BeginPlay
3. Выполните Python скрипт через Blueprint

## Что будет изменено автоматически

### ✅ Animation Blueprint:
- **Переменные IK:**
  - `LeftFootEffectorLocation` (Vector) - Позиция левой ноги
  - `RightFootEffectorLocation` (Vector) - Позиция правой ноги
  - `LeftFootIKAlpha` (Float) - Сила IK левой ноги
  - `RightFootIKAlpha` (Float) - Сила IK правой ноги

- **IK ноды в AnimGraph:**
  - `LeftFootIK` (Two Bone IK) - IK для левой ноги
  - `RightFootIK` (Two Bone IK) - IK для правой ноги

### ✅ PlayerCharacter Blueprint:
- **IK переменные:**
  - `bEnableFootIK` (Bool) - Включение IK ног
  - `FootIKStrength` (Float) - Сила IK
  - `FootIKBlendSpeed` (Float) - Скорость смешивания IK

### ✅ Blueprint Interface:
- `IKInterface` - Интерфейс для IK функций
  - `SetLeftFootIK` - Установка IK левой ноги
  - `SetRightFootIK` - Установка IK правой ноги

## Следующие шаги после автоматического изменения

### 1. Откройте Animation Blueprint персонажа
### 2. В AnimGraph подключите переменные к IK нодам:
- `LeftFootEffectorLocation` → Effector Location (LeftFootIK)
- `RightFootEffectorLocation` → Effector Location (RightFootIK)
- `LeftFootIKAlpha` → Alpha (LeftFootIK)
- `RightFootIKAlpha` → Alpha (RightFootIK)

### 3. Настройте кости для IK:
- **LeftFootIK**: IKBone = `foot_l`, Joint Target = `calf_l`, Effector Target = `thigh_l`
- **RightFootIK**: IKBone = `foot_r`, Joint Target = `calf_r`, Effector Target = `thigh_r`

### 4. Проверьте сокеты персонажа:
- Убедитесь, что в скелете есть сокеты `foot_l` и `foot_r`
- Если нет - добавьте их вручную в скелет персонажа

## Проверка работы

После настройки в игре должны появиться отладочные сообщения:
- "IK Final - Left: X.XX, Right: X.XX"
- "IK Updated in ABP - Left: X.XX, Right: X.XX"

## Если что-то не работает

1. **Проверьте логи** в консоли редактора
2. **Убедитесь**, что переменные созданы в Animation Blueprint
3. **Проверьте**, что сокеты `foot_l` и `foot_r` существуют
4. **Убедитесь**, что переменные подключены к IK нодам
5. **Проверьте**, что персонаж стоит на земле

## Файлы скриптов

- `DirectEditBlueprints.py` - Прямое изменение Blueprint'ов (рекомендуется)
- `QuickEditBlueprints.py` - Быстрое изменение Blueprint'ов
- `EditBlueprints.py` - Простое изменение Blueprint'ов
- `ModifyBlueprints.py` - Расширенное изменение Blueprint'ов

## Поддержка

Если возникают проблемы:
1. Перезапустите редактор
2. Проверьте права доступа к файлам
3. Убедитесь, что все файлы на месте
4. Проверьте версию Unreal Engine (требуется 5.3+)

## Результат

После выполнения всех шагов:
- 🎯 **Animation Blueprint полностью настроен**
- 🎯 **PlayerCharacter Blueprint полностью настроен**
- 🎯 **Blueprint Interface создан**
- 🎯 **Система готова к работе**
- 🎯 **Ноги персонажа будут адаптироваться к поверхности**

## Быстрое прямое изменение Blueprint'ов

Просто скопируйте и вставьте в консоль Python:

```python
exec(open('Content/Python/DirectEditBlueprints.py').read())
```

И все Blueprint'ы будут изменены автоматически! 🚀
