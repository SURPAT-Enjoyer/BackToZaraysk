# 🤖 РАБОЧЕЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK СИСТЕМЫ

## Рабочее автоматическое выполнение скриптов

### Способ 1: Рабочая версия (РЕКОМЕНДУЕТСЯ)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/WorkingIKSetup.py').read())
```

### Способ 2: Базовая версия (если основная не работает)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/BasicIKSetup.py').read())
```

### Способ 3: Через консоль игры
1. Запустите игру в редакторе
2. Нажмите `~` для консоли
3. Выполните команду:
```
py Content/Python/WorkingIKSetup.py
```

## Что будет выполнено автоматически

### ✅ Blueprint Interface:
- `IKInterface` - Интерфейс для IK функций
  - `SetLeftFootIK` - Установка IK левой ноги
  - `SetRightFootIK` - Установка IK правой ноги

### ✅ Blueprint Actor:
- `IKSetupActor` - Actor для автоматической настройки IK

### ✅ Переменные в Animation Blueprint (если найдены):
- `LeftFootEffectorLocation` (Vector) - Позиция левой ноги
- `RightFootEffectorLocation` (Vector) - Позиция правой ноги
- `LeftFootIKAlpha` (Float) - Сила IK левой ноги
- `RightFootIKAlpha` (Float) - Сила IK правой ноги

### ✅ IK ноды в AnimGraph (если найдены):
- `LeftFootIK` (Two Bone IK) - IK для левой ноги
- `RightFootIK` (Two Bone IK) - IK для правой ноги

## Следующие шаги после автоматического выполнения

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

1. **Попробуйте базовую версию** скрипта
2. **Проверьте логи** в консоли редактора
3. **Убедитесь**, что переменные созданы в Animation Blueprint
4. **Проверьте**, что сокеты `foot_l` и `foot_r` существуют
5. **Убедитесь**, что переменные подключены к IK нодам
6. **Проверьте**, что персонаж стоит на земле

## Файлы скриптов

- `WorkingIKSetup.py` - Рабочее автоматическое выполнение (рекомендуется)
- `BasicIKSetup.py` - Базовая версия (если основная не работает)
- `FixedRunIKSetup.py` - Исправленная версия (может не работать)
- `SimpleIKSetup.py` - Простая версия (может не работать)

## Поддержка

Если возникают проблемы:
1. Попробуйте базовую версию скрипта
2. Перезапустите редактор
3. Проверьте права доступа к файлам
4. Убедитесь, что все файлы на месте
5. Проверьте версию Unreal Engine (требуется 5.3+)

## Результат

После выполнения всех шагов:
- 🎯 **Blueprint Interface создан**
- 🎯 **Blueprint Actor создан**
- 🎯 **Animation Blueprint настроен (если найден)**
- 🎯 **Система готова к работе**
- 🎯 **Ноги персонажа будут адаптироваться к поверхности**

## Быстрое рабочее автоматическое выполнение

Просто скопируйте и вставьте в консоль Python:

```python
exec(open('Content/Python/WorkingIKSetup.py').read())
```

Или для базовой версии:

```python
exec(open('Content/Python/BasicIKSetup.py').read())
```

И все будет выполнено автоматически! 🚀
