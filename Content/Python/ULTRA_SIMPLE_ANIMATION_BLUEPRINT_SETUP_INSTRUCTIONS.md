# 🤖 УЛЬТРА-ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ANIMATION BLUEPRINT ДЛЯ IK СИСТЕМЫ

## Ультра-простая автоматическая настройка Animation Blueprint

### Способ 1: Ультра-простая версия (РЕКОМЕНДУЕТСЯ)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/UltraSimpleAnimationBlueprintSetup.py').read())
```

### Способ 2: Финальная версия (если основная не работает)
1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/FinalAnimationBlueprintSetup.py').read())
```

### Способ 3: Через консоль игры
1. Запустите игру в редакторе
2. Нажмите `~` для консоли
3. Выполните команду:
```
py Content/Python/UltraSimpleAnimationBlueprintSetup.py
```

## Что будет выполнено автоматически

### ✅ Переменные IK в Animation Blueprint:
- `LeftFootEffectorLocation` (Vector) - Позиция левой ноги
- `RightFootEffectorLocation` (Vector) - Позиция правой ноги
- `LeftFootIKAlpha` (Float) - Сила IK левой ноги
- `RightFootIKAlpha` (Float) - Сила IK правой ноги

### ✅ IK ноды в AnimGraph:
- `LeftFootIK` (Two Bone IK) - IK для левой ноги
- `RightFootIK` (Two Bone IK) - IK для правой ноги

## Следующие шаги после автоматического выполнения

### 1. Откройте Animation Blueprint персонажа
### 2. Подключите переменные к IK нодам:
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

1. **Попробуйте финальную версию** скрипта
2. **Проверьте логи** в консоли редактора
3. **Убедитесь**, что переменные созданы в Animation Blueprint
4. **Проверьте**, что сокеты `foot_l` и `foot_r` существуют
5. **Убедитесь**, что переменные подключены к IK нодам
6. **Проверьте**, что персонаж стоит на земле

## Файлы скриптов

- `UltraSimpleAnimationBlueprintSetup.py` - Ультра-простая автоматическая настройка (рекомендуется)
- `FinalAnimationBlueprintSetup.py` - Финальная автоматическая настройка (если основная не работает)
- `SimpleAnimationBlueprintSetup.py` - Простая автоматическая настройка (может не работать)
- `AutoSetupAnimationBlueprint.py` - Полная автоматическая настройка (может не работать)

## Поддержка

Если возникают проблемы:
1. Попробуйте финальную версию скрипта
2. Перезапустите редактор
3. Проверьте права доступа к файлам
4. Убедитесь, что все файлы на месте
5. Проверьте версию Unreal Engine (требуется 5.3+)

## Результат

После выполнения всех шагов:
- 🎯 **Переменные IK созданы в Animation Blueprint**
- 🎯 **IK ноды созданы в AnimGraph**
- 🎯 **Система готова к работе**
- 🎯 **Ноги персонажа будут адаптироваться к поверхности**

## Быстрая ультра-простая автоматическая настройка Animation Blueprint

Просто скопируйте и вставьте в консоль Python:

```python
exec(open('Content/Python/UltraSimpleAnimationBlueprintSetup.py').read())
```

Или для финальной версии:

```python
exec(open('Content/Python/FinalAnimationBlueprintSetup.py').read())
```

И все будет выполнено автоматически! 🚀
