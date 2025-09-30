# 🤖 УЛЬТРА-ПРОСТОЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK СИСТЕМЫ

## Ультра-простое автоматическое выполнение скриптов

### Способ 1: Ультра-простая версия (РЕКОМЕНДУЕТСЯ)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/UltraSimpleIKSetup.py').read())
```

### Способ 2: Финальная версия (если основная не работает)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/FinalIKSetup.py').read())
```

### Способ 3: Через консоль игры
1. Запустите игру в редакторе
2. Нажмите `~` для консоли
3. Выполните команду:
```
py Content/Python/UltraSimpleIKSetup.py
```

## Что будет выполнено автоматически

### ✅ Blueprint Interface:
- `IKInterface` - Интерфейс для IK функций
  - `SetLeftFootIK` - Установка IK левой ноги
  - `SetRightFootIK` - Установка IK правой ноги

### ✅ Blueprint Actor:
- `IKSetupActor` - Actor для автоматической настройки IK

## Следующие шаги после автоматического выполнения

### 1. Откройте Animation Blueprint персонажа
### 2. Создайте переменные IK вручную:
- `LeftFootEffectorLocation` (Vector) - Позиция левой ноги
- `RightFootEffectorLocation` (Vector) - Позиция правой ноги
- `LeftFootIKAlpha` (Float) - Сила IK левой ноги
- `RightFootIKAlpha` (Float) - Сила IK правой ноги

### 3. Создайте IK ноды в AnimGraph:
- `LeftFootIK` (Two Bone IK) - IK для левой ноги
- `RightFootIK` (Two Bone IK) - IK для правой ноги

### 4. Подключите переменные к IK нодам:
- `LeftFootEffectorLocation` → Effector Location (LeftFootIK)
- `RightFootEffectorLocation` → Effector Location (RightFootIK)
- `LeftFootIKAlpha` → Alpha (LeftFootIK)
- `RightFootIKAlpha` → Alpha (RightFootIK)

### 5. Настройте кости для IK:
- **LeftFootIK**: IKBone = `foot_l`, Joint Target = `calf_l`, Effector Target = `thigh_l`
- **RightFootIK**: IKBone = `foot_r`, Joint Target = `calf_r`, Effector Target = `thigh_r`

### 6. Проверьте сокеты персонажа:
- Убедитесь, что в скелете есть сокеты `foot_l` и `foot_r`
- Если нет - добавьте их вручную в скелет персонажа

## Проверка работы

После настройки в игре должны появиться отладочные сообщения:
- "IK Final - Left: X.XX, Right: X.XX"
- "IK Updated in ABP - Left: X.XX, Right: X.XX"

## Если что-то не работает

1. **Попробуйте финальную версию** скрипта
2. **Проверьте логи** в консоли редактора
3. **Убедитесь**, что переменные созданы в Animation Blueprint
4. **Проверьте**, что сокеты `foot_l` и `foot_r` существуют
5. **Убедитесь**, что переменные подключены к IK нодам
6. **Проверьте**, что персонаж стоит на земле

## Файлы скриптов

- `UltraSimpleIKSetup.py` - Ультра-простое автоматическое выполнение (рекомендуется)
- `FinalIKSetup.py` - Финальная версия (если основная не работает)
- `WorkingIKSetup.py` - Рабочая версия (может не работать)
- `BasicIKSetup.py` - Базовая версия (может не работать)

## Поддержка

Если возникают проблемы:
1. Попробуйте финальную версию скрипта
2. Перезапустите редактор
3. Проверьте права доступа к файлам
4. Убедитесь, что все файлы на месте
5. Проверьте версию Unreal Engine (требуется 5.3+)

## Результат

После выполнения всех шагов:
- 🎯 **Blueprint Interface создан**
- 🎯 **Blueprint Actor создан**
- 🎯 **Система готова к работе**
- 🎯 **Ноги персонажа будут адаптироваться к поверхности**

## Быстрое ультра-простое автоматическое выполнение

Просто скопируйте и вставьте в консоль Python:

```python
exec(open('Content/Python/UltraSimpleIKSetup.py').read())
```

Или для финальной версии:

```python
exec(open('Content/Python/FinalIKSetup.py').read())
```

И все будет выполнено автоматически! 🚀
