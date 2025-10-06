# Пошаговое руководство по ручному созданию анимаций лазания в UE 5.3

## 🎯 **ЗАДАЧА**
Создать анимации лазания вручную в Unreal Engine 5.3, так как Python скрипты не работают с новой версией API.

## 📋 **ПОШАГОВЫЕ ИНСТРУКЦИИ**

### **Шаг 1: Создание папки для анимаций**

1. **Откройте Unreal Editor**
2. **Откройте Content Browser**
3. **Перейдите в папку:**
   ```
   BackToZaraysk → Core → Characters → Player → Animations
   ```
4. **Создайте новую папку:**
   - **Right Click** в пустом месте
   - **Miscellaneous** → **New Folder** (в UE 5.3)
   - **Название:** `Climbing`
   - **Нажмите Enter**

### **Шаг 2: Создание анимации AS_Vault**

1. **Откройте папку Climbing**
2. **Right Click** в пустом месте
3. **Animation** → **Animation Sequence** (в UE 5.3 может быть в подменю)
4. **В диалоге создания Animation Sequence:**
   - **Asset Name:** `AS_Vault`
   - **Target Skeleton:** `SK_Mannequin` (выберите из списка)
   - **Package Path:** автоматически заполнится
   - **Нажмите Create**
5. **Откройте созданную анимацию** (Double Click)
6. **В Details Panel (справа) установите:**
   - **Sequence Length:** `2.0` (в секции Basic)
   - **Enable Root Motion:** ✅ (в секции Root Motion)
   - **Root Motion Root Lock:** `Unlocked` (в секции Root Motion)
   - **Compression:** `Key Reduce` (рекомендуется для UE 5.3)
7. **Сохраните:** `Ctrl + S` или кнопка Save в панели инструментов

### **Шаг 3: Создание анимации AS_Mantle**

1. **В папке Climbing**
2. **Right Click** → **Animation** → **Animation Sequence**
3. **В диалоге создания:**
   - **Asset Name:** `AS_Mantle`
   - **Target Skeleton:** `SK_Mannequin`
   - **Нажмите Create**
4. **Откройте созданную анимацию**
5. **В Details Panel установите:**
   - **Sequence Length:** `2.0`
   - **Enable Root Motion:** ✅ (поставьте галочку)
   - **Root Motion Root Lock:** `Unlocked`
6. **Сохраните:** `Ctrl + S`

### **Шаг 4: Создание анимации AS_LedgeClimb**

1. **В папке Climbing**
2. **Right Click** → **Animation** → **Animation Sequence**
3. **В диалоге создания:**
   - **Asset Name:** `AS_LedgeClimb`
   - **Target Skeleton:** `SK_Mannequin`
   - **Нажмите Create**
4. **Откройте созданную анимацию**
5. **В Details Panel установите:**
   - **Sequence Length:** `2.0`
   - **Enable Root Motion:** ✅ (поставьте галочку)
   - **Root Motion Root Lock:** `Unlocked`
6. **Сохраните:** `Ctrl + S`

---

## 🎮 **НАСТРОЙКА ABP_PLAYER**

### **Шаг 5: Открытие ABP_Player**

1. **В Content Browser перейдите:**
   ```
   BackToZaraysk → Core → Characters → Player
   ```
2. **Double Click** на `ABP_Player`
3. **Дождитесь загрузки Animation Blueprint**

### **Шаг 6: Создание State Machine**

1. **В AnimGraph** (вкладка в верхней части ABP_Player)
2. **Right Click** в пустом месте графа
3. **State Machines** → **Add New State Machine** (в UE 5.3)
4. **Название:** `ClimbingStateMachine`
5. **Нажмите Enter** или кликните вне поля ввода
6. **Перетащите** `ClimbingStateMachine` на главный граф (рядом с другими нодами)
7. **Подключите:** `Output Pose` → `ClimbingStateMachine` → `Output Pose`
   - **Кликните** на выходе `ClimbingStateMachine` и протяните к `Output Pose`

### **Шаг 7: Создание состояний**

1. **Double Click** на `ClimbingStateMachine` (откроется новое окно State Machine)
2. **Создайте состояния:**
   - **Right Click** в пустом месте → **Add State** → **Название:** `Vaulting`
   - **Right Click** в пустом месте → **Add State** → **Название:** `Mantling`
   - **Right Click** в пустом месте → **Add State** → **Название:** `LedgeClimbing`
3. **Подключите Entry к Locomotion** (если не подключено):
   - **Кликните** на выходе `Entry` и протяните к `Locomotion`

### **Шаг 8: Подключение анимаций к состояниям**

#### **Vaulting State:**
1. **Double Click** на состояние `Vaulting` (откроется граф состояния)
2. **В Content Browser найдите** `AS_Vault` (в папке Climbing)
3. **Перетащите** `AS_Vault` из Content Browser в граф состояния
4. **Подключите:** `AS_Vault` → `Output Pose` (кликните на выходе анимации и протяните к Output Pose)
5. **Выберите** `AS_Vault` в графе (кликните на ноде)
6. **В Details Panel (справа):**
   - **Enable Root Motion:** ✅ (в секции Root Motion)
   - **Root Motion Mode:** `Root Motion From Everything` (в секции Root Motion)

#### **Mantling State:**
1. **Double Click** на состояние `Mantling`
2. **Перетащите** `AS_Mantle` в граф состояния
3. **Подключите:** `AS_Mantle` → `Output Pose`
4. **Выберите** `AS_Mantle` в графе
5. **В Details Panel:**
   - **Enable Root Motion:** ✅
   - **Root Motion Mode:** `Root Motion From Everything`

#### **LedgeClimbing State:**
1. **Double Click** на состояние `LedgeClimbing`
2. **Перетащите** `AS_LedgeClimb` в граф состояния
3. **Подключите:** `AS_LedgeClimb` → `Output Pose`
4. **Выберите** `AS_LedgeClimb` в графе
5. **В Details Panel:**
   - **Enable Root Motion:** ✅
   - **Root Motion Mode:** `Root Motion From Everything`

### **Шаг 9: Настройка переходов**

#### **Locomotion → Vaulting:**
1. **В окне State Machine** (где создавали состояния)
2. **Right Click** на состоянии `Locomotion`
3. **Add Transition** → **Кликните** на `Vaulting` (появится стрелка)
4. **Выберите переход** (кликните на стрелке между состояниями)
5. **В Details Panel (справа):**
   - **Transition Rule:** `bIsVaulting == true` (в секции Transition Rule)
   - **Transition Time:** `0.2` (в секции Blend Settings)
   - **Blend Mode:** `Linear` (в секции Blend Settings)

#### **Locomotion → Mantling:**
1. **Right Click** на состоянии `Locomotion`
2. **Add Transition** → **Кликните** на `Mantling`
3. **Выберите переход**
4. **В Details Panel:**
   - **Transition Rule:** `bIsMantling == true`
   - **Transition Time:** `0.15`
   - **Blend Mode:** `Linear`

#### **Locomotion → LedgeClimbing:**
1. **Right Click** на состоянии `Locomotion`
2. **Add Transition** → **Кликните** на `LedgeClimbing`
3. **Выберите переход**
4. **В Details Panel:**
   - **Transition Rule:** `bIsLedgeClimbing == true`
   - **Transition Time:** `0.1`
   - **Blend Mode:** `Linear`

#### **Обратные переходы:**

**Vaulting → Locomotion:**
1. **Right Click** на `Vaulting`
2. **Add Transition** → **Кликните** на `Locomotion`
3. **В Details Panel:**
   - **Transition Rule:** `bIsVaulting == false`
   - **Transition Time:** `0.3`
   - **Blend Mode:** `Linear`

**Mantling → Locomotion:**
1. **Right Click** на `Mantling`
2. **Add Transition** → **Кликните** на `Locomotion`
3. **В Details Panel:**
   - **Transition Rule:** `bIsMantling == false`
   - **Transition Time:** `0.25`
   - **Blend Mode:** `Linear`

**LedgeClimbing → Locomotion:**
1. **Right Click** на `LedgeClimbing`
2. **Add Transition** → **Кликните** на `Locomotion`
3. **В Details Panel:**
   - **Transition Rule:** `bIsLedgeClimbing == false`
   - **Transition Time:** `0.2`
   - **Blend Mode:** `Linear`

---

## 🧪 **ТЕСТИРОВАНИЕ**

### **Шаг 10: Компиляция и сохранение**

1. **Compile** ABP_Player:
   - **Кнопка Compile** в верхней панели ABP_Player
   - Или **Right Click** на ABP_Player в Content Browser → **Compile**
2. **Save All** (`Ctrl + Shift + S` или File → Save All)
3. **Проверьте Output Log** на наличие ошибок:
   - **Window** → **Developer Tools** → **Output Log**
   - Ищите ошибки связанные с Animation Blueprint

### **Шаг 11: Тест в игре**

1. **Play** → **New Editor Window**
2. **Подойдите к препятствию**
3. **Нажмите Space** для лазания
4. **Наблюдайте переключение анимаций**

### **Шаг 12: Проверка Debug информации**

В игре должно появиться:
```
bIsClimbing: true
bIsVaulting: true (для перепрыгивания)
bIsMantling: true (для взбирания)
bIsLedgeClimbing: true (для взбирания и спуска)
```

---

## ✅ **РЕЗУЛЬТАТ**

После выполнения всех шагов:

✅ **Анимации лазания созданы с правильным скелетом**
✅ **ABP_Player State Machine настроена**
✅ **Переходы между состояниями настроены**
✅ **Root Motion включен для всех анимаций**
✅ **Система готова к тестированию в игре**

---

## 🚀 **СЛЕДУЮЩИЕ ШАГИ**

1. **Протестируйте** анимации лазания в игре
2. **Настройте** время переходов при необходимости
3. **Добавьте** звуковые эффекты
4. **Создайте** более детализированные анимации
5. **Оптимизируйте** производительность

**Ручное создание анимаций лазания завершено!** 🎯
