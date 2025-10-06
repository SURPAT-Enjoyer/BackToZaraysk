# ⚡ БЫСТРАЯ НАСТРОЙКА - ШПАРГАЛКА

## 🎯 **ПОШАГОВЫЙ ЧЕКЛИСТ (15 минут)**

### ✅ **ШАГ 1: Input Actions (3 минуты)**
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
├─ Правый клик → Input → Input Action → IA_A (Boolean)
├─ Правый клик → Input → Input Action → IA_D (Boolean)
└─ Правый клик → Input → Input Action → IA_Space (Boolean)
```

### ✅ **ШАГ 2: Input Mapping Context (2 минуты)**
```
IMC_Default → Details Panel → Mappings:
├─ + → Action: IA_A, Key: A, Trigger: Impulse
├─ + → Action: IA_D, Key: D, Trigger: Impulse
└─ + → Action: IA_Space, Key: Space Bar, Trigger: Impulse
```

### ✅ **ШАГ 3: Curve Float (1 минута)**
```
Content/BackToZaraysk/Core/Data/Curves/
└─ Правый клик → Miscellaneous → Curve Float → CF_StrafeMovement
   └─ Add Key: (0,0), (0.5,1), (1,0)
```

### ✅ **ШАГ 4: BP_Player Input Events (5 минут)**

#### **IA_A Events:**
```
Event Graph:
├─ IA_A Pressed → Set bIsAPressed = True → Handle A Input (True)
└─ IA_A Released → Set bIsAPressed = False → Handle A Input (False)
```

#### **IA_D Events:**
```
Event Graph:
├─ IA_D Pressed → Set bIsDPressed = True → Handle D Input (True)
└─ IA_D Released → Set bIsDPressed = False → Handle D Input (False)
```

#### **IA_Space Event:**
```
Event Graph:
└─ IA_Space Pressed → Handle Space Input (True)
```

#### **Functions:**
```
My Blueprint → Functions:
├─ Handle A Input (bPressed: Boolean) → Call Parent Function
├─ Handle D Input (bPressed: Boolean) → Call Parent Function
└─ Handle Space Input (bPressed: Boolean) → Call Parent Function
```

### ✅ **ШАГ 5: StrafeComponent Settings (2 минуты)**
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

### ✅ **ШАГ 6: Animation Blueprint (2 минуты)**
```
ABP_Player → My Blueprint → Variables:
├─ bIsStrafing (Boolean, Default: False)
└─ StrafeDirection (Float, Default: 0.0)

AnimGraph:
└─ Blend Poses by Bool:
   ├─ Pose A: Base Animation (Idle/Walk)
   ├─ Pose B: Strafing Animation
   └─ Active: bIsStrafing
```

---

## 🔧 **БЫСТРЫЕ КОМАНДЫ**

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

---

## 🚨 **ЧАСТЫЕ ОШИБКИ**

### ❌ **Input Events не работают:**
- Проверьте, что IA_A/IA_D/IA_Space подключены к Handle A/D/Space Input
- Убедитесь, что Handle функции вызывают Parent Function

### ❌ **Анимация не проигрывается:**
- Проверьте, что strafing.fbx импортирован с правильным Skeleton
- Убедитесь, что Blend Poses by Bool настроен в AnimBP

### ❌ **Стрейф не активируется:**
- Проверьте настройки StrafeComponent
- Убедитесь, что CF_StrafeMovement назначена как Strafe Curve

### ❌ **Сообщения в консоли не появляются:**
- Проверьте, что все Parent Functions вызываются
- Убедитесь, что StrafeComponent правильно настроен

---

## ✅ **ФИНАЛЬНАЯ ПРОВЕРКА**

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
3. Убедитесь, что все Parent Functions вызываются

---

## 🎯 **ГОТОВО!**

После выполнения всех пунктов система стрейфа должна работать идеально! 🚀


