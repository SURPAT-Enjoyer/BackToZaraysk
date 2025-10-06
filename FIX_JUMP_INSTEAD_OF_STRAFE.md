# 🚨 ИСПРАВЛЕНИЕ ПРОБЛЕМЫ: ПЕРСОНАЖ ПРЫГАЕТ ВМЕСТО СТРЕЙФА

## 🎯 **ПРОБЛЕМА: При удерживании A/D и двойном нажатии Space персонаж просто прыгает влево/вправо вместо стрейфа**

### 🔍 **ДИАГНОСТИКА ПРОБЛЕМЫ:**

#### **Возможные причины:**
1. ❌ **Input Events не подключены** к функциям Handle A/D/Space Input
2. ❌ **Функции Handle A/D/Space Input не вызывают Parent Function**
3. ❌ **StrafeComponent не настроен** правильно
4. ❌ **Space Input конфликтует** с Jump Input
5. ❌ **Input Actions не настроены** как Digital (bool)

---

## 🔧 **ШАГ 1: Диагностика (2 минуты)**

Запустите диагностику:
```bash
python Content/Python/DiagnoseStrafeInputIssue.py
```

**Ожидаемый результат:**
- ✅ Все Input Actions найдены
- ✅ IMC_Default настроен
- ✅ StrafeComponent найден и настроен
- ✅ Функции Handle A/D/Space Input найдены

---

## 🔧 **ШАГ 2: Проверка Input Events в BP_Player (3 минуты)**

### 2.1 Откройте BP_Player

### 2.2 Проверьте Event Graph на наличие Input Events:

#### **Должны быть подключены:**
```
IA_A Started → Handle A Input (True)
IA_A Completed → Handle A Input (False)
IA_D Started → Handle D Input (True)
IA_D Completed → Handle D Input (False)
IA_Space Started → Handle Space Input (True)
```

#### **Если Input Events отсутствуют:**
1. **Правый клик** в Event Graph
2. Введите: `IA_A`
3. Выберите **Add Event for IA_A** → **Started**
4. Подключите к **Handle A Input** с параметром `True`

5. Повторите для **IA_A Completed** с параметром `False`
6. Повторите для **IA_D Started/Completed**
7. Повторите для **IA_Space Started**

---

## 🔧 **ШАГ 3: Проверка функций Handle Input (2 минуты)**

### 3.1 Проверьте функции Handle A/D/Space Input:

#### **В каждой функции должно быть:**
```
Handle A Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle A Input
    └─ Pass: bPressed

Handle D Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle D Input
    └─ Pass: bPressed

Handle Space Input:
├─ Input: bPressed (Boolean)
└─ Call Parent Function: Handle Space Input
    └─ Pass: bPressed
```

#### **Если функции отсутствуют:**
1. В **My Blueprint** → **Functions**
2. Нажмите **+** для добавления функции
3. Назовите: `Handle A Input`
4. Добавьте параметр: `bPressed` (Boolean)
5. **Правый клик** → **Call Parent Function**
6. Найдите **Handle A Input** и подключите параметр

---

## 🔧 **ШАГ 4: Проверка StrafeComponent (2 минуты)**

### 4.1 Выберите StrafeComponent в BP_Player

### 4.2 Проверьте настройки в Details Panel:
```
Strafe Distance: 200.0
Strafe Duration: 0.3
Strafe Cooldown: 1.0
Space Double Press Window: 0.5
Strafe Speed: 400.0
Smoothness Factor: 2.0
Strafe Curve: CF_StrafeMovement
```

### 4.3 Если StrafeComponent отсутствует:
1. В **Components** нажмите **+**
2. Найдите **StrafeComponent**
3. Добавьте его к персонажу

---

## 🔧 **ШАГ 5: Проверка конфликта с Jump (1 минута)**

### 5.1 Проверьте IMC_Default:

#### **Убедитесь, что Space настроен правильно:**
```
IMC_Default → Mappings:
├─ IA_Space → Space Bar (Triggers: Down)
└─ НЕТ ДРУГИХ маппингов на Space Bar
```

#### **Если есть конфликт:**
1. Откройте IMC_Default
2. Найдите все маппинги на **Space Bar**
3. Убедитесь, что только **IA_Space** использует Space Bar
4. Если есть Jump маппинг, удалите его или измените клавишу

---

## 🔧 **ШАГ 6: Проверка Input Actions (1 минута)**

### 6.1 Проверьте настройки Input Actions:

#### **Каждый Input Action должен иметь:**
```
IA_A:
├─ Value Type: Digital (bool)
├─ Consume Input: false
└─ Trigger When Paused: false

IA_D:
├─ Value Type: Digital (bool)
├─ Consume Input: false
└─ Trigger When Paused: false

IA_Space:
├─ Value Type: Digital (bool)
├─ Consume Input: false
└─ Trigger When Paused: false
```

---

## 🔧 **ШАГ 7: Тестирование (1 минута)**

### 7.1 Соберите проект:
```bash
.\build_ue53.bat
```

### 7.2 Запустите игру

### 7.3 Тестируйте:
1. **Удерживайте A** и быстро нажмите **Space дважды**
2. **Удерживайте D** и быстро нажмите **Space дважды**

### 7.4 Проверьте консоль:
Должны появиться сообщения:
- **Cyan**: "Strafe Animation: Starting from..."
- **Green**: "Strafe: Starting Left/Right strafe!"
- **Yellow**: "Strafe Progress: X.X/1.0"

---

## 🚨 **ЭКСТРЕННОЕ ИСПРАВЛЕНИЕ**

Если ничего не помогает:

### 1. Создайте тестовую функцию в BP_Player:
```
Test Strafe:
├─ Call StrafeComponent.TryStrafe
└─ Pass: EStrafeType::Right
```

### 2. Вызовите функцию вручную
- Если работает → проблема в Input Events
- Если не работает → проблема в StrafeComponent

### 3. Проверьте логи в консоли:
- Если сообщений нет → Input Events не подключены
- Если есть сообщения → StrafeComponent работает

---

## ✅ **ПРОВЕРКА РАБОТЫ**

### **Ожидаемый результат:**
- Плавное смещение на 200 см
- Анимация strafing проигрывается
- Сообщения в консоли (Cyan/Green/Yellow)
- Персонаж НЕ прыгает

### **Если персонаж все еще прыгает:**
1. Проверьте, что Space Input не конфликтует с Jump
2. Убедитесь, что все Parent Functions вызываются
3. Проверьте настройки StrafeComponent

---

## 🎯 **ГОТОВО!**

После выполнения всех шагов стрейф должен работать корректно без прыжков!


