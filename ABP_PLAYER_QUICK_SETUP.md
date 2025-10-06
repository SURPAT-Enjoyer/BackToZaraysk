# Быстрая настройка анимаций лазания в ABP_Player (UE 5.3)

## ⚡ **БЫСТРЫЙ СТАРТ - 15 минут**

---

## 🎯 **1. Откройте ABP_Player (2 минуты)**

```
Content Browser → BackToZaraysk → Characters → Player → ABP_Player
```

**Проверьте:**
- ✅ Compile без ошибок
- ✅ Parent Class: BTZBaseCharacterAnimInstance
- ✅ Target Skeleton: SK_Mannequin

---

## 🔧 **2. Создайте State Machine (3 минуты)**

### **2.1 Добавьте State Machine:**
```
1. Right Click → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph
4. Connect: Output Pose → ClimbingStateMachine → Output Pose
```

### **2.2 Создайте состояния:**
```
Double Click ClimbingStateMachine:
├── Entry → Locomotion (Default)
├── Locomotion (существующее движение)
├── Vaulting (новое)
├── Mantling (новое)
└── LedgeClimbing (новое)
```

---

## 🎭 **3. Настройте состояния (5 минут)**

### **3.1 Vaulting State:**
```
Double Click Vaulting:
├── Drag anim_Vault из Content Browser
├── Connect: anim_Vault → Output Pose
├── Enable Root Motion: ✅
└── Root Motion Mode: Root Motion From Everything
```

### **3.2 Mantling State:**
```
Double Click Mantling:
├── Right Click → Animation → Blend Space 1D
├── Name: "BS_Mantle"
├── X Axis: Height (0-200)
├── Sample Points:
│   ├── Height 50 → anim_Mantle_1M_R
│   └── Height 150 → anim_Mantle_2M_R
├── Connect: BS_Mantle → Output Pose
├── Input: ClimbingHeight
├── Enable Root Motion: ✅
└── Play Rate: ClimbingSpeed / 100.0
```

### **3.3 LedgeClimbing State:**
```
Double Click LedgeClimbing:
├── Drag anim_LedgeClimb_ClimbUp
├── Connect: anim_LedgeClimb_ClimbUp → Output Pose
├── Enable Root Motion: ✅
└── Root Motion Mode: Root Motion From Everything
```

---

## 🔄 **4. Настройте переходы (3 минуты)**

### **4.1 Переходы из Locomotion:**
```
Locomotion → Vaulting:
├── Condition: bIsVaulting == true
├── Transition Time: 0.2
└── Blend Mode: Linear

Locomotion → Mantling:
├── Condition: bIsMantling == true
├── Transition Time: 0.15
└── Blend Mode: Linear

Locomotion → LedgeClimbing:
├── Condition: bIsLedgeClimbing == true
├── Transition Time: 0.1
└── Blend Mode: Linear
```

### **4.2 Обратные переходы:**
```
Vaulting → Locomotion:
├── Condition: bIsVaulting == false
├── Transition Time: 0.3
└── Blend Mode: Linear

Mantling → Locomotion:
├── Condition: bIsMantling == false
├── Transition Time: 0.25
└── Blend Mode: Linear

LedgeClimbing → Locomotion:
├── Condition: bIsLedgeClimbing == false
├── Transition Time: 0.2
└── Blend Mode: Linear
```

---

## 🧪 **5. Тестирование (2 минуты)**

### **5.1 Compile и Save:**
```
1. Compile ABP_Player
2. Save All
3. Check for errors
```

### **5.2 Тест в игре:**
```
1. Play → New Editor Window
2. Подойдите к препятствию
3. Нажмите Space для лазания
4. Наблюдайте переключение анимаций
```

---

## ✅ **ГОТОВО! (15 минут)**

### **Результат:**
- ✅ **State Machine** создана и настроена
- ✅ **Анимации** подключены к состояниям
- ✅ **Переходы** настроены с правильными условиями
- ✅ **Root Motion** включен для всех анимаций
- ✅ **Тестирование** прошло успешно

### **Что работает:**
- 🎮 **Vaulting** - перепрыгивание через препятствия
- 🎮 **Mantling** - взбирание на препятствия (1м и 2м)
- 🎮 **LedgeClimbing** - взбирание и спуск с краев
- 🎮 **Плавные переходы** между состояниями
- 🎮 **Автоматическое переключение** по типу препятствия

---

## 🔧 **Дополнительные настройки (опционально)**

### **Debug режим:**
```
ABP_Player → Debug:
├── Enable Debug: ✅
├── Show State Names: ✅
└── Show Transition Names: ✅
```

### **Оптимизация:**
```
Transition Times:
├── Fast transitions: 0.1s
├── Medium transitions: 0.15-0.2s
└── Slow transitions: 0.25-0.3s
```

### **Звуковые эффекты:**
```
В каждом состоянии добавьте:
├── Audio Component
├── Sound Cue
└── Trigger на ключевых кадрах
```

---

## 🎯 **Быстрая проверка:**

### **Checklist:**
- [ ] State Machine создана
- [ ] Состояния настроены
- [ ] Анимации подключены
- [ ] Переходы работают
- [ ] Root Motion включен
- [ ] Тестирование успешно

### **Если что-то не работает:**
1. **Compile** ABP_Player
2. **Check** переменные анимации
3. **Verify** переходы настроены
4. **Test** в игре

---

## 🚀 **Анимации лазания готовы!**

**Теперь ваш персонаж может красиво лазать по препятствиям!** 🎮

**Время настройки: 15 минут**
**Результат: Полнофункциональная система анимаций лазания**





