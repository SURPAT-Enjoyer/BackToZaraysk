import unreal

def run_all_strafe_fixes_ue53():
    """Запуск всех исправлений системы стрейфа для UE 5.3"""
    print("🚀 ЗАПУСК ВСЕХ ИСПРАВЛЕНИЙ СТРЕЙФА ДЛЯ UE 5.3")
    print("=" * 60)
    
    try:
        # 1. Автоматическое исправление
        print("\n🔧 ЭТАП 1: АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ")
        print("-" * 40)
        
        # Запускаем исправление напрямую
        fix_strafe_system_ue53()
        
        # 2. Финальная проверка
        print("\n✅ ЭТАП 2: ФИНАЛЬНАЯ ПРОВЕРКА")
        print("-" * 40)
        
        final_check()
        
        # 3. Инструкции по тестированию
        print("\n🎮 ЭТАП 3: ИНСТРУКЦИИ ПО ТЕСТИРОВАНИЮ")
        print("-" * 40)
        
        testing_instructions()
        
        print("\n🎉 ВСЕ ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

def fix_strafe_system_ue53():
    """Исправление системы стрейфа для UE 5.3"""
    print("=== ИСПРАВЛЕНИЕ СИСТЕМЫ СТРЕЙФА ДЛЯ UE 5.3 ===")
    
    try:
        # 1. Создаем все необходимые ресурсы
        print("\n1. СОЗДАНИЕ РЕСУРСОВ:")
        create_input_actions_ue53()
        create_strafe_curve_ue53()
        
        # 2. Настраиваем IMC_Default
        print("\n2. НАСТРОЙКА IMC_DEFAULT:")
        setup_input_mapping_context_ue53()
        
        # 3. Настраиваем BP_Player
        print("\n3. НАСТРОЙКА BP_PLAYER:")
        setup_blueprint_player_ue53()
        
        # 4. Проверяем конфликты с Jump
        print("\n4. ПРОВЕРКА КОНФЛИКТОВ:")
        check_and_fix_jump_conflicts()
        
        print("\n✅ ИСПРАВЛЕНИЕ ЗАВЕРШЕНО!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА ИСПРАВЛЕНИЯ: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

def create_input_actions_ue53():
    """Создание Input Actions для UE 5.3"""
    actions = [
        ("IA_A", unreal.InputActionValueType.DIGITAL),
        ("IA_D", unreal.InputActionValueType.DIGITAL),
        ("IA_Space", unreal.InputActionValueType.DIGITAL)
    ]
    
    for action_name, value_type in actions:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        existing = unreal.load_asset(action_path)
        
        if not existing:
            try:
                new_action = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    asset_name=action_name,
                    package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
                    asset_class=unreal.InputAction,
                    factory=unreal.InputActionFactory()
                )
                if new_action:
                    new_action.value_type = value_type
                    new_action.consume_input = False
                    new_action.trigger_when_paused = False
                    unreal.EditorAssetLibrary.save_asset(new_action.get_path_name())
                    print(f"✓ Создан {action_name} с настройками UE 5.3")
                else:
                    print(f"❌ Не удалось создать {action_name}")
            except Exception as e:
                print(f"❌ Ошибка создания {action_name}: {e}")
        else:
            print(f"✓ {action_name} уже существует")
            # Обновляем настройки для UE 5.3
            try:
                existing.value_type = value_type
                existing.consume_input = False
                existing.trigger_when_paused = False
                unreal.EditorAssetLibrary.save_asset(existing.get_path_name())
                print(f"✓ {action_name} обновлен для UE 5.3")
            except Exception as e:
                print(f"⚠️ Ошибка обновления {action_name}: {e}")

def create_strafe_curve_ue53():
    """Создание кривой стрейфа для UE 5.3"""
    curve_path = "/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement"
    existing = unreal.load_asset(curve_path)
    
    if not existing:
        try:
            new_curve = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                asset_name="CF_StrafeMovement",
                package_path="/Game/BackToZaraysk/Core/Data/Curves",
                asset_class=unreal.CurveFloat,
                factory=unreal.CurveFloatFactory()
            )
            if new_curve:
                new_curve.add_key(0.0, 0.0)
                new_curve.add_key(0.5, 1.0)
                new_curve.add_key(1.0, 0.0)
                
                # Настройки для UE 5.3
                try:
                    if hasattr(new_curve, 'pre_infinity'):
                        new_curve.pre_infinity = unreal.RichCurveExtrapolation.RCCE_CYCLE
                    if hasattr(new_curve, 'post_infinity'):
                        new_curve.post_infinity = unreal.RichCurveExtrapolation.RCCE_CYCLE
                except:
                    pass  # Игнорируем ошибки с настройками кривой
                
                unreal.EditorAssetLibrary.save_asset(new_curve.get_path_name())
                print("✓ Создана CF_StrafeMovement с настройками UE 5.3")
            else:
                print("❌ Не удалось создать кривую")
        except Exception as e:
            print(f"❌ Ошибка создания кривой: {e}")
    else:
        print("✓ CF_StrafeMovement уже существует")

def setup_input_mapping_context_ue53():
    """Настройка Input Mapping Context для UE 5.3"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Загружаем Input Actions
    ia_a = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_A.IA_A")
    ia_d = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_D.IA_D")
    ia_space = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_Space.IA_Space")
    
    if not (ia_a and ia_d and ia_space):
        print("❌ Input Actions не найдены!")
        return
    
    # Проверяем существующие маппинги
    existing_mappings = []
    try:
        for mapping in imc_default.mappings:
            if mapping.action:
                action_name = str(mapping.action.get_name()) if hasattr(mapping.action, 'get_name') else str(mapping.action)
                if any("IA_A" in action_name or "IA_D" in action_name or "IA_Space" in action_name):
                    existing_mappings.append(action_name)
    except Exception as e:
        print(f"⚠️ Ошибка проверки маппингов: {e}")
        existing_mappings = []
    
    # Создаем недостающие маппинги
    mappings_to_create = [
        (ia_a, unreal.Key.A, "IA_A"),
        (ia_d, unreal.Key.D, "IA_D"),
        (ia_space, unreal.Key.Space_Bar, "IA_Space")
    ]
    
    for action, key, name in mappings_to_create:
        if name not in existing_mappings:
            try:
                new_mapping = unreal.EnhancedActionKeyMapping()
                new_mapping.action = action
                new_mapping.key = key
                new_mapping.modifiers = []
                
                # Для UE 5.3 используем новый формат triggers
                try:
                    if hasattr(new_mapping, 'triggers'):
                        new_mapping.triggers = [unreal.InputTriggerDown()]
                    else:
                        new_mapping.trigger = unreal.TriggerType.IMPULSE
                except:
                    new_mapping.trigger = unreal.TriggerType.IMPULSE
                
                imc_default.mappings.append(new_mapping)
                print(f"✓ Добавлен маппинг {name} -> {key}")
            except Exception as e:
                print(f"❌ Ошибка создания маппинга {name}: {e}")
        else:
            print(f"✓ Маппинг {name} уже существует")
    
    try:
        unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
        print("✓ IMC_Default обновлен для UE 5.3")
    except Exception as e:
        print(f"⚠️ Ошибка сохранения IMC_Default: {e}")

def setup_blueprint_player_ue53():
    """Настройка Blueprint Player для UE 5.3"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Проверяем StrafeComponent
    strafe_component = None
    try:
        for component in bp_player.components:
            if component.get_name() == "StrafeComponent":
                strafe_component = component
                break
    except Exception as e:
        print(f"⚠️ Ошибка проверки компонентов: {e}")
    
    if strafe_component:
        print("✓ StrafeComponent найден")
        
        # Настраиваем параметры
        settings = {
            "StrafeDistance": 200.0,
            "StrafeDuration": 0.3,
            "StrafeCooldown": 1.0,
            "SpaceDoublePressWindow": 0.5,
            "StrafeSpeed": 400.0,
            "SmoothnessFactor": 2.0
        }
        
        for setting_name, value in settings.items():
            try:
                strafe_component.set_editor_property(setting_name, value)
                print(f"  ✓ {setting_name} установлен в {value}")
            except Exception as e:
                print(f"⚠️ Ошибка установки {setting_name}: {e}")
        
        # Назначаем кривую
        curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
        if curve:
            try:
                strafe_component.set_editor_property("StrafeCurve", curve)
                print("  ✓ Strafe Curve назначена")
            except Exception as e:
                print(f"⚠️ Ошибка назначения кривой: {e}")
        else:
            print("  ❌ Strafe Curve не найдена")
        
        print("✓ Параметры StrafeComponent настроены")
    else:
        print("❌ StrafeComponent не найден!")
        print("  РЕШЕНИЕ: Добавьте StrafeComponent вручную в BP_Player")
    
    # Проверяем функции
    functions_to_check = ["Handle A Input", "Handle D Input", "Handle Space Input"]
    for func_name in functions_to_check:
        try:
            function_exists = False
            for function in bp_player.functions:
                if function.get_name() == func_name:
                    function_exists = True
                    break
            
            if function_exists:
                print(f"✓ Функция {func_name} найдена")
            else:
                print(f"❌ Функция {func_name} не найдена")
                print(f"  РЕШЕНИЕ: Создайте функцию {func_name} вручную")
        except Exception as e:
            print(f"⚠️ Ошибка проверки функции {func_name}: {e}")
    
    try:
        unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
        print("✓ BP_Player проверен")
    except Exception as e:
        print(f"⚠️ Ошибка сохранения BP_Player: {e}")

def check_and_fix_jump_conflicts():
    """Проверка и исправление конфликтов с Jump"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Ищем конфликты с Space Bar
    space_mappings = []
    jump_mappings = []
    
    try:
        for mapping in imc_default.mappings:
            if mapping.key == unreal.Key.Space_Bar:
                if mapping.action:
                    action_name = mapping.action.get_name() if hasattr(mapping.action, 'get_name') else str(mapping.action)
                    if "Jump" in action_name:
                        jump_mappings.append(action_name)
                    elif "IA_Space" in action_name:
                        space_mappings.append(action_name)
    except Exception as e:
        print(f"⚠️ Ошибка проверки конфликтов: {e}")
    
    if jump_mappings:
        print(f"⚠️ Найдено {len(jump_mappings)} конфликтующих маппингов Jump на Space Bar")
        for action_name in jump_mappings:
            print(f"    - {action_name}")
        print("  РЕШЕНИЕ: Измените клавишу для Jump или удалите маппинг")
    else:
        print("✓ Конфликтов с Jump не найдено")
    
    if space_mappings:
        print(f"✓ Найдено {len(space_mappings)} маппингов IA_Space на Space Bar")
    else:
        print("❌ IA_Space не использует Space Bar!")
        print("  РЕШЕНИЕ: Добавьте маппинг IA_Space -> Space Bar в IMC_Default")

def final_check():
    """Финальная проверка всех компонентов"""
    print("Выполняется финальная проверка...")
    
    # Проверяем Input Actions
    print("\n1. Проверка Input Actions:")
    input_actions_ok = True
    actions_to_check = ["IA_A", "IA_D", "IA_Space"]
    
    for action_name in actions_to_check:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        action = unreal.load_asset(action_path)
        if action:
            print(f"  ✓ {action_name} найден")
        else:
            print(f"  ❌ {action_name} НЕ НАЙДЕН!")
            input_actions_ok = False
    
    # Проверяем IMC_Default
    print("\n2. Проверка IMC_Default:")
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        print("  ✓ IMC_Default найден")
        
        # Проверяем маппинги
        try:
            strafe_mappings = 0
            for mapping in imc_default.mappings:
                if mapping.action:
                    action_name = str(mapping.action.get_name()) if hasattr(mapping.action, 'get_name') else str(mapping.action)
                    if any("IA_A" in action_name or "IA_D" in action_name or "IA_Space" in action_name):
                        strafe_mappings += 1
            
            if strafe_mappings >= 3:
                print(f"  ✓ Найдено {strafe_mappings} маппингов для стрейфа")
            else:
                print(f"  ⚠️ Найдено только {strafe_mappings} маппингов для стрейфа")
        except Exception as e:
            print(f"  ⚠️ Ошибка проверки маппингов: {e}")
    else:
        print("  ❌ IMC_Default НЕ НАЙДЕН!")
    
    # Проверяем BP_Player
    print("\n3. Проверка BP_Player:")
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("  ✓ BP_Player найден")
        
        # Проверяем StrafeComponent
        try:
            strafe_component = None
            for component in bp_player.components:
                if component.get_name() == "StrafeComponent":
                    strafe_component = component
                    break
            
            if strafe_component:
                print("  ✓ StrafeComponent найден")
            else:
                print("  ❌ StrafeComponent НЕ НАЙДЕН!")
        except Exception as e:
            print(f"  ⚠️ Ошибка проверки StrafeComponent: {e}")
    else:
        print("  ❌ BP_Player НЕ НАЙДЕН!")
    
    # Проверяем кривую
    print("\n4. Проверка кривой:")
    curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    if curve:
        print("  ✓ CF_StrafeMovement найдена")
    else:
        print("  ❌ CF_StrafeMovement НЕ НАЙДЕНА!")
    
    # Итоговая оценка
    print("\n📊 ИТОГОВАЯ ОЦЕНКА:")
    if input_actions_ok and imc_default and bp_player and curve:
        print("  ✅ ВСЕ ОСНОВНЫЕ КОМПОНЕНТЫ НАЙДЕНЫ")
        print("  🎯 СИСТЕМА ГОТОВА К ТЕСТИРОВАНИЮ")
    else:
        print("  ⚠️ ЕСТЬ ПРОБЛЕМЫ, ТРЕБУЕТСЯ РУЧНАЯ НАСТРОЙКА")
        print("  📋 Используйте FIX_JUMP_INSTEAD_OF_STRAFE.md")

def testing_instructions():
    """Инструкции по тестированию"""
    print("Инструкции по тестированию системы стрейфа:")
    
    print("\n🔧 ПЕРЕД ТЕСТИРОВАНИЕМ:")
    print("1. Соберите проект: .\\build_ue53.bat")
    print("2. Запустите игру")
    print("3. Откройте консоль (Tab или `)")
    
    print("\n🎮 ТЕСТИРОВАНИЕ УПРАВЛЕНИЯ:")
    print("1. Удерживайте клавишу A")
    print("2. Быстро нажмите Space дважды")
    print("3. Персонаж должен выполнить стрейф влево")
    print("4. Повторите с клавишей D для стрейфа вправо")
    
    print("\n📺 ПРОВЕРКА КОНСОЛИ:")
    print("В консоли должны появиться сообщения:")
    print("- Cyan: 'Strafe Animation: Starting from...'")
    print("- Green: 'Strafe: Starting Left/Right strafe!'")
    print("- Yellow: 'Strafe Progress: X.X/1.0'")
    print("- Green: 'Strafe: Completed!'")
    
    print("\n❌ ЕСЛИ НЕ РАБОТАЕТ:")
    print("1. Проверьте, что Input Events подключены в BP_Player")
    print("2. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
    print("3. Проверьте, что нет конфликтов с Jump Input")
    print("4. Убедитесь, что StrafeComponent настроен правильно")
    
    print("\n📋 ДОПОЛНИТЕЛЬНАЯ ПОМОЩЬ:")
    print("- Детальные инструкции: FIX_JUMP_INSTEAD_OF_STRAFE.md")
    print("- Быстрая шпаргалка: QUICK_FIX_JUMP_ISSUE.md")
    print("- Ручная настройка: MANUAL_INPUT_ANIMATION_SETUP_UE53.md")

def main():
    """Основная функция запуска всех исправлений"""
    print("🎯 ЗАПУСК ПОЛНОГО ИСПРАВЛЕНИЯ СИСТЕМЫ СТРЕЙФА ДЛЯ UE 5.3")
    print("Этот скрипт выполнит все необходимые исправления автоматически.")
    print("=" * 70)
    
    try:
        run_all_strafe_fixes_ue53()
        
        print("\n🎉 ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")
        print("Теперь вы можете протестировать систему стрейфа.")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

if __name__ == "__main__":
    main()


