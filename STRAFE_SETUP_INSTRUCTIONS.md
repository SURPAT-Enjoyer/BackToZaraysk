# 🎯 ИНСТРУКЦИИ ПО НАСТРОЙКЕ МЕХАНИКИ СТРЕЙФА

## ✅ Выполненные задачи:

### 1. Код реализован
- ✅ Добавлены функции стрейфа в `PlayerCharacter.h/cpp`
- ✅ Обновлен `BTZBaseCharacterAnimInstance.h/cpp` для поддержки стрейфа
- ✅ Проект успешно собран без ошибок
- ✅ Редактор Unreal Engine запущен

### 2. Структура кода
```cpp
// Новые функции в PlayerCharacter:
void StrafeRight();           // Стрейф вправо
void StrafeLeft();            // Стрейф влево  
bool CanStrafe() const;       // Проверка возможности стрейфа

// Новые переменные:
float StrafeDistance = 200.0f;    // Дистанция стрейфа
float StrafeDuration = 0.3f;      // Длительность стрейфа
float StrafeCooldown = 1.0f;      // Кулдаун между стрейфами
bool bIsStrafing;                 // Состояние стрейфа
float StrafeDirection;            // Направление (-1.0f влево, 1.0f вправо)
```

## 🔧 Настройка в редакторе UE5:

### 3. Создание Input Actions

1. **Откройте Content Browser**
2. **Перейдите в папку**: `Content/BackToZaraysk/Core/Input/Actions/Strafe/`
3. **Создайте два новых Input Action**:
   - `IA_StrafeRight` (Boolean)
   - `IA_StrafeLeft` (Boolean)

### 4. Настройка Input Mapping Context

1. **Откройте**: `Content/BackToZaraysk/Core/Input/IMC_Default`
2. **Добавьте новые маппинги**:
   - **D + Space**: `IA_StrafeRight` (двойное нажатие Space)
   - **A + Space**: `IA_StrafeLeft` (двойное нажатие Space)

### 5. Импорт анимации strafing

1. **В Content Browser перейдите в**: `Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/`
2. **Найдите файл**: `strafing.fbx`
3. **Импортируйте анимацию**:
   - Выберите файл
   - Нажмите "Import"
   - Настройте параметры импорта:
     - **Skeleton**: `SK_Mannequin`
     - **Import Translation**: `(0, 0, 0)`
     - **Import Rotation**: `(0, 0, 0)`
     - **Import Scale**: `1.0`

### 6. Настройка Animation Blueprint

1. **Откройте**: `Content/BackToZaraysk/Core/Characters/Player/ABP_Player`
2. **Добавьте новые переменные** (уже добавлены в код):
   - `bIsStrafing` (Boolean)
   - `StrafeDirection` (Float)
3. **В AnimGraph добавьте**:
   - **Blend Node** для смешивания стрейф-анимации с базовой анимацией
   - **Sequence Player** для анимации strafing
   - **Conditional Logic** основанный на `bIsStrafing`

### 7. Настройка Character Blueprint

1. **Откройте**: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`
2. **В Input Events добавьте**:
   - **IA_StrafeRight** → Call Function: `StrafeRight`
   - **IA_StrafeLeft** → Call Function: `StrafeLeft`
3. **Настройте параметры стрейфа** в Details Panel:
   - **Strafe Distance**: `200.0`
   - **Strafe Duration**: `0.3`
   - **Strafe Cooldown**: `1.0`

### 8. Создание Curve для плавного движения

1. **Создайте новый Curve Float**: `CF_StrafeMovement`
2. **Настройте кривую**:
   - **Time 0**: Value 0
   - **Time 0.5**: Value 1 (пик)
   - **Time 1**: Value 0
3. **Назначьте кривую** в BP_Player → Strafe Curve

## 🎮 Как работает механика:

1. **Условия для стрейфа**:
   - Игрок не в воздухе
   - Игрок не приседает
   - Игрок не лежит
   - Прошел кулдаун с последнего стрейфа

2. **Управление**:
   - **D + Space (2x)**: Стрейф вправо
   - **A + Space (2x)**: Стрейф влево

3. **Движение**:
   - Плавное смещение на заданную дистанцию
   - Анимация strafing во время движения
   - Кулдаун после завершения

## 🔍 Отладка:

- В игре будут отображаться сообщения о начале и завершении стрейфа
- В AnimInstance отображается состояние IK ног
- Все параметры настраиваются в Blueprint

## 📝 Следующие шаги:

1. Протестировать механику в игре
2. Настроить анимацию стрейфа в AnimGraph
3. Добавить звуковые эффекты
4. Настроить визуальные эффекты (пыль, следы и т.д.)

