# Как работает переменная bIsVaulting

## 🎯 **Цепочка событий**

---

## 📋 **Полный цикл установки bIsVaulting**

### **1. Обнаружение препятствия (DetectObstacle)**
```cpp
📍 Местоположение: Source/BackToZaraysk/Components/ObstacleClimbingComponent.cpp

Функция: DetectObstacle()
Строка: ~79

CurrentObstacle = Obstacle;
CurrentClimbType = Obstacle.ClimbType;  // ← Здесь задается тип лазания
bIsClimbing = true;
```

**Что происходит:**
- Система обнаруживает препятствие перед персонажем
- Определяет **тип препятствия** (Vault, Climb, ClimbOver)
- Сохраняет тип в `CurrentClimbType`
- Устанавливает `bIsClimbing = true`

---

### **2. Обновление AnimInstance (NativeUpdateAnimation)**
```cpp
📍 Местоположение: Source/BackToZaraysk/Characters/Animations/BTZBaseCharacterAnimInstance.cpp

Функция: NativeUpdateAnimation()
Строки: 140-165

bIsClimbing = PlayerCharacter->ObstacleClimbingComponent->bIsClimbing;

switch (PlayerCharacter->ObstacleClimbingComponent->CurrentClimbType)
{
case EObstacleClimbType::Vault:
    bIsVaulting = true;      // ← Здесь задается bIsVaulting
    bIsMantling = false;
    bIsLedgeClimbing = false;
    break;
case EObstacleClimbType::Climb:
    bIsVaulting = false;
    bIsMantling = true;
    bIsLedgeClimbing = false;
    break;
case EObstacleClimbType::ClimbOver:
    bIsVaulting = false;
    bIsMantling = false;
    bIsLedgeClimbing = true;
    break;
default:
    bIsVaulting = false;
    bIsMantling = false;
    bIsLedgeClimbing = false;
    break;
}
```

**Что происходит:**
- AnimInstance проверяет `CurrentClimbType` из `ObstacleClimbingComponent`
- В зависимости от типа препятствия устанавливает соответствующую переменную:
  - **Vault** → `bIsVaulting = true`
  - **Climb** → `bIsMantling = true`
  - **ClimbOver** → `bIsLedgeClimbing = true`

---

### **3. Использование в ABP_Player (State Machine)**
```
📍 Местоположение: ABP_Player → State Machine

Переход: Locomotion → Vaulting
Условие: bIsVaulting == true

┌─────────────┐           ┌──────────┐
│ Locomotion  │──────────>│ Vaulting │
└─────────────┘           └──────────┘
    Условие: bIsVaulting == true
```

**Что происходит:**
- State Machine проверяет значение `bIsVaulting`
- Если `bIsVaulting == true`, происходит переход к состоянию Vaulting
- Проигрывается анимация перепрыгивания

---

### **4. Сброс после завершения (CompleteClimbing)**
```cpp
📍 Местоположение: Source/BackToZaraysk/Components/ObstacleClimbingComponent.cpp

Функция: CompleteClimbing()
Строка: ~661

bIsClimbing = false;
CurrentClimbType = EObstacleClimbType::None;  // ← Сброс типа
```

**Что происходит:**
- После завершения лазания `CurrentClimbType` сбрасывается в `None`
- В следующем обновлении AnimInstance установит `bIsVaulting = false`
- State Machine переключится обратно в Locomotion

---

## 🔄 **Полный жизненный цикл**

### **Схема потока данных:**
```
1. Игрок нажимает Space
   ↓
2. ObstacleClimbingComponent::TryStartClimb()
   ↓
3. ObstacleClimbingComponent::DetectObstacle()
   ↓
4. CurrentClimbType = Obstacle.ClimbType (Vault/Climb/ClimbOver)
   ↓
5. bIsClimbing = true
   ↓
6. BTZBaseCharacterAnimInstance::NativeUpdateAnimation() (каждый кадр)
   ↓
7. Проверка CurrentClimbType через switch
   ↓
8. Установка bIsVaulting/bIsMantling/bIsLedgeClimbing = true
   ↓
9. ABP_Player State Machine проверяет bIsVaulting
   ↓
10. Переход Locomotion → Vaulting
   ↓
11. Проигрывается анимация AS_Vault
   ↓
12. После завершения: CompleteClimbing()
   ↓
13. CurrentClimbType = None, bIsClimbing = false
   ↓
14. AnimInstance устанавливает bIsVaulting = false
   ↓
15. State Machine переключается обратно: Vaulting → Locomotion
```

---

## 📊 **Типы препятствий и соответствующие переменные**

### **Таблица соответствий:**
| Тип препятствия | CurrentClimbType | bIsVaulting | bIsMantling | bIsLedgeClimbing |
|----------------|------------------|-------------|-------------|------------------|
| **Vault**      | EObstacleClimbType::Vault | ✅ true | ❌ false | ❌ false |
| **Climb**      | EObstacleClimbType::Climb | ❌ false | ✅ true | ❌ false |
| **ClimbOver**  | EObstacleClimbType::ClimbOver | ❌ false | ❌ false | ✅ true |
| **None**       | EObstacleClimbType::None | ❌ false | ❌ false | ❌ false |

---

## 🔍 **Где искать значение в Runtime**

### **1. В C++ коде (Debug):**
```cpp
// В BTZBaseCharacterAnimInstance.cpp добавьте:
if (GEngine)
{
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, 
        FString::Printf(TEXT("bIsVaulting: %s"), bIsVaulting ? TEXT("true") : TEXT("false")));
}
```

### **2. В Blueprint:**
```
ABP_Player → EventGraph → Print String
Переменная: bIsVaulting
```

### **3. В игре:**
```
При лазании на экране появится:
bIsVaulting: true (для перепрыгивания)
bIsMantling: true (для взбирания)
bIsLedgeClimbing: true (для взбирания и спуска)
```

---

## ⚙️ **Условия для установки bIsVaulting = true**

### **Необходимые условия:**
1. ✅ **bIsClimbing** = true (лазание активно)
2. ✅ **CurrentClimbType** = EObstacleClimbType::Vault
3. ✅ **PlayerCharacter** имеет **ObstacleClimbingComponent**
4. ✅ **ObstacleClimbingComponent** инициализирован
5. ✅ **AnimInstance** обновляется (NativeUpdateAnimation вызывается)

### **Проверка в коде:**
```cpp
// В BTZBaseCharacterAnimInstance::NativeUpdateAnimation()
if (CachedBaseCharacter.IsValid())
{
    if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
    {
        if (PlayerCharacter->ObstacleClimbingComponent)
        {
            bIsClimbing = PlayerCharacter->ObstacleClimbingComponent->bIsClimbing;
            
            // Если bIsClimbing = true и CurrentClimbType = Vault
            if (bIsClimbing && CurrentClimbType == EObstacleClimbType::Vault)
            {
                bIsVaulting = true;  // ← Устанавливается здесь
            }
        }
    }
}
```

---

## 🚨 **Частые проблемы**

### **Проблема 1: bIsVaulting всегда false**
```
Причины:
❌ ObstacleClimbingComponent не инициализирован
❌ CurrentClimbType не установлен в Vault
❌ bIsClimbing = false
❌ AnimInstance не обновляется

Решение:
✅ Проверьте, что компонент создан в PlayerCharacter
✅ Проверьте DetectObstacle() - правильно ли определяется тип
✅ Добавьте debug-вывод в NativeUpdateAnimation()
```

### **Проблема 2: bIsVaulting не сбрасывается**
```
Причины:
❌ CompleteClimbing() не вызывается
❌ CurrentClimbType не сбрасывается в None
❌ Логика сброса в AnimInstance не работает

Решение:
✅ Проверьте вызов CompleteClimbing() после анимации
✅ Проверьте условие сброса (строка 173-179)
✅ Убедитесь, что bIsClimbing сбрасывается в false
```

### **Проблема 3: Неправильный тип лазания**
```
Причины:
❌ DetectObstacle() определяет неправильный тип
❌ Настройки препятствия некорректны
❌ Логика определения типа сломана

Решение:
✅ Проверьте параметры препятствия (высота, ширина)
✅ Добавьте debug-вывод в DetectObstacle()
✅ Проверьте условия определения типа
```

---

## 📝 **Резюме**

### **Ключевые моменты:**
1. **bIsVaulting** устанавливается в **BTZBaseCharacterAnimInstance::NativeUpdateAnimation()**
2. **Значение** зависит от **CurrentClimbType** в **ObstacleClimbingComponent**
3. **CurrentClimbType** устанавливается в **DetectObstacle()** при обнаружении препятствия
4. **bIsVaulting** используется в **ABP_Player State Machine** для переходов
5. **Сброс** происходит в **CompleteClimbing()** или **CancelClimb()**

### **Путь данных:**
```
DetectObstacle() → CurrentClimbType → NativeUpdateAnimation() → bIsVaulting → State Machine → Анимация
```

### **Файлы для проверки:**
1. `ObstacleClimbingComponent.cpp` - установка CurrentClimbType
2. `BTZBaseCharacterAnimInstance.cpp` - установка bIsVaulting
3. `ABP_Player` - использование bIsVaulting в переходах

---

## 🎯 **Теперь вы знаете, как и когда задается bIsVaulting!** 🎮





