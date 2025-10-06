# ⚡ БЫСТРАЯ НАСТРОЙКА СТРЕЙФА (5 минут)

## ✅ Код готов! Осталось только настроить Blueprint.

---

## 🎯 **БЫСТРЫЙ ЧЕКЛИСТ:**

### ✅ **1. Input Actions** (2 минуты)
```
Content/BackToZaraysk/Core/Input/Actions/Strafe/
├─ IA_A (Boolean)
├─ IA_D (Boolean)
└─ IA_Space (Boolean)
```

### ✅ **2. Input Mapping** (1 минута)
```
IMC_Default:
├─ IA_A → Key: A
├─ IA_D → Key: D
└─ IA_Space → Key: Space Bar
```

### ✅ **3. Curve** (1 минута)
```
CF_StrafeMovement:
├─ (0, 0) → (0.5, 1) → (1, 0)
```

### ✅ **4. BP_Player Events** (1 минута)
```
Input Events:
├─ IA_A → Handle A Input
├─ IA_D → Handle D Input
└─ IA_Space → Handle Space Input

Functions:
├─ Handle A Input → Call Parent: Handle A Input
├─ Handle D Input → Call Parent: Handle D Input
└─ Handle Space Input → Call Parent: Handle Space Input
```

### ✅ **5. Import Animation** (30 секунд)
```
strafing.fbx → SK_Mannequin
```

---

## 🎮 **УПРАВЛЕНИЕ:**
- **A + Space(2x)** → Стрейф влево
- **D + Space(2x)** → Стрейф вправо

---

## 🚀 **ГОТОВО!**

Система стрейфа полностью реализована через компонент по аналогии с ObstacleClimbingComponent. Все настройки занимают 5 минут!

**Подробные инструкции**: `DETAILED_STRAFE_SETUP.md`


