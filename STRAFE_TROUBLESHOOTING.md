# 🔧 ИСПРАВЛЕНИЕ ПРОБЛЕМ СО СТРЕЙФОМ

## 🚨 **ПРОБЛЕМА: Стрейф не работает, анимация не проигрывается**

### 🔍 **ДИАГНОСТИКА:**

#### 1. Проверьте Input Actions
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
├─ IA_A (Boolean) ✅
├─ IA_D (Boolean) ✅
└─ IA_Space (Boolean) ✅
```

#### 2. Проверьте Input Mapping Context
```
IMC_Default должен содержать:
├─ IA_A → Key: A ✅
├─ IA_D → Key: D ✅
└─ IA_Space → Key: Space Bar ✅
```

#### 3. Проверьте BP_Player
```
Input Events:
├─ IA_A (Pressed/Released) → Handle A Input ❓
├─ IA_D (Pressed/Released) → Handle D Input ❓
└─ IA_Space (Pressed) → Handle Space Input ❓

Functions:
├─ Handle A Input → Call Parent Function ❓
├─ Handle D Input → Call Parent Function ❓
└─ Handle Space Input → Call Parent Function ❓
```

#### 4. Проверьте StrafeComponent
```
В BP_Player должен быть StrafeComponent с настройками:
├─ Strafe Distance: 200.0
├─ Strafe Duration: 0.3
├─ Strafe Cooldown: 1.0
├─ Space Double Press Window: 0.5
└─ Strafe Curve: CF_StrafeMovement
```

#### 5. Проверьте Animation Blueprint
```
ABP_Player должен содержать переменные:
├─ bIsStrafing (Boolean)
└─ StrafeDirection (Float)
```

---

## 🔧 **ИСПРАВЛЕНИЯ:**

### ❌ **Проблема 1: Input Events не подключены**

**Решение:**
1. Откройте BP_Player
2. В Event Graph добавьте Input Events:
   - **IA_A Pressed** → Set `bIsAPressed = True`
   - **IA_A Released** → Set `bIsAPressed = False`
   - **IA_D Pressed** → Set `bIsDPressed = True`
   - **IA_D Released** → Set `bIsDPressed = False`
   - **IA_Space Pressed** → Call `Handle Space Input (True)`

### ❌ **Проблема 2: Функции не вызывают Parent Function**

**Решение:**
1. В функции **Handle A Input**:
   ```
   Input: bPressed (Boolean)
   → Call Parent Function: Handle A Input
   → Pass: bPressed
   ```

2. В функции **Handle D Input**:
   ```
   Input: bPressed (Boolean)
   → Call Parent Function: Handle D Input
   → Pass: bPressed
   ```

3. В функции **Handle Space Input**:
   ```
   Input: bPressed (Boolean)
   → Call Parent Function: Handle Space Input
   → Pass: bPressed
   ```

### ❌ **Проблема 3: StrafeComponent не настроен**

**Решение:**
1. Выберите StrafeComponent в BP_Player
2. В Details Panel настройте:
   - **Strafe Distance**: `200.0`
   - **Strafe Duration**: `0.3`
   - **Strafe Cooldown**: `1.0`
   - **Space Double Press Window**: `0.5`
   - **Strafe Speed**: `400.0`
   - **Smoothness Factor**: `2.0`
   - **Strafe Curve**: `CF_StrafeMovement`

### ❌ **Проблема 4: Анимация не импортирована**

**Решение:**
1. Найдите `strafing.fbx` в папке Strafing
2. Импортируйте с настройками:
   - **Skeleton**: `SK_Mannequin`
   - **Import Translation**: `(0, 0, 0)`
   - **Import Rotation**: `(0, 0, 0)`
   - **Import Scale**: `1.0`

### ❌ **Проблема 5: AnimBP не настроен**

**Решение:**
1. Откройте ABP_Player
2. Добавьте переменные:
   - `bIsStrafing` (Boolean)
   - `StrafeDirection` (Float)
3. В AnimGraph:
   - Добавьте **Blend Node**
   - Добавьте **Sequence Player** для анимации strafing
   - Подключите логику на основе `bIsStrafing`

---

## 🎯 **БЫСТРОЕ ИСПРАВЛЕНИЕ:**

### Шаг 1: Запустите диагностику
```bash
python Content/Python/DiagnoseStrafeSystem.py
```

### Шаг 2: Автоматическое исправление
```bash
python Content/Python/FixStrafeSystem.py
```

### Шаг 3: Ручная настройка Blueprint
1. **BP_Player** → Input Events → подключить к функциям
2. **ABP_Player** → добавить переменные и логику анимации

---

## 🔍 **ОТЛАДКА:**

### Проверьте консоль в игре:
- **Cyan**: Информация о движении
- **Green**: Успешный стрейф
- **Red**: Ошибки
- **Yellow**: Прогресс анимации

### Если сообщений нет:
1. Проверьте подключение Input Events
2. Проверьте вызов Parent Functions
3. Проверьте настройки StrafeComponent

---

## ✅ **ПРОВЕРКА РАБОТЫ:**

### Тест управления:
1. **Удерживайте A** и быстро нажмите **Space дважды**
2. **Удерживайте D** и быстро нажмите **Space дважды**
3. Персонаж должен сместиться в соответствующем направлении

### Ожидаемый результат:
- Плавное смещение на 200 см
- Анимация strafing
- Кулдаун 1 секунда между стрейфами


