# Настройка системы стрейфа в редакторе UE5

## 🎮 **1. Проверка Input Mapping Context**

### **Где найти:**
1. Откройте **Edit → Project Settings**
2. Перейдите в раздел **Input → Bindings**
3. Найдите **Input Mapping Context** (или создайте новый)

### **Что проверить:**
Убедитесь, что есть следующие привязки:

```
Action Mappings:
├── StrafeLeft (A)
├── StrafeRight (D)  
├── StrafeSpace (Space)
├── LeanLeft (Q)
├── LeanRight (E)
└── ClimbObstacle (C)

Axis Mappings:
├── MoveForward (W/S)
├── MoveRight (A/D)
├── Turn (Mouse X)
└── LookUp (Mouse Y)
```

### **Если привязок нет - создайте:**
1. Нажмите **+** рядом с Action Mappings
2. Добавьте новые действия:
   - **StrafeLeft** → привязать к клавише **A**
   - **StrafeRight** → привязать к клавише **D**
   - **StrafeSpace** → привязать к клавише **Space**

---

## 🎭 **2. Проверка PlayerController**

### **Где найти:**
1. Откройте **Content Browser**
2. Найдите **BP_PlayerController** или создайте новый
3. Откройте Blueprint редактор

### **Что проверить:**
1. **Class Settings** → **Parent Class** должен быть `ABTZPlayerController`
2. **Event Graph** → убедитесь что есть события:
   - `StartStrafeLeft`
   - `StopStrafeLeft` 
   - `StartStrafeRight`
   - `StopStrafeRight`
   - `StartStrafeSpace`
   - `StopStrafeSpace`

### **Если событий нет:**
1. Нажмите **+** в Event Graph
2. Добавьте **Custom Events** с именами выше
3. Подключите их к соответствующим **Input Actions**

---

## 🏃 **3. Проверка PlayerCharacter**

### **Где найти:**
1. Откройте **BP_Player** в Content Browser
2. Откройте Blueprint редактор

### **Что проверить:**

#### **Components:**
Убедитесь что есть компоненты:
- ✅ **StrafeComponent** (тип: `UStrafeComponent`)
- ✅ **InventoryComponent** (тип: `UInventoryComponent`)
- ✅ **ObstacleClimbingComponent** (тип: `UObstacleClimbingComponent`)

#### **Class Settings:**
- **Parent Class** должен быть `APlayerCharacter`

#### **Если компонентов нет:**
1. Нажмите **Add Component**
2. Найдите и добавьте:
   - `StrafeComponent`
   - `InventoryComponent`
   - `ObstacleClimbingComponent`

---

## ⚙️ **4. Настройка StrafeComponent**

### **Где найти:**
1. В **BP_Player** → **Components** → выберите **StrafeComponent**
2. В **Details** панели найдите секцию **Strafe**

### **Настройки для проверки:**

```
Strafe Settings:
├── Strafe Distance: 200.0
├── Strafe Duration: 0.3
├── Strafe Cooldown: 1.0
├── Strafe Speed: 400.0
└── Smoothness Factor: 2.0

Strafe State (только для чтения):
├── Is Strafing: false
├── Current Strafe Type: None
└── Current Strafe Info: (детали стрейфа)
```

### **Рекомендуемые значения:**
- **Strafe Distance**: 150-300 (расстояние в см)
- **Strafe Duration**: 0.2-0.5 (секунды)
- **Strafe Cooldown**: 0.5-2.0 (секунды)
- **Strafe Speed**: 300-600 (скорость анимации)

---

## 🎯 **5. Проверка GameMode**

### **Где найти:**
1. **Edit → Project Settings**
2. **Maps & Modes → Default Modes**
3. **Default Pawn Class**

### **Что проверить:**
- **Default Pawn Class** должен быть `BP_Player`
- **Player Controller Class** должен быть `BP_PlayerController`

---

## 🔧 **6. Проверка Animation Blueprint**

### **Где найти:**
1. Откройте **ABP_Player** в Content Browser
2. Откройте Animation Blueprint редактор

### **Что проверить:**

#### **Variables:**
Убедитесь что есть переменные:
- ✅ `LeftFootEffectorLocation` (Vector)
- ✅ `RightFootEffectorLocation` (Vector)
- ✅ `LeftFootIKAlpha` (Float)
- ✅ `RightFootIKAlpha` (Float)

#### **Class Settings:**
- **Parent Class** должен быть `UBTZBaseCharacterAnimInstance`

#### **Если переменных нет:**
1. В **My Blueprint** → **Variables** → нажмите **+**
2. Добавьте переменные с типами выше
3. Установите **Variable Type** и **Default Value**

---

## 🧪 **7. Тестирование в редакторе**

### **Запуск тестов:**
1. Нажмите **Play** в редакторе
2. Используйте следующие комбинации:

```
Тестовые комбинации:
├── A + Space → стрейф влево
├── D + Space → стрейф вправо
├── Space (одиночно) → прыжок
├── Q (зажать) → наклон влево
├── E (зажать) → наклон вправо
└── C → преодоление препятствий
```

### **Что должно происходить:**
- ✅ Плавное боковое движение при стрейфе
- ✅ Отсутствие конфликтов с обычным движением
- ✅ Debug сообщения в левом верхнем углу
- ✅ Корректная работа ИК ног на неровностях

---

## 🐛 **8. Отладка проблем**

### **Если стрейф не работает:**

#### **Проверьте консоль:**
1. Нажмите **`** (тильда) для открытия консоли
2. Ищите сообщения:
   - `"Strafe: Starting Left/Right strafe!"`
   - `"Strafe: Cannot strafe now!"`
   - `"MoveRight: Blocked during strafe"`

#### **Проверьте Input:**
1. **Window → Developer Tools → Input Debugger**
2. Убедитесь что клавиши регистрируются

#### **Проверьте компоненты:**
1. В **World Outliner** выберите персонажа
2. В **Details** проверьте что все компоненты активны

### **Если есть ошибки компиляции:**
1. **Window → Developer Tools → Output Log**
2. Ищите ошибки связанные с:
   - `StrafeComponent`
   - `BTZPlayerController`
   - `PlayerCharacter`

---

## 📋 **9. Чек-лист настройки**

### **Обязательные проверки:**
- [ ] Input Mapping Context настроен
- [ ] PlayerController использует правильный класс
- [ ] PlayerCharacter имеет все компоненты
- [ ] StrafeComponent настроен корректно
- [ ] Animation Blueprint имеет ИК переменные
- [ ] GameMode использует правильные классы
- [ ] Нет ошибок компиляции
- [ ] Тестирование в Play режиме прошло успешно

### **Дополнительные настройки:**
- [ ] Звуки для стрейфа
- [ ] Эффекты частиц
- [ ] Анимации стрейфа в ABP
- [ ] Настройка кривых движения

---

## 🎯 **10. Быстрая диагностика**

### **Команды консоли для отладки:**
```
showdebug ai          - показать debug информацию
showdebug animation   - показать анимационную отладку
showdebug collision   - показать коллизии
showdebug input       - показать ввод
```

### **Если ничего не работает:**
1. Перезапустите редактор
2. Перекомпилируйте проект
3. Проверьте что все C++ классы загружены
4. Убедитесь что нет конфликтов версий

---

**После выполнения всех проверок система стрейфа должна работать корректно!** 🎮



