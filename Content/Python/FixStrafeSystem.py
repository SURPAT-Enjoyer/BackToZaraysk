import unreal

def fix_strafe_system():
    """Автоматическое исправление системы стрейфа"""
    print("=== АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ СТРЕЙФА ===")
    
    # 1. Создаем все необходимые ресурсы
    print("\n1. СОЗДАНИЕ РЕСУРСОВ:")
    
    # Input Actions
    create_input_actions()
    
    # Curve
    create_strafe_curve()
    
    # 2. Настраиваем IMC_Default
    print("\n2. НАСТРОЙКА IMC_DEFAULT:")
    setup_input_mapping_context()
    
    # 3. Настраиваем BP_Player
    print("\n3. НАСТРОЙКА BP_PLAYER:")
    setup_blueprint_player()
    
    # 4. Проверяем ABP_Player
    print("\n4. ПРОВЕРКА ABP_PLAYER:")
    check_animation_blueprint()
    
    print("\n✅ ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ!")

def create_input_actions():
    """Создание Input Actions"""
    actions = [
        ("IA_A", unreal.InputActionValueType.BOOLEAN),
        ("IA_D", unreal.InputActionValueType.BOOLEAN),
        ("IA_Space", unreal.InputActionValueType.BOOLEAN)
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
                unreal.EditorAssetLibrary.save_asset(new_action.get_path_name())
                print(f"✓ Создан {action_name}")
        else:
            print(f"✓ {action_name} уже существует")

def create_strafe_curve():
    """Создание кривой стрейфа"""
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
            unreal.EditorAssetLibrary.save_asset(new_curve.get_path_name())
            print("✓ Создана CF_StrafeMovement")
    else:
        print("✓ CF_StrafeMovement уже существует")

def setup_input_mapping_context():
    """Настройка Input Mapping Context"""
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
            new_mapping.trigger = unreal.TriggerType.IMPULSE
            imc_default.mappings.append(new_mapping)
            print(f"✓ Добавлен маппинг {name} -> {key}")
        else:
            print(f"✓ Маппинг {name} уже существует")
    
    unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
    print("✓ IMC_Default обновлен")

def setup_blueprint_player():
    """Настройка Blueprint Player"""
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
        strafe_component.set_editor_property("StrafeDistance", 200.0)
        strafe_component.set_editor_property("StrafeDuration", 0.3)
        strafe_component.set_editor_property("StrafeCooldown", 1.0)
        strafe_component.set_editor_property("SpaceDoublePressWindow", 0.5)
        strafe_component.set_editor_property("StrafeSpeed", 400.0)
        strafe_component.set_editor_property("SmoothnessFactor", 2.0)
        
        # Назначаем кривую
        curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
        if curve:
            strafe_component.set_editor_property("StrafeCurve", curve)
            print("✓ Strafe Curve назначена")
        
        print("✓ Параметры StrafeComponent настроены")
    else:
        print("❌ StrafeComponent не найден!")
    
    # Создаем функции если их нет
    functions_to_create = ["Handle A Input", "Handle D Input", "Handle Space Input"]
    for func_name in functions_to_create:
        function_exists = False
        for function in bp_player.functions:
            if function.get_name() == func_name:
                function_exists = True
                break
        
        if not function_exists:
            new_function = bp_player.add_function(func_name)
            if new_function:
                print(f"✓ Создана функция {func_name}")
    
    unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
    print("✓ BP_Player обновлен")

def check_animation_blueprint():
    """Проверка Animation Blueprint"""
    abp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/ABP_Player.ABP_Player")
    if not abp_player:
        print("❌ ABP_Player не найден!")
        return
    
    # Проверяем переменные стрейфа
    required_variables = ["bIsStrafing", "StrafeDirection"]
    for var_name in required_variables:
        variable_exists = False
        for variable in abp_player.variables:
            if variable.get_name() == var_name:
                variable_exists = True
                break
        
        if variable_exists:
            print(f"✓ Переменная {var_name} найдена")
        else:
            print(f"❌ Переменная {var_name} НЕ НАЙДЕНА!")
            print(f"  Добавьте переменную {var_name} в AnimBP вручную")
    
    print("✓ ABP_Player проверен")

def main():
    """Основная функция"""
    try:
        fix_strafe_system()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Откройте BP_Player")
        print("2. Подключите Input Events к функциям:")
        print("   - IA_A (Pressed/Released) → Handle A Input")
        print("   - IA_D (Pressed/Released) → Handle D Input")
        print("   - IA_Space (Pressed) → Handle Space Input")
        print("3. В функциях добавьте:")
        print("   - Call Parent Function: Handle A/D/Space Input")
        print("4. Импортируйте анимацию strafing.fbx")
        print("5. Настройте AnimBP для воспроизведения анимации")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")

if __name__ == "__main__":
    main()


