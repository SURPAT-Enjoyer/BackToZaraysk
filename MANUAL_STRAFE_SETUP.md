# 🎮 РУЧНАЯ НАСТРОЙКА СТРЕЙФА В UE5

## ✅ Код уже готов! Нужно только настроить Blueprint.

### 🔧 **Шаг 1: Создание Input Actions**

1. **Откройте Content Browser**
2. **Перейдите в**: `Content/BackToZaraysk/Core/Input/Actions/Strafe/`
3. **Создайте 3 новых Input Action**:

#### IA_A
- **Тип**: Boolean
- **Название**: `IA_A`

#### IA_D  
- **Тип**: Boolean
- **Название**: `IA_D`

#### IA_Space
- **Тип**: Boolean  
- **Название**: `IA_Space`

### 🔧 **Шаг 2: Настройка Input Mapping Context**

1. **Откройте**: `Content/BackToZaraysk/Core/Input/IMC_Default`
2. **Добавьте новые маппинги**:

```
IA_A:
├─ Key: A
├─ Modifiers: None
└─ Trigger: Impulse

IA_D:
├─ Key: D  
├─ Modifiers: None
└─ Trigger: Impulse

IA_Space:
├─ Key: Space Bar
├─ Modifiers: None
└─ Trigger: Impulse
```

### 🔧 **Шаг 3: Создание Curve для стрейфа**

1. **Создайте новый Curve Float**: `CF_StrafeMovement`
2. **Путь**: `Content/BackToZaraysk/Core/Data/Curves/`
3. **Настройте кривую**:
   - **Time 0.0**: Value 0.0
   - **Time 0.5**: Value 1.0
   - **Time 1.0**: Value 0.0

### 🔧 **Шаг 4: Настройка BP_Player**

1. **Откройте**: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`

#### 4.1 Input Events
В **Event Graph** добавьте:

```
IA_A (Pressed/Released):
├─ Call Function: Handle A Input
└─ Pass: Pressed/Released

IA_D (Pressed/Released):
├─ Call Function: Handle D Input  
└─ Pass: Pressed/Released

IA_Space (Pressed):
├─ Call Function: Handle Space Input
└─ Pass: True
```

#### 4.2 Custom Functions

**Создайте функцию "Handle A Input":**
- **Input**: `bPressed` (Boolean)
- **Action**: Call Parent Function → `Handle A Input`
- **Pass**: `bPressed`

**Создайте функцию "Handle D Input":**
- **Input**: `bPressed` (Boolean)
- **Action**: Call Parent Function → `Handle D Input`  
- **Pass**: `bPressed`

**Создайте функцию "Handle Space Input":**
- **Input**: `bPressed` (Boolean)
- **Action**: Call Parent Function → `Handle Space Input`
- **Pass**: `bPressed`

#### 4.3 Настройка свойств

В **Details Panel** найдите секцию **"Strafe"**:

- **Strafe Distance**: `200.0`
- **Strafe Duration**: `0.3`
- **Strafe Cooldown**: `1.0`
- **Space Double Press Window**: `0.5`
- **Strafe Curve**: `CF_StrafeMovement`

### 🔧 **Шаг 5: Импорт анимации**

1. **Перейдите в**: `Content/BackToZaraysk/Characters/Mannequins/Animations/Strafing/`
2. **Найдите**: `strafing.fbx`
3. **Импортируйте**:
   - **Skeleton**: `SK_Mannequin`
   - **Import Translation**: `(0, 0, 0)`
   - **Import Rotation**: `(0, 0, 0)`
   - **Import Scale**: `1.0`

### 🔧 **Шаг 6: Настройка Animation Blueprint**

1. **Откройте**: `Content/BackToZaraysk/Core/Characters/Player/ABP_Player`
2. **Проверьте переменные** (уже добавлены в код):
   - `bIsStrafing` (Boolean)
   - `StrafeDirection` (Float)
3. **В AnimGraph добавьте**:
   - **Blend Node** для смешивания анимации стрейфа
   - **Sequence Player** для анимации strafing
   - **Conditional Logic** на основе `bIsStrafing`

## 🎮 **Тестирование:**

1. **Соберите проект**: `.\build_ue53.bat`
2. **Запустите игру**
3. **Тестируйте**:
   - **A + Space(2x)** → Стрейф влево
   - **D + Space(2x)** → Стрейф вправо

## 🔍 **Отладка:**

В игре будут сообщения:
- **Cyan**: "Strafe Right/Left started"
- **Green**: "Strafe finished"  
- **Yellow**: Информация о Stamina

## ⚠️ **Важно:**

- **A/D должны быть нажаты ДО двойного нажатия Space**
- **Двойное нажатие Space в течение 0.5 секунды**
- **Стрейф не работает в воздухе/при приседании/лежании**
- **Кулдаун 1 секунда между стрейфами**

## 🎯 **Готово!**

После выполнения всех шагов стрейф будет работать с комбинацией **A/D + Space(2x)**!


