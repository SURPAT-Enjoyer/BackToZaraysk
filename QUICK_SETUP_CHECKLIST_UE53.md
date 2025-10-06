# ⚡ БЫСТРАЯ НАСТРОЙКА - ШПАРГАЛКА ДЛЯ UE 5.3

## 🎯 **ПОШАГОВЫЙ ЧЕКЛИСТ (15 минут) - UE 5.3**

### ✅ **ШАГ 1: Input Actions (3 минуты) - UE 5.3**
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
├─ Правый клик → Input → Input Action → IA_A
   └─ Value Type: Digital (bool) ← НОВОЕ В UE 5.3
   └─ Consume Input: false
   └─ Trigger When Paused: false
├─ Правый клик → Input → Input Action → IA_D
   └─ Value Type: Digital (bool)
   └─ Consume Input: false
   └─ Trigger When Paused: false
└─ Правый клик → Input → Input Action → IA_Space
   └─ Value Type: Digital (bool)
   └─ Consume Input: false
   └─ Trigger When Paused: false
```

### ✅ **ШАГ 2: Input Mapping Context (2 минуты) - UE 5.3**
```
IMC_Default → Details Panel → Mappings:
├─ + → Action: IA_A, Key: A, Triggers: Down ← НОВОЕ В UE 5.3
├─ + → Action: IA_D, Key: D, Triggers: Down
└─ + → Action: IA_Space, Key: Space Bar, Triggers: Down
```

### ✅ **ШАГ 3: Curve Float (1 минута) - UE 5.3**
```
Content/BackToZaraysk/Core/Data/Curves/
└─ Правый клик → Miscellaneous → Curve Float → CF_StrafeMovement
   └─ Add Key: (0,0), (0.5,1), (1,0)
   └─ Pre-Infinity: Cycle ← НОВОЕ В UE 5.3
   └─ Post-Infinity: Cycle
```

### ✅ **ШАГ 4: BP_Player Input Events (5 минут) - UE 5.3**

#### **IA_A Events (UE 5.3):**
```
Event Graph:
├─ IA_A Started ← ВМЕСТО Pressed
   └─ Set bIsAPressed = True → Handle A Input (True)
└─ IA_A Completed ← ВМЕСТО Released
   └─ Set bIsAPressed = False → Handle A Input (False)
```

#### **IA_D Events (UE 5.3):**
```
Event Graph:
├─ IA_D Started
   └─ Set bIsDPressed = True → Handle D Input (True)
└─ IA_D Completed
   └─ Set bIsDPressed = False → Handle D Input (False)
```

#### **IA_Space Event (UE 5.3):**
```
Event Graph:
└─ IA_Space Started ← ТОЛЬКО Started в UE 5.3
   └─ Handle Space Input (True)
```

#### **Functions:**
```
My Blueprint → Functions:
├─ Handle A Input (bPressed: Boolean) → Call Parent Function
├─ Handle D Input (bPressed: Boolean) → Call Parent Function
└─ Handle Space Input (bPressed: Boolean) → Call Parent Function
```

### ✅ **ШАГ 5: StrafeComponent Settings (2 минуты) - UE 5.3**
```
Components → StrafeComponent → Details Panel:
├─ Strafe Distance: 200.0
├─ Strafe Duration: 0.3
├─ Strafe Cooldown: 1.0
├─ Space Double Press Window: 0.5
├─ Strafe Speed: 400.0
├─ Smoothness Factor: 2.0
└─ Strafe Curve: CF_StrafeMovement
```

### ✅ **ШАГ 6: Animation Blueprint (2 минуты) - UE 5.3**
```
ABP_Player → My Blueprint → Variables:
├─ bIsStrafing (Boolean, Default: False)
└─ StrafeDirection (Float, Default: 0.0)

AnimGraph (UE 5.3):
└─ Blend Poses by Bool ← УЛУЧШЕННЫЙ В UE 5.3
   ├─ Pose A: Base Animation (Idle/Walk)
   ├─ Pose B: Strafing Animation
   └─ Active: bIsStrafing

Sequence Player (UE 5.3):
├─ Sequence: strafing
├─ Play Rate: 1.0
├─ Start Position: 0.0
├─ Loop Animation: false
├─ Enable Root Motion: false ← НОВОЕ В UE 5.3
└─ Root Motion Root Lock: Ignore
```

---

## 🔧 **БЫСТРЫЕ КОМАНДЫ (UE 5.3)**

### **Создание папок:**
```bash
# Запустите для создания всех необходимых папок
.\setup_strafe_system.bat
```

### **Диагностика:**
```bash
# В UE5 Editor Python Console
exec(open('Content/Python/DiagnoseStrafeSystem.py').read())
```

### **Автоматическое исправление:**
```bash
# В UE5 Editor Python Console  
exec(open('Content/Python/FixStrafeSystem.py').read())
```

### **Компиляция Blueprint (UE 5.3):**
```
Ctrl+Alt+F11 ← НОВАЯ ГОРЯЧАЯ КЛАВИША В UE 5.3
```

---

## 🚨 **ЧАСТЫЕ ОШИБКИ В UE 5.3**

### ❌ **Input Events не работают:**
- Проверьте, что используете **Started/Completed** вместо Pressed/Released
- Убедитесь, что **Triggers: Down** установлен в IMC_Default
- Проверьте, что **Value Type: Digital (bool)** в Input Actions

### ❌ **Анимация не проигрывается:**
- Проверьте, что strafing.fbx импортирован с **Use Default Sample Rate: true**
- Убедитесь, что **Enable Root Motion: false** в Sequence Player
- Проверьте настройки **Import Baked Animation: true**

### ❌ **Стрейф не активируется:**
- Проверьте настройки StrafeComponent
- Убедитесь, что CF_StrafeMovement назначена как Strafe Curve
- Проверьте, что все Parent Functions вызываются

### ❌ **Сообщения в консоли не появляются:**
- Проверьте, что все Parent Functions вызываются
- Убедитесь, что StrafeComponent правильно настроен
- Проверьте, что Input Events подключены к правильным функциям

---

## 🔧 **ОСОБЕННОСТИ UE 5.3**

### **Новые возможности:**
- **Digital (bool)** вместо Boolean в Input Actions
- **Started/Completed** вместо Pressed/Released в Events
- **Triggers: Down** вместо Trigger в Input Mapping
- **Ctrl+Alt+F11** для Compile Blueprint
- Улучшенный интерфейс AnimGraph
- Новые опции импорта анимаций
- Расширенные настройки кривых

### **Изменения в интерфейсе:**
- Более детальный Details Panel
- Улучшенная навигация в Content Browser
- Новые опции в Sequence Player
- Расширенные настройки кривых

---

## ✅ **ФИНАЛЬНАЯ ПРОВЕРКА (UE 5.3)**

### **Тест управления:**
1. **A + Space(2x)** → Стрейф влево
2. **D + Space(2x)** → Стрейф вправо

### **Ожидаемый результат:**
- Плавное смещение на 200 см
- Анимация strafing проигрывается
- Сообщения в консоли (Cyan/Green/Yellow)

### **Если не работает:**
1. Запустите диагностику
2. Проверьте каждый пункт чеклиста
3. Убедитесь, что используете **Started/Completed** события
4. Проверьте, что **Value Type: Digital (bool)** в Input Actions

---

## 🎯 **ГОТОВО ДЛЯ UE 5.3!**

После выполнения всех пунктов система стрейфа должна работать идеально в UE 5.3! 🚀

### **Ключевые отличия от предыдущих версий:**
- **Digital (bool)** в Input Actions
- **Started/Completed** в Events
- **Triggers: Down** в Input Mapping
- **Ctrl+Alt+F11** для Compile
- Улучшенный AnimGraph интерфейс
- Новые опции импорта анимаций


