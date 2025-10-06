# Подробное руководство по настройке анимаций лазания в ABP_Player для UE 5.3

## 📋 **Содержание:**
1. [Подготовка к работе](#подготовка-к-работе)
2. [Настройка переменных анимации](#настройка-переменных-анимации)
3. [Создание State Machine](#создание-state-machine)
4. [Настройка состояний лазания](#настройка-состояний-лазания)
5. [Подключение анимаций](#подключение-анимаций)
6. [Настройка переходов](#настройка-переходов)
7. [Блендинг и Root Motion](#блендинг-и-root-motion)
8. [Тестирование и отладка](#тестирование-и-отладка)

---

## 🎯 **1. Подготовка к работе**

### **1.1 Откройте ABP_Player:**
```
Content Browser → BackToZaraysk → Characters → Player → ABP_Player
```

### **1.2 Проверьте настройки:**
- **Target Skeleton**: SK_Mannequin (или ваш скелет персонажа)
- **Parent Class**: BTZBaseCharacterAnimInstance
- **Compile**: Убедитесь, что BP скомпилирован без ошибок

### **1.3 Создайте резервную копию:**
- **Правый клик** на ABP_Player → **Duplicate**
- Назовите **ABP_Player_Backup**

---

## 🔧 **2. Настройка переменных анимации**

### **2.1 Проверьте доступные переменные:**
В **My Blueprint** → **Variables** должны быть доступны:

```
✅ bIsClimbing (bool)
✅ bIsVaulting (bool) 
✅ bIsMantling (bool)
✅ bIsLedgeClimbing (bool)
✅ ClimbingSpeed (float)
✅ ClimbingHeight (float)
```

### **2.2 Если переменных нет:**
1. **Compile** ABP_Player
2. **Refresh** Variables panel
3. Перезапустите редактор если нужно

---

## 🏗️ **3. Создание State Machine**

### **3.1 Создайте State Machine:**
1. **Правый клик** в графе → **State Machines** → **Add New State Machine**
2. Назовите **"ClimbingStateMachine"**
3. **Drag & Drop** на главный граф

### **3.2 Создайте основные состояния:**
```
📁 ClimbingStateMachine
├── 🟢 Locomotion (основное движение)
├── 🔵 Vaulting (перепрыгивание)
├── 🟡 Mantling (взбирание)
└── 🟣 LedgeClimbing (взбирание и спуск)
```

### **3.3 Настройте State Machine:**
1. **Double-click** на ClimbingStateMachine
2. **Добавьте состояния:**
   - **Entry** → **Locomotion** (по умолчанию)
   - **Vaulting** (новое состояние)
   - **Mantling** (новое состояние)
   - **LedgeClimbing** (новое состояние)

---

## 🎭 **4. Настройка состояний лазания**

### **4.1 Состояние Locomotion:**
1. **Double-click** на **Locomotion**
2. **Подключите** существующую логику движения:
   - Walk/Run animations
   - Idle animations
   - Jump animations

### **4.2 Состояние Vaulting:**
1. **Double-click** на **Vaulting**
2. **Добавьте анимацию:**
   - **Drag** `anim_Vault` из Content Browser
   - **Подключите** к **Output Pose**

### **4.3 Состояние Mantling:**
1. **Double-click** на **Mantling**
2. **Создайте Blend Space для высоты:**
   - **Правый клик** → **Animation** → **Blend Space 1D**
   - Назовите **"MantleBlendSpace"**
3. **Настройте Blend Space:**
   - **X Axis**: Height (0.0 - 200.0)
   - **Sample Points:**
     - Height = 50.0 → `anim_Mantle_1M_R`
     - Height = 150.0 → `anim_Mantle_2M_R`

### **4.4 Состояние LedgeClimbing:**
1. **Double-click** на **LedgeClimbing**
2. **Создайте под-State Machine:**
   - **Add State Machine** → **"LedgeSubStates"**
3. **В LedgeSubStates добавьте:**
   - **ClimbUp** → `anim_LedgeClimb_ClimbUp`
   - **Idle** → `anim_LedgeClimb_Idle`
   - **ClimbDown** → `anim_LedgeClimbing_Down`

---

## 🎬 **5. Подключение анимаций**

### **5.1 Импорт анимаций:**
```
Content Browser → FreeAnimationLibrary → Animations:
├── Vault → anim_Vault.uasset
├── Mantle → anim_Mantle_1M_R.uasset, anim_Mantle_2M_R.uasset
└── LedgeClimb → anim_LedgeClimb_*.uasset
```

### **5.2 Настройка анимаций в состояниях:**

#### **Vaulting State:**
```
anim_Vault → Output Pose
```

#### **Mantling State:**
```
MantleBlendSpace → Output Pose
├── Height Input: ClimbingHeight
└── Sample Points: [50.0 → 1M, 150.0 → 2M]
```

#### **LedgeClimbing State:**
```
LedgeSubStates → Output Pose
├── ClimbUp → anim_LedgeClimb_ClimbUp
├── Idle → anim_LedgeClimb_Idle
└── ClimbDown → anim_LedgeClimbing_Down
```

---

## 🔄 **6. Настройка переходов**

### **6.1 Переходы из Locomotion:**
1. **Locomotion** → **Vaulting**
   - **Condition**: `bIsVaulting == true`
   - **Transition Time**: 0.2s
   - **Blend Mode**: Linear

2. **Locomotion** → **Mantling**
   - **Condition**: `bIsMantling == true`
   - **Transition Time**: 0.15s
   - **Blend Mode**: Linear

3. **Locomotion** → **LedgeClimbing**
   - **Condition**: `bIsLedgeClimbing == true`
   - **Transition Time**: 0.1s
   - **Blend Mode**: Linear

### **6.2 Переходы обратно в Locomotion:**
1. **Vaulting** → **Locomotion**
   - **Condition**: `bIsVaulting == false`
   - **Transition Time**: 0.3s

2. **Mantling** → **Locomotion**
   - **Condition**: `bIsMantling == false`
   - **Transition Time**: 0.25s

3. **LedgeClimbing** → **Locomotion**
   - **Condition**: `bIsLedgeClimbing == false`
   - **Transition Time**: 0.2s

### **6.3 Переходы между состояниями лазания:**
```
Vaulting ↔ Mantling (если нужно)
Mantling ↔ LedgeClimbing (если нужно)
```

---

## 🎨 **7. Блендинг и Root Motion**

### **7.1 Настройка Root Motion:**
1. **В каждом состоянии лазания:**
   - **Enable Root Motion**: ✅
   - **Root Motion Mode**: Root Motion From Everything

2. **Для анимаций:**
   - **anim_Vault**: Root Motion включен
   - **anim_Mantle_***: Root Motion включен
   - **anim_LedgeClimb_***: Root Motion включен

### **7.2 Настройка блендинга:**
1. **MantleBlendSpace:**
   - **Interpolation**: Linear
   - **Extrapolation**: Clamp

2. **Transition Settings:**
   - **Blend Time**: 0.15-0.3s
   - **Blend Function**: Linear
   - **Custom Blend Curve**: При необходимости

### **7.3 Настройка скорости анимации:**
```
Play Rate = ClimbingSpeed / BaseSpeed
```
- **BaseSpeed**: 100.0 (базовая скорость)
- **ClimbingSpeed**: из переменной AnimInstance

---

## 🧪 **8. Тестирование и отладка**

### **8.1 Компиляция:**
1. **Compile** ABP_Player
2. **Save** все изменения
3. **Проверьте** отсутствие ошибок

### **8.2 Тестирование в игре:**
1. **Play** → **New Editor Window**
2. **Подойдите к препятствию**
3. **Нажмите Space** для лазания
4. **Наблюдайте** переключение анимаций

### **8.3 Отладка:**
1. **В ABP_Player** → **Debug**:
   - **Enable Debug**: ✅
   - **Show State Names**: ✅
   - **Show Transition Names**: ✅

2. **В игре** наблюдайте:
   - Переключение состояний
   - Время переходов
   - Корректность анимаций

### **8.4 Проверка переменных:**
```
Debug Output:
bIsClimbing: true
bIsVaulting: true
bIsMantling: false
bIsLedgeClimbing: false
ClimbingHeight: 150.0
ClimbingSpeed: 120.0
```

---

## 🔧 **9. Дополнительные настройки**

### **9.1 IK во время лазания:**
1. **В состояниях лазания:**
   - **LeftFootIKAlpha**: 0.0 (отключить IK ног)
   - **RightFootIKAlpha**: 0.0 (отключить IK ног)

### **9.2 Настройка камеры:**
1. **В PlayerController** добавьте логику:
   - **Climbing Camera Offset**
   - **Smooth Camera Transition**

### **9.3 Звуковые эффекты:**
1. **Добавьте Audio Components** в состояния
2. **Trigger Sounds** на ключевых кадрах

---

## ✅ **10. Финальная проверка**

### **10.1 Checklist:**
- [ ] Все переменные анимации доступны
- [ ] State Machine создана и настроена
- [ ] Анимации подключены к состояниям
- [ ] Переходы настроены с правильными условиями
- [ ] Root Motion включен
- [ ] Блендинг работает корректно
- [ ] Тестирование в игре прошло успешно

### **10.2 Оптимизация:**
- [ ] Удалены неиспользуемые анимации
- [ ] Оптимизированы переходы
- [ ] Настроены правильные времена блендинга

---

## 🎯 **Результат:**

После выполнения всех шагов у вас будет:

✅ **Полнофункциональная система анимаций лазания**
✅ **Плавные переходы между состояниями**
✅ **Автоматическое переключение по типу препятствия**
✅ **Корректный Root Motion**
✅ **Готовность к использованию в игре**

**Анимации лазания готовы к использованию!** 🚀





