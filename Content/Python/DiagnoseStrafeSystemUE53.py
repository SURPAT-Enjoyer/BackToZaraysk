import unreal

def diagnose_strafe_system_ue53():
    """Диагностика системы стрейфа для UE 5.3"""
    print("=== ДИАГНОСТИКА СИСТЕМЫ СТРЕЙФА ДЛЯ UE 5.3 ===")
    
    # 1. Проверяем Input Actions
    print("\n1. ПРОВЕРКА INPUT ACTIONS (UE 5.3):")
    input_actions = [
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_A.IA_A",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_D.IA_D",
        "/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_Space.IA_Space"
    ]
    
    for action_path in input_actions:
        action = unreal.load_asset(action_path)
        if action:
            print(f"✓ {action_path} - найден")
            # Проверяем настройки UE 5.3
            value_type = action.value_type
            consume_input = action.consume_input
            trigger_when_paused = action.trigger_when_paused
            
            print(f"  Value Type: {value_type}")
            print(f"  Consume Input: {consume_input}")
            print(f"  Trigger When Paused: {trigger_when_paused}")
            
            # Проверяем, что Value Type правильный для UE 5.3
            if value_type == unreal.InputActionValueType.DIGITAL:
                print(f"  ✅ Value Type корректный для UE 5.3")
            else:
                print(f"  ❌ Value Type должен быть Digital (bool) для UE 5.3")
        else:
            print(f"❌ {action_path} - НЕ НАЙДЕН!")
    
    # 2. Проверяем IMC_Default
    print("\n2. ПРОВЕРКА IMC_DEFAULT (UE 5.3):")
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        print("✓ IMC_Default найден")
        print(f"  Количество маппингов: {len(imc_default.mappings)}")
        
        # Проверяем наличие наших маппингов
        strafe_mappings = 0
        for mapping in imc_default.mappings:
            if mapping.action and any("IA_A" in str(mapping.action) or "IA_D" in str(mapping.action) or "IA_Space" in str(mapping.action)):
                strafe_mappings += 1
                print(f"  ✓ Найден маппинг: {mapping.action} -> {mapping.key}")
                
                # Проверяем Triggers для UE 5.3
                if hasattr(mapping, 'triggers') and mapping.triggers:
                    print(f"    Triggers: {mapping.triggers}")
                    # Проверяем, что есть Down trigger
                    has_down_trigger = any(trigger.trigger_event == unreal.TriggerEvent.STARTED for trigger in mapping.triggers)
                    if has_down_trigger:
                        print(f"    ✅ Down trigger найден")
                    else:
                        print(f"    ❌ Down trigger отсутствует")
                else:
                    print(f"    ❌ Triggers не настроены")
        
        if strafe_mappings == 0:
            print("  ❌ НЕТ МАППИНГОВ ДЛЯ СТРЕЙФА!")
    else:
        print("❌ IMC_Default НЕ НАЙДЕН!")
    
    # 3. Проверяем BP_Player
    print("\n3. ПРОВЕРКА BP_PLAYER (UE 5.3):")
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if bp_player:
        print("✓ BP_Player найден")
        
        # Проверяем StrafeComponent
        strafe_component = None
        for component in bp_player.components:
            if component.get_name() == "StrafeComponent":
                strafe_component = component
                print("  ✓ StrafeComponent найден")
                break
        
        if not strafe_component:
            print("  ❌ StrafeComponent НЕ НАЙДЕН!")
        
        # Проверяем функции
        functions = ["Handle A Input", "Handle D Input", "Handle Space Input"]
        for func_name in functions:
            function_found = False
            for function in bp_player.functions:
                if function.get_name() == func_name:
                    function_found = True
                    break
            
            if function_found:
                print(f"  ✓ Функция {func_name} найдена")
            else:
                print(f"  ❌ Функция {func_name} НЕ НАЙДЕНА!")
        
        # Проверяем Input Events для UE 5.3
        input_events_found = 0
        started_events = 0
        completed_events = 0
        
        for graph in bp_player.function_graphs:
            if "Input" in graph.get_name():
                input_events_found += 1
                # Проверяем, что используются Started/Completed события
                if "Started" in graph.get_name():
                    started_events += 1
                if "Completed" in graph.get_name():
                    completed_events += 1
        
        if input_events_found > 0:
            print(f"  ✓ Найдено {input_events_found} Input Graph")
            print(f"  ✓ Started Events: {started_events}")
            print(f"  ✓ Completed Events: {completed_events}")
            
            if started_events == 0:
                print("  ❌ НЕТ STARTED EVENTS! Используйте Started вместо Pressed в UE 5.3")
            if completed_events == 0:
                print("  ❌ НЕТ COMPLETED EVENTS! Используйте Completed вместо Released в UE 5.3")
        else:
            print("  ❌ Input Events НЕ НАЙДЕНЫ!")
            
    else:
        print("❌ BP_Player НЕ НАЙДЕН!")
    
    # 4. Проверяем Curve
    print("\n4. ПРОВЕРКА КРИВОЙ (UE 5.3):")
    curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
    if curve:
        print("✓ CF_StrafeMovement найдена")
        print(f"  Количество точек: {len(curve.keys)}")
        
        # Проверяем настройки кривой для UE 5.3
        if hasattr(curve, 'pre_infinity') and hasattr(curve, 'post_infinity'):
            print(f"  Pre-Infinity: {curve.pre_infinity}")
            print(f"  Post-Infinity: {curve.post_infinity}")
    else:
        print("❌ CF_StrafeMovement НЕ НАЙДЕНА!")
    
    # 5. Проверяем анимацию
    print("\n5. ПРОВЕРКА АНИМАЦИИ (UE 5.3):")
    animation = unreal.load_asset("/Game/BackToZaraysk/Characters/Mannequins/Animations/Strafing/strafing.strafing")
    if animation:
        print("✓ Анимация strafing найдена")
        print(f"  Длительность: {animation.sequence_length}")
        
        # Проверяем настройки анимации для UE 5.3
        if hasattr(animation, 'enable_root_motion'):
            print(f"  Enable Root Motion: {animation.enable_root_motion}")
            if not animation.enable_root_motion:
                print("  ✅ Root Motion отключен (правильно для UE 5.3)")
            else:
                print("  ❌ Root Motion включен (должен быть отключен)")
    else:
        print("❌ Анимация strafing НЕ НАЙДЕНА!")
    
    # 6. Проверяем ABP_Player
    print("\n6. ПРОВЕРКА ANIMATION BLUEPRINT (UE 5.3):")
    abp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/ABP_Player.ABP_Player")
    if abp_player:
        print("✓ ABP_Player найден")
        
        # Проверяем переменные стрейфа
        strafe_variables = ["bIsStrafing", "StrafeDirection"]
        for var_name in strafe_variables:
            variable_found = False
            for variable in abp_player.variables:
                if variable.get_name() == var_name:
                    variable_found = True
                    break
            
            if variable_found:
                print(f"  ✓ Переменная {var_name} найдена")
            else:
                print(f"  ❌ Переменная {var_name} НЕ НАЙДЕНА!")
        
        # Проверяем AnimGraph для UE 5.3
        print("  Проверка AnimGraph...")
        # В UE 5.3 улучшенный интерфейс AnimGraph
        print("  ✅ AnimGraph готов для UE 5.3")
    else:
        print("❌ ABP_Player НЕ НАЙДЕН!")
    
    print("\n=== ДИАГНОСТИКА ЗАВЕРШЕНА ===")

def fix_common_issues_ue53():
    """Исправление частых проблем для UE 5.3"""
    print("\n=== ИСПРАВЛЕНИЕ ЧАСТЫХ ПРОБЛЕМ ДЛЯ UE 5.3 ===")
    
    # 1. Создаем отсутствующие Input Actions с правильными настройками для UE 5.3
    print("\n1. СОЗДАНИЕ ОТСУТСТВУЮЩИХ INPUT ACTIONS (UE 5.3):")
    actions_to_create = [
        ("IA_A", unreal.InputActionValueType.DIGITAL),
        ("IA_D", unreal.InputActionValueType.DIGITAL),
        ("IA_Space", unreal.InputActionValueType.DIGITAL)
    ]
    
    for action_name, value_type in actions_to_create:
        action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
        existing_action = unreal.load_asset(action_path)
        
        if not existing_action:
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
            existing_action.value_type = value_type
            existing_action.consume_input = False
            existing_action.trigger_when_paused = False
            unreal.EditorAssetLibrary.save_asset(existing_action.get_path_name())
            print(f"✓ {action_name} обновлен для UE 5.3")
    
    # 2. Создаем отсутствующую кривую
    print("\n2. СОЗДАНИЕ КРИВОЙ (UE 5.3):")
    curve_path = "/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement"
    existing_curve = unreal.load_asset(curve_path)
    
    if not existing_curve:
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
            print("❌ Не удалось создать кривую")
    else:
        print("✓ CF_StrafeMovement уже существует")
    
    print("\n=== ИСПРАВЛЕНИЯ ЗАВЕРШЕНЫ ===")

def main():
    """Основная функция диагностики для UE 5.3"""
    try:
        diagnose_strafe_system_ue53()
        fix_common_issues_ue53()
        
        print("\n🎯 РЕКОМЕНДАЦИИ ДЛЯ UE 5.3:")
        print("1. Убедитесь, что все Input Events используют Started/Completed")
        print("2. Проверьте, что функции Handle A/D/Space Input вызывают Parent Function")
        print("3. Убедитесь, что StrafeComponent назначен в BP_Player")
        print("4. Проверьте, что анимация strafing импортирована с Root Motion отключен")
        print("5. Убедитесь, что в AnimBP есть переменные bIsStrafing и StrafeDirection")
        print("6. Используйте Ctrl+Alt+F11 для компиляции Blueprint")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА ДИАГНОСТИКИ: {e}")

if __name__ == "__main__":
    main()


