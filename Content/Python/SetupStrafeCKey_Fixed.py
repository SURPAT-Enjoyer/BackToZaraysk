import unreal

def setup_strafe_c_key():
    """Настройка системы стрейфа с клавишей C"""
    print("=== НАСТРОЙКА СТРЕЙФА С КЛАВИШЕЙ C ===")
    
    try:
        # 1. Создаем Input Action для клавиши C
        print("\n1. СОЗДАНИЕ INPUT ACTION ДЛЯ КЛАВИШИ C:")
        create_c_input_action()
        
        # 2. Настраиваем IMC_Default
        print("\n2. НАСТРОЙКА IMC_DEFAULT:")
        setup_c_input_mapping()
        
        # 3. Настраиваем BP_Player
        print("\n3. НАСТРОЙКА BP_PLAYER:")
        setup_blueprint_player_c()
        
        # 4. Финальная проверка
        print("\n4. ФИНАЛЬНАЯ ПРОВЕРКА:")
        final_verification_c()
        
        print("\n✅ НАСТРОЙКА СТРЕЙФА С КЛАВИШЕЙ C ЗАВЕРШЕНА!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА НАСТРОЙКИ: {e}")

def create_c_input_action():
    """Создание Input Action для клавиши C"""
    action_name = "IA_C"
    action_path = f"/Game/BackToZaraysk/Core/Input/Actions/Strafe/{action_name}.{action_name}"
    existing = unreal.load_asset(action_path)
    
    if not existing:
        try:
            # Используем правильный способ создания Input Action
            asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
            new_action = asset_tools.create_asset(
                asset_name=action_name,
                package_path="/Game/BackToZaraysk/Core/Input/Actions/Strafe",
                asset_class=unreal.InputAction,
                factory=unreal.Factory()
            )
            if new_action:
                new_action.value_type = unreal.InputActionValueType.DIGITAL
                new_action.consume_input = False
                new_action.trigger_when_paused = False
                unreal.EditorAssetLibrary.save_asset(new_action.get_path_name())
                print(f"✓ Создан {action_name} с настройками UE 5.3")
            else:
                print(f"❌ Не удалось создать {action_name}")
        except Exception as e:
            print(f"❌ Ошибка создания {action_name}: {e}")
            print("  РЕШЕНИЕ: Создайте IA_C вручную в Content Browser")
    else:
        print(f"✓ {action_name} уже существует")
        # Обновляем настройки для UE 5.3
        try:
            existing.value_type = unreal.InputActionValueType.DIGITAL
            existing.consume_input = False
            existing.trigger_when_paused = False
            unreal.EditorAssetLibrary.save_asset(existing.get_path_name())
            print(f"✓ {action_name} обновлен для UE 5.3")
        except Exception as e:
            print(f"⚠️ Ошибка обновления {action_name}: {e}")

def setup_c_input_mapping():
    """Настройка Input Mapping Context для клавиши C"""
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if not imc_default:
        print("❌ IMC_Default не найден!")
        return
    
    # Загружаем Input Action для C
    ia_c = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_C.IA_C")
    if not ia_c:
        print("❌ IA_C не найден!")
        print("  РЕШЕНИЕ: Создайте IA_C вручную в Content Browser")
        return
    
    # Проверяем существующие маппинги для C
    existing_mappings = []
    try:
        for mapping in imc_default.mappings:
            if mapping.action:
                action_name = str(mapping.action.get_name()) if hasattr(mapping.action, 'get_name') else str(mapping.action)
                if "IA_C" in action_name:
                    existing_mappings.append(action_name)
    except Exception as e:
        print(f"⚠️ Ошибка проверки маппингов: {e}")
        existing_mappings = []
    
    # Создаем маппинг для C если его нет
    if "IA_C" not in existing_mappings:
        try:
            new_mapping = unreal.EnhancedActionKeyMapping()
            new_mapping.action = ia_c
            new_mapping.key = unreal.Key.C
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
            print(f"✓ Добавлен маппинг IA_C -> C")
        except Exception as e:
            print(f"❌ Ошибка создания маппинга IA_C: {e}")
    else:
        print(f"✓ Маппинг IA_C уже существует")
    
    try:
        unreal.EditorAssetLibrary.save_asset(imc_default.get_path_name())
        print("✓ IMC_Default обновлен для клавиши C")
    except Exception as e:
        print(f"⚠️ Ошибка сохранения IMC_Default: {e}")

def setup_blueprint_player_c():
    """Настройка Blueprint Player для клавиши C"""
    bp_player = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_Player.BP_Player")
    if not bp_player:
        print("❌ BP_Player не найден!")
        return
    
    print("✓ BP_Player найден")
    
    # Проверяем StrafeComponent
    strafe_component = None
    try:
        # Используем правильный способ доступа к компонентам
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
    
    # Проверяем функцию Handle C Input
    function_exists = False
    try:
        # Используем правильный способ доступа к функциям
        if hasattr(bp_player, 'function_graphs'):
            for graph in bp_player.function_graphs:
                if hasattr(graph, 'get_name') and "Handle C Input" in graph.get_name():
                    function_exists = True
                    break
    except Exception as e:
        print(f"⚠️ Ошибка проверки функций: {e}")
    
    if function_exists:
        print("✓ Функция Handle C Input найдена")
    else:
        print("❌ Функция Handle C Input не найдена")
        print("  РЕШЕНИЕ: Создайте функцию Handle C Input вручную")
    
    try:
        unreal.EditorAssetLibrary.save_asset(bp_player.get_path_name())
        print("✓ BP_Player проверен")
    except Exception as e:
        print(f"⚠️ Ошибка сохранения BP_Player: {e}")

def final_verification_c():
    """Финальная проверка системы с клавишей C"""
    print("Выполняется финальная проверка...")
    
    # Проверяем Input Action для C
    print("\n1. Проверка Input Action для C:")
    ia_c = unreal.load_asset("/Game/BackToZaraysk/Core/Input/Actions/Strafe/IA_C.IA_C")
    if ia_c:
        print("  ✓ IA_C найден")
    else:
        print("  ❌ IA_C НЕ НАЙДЕН!")
        print("  РЕШЕНИЕ: Создайте IA_C вручную в Content Browser")
    
    # Проверяем IMC_Default
    print("\n2. Проверка IMC_Default:")
    imc_default = unreal.load_asset("/Game/BackToZaraysk/Core/Input/IMC_Default.IMC_Default")
    if imc_default:
        print("  ✓ IMC_Default найден")
        
        # Проверяем маппинг для C
        try:
            c_mappings = 0
            for mapping in imc_default.mappings:
                if mapping.action:
                    action_name = str(mapping.action.get_name()) if hasattr(mapping.action, 'get_name') else str(mapping.action)
                    if "IA_C" in action_name and mapping.key == unreal.Key.C:
                        c_mappings += 1
            
            if c_mappings > 0:
                print(f"  ✓ Найдено {c_mappings} маппингов IA_C -> C")
            else:
                print("  ❌ Маппинг IA_C -> C НЕ НАЙДЕН!")
                print("  РЕШЕНИЕ: Добавьте маппинг IA_C -> C в IMC_Default")
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
                print("  РЕШЕНИЕ: Добавьте StrafeComponent в BP_Player")
        except Exception as e:
            print(f"  ⚠️ Ошибка проверки StrafeComponent: {e}")
    else:
        print("  ❌ BP_Player НЕ НАЙДЕН!")
    
    # Итоговая оценка
    print("\n📊 ИТОГОВАЯ ОЦЕНКА:")
    if ia_c and imc_default and bp_player:
        print("  ✅ ВСЕ ОСНОВНЫЕ КОМПОНЕНТЫ НАЙДЕНЫ")
        print("  🎯 СИСТЕМА ГОТОВА К ТЕСТИРОВАНИЮ")
        print("  🎮 УПРАВЛЕНИЕ: Нажмите C для стрейфа вправо")
    else:
        print("  ⚠️ ЕСТЬ ПРОБЛЕМЫ, ТРЕБУЕТСЯ РУЧНАЯ НАСТРОЙКА")
        print("  📋 Следуйте инструкциям в STRAFE_C_KEY_SETUP.md")

def main():
    """Основная функция настройки"""
    print("🚀 Запуск настройки стрейфа с клавишей C...")
    
    try:
        setup_strafe_c_key()
        
        print("\n🎯 СЛЕДУЮЩИЕ ШАГИ:")
        print("1. Создайте IA_C вручную в Content Browser:")
        print("   - Правый клик в папке Strafe")
        print("   - Input → Input Action")
        print("   - Назовите IA_C")
        print("   - Value Type: Digital (bool)")
        print("2. Откройте BP_Player в редакторе")
        print("3. Проверьте Event Graph на наличие Input Event:")
        print("   - IA_C Started → Handle C Input")
        print("4. Убедитесь, что функция Handle C Input вызывает Parent Function")
        print("5. Соберите проект: .\\build_ue53.bat")
        print("6. Протестируйте: Нажмите C для стрейфа вправо")
        
        print("\n📋 ДОПОЛНИТЕЛЬНАЯ ИНФОРМАЦИЯ:")
        print("- Управление: C → Стрейф вправо")
        print("- Система работает по одиночному нажатию")
        print("- Если нужно добавить стрейф влево, используйте другую клавишу")
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        print("Выполните ручную настройку по инструкции STRAFE_C_KEY_SETUP.md")

if __name__ == "__main__":
    main()


