# 🔧 РУЧНАЯ НАСТРОЙКА СТРЕЙФА С КОМБИНАЦИЯМИ A/D + SPACE

## 🎯 **СИСТЕМА: A/D + Space (одно нажатие) для стрейфа влево/вправо**

### 📋 **ПОШАГОВАЯ ИНСТРУКЦИЯ:**

---

## 🔧 **ШАГ 1: Создание Input Actions (3 минуты)**

### 1.1 Откройте Content Browser
- Нажмите **Content Browser** в нижней части экрана
- Перейдите в папку: `Content/BackToZaraysk/Core/Input/Actions/Strafe/`

### 1.2 Создайте IA_A
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_A`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

### 1.3 Создайте IA_D
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_D`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

### 1.4 Создайте IA_Space
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_Space`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

---

## 🔧 **ШАГ 2: Настройка Input Mapping Context (3 минуты)**

### 2.1 Откройте IMC_Default
1. В Content Browser найдите: `Content/BackToZaraysk/Core/Input/IMC_Default`
2. **Двойной клик** на файле

### 2.2 Добавьте маппинг для IA_A
1. В **Details Panel** найдите секцию **Mappings**
2. Нажмите **+** для добавления нового маппинга
3. В новом маппинге установите:
   - **Action**: `IA_A` (выберите из списка)
   - **Key**: `A` (выберите из списка клавиш)
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`
4. **Сохраните** (Ctrl+S)

### 2.3 Добавьте маппинг для IA_D
1. Нажмите **+** для добавления нового маппинга
2. В новом маппинге установите:
   - **Action**: `IA_D` (выберите из списка)
   - **Key**: `D` (выберите из списка клавиш)
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`
3. **Сохраните** (Ctrl+S)

### 2.4 Добавьте маппинг для IA_Space
1. Нажмите **+** для добавления нового маппинга
2. В новом маппинге установите:
   - **Action**: `IA_Space` (выберите из списка)
   - **Key**: `Space Bar` (выберите из списка клавиш)
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`
3. **Сохраните** (Ctrl+S)

---

## 🔧 **ШАГ 3: Настройка Blueprint Player (5 минут)**

### 3.1 Откройте BP_Player
1. Найдите: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`
2. **Двойной клик** на файле

### 3.2 Создайте Input Events
1. Перейдите в **Event Graph**
2. **Правый клик** в пустом месте
3. Введите: `IA_A`
4. Выберите **Add Event for IA_A**
5. Выберите **Started** и **Completed**

### 3.3 Создайте Input Event для IA_D
1. **Правый клик** в пустом месте
2. Введите: `IA_D`
3. Выберите **Add Event for IA_D**
4. Выберите **Started** и **Completed**

### 3.4 Создайте Input Event для IA_Space
1. **Правый клик** в пустом месте
2. Введите: `IA_Space`
3. Выберите **Add Event for IA_Space**
4. Выберите **Started**

### 3.5 Создайте функции
1. В **My Blueprint** найдите секцию **Functions**
2. Нажмите **+** для добавления функции
3. Назовите: `Handle A Input`
4. Добавьте параметр `bPressed` (Boolean)

5. Создайте функцию: `Handle D Input`
6. Добавьте параметр `bPressed` (Boolean)

7. Создайте функцию: `Handle Space Input`
8. Добавьте параметр `bPressed` (Boolean)

### 3.6 Настройте функции
1. В функции **Handle A Input**:
   - **Правый клик** → **Call Parent Function**
   - Найдите **Handle A Input**
   - Подключите **bPressed** к параметру

2. В функции **Handle D Input**:
   - **Правый клик** → **Call Parent Function**
   - Найдите **Handle D Input**
   - Подключите **bPressed** к параметру

3. В функции **Handle Space Input**:
   - **Правый клик** → **Call Parent Function**
   - Найдите **Handle Space Input**
   - Подключите **bPressed** к параметру

### 3.7 Подключите Input Events к функциям
1. **IA_A Started** → **Handle A Input** (bPressed = True)
2. **IA_A Completed** → **Handle A Input** (bPressed = False)
3. **IA_D Started** → **Handle D Input** (bPressed = True)
4. **IA_D Completed** → **Handle D Input** (bPressed = False)
5. **IA_Space Started** → **Handle Space Input** (bPressed = True)

---

## 🔧 **ШАГ 4: Настройка StrafeComponent (2 минуты)**

### 4.1 Выберите StrafeComponent
1. В **Components** найдите **StrafeComponent**
2. Выберите его

### 4.2 Настройте параметры в Details Panel
```
Strafe Distance: 200.0
Strafe Duration: 0.3
Strafe Cooldown: 1.0
Strafe Speed: 400.0
Smoothness Factor: 2.0
Strafe Curve: CF_StrafeMovement
```

### 4.3 Сохраните BP_Player
- **Compile** (Ctrl+Alt+F11)
- **Сохраните** (Ctrl+S)

---

## 🔧 **ШАГ 5: Тестирование (1 минута)**

### 5.1 Соберите проект
```bash
.\build_ue53.bat
```

### 5.2 Запустите игру

### 5.3 Тестируйте
- **A + Space** → Стрейф влево
- **D + Space** → Стрейф вправо

### 5.4 Проверьте консоль
Должны появиться сообщения:
- **Cyan**: "Strafe Animation: Starting from..."
- **Green**: "Strafe: Starting Left/Right strafe!"
- **Yellow**: "Strafe Progress: X.X/1.0"
- **Green**: "Strafe: Completed!"

---

## ✅ **ПРОВЕРКА РАБОТЫ**

### **Ожидаемый результат:**
- Плавное смещение влево/вправо на 200 см
- Анимация strafing проигрывается
- Сообщения в консоли (Cyan/Green/Yellow)
- Кулдаун 1 секунда между стрейфами

### **Если стрейф не работает:**
1. Проверьте, что все Input Actions созданы и настроены
2. Убедитесь, что маппинги A/D/Space → IA_A/IA_D/IA_Space существуют
3. Проверьте подключение Input Events в BP_Player
4. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function

---

## 🎯 **ГОТОВО!**

После выполнения всех шагов система стрейфа с комбинациями A/D + Space должна работать корректно!

### **Ключевые моменты:**
- **Комбинированное управление**: A + Space / D + Space
- **Одиночное нажатие**: Space активирует стрейф при удержании A/D
- **Двойное направление**: Влево и вправо
- **Настраиваемые параметры**: Расстояние, скорость, кулдаун


