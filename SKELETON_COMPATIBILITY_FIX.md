# Исправление ошибки "skeletons are not compatible" в ABP_Player

## 🚨 **ПРОБЛЕМА**
При попытке подключить анимации лазания из Free Animation Library в State Machine ABP_Player появляется ошибка:
```
"skeletons are not compatible"
```

## 🔍 **ПРИЧИНА**
Анимации из Free Animation Library могут быть созданы для другой версии скелета `SK_Mannequin` или иметь несовместимые настройки.

## ✅ **РЕШЕНИЕ**

### **Способ 1: Автоматическое исправление (РЕКОМЕНДУЕТСЯ)**

1. **Откройте Unreal Editor**
2. **Нажмите `Ctrl + Shift + P`** для консоли Python
3. **Скопируйте и вставьте эту команду:**
```python
exec(open('Content/Python/FixSkeletonCompatibility.py').read())
```

### **Способ 2: Создание совместимых анимаций**

1. **Откройте Unreal Editor**
2. **Нажмите `Ctrl + Shift + P`** для консоли Python
3. **Скопируйте и вставьте эту команду:**
```python
exec(open('Content/Python/CreateCompatibleClimbingAnimations.py').read())
```

---

## 🎯 **ЧТО ДЕЛАЮТ СКРИПТЫ**

### **FixSkeletonCompatibility.py:**
- ✅ **Исправляет скелет** существующих анимаций лазания
- ✅ **Проверяет ABP_Player** на правильность скелета
- ✅ **Создает резервные копии** оригинальных анимаций
- ✅ **Заменяет несовместимые** анимации на исправленные

### **CreateCompatibleClimbingAnimations.py:**
- ✅ **Создает новые анимации** лазания с правильным скелетом
- ✅ **Создает Blend Space** для анимаций Mantle
- ✅ **Настраивает Root Motion** для всех анимаций
- ✅ **Проверяет совместимость** созданных анимаций

---

## 📁 **СОЗДАННЫЕ АНИМАЦИИ**

После выполнения скриптов будут созданы:

```
Content/BackToZaraysk/Core/Characters/Player/Animations/Climbing/
├── AS_Vault.uasset (перепрыгивание)
├── AS_Mantle_1M.uasset (взбирание 1м)
├── AS_Mantle_2M.uasset (взбирание 2м)
├── AS_LedgeClimb_Up.uasset (взбирание на край)
├── AS_LedgeClimb_Idle.uasset (ожидание на краю)
├── AS_LedgeClimb_Down.uasset (спуск с края)
└── BS_Mantle.uasset (Blend Space для взбирания)
```

---

## 🎮 **ИСПОЛЬЗОВАНИЕ В ABP_PLAYER**

### **1. Откройте ABP_Player:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → ABP_Player
```

### **2. Создайте State Machine:**
```
1. Right Click → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph
```

### **3. Создайте состояния:**
```
ClimbingStateMachine:
├── Entry → Locomotion (Default)
├── Locomotion (существующее движение)
├── Vaulting (новое)
├── Mantling (новое)
└── LedgeClimbing (новое)
```

### **4. Подключите анимации:**

#### **Vaulting State:**
```
Double Click Vaulting:
├── Drag AS_Vault из Content Browser
├── Connect: AS_Vault → Output Pose
├── Enable Root Motion: ✅
└── Root Motion Mode: Root Motion From Everything
```

#### **Mantling State:**
```
Double Click Mantling:
├── Drag BS_Mantle из Content Browser
├── Connect: BS_Mantle → Output Pose
├── Input: ClimbingHeight
├── Enable Root Motion: ✅
└── Play Rate: ClimbingSpeed / 100.0
```

#### **LedgeClimbing State:**
```
Double Click LedgeClimbing:
├── Drag AS_LedgeClimb_Up из Content Browser
├── Connect: AS_LedgeClimb_Up → Output Pose
├── Enable Root Motion: ✅
└── Root Motion Mode: Root Motion From Everything
```

### **5. Настройте переходы:**
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

---

## 🔧 **РУЧНОЕ ИСПРАВЛЕНИЕ (если скрипты не работают)**

### **1. Проверьте скелет ABP_Player:**
```
1. Откройте ABP_Player
2. В Details panel найдите Target Skeleton
3. Должно быть: SK_Mannequin
```

### **2. Проверьте скелет анимаций:**
```
1. Откройте анимацию лазания
2. В Details panel найдите Skeleton
3. Должно быть: SK_Mannequin
```

### **3. Исправьте несовместимые анимации:**
```
1. Right Click на анимации → Reimport
2. Выберите правильный Skeleton: SK_Mannequin
3. Нажмите Import
```

### **4. Создайте новые анимации:**
```
1. Right Click в Content Browser → Animation → Animation Sequence
2. Target Skeleton: SK_Mannequin
3. Создайте простую анимацию лазания
4. Enable Root Motion: ✅
```

---

## 🧪 **ТЕСТИРОВАНИЕ**

### **1. Compile и Save:**
```
1. Compile ABP_Player
2. Save All
3. Check for errors
```

### **2. Тест в игре:**
```
1. Play → New Editor Window
2. Подойдите к препятствию
3. Нажмите Space для лазания
4. Наблюдайте переключение анимаций
```

### **3. Debug информация:**
```
В игре должно появиться:
bIsClimbing: true
bIsVaulting: true (для перепрыгивания)
bIsMantling: true (для взбирания)
bIsLedgeClimbing: true (для взбирания и спуска)
```

---

## ✅ **РЕЗУЛЬТАТ**

После исправления:

✅ **Ошибка "skeletons are not compatible" устранена**
✅ **Анимации лазания подключены к ABP_Player**
✅ **State Machine работает корректно**
✅ **Переходы между состояниями настроены**
✅ **Root Motion включен для всех анимаций**
✅ **Система готова к использованию в игре**

---

## 🚀 **СЛЕДУЮЩИЕ ШАГИ**

1. **Протестируйте** анимации лазания в игре
2. **Настройте** переходы между состояниями
3. **Добавьте** звуковые эффекты
4. **Оптимизируйте** производительность
5. **Создайте** дополнительные анимации при необходимости

**Проблема с несовместимостью скелетов решена!** 🎯





