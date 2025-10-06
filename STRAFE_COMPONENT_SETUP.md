# 🎯 СИСТЕМА СТРЕЙФА ЧЕРЕЗ КОМПОНЕНТ

## ✅ Реализовано по аналогии с ObstacleClimbingComponent!

### 🏗️ **Архитектура:**

```
PlayerCharacter
├─ StrafeComponent (новый)
├─ ObstacleClimbingComponent (существующий)
└─ InventoryComponent (существующий)
```

### 🔧 **Новый StrafeComponent:**

#### **Основные функции:**
- `TryStrafe(EStrafeType Direction)` - выполнение стрейфа
- `CanStrafe()` - проверка возможности стрейфа
- `HandleAInput()`, `HandleDInput()`, `HandleSpaceInput()` - обработка ввода

#### **Типы стрейфа:**
```cpp
enum class EStrafeType : uint8
{
    None,           // Стрейф недоступен
    Left,           // Стрейф влево
    Right,          // Стрейф вправо
    Forward,        // Стрейф вперед (для будущего)
    Backward        // Стрейф назад (для будущего)
}
```

#### **Настройки:**
- **Strafe Distance**: `200.0` см
- **Strafe Duration**: `0.3` секунды
- **Strafe Cooldown**: `1.0` секунда
- **Space Double Press Window**: `0.5` секунды
- **Strafe Speed**: `400.0` (скорость анимации)
- **Smoothness Factor**: `2.0` (плавность)

### 🎮 **Управление:**
- **A + Space(2x)** → Стрейф влево
- **D + Space(2x)** → Стрейф вправо

### 🔧 **Настройка в UE5:**

#### 1. Создание Input Actions
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
├─ IA_A (Boolean)
├─ IA_D (Boolean)
└─ IA_Space (Boolean)
```

#### 2. Настройка IMC_Default
```
IA_A → Key: A
IA_D → Key: D
IA_Space → Key: Space Bar
```

#### 3. Создание Curve
```
Content/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement
Points: (0,0) → (0.5,1) → (1,0)
```

#### 4. Настройка BP_Player
```
Input Events:
├─ IA_A (Pressed/Released) → Handle A Input
├─ IA_D (Pressed/Released) → Handle D Input
└─ IA_Space (Pressed) → Handle Space Input

StrafeComponent Properties:
├─ Strafe Distance: 200.0
├─ Strafe Duration: 0.3
├─ Strafe Cooldown: 1.0
├─ Space Double Press Window: 0.5
├─ Strafe Speed: 400.0
├─ Smoothness Factor: 2.0
└─ Strafe Curve: CF_StrafeMovement
```

### 🎬 **Анимация:**

#### **В ABP_Player:**
- Переменные `bIsStrafing` и `StrafeDirection` уже настроены
- Автоматическая синхронизация с `StrafeComponent`
- `StrafeDirection`: `-1.0f` (влево), `1.0f` (вправо)

#### **Импорт анимации:**
```
strafing.fbx → SK_Mannequin
```

### 🎯 **Преимущества новой системы:**

1. **Модульность** - отдельный компонент как ObstacleClimbingComponent
2. **Расширяемость** - легко добавить Forward/Backward стрейфы
3. **Переиспользование** - можно использовать на других персонажах
4. **События** - OnStrafeStarted, OnStrafeCompleted, OnStrafeFailed
5. **Настраиваемость** - все параметры в компоненте
6. **Отладка** - подробные сообщения в консоли

### 🔍 **Отладка:**

В игре будут отображаться:
- **Cyan**: Информация о трассировке и движении
- **Green**: Успешное начало/завершение стрейфа
- **Red**: Ошибки и отмены
- **Yellow**: Прогресс анимации

### 🎮 **Готово к тестированию!**

Система стрейфа полностью реализована по аналогии с ObstacleClimbingComponent. Проект собран, редактор запущен. Осталось только настроить Input Actions в Blueprint!


