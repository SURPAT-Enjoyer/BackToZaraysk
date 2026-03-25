import unreal

def setup_strafe_ad_space():
    """Настройка системы стрейфа с комбинациями A/D + Space"""
    print("=== НАСТРОЙКА СТРЕЙФА С КОМБИНАЦИЯМИ A/D + SPACE ===")
    
    try:
        # 1. Создаем Input Actions
        print("\n1. СОЗДАНИЕ INPUT ACTIONS:")
        create_input_actions()
        
        # 2. Настраиваем IMC_Default
        print("\n2. НАСТРОЙКА IMC_DEFAULT:")
        setup_input_mapping_context()
        
        # 3. Настраиваем BP_Player
        print("\n3. НАСТРОЙКА BP_PLAYER:")
        setup_blueprint_player()
        
        # 4. Финальная проверка
        print("\n4. ФИНАЛЬНАЯ ПРОВЕРКА:")
        final_verification()
        
        print("\n✅ НАСТРОЙКА СТРЕЙФА С КОМБИНАЦИЯМИ A/D + SPACE ЗАВЕРШЕНА!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА НАСТРОЙКИ: {e}")

def create_input_actions():
    """Создание Input Actions"""
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
                asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
                new_action = asset_tools.create_asset(
                    asset_name=action_name,
                    package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
                    asset_class=unreal.InputAction,
                    factory=unreal.Factory()
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
                print(f"  РЕШЕНИЕ: Создайте {action_name} вручную в Content Browser")
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
        print("  РЕШЕНИЕ: Создайте IA_A, IA_D, IA_Space вручную в Content Browser")
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
        print("✓ IMC_Default обновлен для комбинаций A/D + Space")
    except Exception as e:
        print(f"⚠️ Ошибка сохранения IMC_Default: {e}")

def setup_blueprint_player():
    """Настройка Blueprint Player"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    print("✓ BP_Player найден")
    
    # Проверяем StrafeComponent
    strafe_component = None
    try:
        if hasattr(bp_player, 'simple_construction_script'):
            for component in bp_player.simple_construction_script.get_all_nodes():
                if hasattr(component, 'get_name') and component.get_name() == "StrafeComponent":
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
            "StrafeSpeed": 400.0,
            "SmoothnessFactor": 2.0
        }
        
        for setting_name, value in settings.items():
            try:
                strafe_component.set_editor_property(setting_name, value)
                print(f"  ✓ {setting_name} установлен в {value}")
            except Exception as e:
                print(f"  ⚠️ Ошибка установки {setting_name}: {e}")
        
        # Назначаем кривую
        curve = unreal.load_asset("/Game/BackToZaraysk/Core/Data/Curves/CF_StrafeMovement.CF_StrafeMovement")
        if curve:
            try:
                strafe_component.set_editor_property("StrafeCurve", curve)
                print("  ✓ Strafe Curve назначена")
            except Exception as e:
                print(f"  ⚠️ Ошибка назначения кривой: {e}")
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
            if hasattr(bp_player, 'function_graphs'):
                for graph in bp_player.function_graphs:
                    if hasattr(graph, 'get_name') and func_name in graph.get_name():
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

def final_verification():
    """Финальная проверка системы"""
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
            if hasattr(bp_player, 'simple_construction_script'):
                for component in bp_player.simple_construction_script.get_all_nodes():
                    if hasattr(component, 'get_name') and component.get_name() == "StrafeComponent":
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
        print("  🎮 УПРАВЛЕНИЕ:")
        print("    - A + Space → Стрейф влево")
        print("    - D + Space → Стрейф вправо")
    else:
        print("  ⚠️ ЕСТЬ ПРОБЛЕМЫ, ТРЕБУЕТСЯ РУЧНАЯ НАСТРОЙКА")

def main():
    """Основная функция настройки"""
    print("🚀 Запуск настройки стрейфа с комбинациями A/D + Space...")
    
    try:
        setup_strafe_ad_space()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Создайте Input Actions вручную в Content Browser:")
        print("   - IA_A (Value Type: Digital bool)")
        print("   - IA_D (Value Type: Digital bool)")
        print("   - IA_Space (Value Type: Digital bool)")
        print("2. Откройте BP_Player в редакторе")
        print("3. Проверьте Event Graph на наличие Input Events:")
        print("   - IA_A Started/Completed → Handle A Input")
        print("   - IA_D Started/Completed → Handle D Input")
        print("   - IA_Space Started → Handle Space Input")
        print("4. Убедитесь, что функции Handle A/D/Space Input вызывают Parent Function")
        print("5. Соберите проект: .\\build_ue53.bat")
        print("6. Протестируйте:")
        print("   - A + Space → Стрейф влево")
        print("   - D + Space → Стрейф вправо")
        
        print("\n📋 ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:")
        print("- Управление: A/D + Space (одно нажатие)")
        print("- A + Space → Стрейф влево")
        print("- D + Space → Стрейф вправо")
        print("- Система работает с комбинациями клавиш")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции")

if __name__ == "__main__":
    main()


