# Настройка поворота головы через Transform (Modify) Bone

## Проблема
Голова не поворачивается, хотя значения HeadYawRotation и HeadPitchRotation меняются в C++.

## Решение: Настройка Animation Blueprint

### Шаг 1: Откройте Animation Blueprint
1. Найдите ваш Animation Blueprint для персонажа
2. Откройте его в редакторе

### Шаг 2: Найдите Transform (Modify) Bone узел
1. В Animation Blueprint найдите узел **Transform (Modify) Bone**
2. Если его нет, создайте:
   - **Правый клик** → **Animation** → **Transform (Modify) Bone**

### Шаг 3: Настройте Bone Target
1. В узле **Transform (Modify) Bone**:
   - **Bone Name**: `head` (или `neck`, в зависимости от скелета)
   - **Translation**: `(0, 0, 0)` - без смещения
   - **Rotation**: `(HeadPitchRotation, 0, HeadYawRotation)`
   - **Scale**: `(1, 1, 1)` - без масштабирования

### Шаг 4: Подключите переменные
1. **Создайте переменные** в Animation Blueprint:
   - `HeadYawRotation` (Float)
   - `HeadPitchRotation` (Float)

2. **Подключите к узлу**:
   - **Rotation X**: `HeadPitchRotation`
   - **Rotation Z**: `HeadYawRotation`
   - **Rotation Y**: `0.0`

### Шаг 5: Настройте порядок узлов
```
[Base Animation] → [Transform (Modify) Bone] → [Output Pose]
```

### Шаг 6: Альтернативные Bone Names
Попробуйте разные имена костей:
- `head`
- `neck_01`
- `neck_02`
- `spine_03`

### Шаг 7: Проверьте в игре
1. Зажмите среднюю кнопку мыши
2. Двигайте мышь
3. Голова должна поворачиваться

## Диагностика

### Если голова все еще не поворачивается:

1. **Проверьте Bone Name**:
   - В Persona редакторе посмотрите имена костей скелета
   - Убедитесь что используете правильное имя

2. **Проверьте порядок узлов**:
   - Transform (Modify) Bone должен быть **после** базовой анимации
   - **До** Output Pose

3. **Проверьте значения**:
   - В игре должны появляться сообщения с градусами
   - Если значения не меняются - проблема в C++
   - Если меняются, но голова не поворачивается - проблема в Blueprint

4. **Попробуйте другой подход**:
   - Используйте **Aim Offset** вместо Transform (Modify) Bone
   - Или **Bone Controllers** в скелете

## Пример настройки Transform (Modify) Bone

```
Bone Name: head
Translation: (0, 0, 0)
Rotation: (HeadPitchRotation, 0, HeadYawRotation)
Scale: (1, 1, 1)
Space: Local
```

Где:
- **HeadPitchRotation**: наклон головы вверх/вниз
- **HeadYawRotation**: поворот головы влево/вправо
