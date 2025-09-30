# 🔧 РУЧНОЕ ИСПРАВЛЕНИЕ ИК В ABP_PLAYER

## Проблема
В Animation Blueprint ABP_Player неправильно настроена система инверсной кинематики (ИК) для ног персонажа.

## Решение

### Способ 1: Автоматическое исправление (РЕКОМЕНДУЕТСЯ)

1. Откройте Unreal Editor
2. Нажмите `Ctrl + Shift + P` для консоли Python
3. Скопируйте и вставьте эту команду:
```python
exec(open('Content/Python/QuickFixABP_Player.py').read())
```

### Способ 2: Ручная настройка

#### Шаг 1: Откройте Animation Blueprint
1. В Content Browser найдите `ABP_Player`
2. Дважды кликните для открытия

#### Шаг 2: Добавьте переменные ИК
В окне **My Blueprint** → **Variables** добавьте следующие переменные:

1. **LeftFootEffectorLocation** (Vector)
   - Тип: Vector
   - Значение по умолчанию: (0, 0, 0)
   - Категория: IK Settings

2. **RightFootEffectorLocation** (Vector)
   - Тип: Vector
   - Значение по умолчанию: (0, 0, 0)
   - Категория: IK Settings

3. **LeftFootIKAlpha** (Float)
   - Тип: Float
   - Значение по умолчанию: 1.0
   - Категория: IK Settings

4. **RightFootIKAlpha** (Float)
   - Тип: Float
   - Значение по умолчанию: 1.0
   - Категория: IK Settings

#### Шаг 3: Добавьте ИК ноды в AnimGraph
1. Откройте вкладку **AnimGraph**
2. Добавьте два нода **Two Bone IK**:
   - `LeftFootIK`
   - `RightFootIK`

#### Шаг 4: Настройте ИК ноды

**Для LeftFootIK:**
- **IK Bone**: `foot_l`
- **Joint Target**: `calf_l`
- **Effector Target**: `thigh_l`
- **Effector Location**: Подключите к `LeftFootEffectorLocation`
- **Alpha**: Подключите к `LeftFootIKAlpha`

**Для RightFootIK:**
- **IK Bone**: `foot_r`
- **Joint Target**: `calf_r`
- **Effector Target**: `thigh_r`
- **Effector Location**: Подключите к `RightFootEffectorLocation`
- **Alpha**: Подключите к `RightFootIKAlpha`

#### Шаг 5: Подключите ИК к анимации
1. Подключите выходы ИК нодов к основному анимационному графу
2. Используйте **Lerp** ноды для плавного смешивания ИК с обычной анимацией

### Шаг 6: Проверьте сокеты персонажа
Убедитесь, что в скелете персонажа есть сокеты:
- `foot_l` (левая нога)
- `foot_r` (правая нога)

Если сокетов нет:
1. Откройте скелет персонажа
2. В **Skeleton Tree** найдите кости ног
3. Правый клик → **Add Socket**
4. Создайте сокеты `foot_l` и `foot_r` на соответствующих костях

## Проверка работы

После настройки в игре должны появиться отладочные сообщения:
- "IK Final - Left: X.XX, Right: X.XX"
- "IK Updated in ABP - Left: X.XX, Right: X.XX"

## Если что-то не работает

1. **Проверьте логи** в консоли редактора
2. **Убедитесь**, что переменные созданы в Animation Blueprint
3. **Проверьте**, что сокеты `foot_l` и `foot_r` существуют
4. **Убедитесь**, что переменные подключены к ИК нодам
5. **Проверьте**, что персонаж стоит на земле
6. **Убедитесь**, что ИК ноды подключены к основному графу

## Результат

После выполнения всех шагов:
- 🎯 **Переменные ИК созданы в Animation Blueprint**
- 🎯 **ИК ноды настроены в AnimGraph**
- 🎯 **Система готова к работе**
- 🎯 **Ноги персонажа будут адаптироваться к поверхности**

## Быстрое исправление

Просто скопируйте и вставьте в консоль Python:

```python
exec(open('Content/Python/QuickFixABP_Player.py').read())
```

И все будет исправлено автоматически! 🚀
