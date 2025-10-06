# Быстрая настройка анимаций лазания вручную в UE 5.3

## ⚡ **БЫСТРЫЙ СТАРТ - 10 минут**

---

## 📁 **1. Создание анимаций (5 минут)**

### **Создайте папку:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → Animations
Right Click → Miscellaneous → New Folder → "Climbing"
```

### **Создайте 3 анимации:**
```
В папке Climbing:
1. Right Click → Animation → Animation Sequence
   - Name: "AS_Vault"
   - Target Skeleton: SK_Mannequin
   
2. Right Click → Animation → Animation Sequence
   - Name: "AS_Mantle"
   - Target Skeleton: SK_Mannequin
   
3. Right Click → Animation → Animation Sequence
   - Name: "AS_LedgeClimb"
   - Target Skeleton: SK_Mannequin
```

### **Настройте каждую анимацию:**
```
Откройте анимацию (Double Click) → Details Panel (справа):
- Sequence Length: 2.0 (в секции Basic)
- Enable Root Motion: ✅ (в секции Root Motion)
- Root Motion Root Lock: Unlocked (в секции Root Motion)
- Compression: Key Reduce (рекомендуется для UE 5.3)
- Save (Ctrl + S)
```

---

## 🎮 **2. Настройка ABP_Player (5 минут)**

### **Откройте ABP_Player:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → ABP_Player
```

### **Создайте State Machine:**
```
1. Right Click in AnimGraph → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph (рядом с другими нодами)
4. Connect: Output Pose → ClimbingStateMachine → Output Pose
   (кликните на выходе State Machine и протяните к Output Pose)
```

### **Создайте состояния:**
```
Double Click ClimbingStateMachine:
- Right Click → Add State → "Vaulting"
- Right Click → Add State → "Mantling"
- Right Click → Add State → "LedgeClimbing"
```

### **Подключите анимации:**
```
Vaulting State:
- Drag AS_Vault → Connect to Output Pose
- Enable Root Motion: ✅

Mantling State:
- Drag AS_Mantle → Connect to Output Pose
- Enable Root Motion: ✅

LedgeClimbing State:
- Drag AS_LedgeClimb → Connect to Output Pose
- Enable Root Motion: ✅
```

### **Настройте переходы:**
```
В окне State Machine:
Locomotion → Vaulting:
- Right Click на Locomotion → Add Transition → Click на Vaulting
- Select transition arrow → Details Panel:
  - Transition Rule: bIsVaulting == true
  - Transition Time: 0.2
  - Blend Mode: Linear

Locomotion → Mantling:
- Right Click на Locomotion → Add Transition → Click на Mantling
- Select transition arrow → Details Panel:
  - Transition Rule: bIsMantling == true
  - Transition Time: 0.15
  - Blend Mode: Linear

Locomotion → LedgeClimbing:
- Right Click на Locomotion → Add Transition → Click на LedgeClimbing
- Select transition arrow → Details Panel:
  - Transition Rule: bIsLedgeClimbing == true
  - Transition Time: 0.1
  - Blend Mode: Linear

Обратные переходы:
Vaulting → Locomotion: bIsVaulting == false, Time: 0.3
Mantling → Locomotion: bIsMantling == false, Time: 0.25
LedgeClimbing → Locomotion: bIsLedgeClimbing == false, Time: 0.2
```

---

## 🚀 **Особенности UE 5.3**

### **Важные изменения в интерфейсе:**
- **New Folder** теперь в подменю **Miscellaneous**
- **Animation Sequence** может быть в подменю **Animation**
- **Details Panel** имеет новую структуру секций
- **Compression** рекомендуется устанавливать в **Key Reduce**
- **State Machine** открывается в отдельном окне

### **Рекомендуемые настройки:**
- **Sequence Length**: 2.0 секунды
- **Enable Root Motion**: ✅ (обязательно)
- **Root Motion Root Lock**: Unlocked
- **Compression**: Key Reduce (для оптимизации)
- **Transition Time**: 0.1-0.3 секунды
- **Blend Mode**: Linear

---

## 🧪 **3. Тестирование (1 минута)**

### **Compile и Save:**
```
1. Compile ABP_Player
2. Save All (Ctrl + Shift + S)
```

### **Тест в игре:**
```
1. Play → New Editor Window
2. Подойдите к препятствию
3. Нажмите Space для лазания
4. Наблюдайте переключение анимаций
```

---

## ✅ **ГОТОВО! (10 минут)**

### **Результат:**
- ✅ **3 анимации лазания созданы**
- ✅ **State Machine настроена**
- ✅ **Переходы настроены**
- ✅ **Root Motion включен**
- ✅ **Система готова к использованию**

### **Что работает:**
- 🎮 **Vaulting** - перепрыгивание через препятствия
- 🎮 **Mantling** - взбирание на препятствия
- 🎮 **LedgeClimbing** - взбирание и спуск с краев
- 🎮 **Плавные переходы** между состояниями
- 🎮 **Автоматическое переключение** по типу препятствия

---

## 🔧 **Если что-то не работает:**

### **Проверьте:**
1. **Target Skeleton** у анимаций = SK_Mannequin
2. **Enable Root Motion** включен у всех анимаций
3. **Переходы настроены** с правильными условиями
4. **ABP_Player скомпилирован** без ошибок

### **Debug информация:**
```
В игре должно появиться:
bIsClimbing: true
bIsVaulting: true (для перепрыгивания)
bIsMantling: true (для взбирания)
bIsLedgeClimbing: true (для взбирания и спуска)
```

---

## 🚀 **Анимации лазания готовы!**

**Теперь ваш персонаж может красиво лазать по препятствиям!** 🎮

**Время настройки: 10 минут**
**Результат: Полнофункциональная система анимаций лазания**
