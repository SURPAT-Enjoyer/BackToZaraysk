import unreal

def auto_fix_strafe_ue53():
    """Автоматическое исправление системы стрейфа для UE 5.3"""
    print("=== АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ СТРЕЙФА ДЛЯ UE 5.3 ===")
    
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
        
        # 5. Финальная проверка
        print("\n5. ФИНАЛЬНАЯ ПРОВЕРКА:")
        final_verification()
        
        print("\n✅ АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ ЗАВЕРШЕНО!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА АВТОМАТИЧЕСКОГО ИСПРАВЛЕНИЯ: {e}")
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
        else:
            print(f"✓ {action_name} уже существует")
            # Обновляем настройки для UE 5.3
            existing.value_type = value_type
            existing.consume_input = False
            existing.trigger_when_paused = False
            unreal.EditorAssetLibrary.save_asset(existing.get_path_name())
            print(f"✓ {action_name} обновлен для UE 5.3")

def create_strafe_curve_ue53():
    """Создание кривой стрейфа для UE 5.3"""
    curve_path = "/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement"
    existing = unreal.load_asset(curve_path)
    
    if not existing:
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
            if hasattr(new_curve, 'pre_infinity'):
                new_curve.pre_infinity = unreal.RichCurveExtrapolation.RCCE_CYCLE
            if hasattr(new_curve, 'post_infinity'):
                new_curve.post_infinity = unreal.RichCurveExtrapolation.RCCE_CYCLE
            
            unreal.EditorAssetLibrary.save_asset(new_curve.get_path_name())
            print("✓ Создана CF_StrafeMovement с настройками UE 5.3")
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
    for mapping in imc_default.mappings:
        if mapping.action and any("IA_A" in str(mapping.action) or "IA_D" in str(mapping.action) or "IA_Space" in str(mapping.action)):
            existing_mappings.append(str(mapping.action))
    
    # Создаем недостающие маппинги
    mappings_to_create = [
        (ia_a, unreal.Key.A, "IA_A"),
        (ia_d, unreal.Key.D, "IA_D"),
        (ia_space, unreal.Key.Space_Bar, "IA_Space")
    ]
    
    for action, key, name in mappings_to_create:
        if name not in existing_mappings:
            new_mapping = unreal.EnhancedActionKeyMapping()
            new_mapping.action = action
            new_mapping.key = key
            new_mapping.modifiers = []
            
            # Для UE 5.3 используем новый формат triggers
            if hasattr(new_mapping, 'triggers'):
                new_mapping.triggers = [unreal.InputTriggerDown()]
            else:
                new_mapping.trigger = unreal.TriggerType.IMPULSE
            
            imc_default.mappings.append(new_mapping)
            print(f"✓ Добавлен маппинг {name} -> {key}")
        else:
            print(f"✓ Маппинг {name} уже существует")
    
    unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
    print("✓ IMC_Default обновлен для UE 5.3")

def setup_blueprint_player_ue53():
    """Настройка Blueprint Player для UE 5.3"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    # Проверяем StrafeComponent
    strafe_component = None
    for component in bp_player.components:
        if component.get_name() == "StrafeComponent":
            strafe_component = component
            break
    
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
            strafe_component.set_editor_property(setting_name, value)
            print(f"  ✓ {setting_name} установлен в {value}")
        
        # Назначаем кривую
        curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
        if curve:
            strafe_component.set_editor_property("StrafeCurve", curve)
            print("  ✓ Strafe Curve назначена")
        else:
            print("  ❌ Strafe Curve не найдена")
        
        print("✓ Параметры StrafeComponent настроены")
    else:
        print("❌ StrafeComponent не найден!")
        print("  РЕШЕНИЕ: Добавьте StrafeComponent вручную в BP_Player")
    
    # Создаем функции если их нет
    functions_to_create = ["Handle A Input", "Handle D Input", "Handle Space Input"]
    for func_name in functions_to_create:
        function_exists = False
        for function in bp_player.functions:
            if function.get_name() == func_name:
                function_exists = True
                break
        
        if not function_exists:
            print(f"❌ Функция {func_name} не найдена")
            print(f"  РЕШЕНИЕ: Создайте функцию {func_name} вручную")
        else:
            print(f"✓ Функция {func_name} найдена")
    
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    print("✓ BP_Player проверен")

def check_and_fix_jump_conflicts():
    """Проверка и исправление конфликтов с Jump"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Ищем конфликты с Space Bar
    space_mappings = []
    jump_mappings = []
    
    for mapping in imc_default.mappings:
        if mapping.key == unreal.Key.Space_Bar:
            if mapping.action:
                action_name = mapping.action.get_name()
                if "Jump" in action_name:
                    jump_mappings.append(mapping)
                elif "IA_Space" in action_name:
                    space_mappings.append(mapping)
    
    if jump_mappings:
        print(f"⚠️ Найдено {len(jump_mappings)} конфликтующих маппингов Jump на Space Bar")
        print("  РЕШЕНИЕ: Измените клавишу для Jump или удалите маппинг")
        
        # Предлагаем изменить клавишу для Jump
        for jump_mapping in jump_mappings:
            if jump_mapping.action:
                action_name = jump_mapping.action.get_name()
                print(f"    - {action_name} использует Space Bar")
                print(f"    РЕКОМЕНДАЦИЯ: Измените клавишу для {action_name} на W или другую клавишу")
    else:
        print("✓ Конфликтов с Jump не найдено")
    
    if space_mappings:
        print(f"✓ Найдено {len(space_mappings)} маппингов IA_Space на Space Bar")
    else:
        print("❌ IA_Space не использует Space Bar!")
        print("  РЕШЕНИЕ: Добавьте маппинг IA_Space -> Space Bar в IMC_Default")

def final_verification():
    """Финальная проверка системы"""
    print("Выполняется финальная проверка...")
    
    # Проверяем Input Actions
    input_actions_ok = True
    actions_to_check = ["IA_A", "IA_D", "IA_Space"]
    
    for action_name in actions_to_check:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        action = unreal.load_asset(action_path)
        if not action:
            print(f"❌ {action_name} не найден")
            input_actions_ok = False
    
    if input_actions_ok:
        print("✓ Все Input Actions найдены")
    
    # Проверяем IMC_Default
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        print("✓ IMC_Default найден")
    else:
        print("❌ IMC_Default не найден")
    
    # Проверяем BP_Player
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("✓ BP_Player найден")
    else:
        print("❌ BP_Player не найден")
    
    # Проверяем кривую
    curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    if curve:
        print("✓ CF_StrafeMovement найдена")
    else:
        print("❌ CF_StrafeMovement не найдена")

def main():
    """Основная функция автоматического исправления"""
    print("🚀 Запуск автоматического исправления стрейфа для UE 5.3...")
    
    try:
        auto_fix_strafe_ue53()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Откройте BP_Player в редакторе")
        print("2. Проверьте Event Graph на наличие Input Events:")
        print("   - IA_A Started/Completed → Handle A Input")
        print("   - IA_D Started/Completed → Handle D Input")
        print("   - IA_Space Started → Handle Space Input")
        print("3. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
        print("4. Если есть конфликты с Jump, измените клавишу для Jump")
        print("5. Соберите проект: .\\build_ue53.bat")
        print("6. Протестируйте: A/D + Space(2x)")
        
        print("\n📋 ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:")
        print("- Если автоматическое исправление не помогло, используйте ручную настройку")
        print("- Инструкции по ручной настройке: FIX_JUMP_INSTEAD_OF_STRAFE.md")
        print("- Быстрая шпаргалка: QUICK_FIX_JUMP_ISSUE.md")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции FIX_JUMP_INSTEAD_OF_STRAFE.md")

if __name__ == "__main__":
    main()


