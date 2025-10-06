# Примеры Blueprint кода для ABP_Player - Анимации лазания

## 📋 **Blueprint примеры и логика**

---

## 🎯 **1. Переменные анимации (My Blueprint → Variables)**

### **1.1 Проверка доступных переменных:**
```
✅ bIsClimbing (Boolean) - из BTZBaseCharacterAnimInstance
✅ bIsVaulting (Boolean) - из BTZBaseCharacterAnimInstance  
✅ bIsMantling (Boolean) - из BTZBaseCharacterAnimInstance
✅ bIsLedgeClimbing (Boolean) - из BTZBaseCharacterAnimInstance
✅ ClimbingSpeed (Float) - из BTZBaseCharacterAnimInstance
✅ ClimbingHeight (Float) - из BTZBaseCharacterAnimInstance
```

---

## 🏗️ **2. State Machine структура**

### **2.1 Основная State Machine:**
```
ClimbingStateMachine
├── Entry → Locomotion (Default)
├── Locomotion (Base Movement)
├── Vaulting (Vault Animation)
├── Mantling (Mantle Animation)  
└── LedgeClimbing (Ledge Climb Animation)
```

### **2.2 Создание State Machine:**
```
1. Right Click in Graph → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph
4. Connect: Output Pose → ClimbingStateMachine → Output Pose
```

---

## 🎭 **3. Настройка состояний**

### **3.1 Locomotion State:**
```
Double Click Locomotion State:
├── Input: Base Movement Logic
├── Walk/Run Animations
├── Idle Animation
└── Output: Output Pose
```

### **3.2 Vaulting State:**
```
Double Click Vaulting State:
├── Input: None
├── Animation: anim_Vault
├── Settings:
│   ├── Enable Root Motion: ✅
│   ├── Root Motion Mode: Root Motion From Everything
│   └── Play Rate: 1.0
└── Output: Output Pose
```

### **3.3 Mantling State:**
```
Double Click Mantling State:
├── Input: None
├── Blend Space 1D: MantleBlendSpace
│   ├── X Axis: Height (0.0 - 200.0)
│   ├── Sample Points:
│   │   ├── Height 50.0 → anim_Mantle_1M_R
│   │   └── Height 150.0 → anim_Mantle_2M_R
│   └── Input: ClimbingHeight
├── Settings:
│   ├── Enable Root Motion: ✅
│   ├── Root Motion Mode: Root Motion From Everything
│   └── Play Rate: ClimbingSpeed / 100.0
└── Output: Output Pose
```

### **3.4 LedgeClimbing State:**
```
Double Click LedgeClimbing State:
├── Input: None
├── Sub State Machine: LedgeSubStates
│   ├── ClimbUp → anim_LedgeClimb_ClimbUp
│   ├── Idle → anim_LedgeClimb_Idle
│   └── ClimbDown → anim_LedgeClimbing_Down
├── Settings:
│   ├── Enable Root Motion: ✅
│   ├── Root Motion Mode: Root Motion From Everything
│   └── Play Rate: ClimbingSpeed / 100.0
└── Output: Output Pose
```

---

## 🔄 **4. Настройка переходов (Transitions)**

### **4.1 Locomotion → Vaulting:**
```
Transition Settings:
├── Transition Rule: bIsVaulting == true
├── Transition Time: 0.2
├── Blend Mode: Linear
├── Blend Function: Linear
└── Can Enter Transition: Always
```

### **4.2 Locomotion → Mantling:**
```
Transition Settings:
├── Transition Rule: bIsMantling == true
├── Transition Time: 0.15
├── Blend Mode: Linear
├── Blend Function: Linear
└── Can Enter Transition: Always
```

### **4.3 Locomotion → LedgeClimbing:**
```
Transition Settings:
├── Transition Rule: bIsLedgeClimbing == true
├── Transition Time: 0.1
├── Blend Mode: Linear
├── Blend Function: Linear
└── Can Enter Transition: Always
```

### **4.4 Обратные переходы:**
```
Vaulting → Locomotion:
├── Transition Rule: bIsVaulting == false
├── Transition Time: 0.3
└── Blend Mode: Linear

Mantling → Locomotion:
├── Transition Rule: bIsMantling == false
├── Transition Time: 0.25
└── Blend Mode: Linear

LedgeClimbing → Locomotion:
├── Transition Rule: bIsLedgeClimbing == false
├── Transition Time: 0.2
└── Blend Mode: Linear
```

---

## 🎨 **5. Blend Space настройки**

### **5.1 Создание MantleBlendSpace:**
```
1. Right Click in Content Browser → Animation → Blend Space 1D
2. Name: "BS_Mantle"
3. Target Skeleton: SK_Mannequin
4. Settings:
   ├── X Axis Name: "Height"
   ├── X Axis Min: 0.0
   ├── X Axis Max: 200.0
   └── Interpolation: Linear
```

### **5.2 Добавление анимаций в Blend Space:**
```
Sample Points:
├── Point 1:
│   ├── X Value: 50.0
│   ├── Animation: anim_Mantle_1M_R
│   └── Weight: 1.0
└── Point 2:
    ├── X Value: 150.0
    ├── Animation: anim_Mantle_2M_R
    └── Weight: 1.0
```

### **5.3 Использование в Blueprint:**
```
MantleBlendSpace Node:
├── Input: ClimbingHeight
├── Output: Animation Pose
└── Settings: Play Rate = ClimbingSpeed / 100.0
```

---

## 🔧 **6. Sub State Machine для LedgeClimbing**

### **6.1 Создание LedgeSubStates:**
```
1. In LedgeClimbing State → Add State Machine
2. Name: "LedgeSubStates"
3. States:
   ├── Entry → ClimbUp
   ├── ClimbUp → Idle
   ├── Idle → ClimbDown
   └── ClimbDown → Exit
```

### **6.2 Переходы в Sub State Machine:**
```
ClimbUp → Idle:
├── Transition Rule: Animation Finished
├── Transition Time: 0.1
└── Blend Mode: Linear

Idle → ClimbDown:
├── Transition Rule: bIsLedgeClimbing == false
├── Transition Time: 0.1
└── Blend Mode: Linear
```

---

## 🎮 **7. Настройка Root Motion**

### **7.1 В каждом состоянии лазания:**
```
Animation Settings:
├── Enable Root Motion: ✅
├── Root Motion Mode: Root Motion From Everything
├── Root Motion Root Lock: Unlocked
└── Play Rate: ClimbingSpeed / 100.0
```

### **7.2 Настройка анимаций:**
```
anim_Vault:
├── Root Motion: Enabled
├── Root Motion Root Lock: Unlocked
└── Preview: Check movement

anim_Mantle_1M_R:
├── Root Motion: Enabled
├── Root Motion Root Lock: Unlocked
└── Preview: Check movement

anim_Mantle_2M_R:
├── Root Motion: Enabled
├── Root Motion Root Lock: Unlocked
└── Preview: Check movement
```

---

## 🧪 **8. Debug и тестирование**

### **8.1 Включение Debug:**
```
ABP_Player Settings:
├── Enable Debug: ✅
├── Show State Names: ✅
├── Show Transition Names: ✅
├── Show Active State: ✅
└── Show Transition Rules: ✅
```

### **8.2 Debug Output в игре:**
```
Console Commands:
├── showdebug animation
├── showdebug animtree
└── stat anim
```

### **8.3 Проверка переменных:**
```
In Game Debug:
bIsClimbing: true/false
bIsVaulting: true/false
bIsMantling: true/false
bIsLedgeClimbing: true/false
ClimbingHeight: 0.0 - 200.0
ClimbingSpeed: 0.0 - 200.0
```

---

## 🔧 **9. Оптимизация и настройки**

### **9.1 Настройка переходов:**
```
Transition Optimization:
├── Blend Time: 0.1 - 0.3 seconds
├── Blend Function: Linear (fastest)
├── Custom Blend Curve: If needed
└── Can Enter Transition: Always
```

### **9.2 Настройка анимаций:**
```
Animation Optimization:
├── Compression: Key Reduce (best quality)
├── Root Motion: Enabled
├── Play Rate: Dynamic based on ClimbingSpeed
└── Loop: Disabled for climbing animations
```

### **9.3 Настройка IK:**
```
IK Settings during climbing:
├── LeftFootIKAlpha: 0.0 (disable foot IK)
├── RightFootIKAlpha: 0.0 (disable foot IK)
├── Hand IK: Enable if needed
└── Root Motion: Handle all movement
```

---

## ✅ **10. Финальная проверка**

### **10.1 Compile и Save:**
```
1. Compile ABP_Player
2. Save All
3. Check for errors
4. Test in game
```

### **10.2 Тестирование:**
```
Test Scenarios:
├── Low obstacle (Vault)
├── Medium obstacle (Mantle 1M)
├── High obstacle (Mantle 2M)
├── Ledge obstacle (LedgeClimb)
└── No obstacle (Locomotion)
```

### **10.3 Performance Check:**
```
Performance:
├── Animation Budget: < 5ms
├── State Machine: < 1ms
├── Transitions: < 2ms
└── Total: < 8ms per frame
```

---

## 🎯 **Результат:**

После выполнения всех настроек:

✅ **State Machine готова к использованию**
✅ **Все переходы настроены**
✅ **Root Motion работает корректно**
✅ **Анимации блендятся плавно**
✅ **Debug информация доступна**
✅ **Производительность оптимизирована**

**ABP_Player готов к использованию с анимациями лазания!** 🚀





