# ⚡ БЫСТРОЕ ИСПРАВЛЕНИЕ: ПЕРСОНАЖ ПРЫГАЕТ ВМЕСТО СТРЕЙФА

## 🚨 **ПРОБЛЕМА: При A/D + Space(2x) персонаж прыгает вместо плавного стрейфа**

### ⚡ **БЫСТРОЕ РЕШЕНИЕ (5 минут):**

---

## 🔧 **ШАГ 1: Диагностика (1 минута)**

```bash
python Content/Python/DiagnoseStrafeInputIssue.py
```

**Ожидаемый результат:**
- ✅ Все Input Actions найдены
- ✅ IMC_Default настроен
- ✅ StrafeComponent найден
- ✅ Функции Handle A/D/Space Input найдены

---

## 🔧 **ШАГ 2: Автоматическое исправление (1 минута)**

```bash
python Content/Python/FixJumpInsteadOfStrafe.py
```

**Что исправится автоматически:**
- ✅ Проверка Input Actions
- ✅ Проверка IMC_Default
- ✅ Проверка StrafeComponent
- ✅ Проверка конфликтов с Jump

---

## 🔧 **ШАГ 3: Ручная проверка BP_Player (3 минуты)**

### 3.1 Откройте BP_Player

### 3.2 Проверьте Event Graph:

#### **Должны быть подключены:**
```
IA_A Started → Handle A Input (True)
IA_A Completed → Handle A Input (False)
IA_D Started → Handle D Input (True)
IA_D Completed → Handle D Input (False)
IA_Space Started → Handle Space Input (True)
```

### 3.3 Проверьте функции Handle Input:

#### **В каждой функции должно быть:**
```
Handle A Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle A Input
    └─ Pass: bPressed

Handle D Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle D Input
    └─ Pass: bPressed

Handle Space Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle Space Input
    └─ Pass: bPressed
```

### 3.4 Проверьте StrafeComponent:

#### **Настройки должны быть:**
```
Strafe Distance: 200.0
Strafe Duration: 0.3
Strafe Cooldown: 1.0
Space Double Press Window: 0.5
Strafe Speed: 400.0
Smoothness Factor: 2.0
Strafe Curve: CF_StrafeMovement
```

---

## 🔧 **ШАГ 4: Проверка конфликтов с Jump (1 минута)**

### 4.1 Откройте IMC_Default

### 4.2 Проверьте маппинги на Space Bar:

#### **Должно быть только:**
```
IA_Space → Space Bar (Triggers: Down)
```

#### **НЕ должно быть:**
```
Jump → Space Bar ❌
```

### 4.3 Если есть конфликт:
1. Найдите маппинг Jump на Space Bar
2. Измените клавишу для Jump (например, на W)
3. Или удалите маппинг Jump

---

## 🔧 **ШАГ 5: Тестирование (1 минута)**

### 5.1 Соберите проект:
```bash
.\build_ue53.bat
```

### 5.2 Запустите игру

### 5.3 Тестируйте:
1. **Удерживайте A** и быстро нажмите **Space дважды**
2. **Удерживайте D** и быстро нажмите **Space дважды**

### 5.4 Проверьте консоль:
Должны появиться сообщения:
- **Cyan**: "Strafe Animation: Starting from..."
- **Green**: "Strafe: Starting Left/Right strafe!"
- **Yellow**: "Strafe Progress: X.X/1.0"

---

## 🚨 **ЧАСТЫЕ ПРИЧИНЫ ПРОБЛЕМЫ:**

### ❌ **Input Events не подключены:**
- **Решение**: Добавьте Input Events в Event Graph
- **Подключите**: IA_A/D/Space → Handle A/D/Space Input

### ❌ **Parent Functions не вызываются:**
- **Решение**: Добавьте Call Parent Function в Handle функции
- **Проверьте**: Все Handle функции вызывают Parent Function

### ❌ **Конфликт с Jump Input:**
- **Решение**: Измените клавишу для Jump или удалите маппинг
- **Проверьте**: Только IA_Space использует Space Bar

### ❌ **StrafeComponent не настроен:**
- **Решение**: Проверьте настройки StrafeComponent
- **Убедитесь**: Strafe Curve назначена

---

## ✅ **ПРОВЕРКА РАБОТЫ:**

### **Ожидаемый результат:**
- Плавное смещение на 200 см
- Анимация strafing проигрывается
- Сообщения в консоли (Cyan/Green/Yellow)
- Персонаж НЕ прыгает

### **Если персонаж все еще прыгает:**
1. Проверьте, что Space Input не конфликтует с Jump
2. Убедитесь, что все Parent Functions вызываются
3. Проверьте настройки StrafeComponent

---

## 🎯 **ГОТОВО!**

После выполнения всех шагов стрейф должен работать корректно без прыжков!

### **Ключевые моменты:**
- **Input Events** должны быть подключены к Handle функциям
- **Handle функции** должны вызывать Parent Function
- **Space Input** не должен конфликтовать с Jump
- **StrafeComponent** должен быть правильно настроен


