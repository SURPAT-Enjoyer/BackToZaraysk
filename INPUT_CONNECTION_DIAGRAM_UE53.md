# 🔗 СХЕМА ПОДКЛЮЧЕНИЙ INPUT'ОВ И АНИМАЦИЙ ДЛЯ UE 5.3

## 📊 **ОБЩАЯ АРХИТЕКТУРА СИСТЕМЫ (UE 5.3)**

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   INPUT KEYS    │    │  INPUT ACTIONS   │    │ INPUT MAPPING   │
│                 │    │   (UE 5.3)       │    │    CONTEXT      │
│  A, D, Space    │───▶│ IA_A, IA_D,      │───▶│   IMC_Default   │
│                 │    │ IA_Space         │    │                 │
│                 │    │ Digital (bool)   │    │ Triggers: Down  │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                         │
                                                         ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   BP_PLAYER     │◀───│  INPUT EVENTS    │◀───│   ENHANCED      │
│                 │    │   (UE 5.3)       │    │   INPUT SYSTEM  │
│ StrafeComponent │    │ IA_A Started/    │    │                 │
│                 │    │ Completed, etc.  │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │
         ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   ANIMATION     │◀───│  ANIMATION       │◀───│   ANIMATION     │
│   BLUEPRINT     │    │  VARIABLES       │    │   SEQUENCE      │
│   (UE 5.3)      │    │                  │    │   (UE 5.3)      │
│                 │    │                  │    │                 │
│ bIsStrafing     │    │ bIsStrafing,     │    │ strafing.fbx    │
│ StrafeDirection │    │ StrafeDirection  │    │ Root Motion:    │
└─────────────────┘    └──────────────────┘    │ Disabled        │
                                               └─────────────────┘
```

---

## 🎮 **ПОДРОБНАЯ СХЕМА INPUT EVENTS В BP_PLAYER (UE 5.3)**

### **IA_A Events (UE 5.3):**
```
┌─────────────────┐
│   IA_A Started  │  ← В UE 5.3 это аналог Pressed
└─────────┬───────┘
          │
          ├───▶ Set bIsAPressed = True
          │
          └───▶ Handle A Input (bPressed = True)
                    │
                    ▼
              ┌─────────────────┐
              │ Call Parent     │
              │ Function:       │
              │ Handle A Input  │
              └─────────────────┘

┌─────────────────┐
│ IA_A Completed  │  ← В UE 5.3 это аналог Released
└─────────┬───────┘
          │
          ├───▶ Set bIsAPressed = False
          │
          └───▶ Handle A Input (bPressed = False)
                    │
                    ▼
              ┌─────────────────┐
              │ Call Parent     │
              │ Function:       │
              │ Handle A Input  │
              └─────────────────┘
```

### **IA_D Events (UE 5.3):**
```
┌─────────────────┐
│   IA_D Started  │
└─────────┬───────┘
          │
          ├───▶ Set bIsDPressed = True
          │
          └───▶ Handle D Input (bPressed = True)
                    │
                    ▼
              ┌─────────────────┐
              │ Call Parent     │
              │ Function:       │
              │ Handle D Input  │
              └─────────────────┘

┌─────────────────┐
│ IA_D Completed  │
└─────────┬───────┘
          │
          ├───▶ Set bIsDPressed = False
          │
          └───▶ Handle D Input (bPressed = False)
                    │
                    ▼
              ┌─────────────────┐
              │ Call Parent     │
              │ Function:       │
              │ Handle D Input  │
              └─────────────────┘
```

### **IA_Space Event (UE 5.3):**
```
┌─────────────────┐
│IA_Space Started │  ← В UE 5.3 только Started
└─────────┬───────┘
          │
          └───▶ Handle Space Input (bPressed = True)
                    │
                    ▼
              ┌─────────────────┐
              │ Call Parent     │
              │ Function:       │
              │ Handle Space    │
              │ Input           │
              └─────────────────┘
```

---

## 🎭 **СХЕМА ANIMATION BLUEPRINT (UE 5.3)**

### **Variables:**
```
┌─────────────────┐    ┌─────────────────┐
│   bIsStrafing   │    │ StrafeDirection │
│   (Boolean)     │    │    (Float)      │
│                 │    │                 │
│ Default: False  │    │ Default: 0.0    │
└─────────────────┘    └─────────────────┘
```

### **AnimGraph Logic (UE 5.3):**
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Base Animation │    │ Blend Poses by  │    │   Output Pose   │
│  (Idle/Walk)    │───▶│ Bool (UE 5.3)   │───▶│                 │
│                 │    │                 │    │                 │
└─────────────────┘    │ Pose A: Base    │    └─────────────────┘
                       │ Pose B: Strafing│
┌─────────────────┐    │ Active: bIsStrafing
│ Strafing        │───▶│                 │
│ Animation       │    └─────────────────┘
│ (strafing.fbx)  │
│ Root Motion:    │
│ Disabled        │
└─────────────────┘
```

### **Sequence Player Settings (UE 5.3):**
```
┌─────────────────┐
│ Sequence Player │
├─────────────────┤
│ Sequence        │ = strafing
│ Play Rate       │ = 1.0
│ Start Position  │ = 0.0
│ Loop Animation  │ = false
│ Enable Root     │ = false
│ Motion          │
│ Root Motion     │ = Ignore
│ Root Lock       │
└─────────────────┘
```

---

## 🔧 **СХЕМА STRAFE COMPONENT (UE 5.3)**

### **Component Settings:**
```
┌─────────────────┐
│ StrafeComponent │
├─────────────────┤
│ Strafe Distance │ = 200.0
│ Strafe Duration │ = 0.3
│ Strafe Cooldown │ = 1.0
│ Space Double    │ = 0.5
│ Press Window    │
│ Strafe Speed    │ = 400.0
│ Smoothness      │ = 2.0
│ Factor          │
│ Strafe Curve    │ = CF_StrafeMovement
└─────────────────┘
```

### **Input Logic Flow (UE 5.3):**
```
┌─────────────────┐
│ A/D + Space(2x) │
└─────────┬───────┘
          │
          ▼
┌─────────────────┐
│ CheckStrafeCombo│
│ (StrafeComponent)│
└─────────┬───────┘
          │
          ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│ CanStrafe()     │───▶│ StartStrafe     │───▶│ Set bIsStrafing │
│ Check:          │    │ Animation()     │    │ = True          │
│ - Not strafing  │    │                 │    │                 │
│ - Not falling   │    │                 │    │                 │
│ - Cooldown OK   │    │                 │    │                 │
└─────────────────┘    └─────────────────┘    └─────────────────┘
          │                       │
          ▼                       ▼
┌─────────────────┐    ┌─────────────────┐
│ Return False    │    │ Move Character  │
│ (Show Message)  │    │ with Timeline   │
└─────────────────┘    └─────────────────┘
```

---

## 📁 **СТРУКТУРА ФАЙЛОВ (UE 5.3)**

```
Content/BackToZaraysk/
├── Core/
│   ├── Input/
│   │   ├── Actions/
│   │   │   └── Strafe/
│   │   │       ├── IA_A (Digital bool)
│   │   │       ├── IA_D (Digital bool)
│   │   │       └── IA_Space (Digital bool)
│   │   └── IMC_Default (Triggers: Down)
│   └── Data/
│       └── Curves/
│           └── CF_StrafeMovement
├── Characters/
│   └── Player/
│       ├── BP_Player (Started/Completed Events)
│       └── ABP_Player (UE 5.3 AnimGraph)
└── Mannequins/
    └── Animations/
        └── Strafing/
            └── strafing (Root Motion Disabled)
```

---

## 🔧 **ОСОБЕННОСТИ UE 5.3**

### **Input Actions:**
```
┌─────────────────┐
│ Input Action    │
├─────────────────┤
│ Value Type      │ = Digital (bool) ← НОВОЕ
│ Consume Input   │ = false
│ Trigger When    │ = false
│ Paused          │
└─────────────────┘
```

### **Input Mapping Context:**
```
┌─────────────────┐
│ Input Mapping   │
├─────────────────┤
│ Action          │ = IA_A/IA_D/IA_Space
│ Key             │ = A/D/Space Bar
│ Modifiers       │ = (пусто)
│ Triggers        │ = Down ← НОВОЕ
└─────────────────┘
```

### **Blueprint Events:**
```
┌─────────────────┐
│ Input Events    │
├─────────────────┤
│ IA_A Started    │ ← Вместо Pressed
│ IA_A Completed  │ ← Вместо Released
│ IA_D Started    │
│ IA_D Completed  │
│ IA_Space Started│
└─────────────────┘
```

### **Animation Import:**
```
┌─────────────────┐
│ FBX Import      │
├─────────────────┤
│ Skeleton        │ = SK_Mannequin
│ Import          │ = (0,0,0)
│ Translation     │
│ Import Rotation │ = (0,0,0)
│ Import Scale    │ = 1.0
│ Use Default     │ = true ← НОВОЕ
│ Sample Rate     │
│ Import Baked    │ = true ← НОВОЕ
│ Animation       │
└─────────────────┘
```

---

## ✅ **ПРОВЕРОЧНЫЙ СПИСОК (UE 5.3)**

### **Input Actions:**
- [ ] IA_A создан (Digital bool)
- [ ] IA_D создан (Digital bool)  
- [ ] IA_Space создан (Digital bool)
- [ ] Consume Input = false для всех
- [ ] Trigger When Paused = false для всех

### **Input Mapping Context:**
- [ ] IMC_Default содержит маппинг A → IA_A (Triggers: Down)
- [ ] IMC_Default содержит маппинг D → IA_D (Triggers: Down)
- [ ] IMC_Default содержит маппинг Space → IA_Space (Triggers: Down)

### **Blueprint Player:**
- [ ] IA_A Started/Completed события созданы
- [ ] IA_D Started/Completed события созданы
- [ ] IA_Space Started событие создано
- [ ] Handle A/D/Space Input функции созданы
- [ ] Parent Functions вызываются
- [ ] StrafeComponent настроен

### **Animation Blueprint:**
- [ ] bIsStrafing переменная создана
- [ ] StrafeDirection переменная создана
- [ ] Blend Poses by Bool настроен (UE 5.3)
- [ ] Анимация strafing подключена
- [ ] Sequence Player настроен (Play Rate, Root Motion)

### **Animation Import:**
- [ ] strafing.fbx импортирован
- [ ] Skeleton: SK_Mannequin
- [ ] Use Default Sample Rate = true
- [ ] Import Baked Animation = true
- [ ] Root Motion отключен
- [ ] Анимация проигрывается корректно

---

## 🎯 **ГОТОВО ДЛЯ UE 5.3!**

Все настройки адаптированы под особенности UE 5.3 для корректной работы системы стрейфа!


