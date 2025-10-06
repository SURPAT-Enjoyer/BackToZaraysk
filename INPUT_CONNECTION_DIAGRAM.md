# 🔗 СХЕМА ПОДКЛЮЧЕНИЙ INPUT'ОВ И АНИМАЦИЙ

## 📊 **ОБЩАЯ АРХИТЕКТУРА СИСТЕМЫ**

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   INPUT KEYS    │    │  INPUT ACTIONS   │    │ INPUT MAPPING   │
│                 │    │                  │    │    CONTEXT      │
│  A, D, Space    │───▶│ IA_A, IA_D,      │───▶│   IMC_Default   │
│                 │    │ IA_Space         │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                                         │
                                                         ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   BP_PLAYER     │◀───│  INPUT EVENTS    │◀───│   ENHANCED      │
│                 │    │                  │    │   INPUT SYSTEM  │
│ StrafeComponent │    │ IA_A Pressed/    │    │                 │
│                 │    │ Released, etc.   │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
         │
         ▼
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   ANIMATION     │◀───│  ANIMATION       │◀───│   ANIMATION     │
│   BLUEPRINT     │    │  VARIABLES       │    │   SEQUENCE      │
│                 │    │                  │    │                 │
│ bIsStrafing     │    │ bIsStrafing,     │    │ strafing.fbx    │
│ StrafeDirection │    │ StrafeDirection  │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
```

---

## 🎮 **ПОДРОБНАЯ СХЕМА INPUT EVENTS В BP_PLAYER**

### **IA_A Events:**
```
┌─────────────────┐
│   IA_A Pressed  │
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
│  IA_A Released  │
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

### **IA_D Events:**
```
┌─────────────────┐
│   IA_D Pressed  │
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
│  IA_D Released  │
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

### **IA_Space Event:**
```
┌─────────────────┐
│ IA_Space Pressed│
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

## 🎭 **СХЕМА ANIMATION BLUEPRINT**

### **Variables:**
```
┌─────────────────┐    ┌─────────────────┐
│   bIsStrafing   │    │ StrafeDirection │
│   (Boolean)     │    │    (Float)      │
│                 │    │                 │
│ Default: False  │    │ Default: 0.0    │
└─────────────────┘    └─────────────────┘
```

### **AnimGraph Logic:**
```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│  Base Animation │    │ Blend Poses by  │    │   Output Pose   │
│  (Idle/Walk)    │───▶│ Bool            │───▶│                 │
│                 │    │                 │    │                 │
└─────────────────┘    │ Pose A: Base    │    └─────────────────┘
                       │ Pose B: Strafing│
┌─────────────────┐    │ Active: bIsStrafing
│ Strafing        │───▶│                 │
│ Animation       │    └─────────────────┘
│ (strafing.fbx)  │
└─────────────────┘
```

---

## 🔧 **СХЕМА STRAFE COMPONENT**

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

### **Input Logic Flow:**
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

## 📁 **СТРУКТУРА ФАЙЛОВ**

```
Content/BackToZaraysk/
├── Core/
│   ├── Input/
│   │   ├── Actions/
│   │   │   └── Strafe/
│   │   │       ├── IA_A
│   │   │       ├── IA_D
│   │   │       └── IA_Space
│   │   └── IMC_Default
│   └── Data/
│       └── Curves/
│           └── CF_StrafeMovement
├── Characters/
│   └── Player/
│       ├── BP_Player
│       └── ABP_Player
└── Mannequins/
    └── Animations/
        └── Strafing/
            └── strafing (Animation Sequence)
```

---

## ✅ **ПРОВЕРОЧНЫЙ СПИСОК**

### **Input Actions:**
- [ ] IA_A создан (Boolean)
- [ ] IA_D создан (Boolean)  
- [ ] IA_Space создан (Boolean)

### **Input Mapping Context:**
- [ ] IMC_Default содержит маппинг A → IA_A
- [ ] IMC_Default содержит маппинг D → IA_D
- [ ] IMC_Default содержит маппинг Space → IA_Space

### **Blueprint Player:**
- [ ] IA_A Pressed/Released события созданы
- [ ] IA_D Pressed/Released события созданы
- [ ] IA_Space Pressed событие создано
- [ ] Handle A/D/Space Input функции созданы
- [ ] Parent Functions вызываются
- [ ] StrafeComponent настроен

### **Animation Blueprint:**
- [ ] bIsStrafing переменная создана
- [ ] StrafeDirection переменная создана
- [ ] Blend Poses by Bool настроен
- [ ] Анимация strafing подключена

### **Animation Import:**
- [ ] strafing.fbx импортирован
- [ ] Skeleton: SK_Mannequin
- [ ] Анимация проигрывается корректно


