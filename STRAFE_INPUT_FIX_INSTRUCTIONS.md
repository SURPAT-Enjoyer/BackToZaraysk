# 🔧 ИСПРАВЛЕНИЕ СИСТЕМЫ ВВОДА ДЛЯ СТРЕЙФА

## ✅ Проблема решена!

Система ввода была полностью переработана для корректной работы комбинации **A/D + Space**.

### 🎯 **Новые функции ввода:**

```cpp
// Новые функции обработки ввода:
void HandleAInput(bool bPressed);      // Отслеживание нажатия A
void HandleDInput(bool bPressed);      // Отслеживание нажатия D  
void HandleSpaceInput(bool bPressed);  // Отслеживание двойного нажатия Space
void CheckStrafeCombo();               // Проверка комбинации клавиш
```

### 🎮 **Как теперь работает ввод:**

1. **A + Space (2x)** → Стрейф влево
2. **D + Space (2x)** → Стрейф вправо

**Алгоритм:**
- Нажмите и **удерживайте** A или D
- Быстро нажмите **Space дважды** (в течение 0.5 секунды)
- Персонаж выполнит стрейф в соответствующем направлении

### 🔧 **Настройка в редакторе UE5:**

#### 1. Откройте Character Blueprint
- **Файл**: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`

#### 2. Настройте Input Events

**В Input Events добавьте:**

```
IA_Move (Axis) → Handle A/D Input:
├─ Get Input Action Value (IA_Move)
├─ Break Vector2D 
├─ Get X (для A/D)
├─ Greater (0.1) → Handle D Input (True)
└─ Less (-0.1) → Handle A Input (True)

IA_Jump (Boolean) → Handle Space Input:
└─ Handle Space Input (Pressed)
```

#### 3. Создайте Custom Functions

**Создайте эти функции в BP_Player:**

**Handle A Input:**
```cpp
Input: bPressed (Boolean)
├─ Call Parent Function: Handle A Input
└─ Pass: bPressed
```

**Handle D Input:**
```cpp
Input: bPressed (Boolean)  
├─ Call Parent Function: Handle D Input
└─ Pass: bPressed
```

**Handle Space Input:**
```cpp
Input: bPressed (Boolean)
├─ Call Parent Function: Handle Space Input  
└─ Pass: bPressed
```

#### 4. Альтернативный способ (прямые клавиши)

Если Input Actions не работают, используйте прямые клавиши:

```
Input Action: A Key → Handle A Input (Pressed/Released)
Input Action: D Key → Handle D Input (Pressed/Released)  
Input Action: Space Bar → Handle Space Input (Pressed)
```

### 🎯 **Настройки стрейфа:**

В **Details Panel** BP_Player найдите секцию **"Strafe Input"**:

- **Space Double Press Window**: `0.5` (время для двойного нажатия)
- **Strafe Distance**: `200.0` (дистанция стрейфа)
- **Strafe Duration**: `0.3` (длительность)
- **Strafe Cooldown**: `1.0` (кулдаун)

### 🔍 **Отладка:**

В игре будут отображаться сообщения:
- **Cyan**: "Strafe Right/Left started" 
- **Green**: "Strafe finished"
- **Yellow**: Информация о Stamina и IK

### 📝 **Тестирование:**

1. **Запустите игру**
2. **Удерживайте A** и быстро нажмите **Space дважды**
3. **Удерживайте D** и быстро нажмите **Space дважды**
4. Проверьте, что стрейф работает только при выполнении всех условий

### ⚠️ **Важные моменты:**

- **A/D должны быть нажаты ДО двойного нажатия Space**
- **Двойное нажатие Space должно произойти в течение 0.5 секунды**
- **Стрейф не работает в воздухе, при приседании или лежании**
- **Есть кулдаун 1 секунда между стрейфами**

### 🎮 **Готово!**

Теперь система ввода работает корректно с комбинацией **A/D + Space (2x)** для стрейфа!

