# Диагностика: Анимация Vault не проигрывается

## 🔍 **Пошаговая диагностика проблемы**

---

## ✅ **Шаг 1: Проверка debug-сообщений в игре**

### **Что должно быть на экране при лазании:**
```
При нажатии Space над препятствием должны появиться:
✅ "Obstacle Climbing: Started! Type: Vault (перепрыгивание)"
✅ "bIsClimbing: true"
✅ "bIsVaulting: true"
✅ "Vault: Moving to (X, Y, Z)"
```

### **Если этих сообщений НЕТ:**
- ❌ Препятствие не определяется правильно
- ❌ Тип лазания определен неверно
- ❌ CurrentClimbType != Vault

---

## ✅ **Шаг 2: Проверка AnimInstance**

### **Проверьте в BTZBaseCharacterAnimInstance.cpp:**
```cpp
Строки 140-165:

if (PlayerCharacter->ObstacleClimbingComponent)
{
    bIsClimbing = PlayerCharacter->ObstacleClimbingComponent->bIsClimbing;
    
    switch (PlayerCharacter->ObstacleClimbingComponent->CurrentClimbType)
    {
    case EObstacleClimbType::Vault:
        bIsVaulting = true;  // ← Проверьте, выполняется ли эта строка
        break;
    }
}
```

### **Добавьте debug-вывод:**
```cpp
// После строки 146 добавьте:
if (bIsVaulting)
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
            TEXT("AnimInstance: bIsVaulting = true"));
    }
}
```

---

## ✅ **Шаг 3: Проверка ABP_Player**

### **Проверьте State Machine:**

#### **1. Откройте ABP_Player**
```
Content Browser → BackToZaraysk → Core → Characters → Player → ABP_Player
```

#### **2. Проверьте, что ClimbingStateMachine подключена**
```
AnimGraph → ClimbingStateMachine подключена к Output Pose
```

#### **3. Проверьте состояния**
```
Double Click на ClimbingStateMachine
Должны быть состояния:
- Locomotion
- Vaulting
- Mantling
- LedgeClimbing
```

#### **4. Проверьте переход Locomotion → Vaulting**
```
Кликните на стрелку перехода
Details Panel → Transition Rule:
Должно быть: bIsVaulting == true
```

#### **5. Проверьте анимацию в состоянии Vaulting**
```
Double Click на состояние Vaulting
Должна быть подключена AS_Vault к Output Pose
```

---

## ✅ **Шаг 4: Проверка анимации AS_Vault**

### **Проверьте, что анимация существует:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → Animations → Climbing
Должен быть файл: AS_Vault
```

### **Проверьте настройки AS_Vault:**
```
Double Click на AS_Vault
Details Panel:
✅ Target Skeleton: SK_Mannequin
✅ Sequence Length: 2.0
✅ Enable Root Motion: ✅
✅ Root Motion Root Lock: Unlocked
```

### **Если анимация НЕ СУЩЕСТВУЕТ:**
```
Создайте анимацию вручную:
1. Right Click → Animation → Animation Sequence
2. Name: AS_Vault
3. Target Skeleton: SK_Mannequin
4. Create
5. Установите настройки (см. выше)
```

---

## ✅ **Шаг 5: Проверка переменной bIsVaulting**

### **В ABP_Player проверьте переменную:**
```
My Blueprint → Variables
Должна быть: bIsVaulting (Boolean)
```

### **Если переменной НЕТ:**
```
1. Она должна быть унаследована от BTZBaseCharacterAnimInstance
2. Проверьте, что ABP_Player использует правильный Parent Class
3. Details Panel → Parent Class: BTZBaseCharacterAnimInstance
```

---

## ✅ **Шаг 6: Проверка компиляции**

### **Скомпилируйте ABP_Player:**
```
1. Откройте ABP_Player
2. Compile (кнопка в верхней панели)
3. Проверьте Output Log на ошибки
```

### **Если есть ошибки компиляции:**
```
Проверьте:
❌ Неправильные условия переходов
❌ Отсутствующие переменные
❌ Несовместимые анимации
❌ Неправильные типы данных
```

---

## 🔧 **Наиболее вероятные проблемы и решения**

### **Проблема 1: Анимация AS_Vault не создана**
```
Решение:
1. Создайте анимацию вручную (см. MANUAL_ANIMATION_CREATION_STEPS.md)
2. Установите Target Skeleton: SK_Mannequin
3. Включите Root Motion
4. Сохраните
```

### **Проблема 2: Неправильный скелет**
```
Решение:
1. Откройте AS_Vault
2. Details Panel → Basic → Target Skeleton
3. Выберите SK_Mannequin
4. Сохраните
```

### **Проблема 3: State Machine не настроена**
```
Решение:
1. Откройте ABP_Player → ClimbingStateMachine
2. Создайте состояние Vaulting (если нет)
3. Подключите AS_Vault к Output Pose
4. Настройте переход: Locomotion → Vaulting (bIsVaulting == true)
5. Настройте переход: Vaulting → Locomotion (bIsVaulting == false)
6. Compile и Save
```

### **Проблема 4: Переход не срабатывает**
```
Решение:
1. Проверьте условие перехода: bIsVaulting == true (без пробелов)
2. Проверьте Transition Time: 0.2
3. Проверьте Blend Mode: Linear
4. Убедитесь, что bIsVaulting устанавливается в true в AnimInstance
```

### **Проблема 5: bIsVaulting всегда false**
```
Решение:
1. Добавьте debug-вывод в BTZBaseCharacterAnimInstance.cpp
2. Проверьте, что CurrentClimbType = EObstacleClimbType::Vault
3. Проверьте, что ObstacleClimbingComponent инициализирован
4. Пересоберите проект
```

---

## 🛠️ **Быстрое исправление**

### **Если ничего не помогает, выполните полный reset:**

#### **1. Удалите старую анимацию (если есть)**
```
Content Browser → AS_Vault → Right Click → Delete
```

#### **2. Создайте новую анимацию**
```
Right Click → Animation → Animation Sequence
Name: AS_Vault
Target Skeleton: SK_Mannequin
Create
```

#### **3. Настройте анимацию**
```
Details Panel:
- Sequence Length: 2.0
- Enable Root Motion: ✅
- Root Motion Root Lock: Unlocked
- Compression: Key Reduce
Save
```

#### **4. Подключите в State Machine**
```
ABP_Player → ClimbingStateMachine → Vaulting State
Перетащите AS_Vault → Подключите к Output Pose
```

#### **5. Проверьте переход**
```
Locomotion → Vaulting
Transition Rule: bIsVaulting == true
Transition Time: 0.2
Blend Mode: Linear
```

#### **6. Compile и Save**
```
Compile ABP_Player
Save All (Ctrl + Shift + S)
```

#### **7. Пересоберите проект**
```
Закройте Unreal Editor
Запустите build_ue53.bat
Откройте проект
Протестируйте
```

---

## 📊 **Проверочный список**

### **Перед тестированием убедитесь:**
- [ ] **AS_Vault создана** с правильным скелетом (SK_Mannequin)
- [ ] **Enable Root Motion** включен в AS_Vault
- [ ] **State Machine** ClimbingStateMachine существует
- [ ] **Состояние Vaulting** создано и AS_Vault подключена
- [ ] **Переход Locomotion → Vaulting** настроен (bIsVaulting == true)
- [ ] **Переход Vaulting → Locomotion** настроен (bIsVaulting == false)
- [ ] **bIsVaulting** существует в BTZBaseCharacterAnimInstance
- [ ] **ABP_Player скомпилирован** без ошибок
- [ ] **Проект собран** после всех изменений
- [ ] **Debug-сообщения** отображаются в игре

---

## 🎮 **Тестирование**

### **Шаги для тестирования:**
```
1. Play → New Editor Window
2. Подойдите к препятствию (высота ~50-100 см)
3. Нажмите Space
4. Смотрите на debug-сообщения на экране
5. Проверьте, проигрывается ли анимация
```

### **Ожидаемый результат:**
```
На экране должно появиться:
✅ "Obstacle Climbing: Started! Type: Vault"
✅ "bIsClimbing: true"
✅ "bIsVaulting: true"
✅ "AnimInstance: bIsVaulting = true"
✅ Персонаж выполняет анимацию перепрыгивания
✅ Персонаж перемещается за препятствие
```

---

## 🚀 **Если все еще не работает**

### **Соберите информацию:**
1. **Скриншот** State Machine в ABP_Player
2. **Скриншот** настроек AS_Vault
3. **Скриншот** debug-сообщений в игре
4. **Output Log** после компиляции ABP_Player

### **Проверьте код:**
```cpp
// В BTZBaseCharacterAnimInstance.cpp добавьте полный debug:
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, 
        FString::Printf(TEXT("bIsClimbing: %s, bIsVaulting: %s, bIsMantling: %s, bIsLedgeClimbing: %s"),
            bIsClimbing ? TEXT("true") : TEXT("false"),
            bIsVaulting ? TEXT("true") : TEXT("false"),
            bIsMantling ? TEXT("true") : TEXT("false"),
            bIsLedgeClimbing ? TEXT("true") : TEXT("false")));
}
```

### **Пересоберите полностью:**
```
1. Закройте Unreal Editor
2. Удалите папки: Binaries, Intermediate, Saved
3. Запустите build_ue53.bat
4. Откройте проект
5. Compile ABP_Player
6. Save All
7. Протестируйте
```

---

## 🎯 **Анимация Vault заработает!** 🚀





