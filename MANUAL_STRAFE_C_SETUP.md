# 🔧 РУЧНАЯ НАСТРОЙКА СТРЕЙФА С КЛАВИШЕЙ C

## 🎯 **СИСТЕМА: Одиночное нажатие клавиши C для стрейфа вправо**

### 📋 **ПОШАГОВАЯ ИНСТРУКЦИЯ:**

---

## 🔧 **ШАГ 1: Создание Input Action (2 минуты)**

### 1.1 Откройте Content Browser
- Нажмите **Content Browser** в нижней части экрана
- Перейдите в папку: `Content/BackToZaraysk/Core/Input/Actions/Strafe/`

### 1.2 Создайте IA_C
1. **Правый клик** в папке Strafe
2. Выберите **Input → Input Action**
3. Назовите: `IA_C`
4. **Двойной клик** на созданном файле
5. В **Details Panel** установите:
   - **Value Type**: `Digital (bool)`
   - **Consume Input**: `false`
   - **Trigger When Paused**: `false`
6. **Сохраните** (Ctrl+S)

---

## 🔧 **ШАГ 2: Настройка Input Mapping Context (2 минуты)**

### 2.1 Откройте IMC_Default
1. В Content Browser найдите: `Content/BackToZaraysk/Core/Input/IMC_Default`
2. **Двойной клик** на файле

### 2.2 Добавьте маппинг для IA_C
1. В **Details Panel** найдите секцию **Mappings**
2. Нажмите **+** для добавления нового маппинга
3. В новом маппинге установите:
   - **Action**: `IA_C` (выберите из списка)
   - **Key**: `C` (выберите из списка клавиш)
   - **Modifiers**: оставьте пустым
   - **Triggers**: `Down`
4. **Сохраните** (Ctrl+S)

---

## 🔧 **ШАГ 3: Настройка Blueprint Player (3 минуты)**

### 3.1 Откройте BP_Player
1. Найдите: `Content/BackToZaraysk/Core/Characters/Player/BP_Player`
2. **Двойной клик** на файле

### 3.2 Создайте Input Event
1. Перейдите в **Event Graph**
2. **Правый клик** в пустом месте
3. Введите: `IA_C`
4. Выберите **Add Event for IA_C**
5. Выберите **Started**

### 3.3 Создайте функцию Handle C Input
1. В **My Blueprint** найдите секцию **Functions**
2. Нажмите **+** для добавления функции
3. Назовите: `Handle C Input`

### 3.4 Настройте функцию Handle C Input
1. В функции **Handle C Input**:
   - **Input**: добавьте параметр `bPressed` (Boolean)
   - **Правый клик** → **Call Parent Function**
   - Найдите **Handle C Input**
   - Подключите **bPressed** к параметру

### 3.5 Подключите Input Event к функции
1. От **IA_C Started** проведите линию к **Handle C Input**
2. Установите **bPressed**: `True`

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
- **Нажмите C** → Стрейф вправо

### 5.4 Проверьте консоль
Должны появиться сообщения:
- **Cyan**: "Strafe Animation: Starting from..."
- **Green**: "Strafe: Starting Right strafe!"
- **Yellow**: "Strafe Progress: X.X/1.0"
- **Green**: "Strafe: Completed!"

---

## ✅ **ПРОВЕРКА РАБОТЫ**

### **Ожидаемый результат:**
- Плавное смещение вправо на 200 см
- Анимация strafing проигрывается
- Сообщения в консоли (Cyan/Green/Yellow)
- Кулдаун 1 секунда между стрейфами

### **Если стрейф не работает:**
1. Проверьте, что IA_C создан и настроен
2. Убедитесь, что маппинг C → IA_C существует
3. Проверьте подключение Input Event в BP_Player
4. Убедитесь, что функция Handle C Input вызывает Parent Function

---

## 🎯 **ГОТОВО!**

После выполнения всех шагов система стрейфа с клавишей C должна работать корректно!

### **Ключевые моменты:**
- **Простое управление**: C → Стрейф вправо
- **Одиночное нажатие**: Нет комбинаций клавиш
- **Быстрый отклик**: Мгновенная активация
- **Настраиваемые параметры**: Расстояние, скорость, кулдаун


