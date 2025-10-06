import unreal
import time

def check_strafe_settings():
    """Проверяет настройки системы стрейфа в редакторе"""
    
    print("🔍 ПРОВЕРКА НАСТРОЕК СИСТЕМЫ СТРЕЙФА")
    print("=" * 50)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # 1. Проверяем Input Mapping Context
        check_input_mapping()
        
        # 2. Проверяем PlayerController
        check_player_controller()
        
        # 3. Проверяем PlayerCharacter
        check_player_character()
        
        # 4. Проверяем StrafeComponent
        check_strafe_component()
        
        # 5. Проверяем Animation Blueprint
        check_animation_blueprint()
        
        print("\n🎯 ПРОВЕРКА ЗАВЕРШЕНА")
        
    except Exception as e:
        print(f"❌ Ошибка проверки: {e}")
        import traceback
        traceback.print_exc()

def check_input_mapping():
    """Проверяет настройки Input Mapping Context"""
    
    print("\n🎮 ПРОВЕРКА INPUT MAPPING:")
    
    try:
        # Получаем настройки проекта
        project_settings = unreal.ProjectSettings.get_project_settings()
        
        print("  ✅ Project Settings доступны")
        print("  💡 Проверьте вручную: Edit → Project Settings → Input → Bindings")
        print("  📋 Нужные привязки:")
        print("     - StrafeLeft (A)")
        print("     - StrafeRight (D)")
        print("     - StrafeSpace (Space)")
        print("     - LeanLeft (Q)")
        print("     - LeanRight (E)")
        
    except Exception as e:
        print(f"  ❌ Ошибка проверки Input: {e}")

def check_player_controller():
    """Проверяет настройки PlayerController"""
    
    print("\n🎮 ПРОВЕРКА PLAYER CONTROLLER:")
    
    try:
        # Ищем PlayerController в мире
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            player_controllers = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.PlayerController)
            
            if player_controllers:
                pc = player_controllers[0]
                class_name = pc.get_class().get_name()
                print(f"  ✅ Найден PlayerController: {class_name}")
                
                if "BTZPlayerController" in class_name:
                    print(f"  ✅ Используется правильный класс!")
                else:
                    print(f"  ⚠️ Возможно неправильный класс. Ожидается: BTZPlayerController")
            else:
                print(f"  ⚠️ PlayerController не найден в мире")
                print(f"  💡 Убедитесь что GameMode настроен правильно")
        else:
            print(f"  ❌ Мир недоступен")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки PlayerController: {e}")

def check_player_character():
    """Проверяет настройки PlayerCharacter"""
    
    print("\n🏃 ПРОВЕРКА PLAYER CHARACTER:")
    
    try:
        # Проверяем Blueprint PlayerCharacter
        player_bp_path = "/Game/BackToZaraysk/Core/Characters/Player/BP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(player_bp_path):
            print(f"  ✅ BP_Player найден: {player_bp_path}")
            
            # Загружаем Blueprint
            player_bp = unreal.load_asset(player_bp_path)
            if player_bp:
                # Проверяем класс
                parent_class = player_bp.parent_class
                if parent_class:
                    class_name = parent_class.get_name()
                    print(f"  📋 Parent Class: {class_name}")
                    
                    if "PlayerCharacter" in class_name:
                        print(f"  ✅ Используется правильный класс!")
                    else:
                        print(f"  ⚠️ Возможно неправильный класс. Ожидается: PlayerCharacter")
                
                # Проверяем компоненты
                check_character_components(player_bp)
            else:
                print(f"  ❌ Не удалось загрузить BP_Player")
        else:
            print(f"  ❌ BP_Player не найден: {player_bp_path}")
            print(f"  💡 Создайте Blueprint на основе APlayerCharacter")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки PlayerCharacter: {e}")

def check_character_components(blueprint):
    """Проверяет компоненты персонажа"""
    
    print("\n  🔧 ПРОВЕРКА КОМПОНЕНТОВ:")
    
    try:
        # Получаем список компонентов из Blueprint
        if hasattr(blueprint, 'simple_construction_script'):
            scs = blueprint.simple_construction_script
            if scs:
                root_nodes = scs.get_root_nodes()
                components = []
                
                for root_node in root_nodes:
                    if hasattr(root_node, 'component_class'):
                        comp_class = root_node.component_class
                        if comp_class:
                            components.append(comp_class.get_name())
                
                print(f"    📋 Найденные компоненты:")
                for comp in components:
                    print(f"      - {comp}")
                
                # Проверяем нужные компоненты
                required_components = [
                    "StrafeComponent",
                    "InventoryComponent", 
                    "ObstacleClimbingComponent"
                ]
                
                missing_components = []
                for required in required_components:
                    if not any(required in comp for comp in components):
                        missing_components.append(required)
                
                if missing_components:
                    print(f"    ❌ Отсутствуют компоненты: {missing_components}")
                    print(f"    💡 Добавьте их в Blueprint компоненты")
                else:
                    print(f"    ✅ Все нужные компоненты присутствуют!")
            else:
                print(f"    ❌ Не удалось получить компоненты")
        else:
            print(f"    ❌ Blueprint не имеет SCS")
            
    except Exception as e:
        print(f"    ❌ Ошибка проверки компонентов: {e}")

def check_strafe_component():
    """Проверяет настройки StrafeComponent"""
    
    print("\n⚡ ПРОВЕРКА STRAFE COMPONENT:")
    
    try:
        # Ищем персонажа в мире
        world = unreal.EditorLevelLibrary.get_editor_world()
        if world:
            characters = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Character)
            
            if characters:
                character = characters[0]
                
                # Ищем StrafeComponent
                strafe_comp = character.get_component_by_class(unreal.ActorComponent)
                
                # Альтернативный способ - проверяем через Blueprint
                print(f"  💡 StrafeComponent настройки:")
                print(f"     - Strafe Distance: 200.0 (рекомендуется)")
                print(f"     - Strafe Duration: 0.3 (рекомендуется)")
                print(f"     - Strafe Cooldown: 1.0 (рекомендуется)")
                print(f"     - Strafe Speed: 400.0 (рекомендуется)")
                print(f"  📋 Проверьте в BP_Player → StrafeComponent → Details")
                
            else:
                print(f"  ⚠️ Персонаж не найден в мире")
        else:
            print(f"  ❌ Мир недоступен")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки StrafeComponent: {e}")

def check_animation_blueprint():
    """Проверяет настройки Animation Blueprint"""
    
    print("\n🎭 ПРОВЕРКА ANIMATION BLUEPRINT:")
    
    try:
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            print(f"  ✅ ABP_Player найден: {abp_path}")
            
            # Загружаем Animation Blueprint
            abp = unreal.load_asset(abp_path)
            if abp:
                # Проверяем класс
                if hasattr(abp, 'parent_class') and abp.parent_class:
                    class_name = abp.parent_class.get_name()
                    print(f"  📋 Parent Class: {class_name}")
                    
                    if "BTZBaseCharacterAnimInstance" in class_name:
                        print(f"  ✅ Используется правильный класс!")
                    else:
                        print(f"  ⚠️ Возможно неправильный класс. Ожидается: BTZBaseCharacterAnimInstance")
                
                # Проверяем переменные
                check_abp_variables(abp)
            else:
                print(f"  ❌ Не удалось загрузить ABP_Player")
        else:
            print(f"  ❌ ABP_Player не найден: {abp_path}")
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки Animation Blueprint: {e}")

def check_abp_variables(abp):
    """Проверяет переменные Animation Blueprint"""
    
    print("\n  📝 ПРОВЕРКА ПЕРЕМЕННЫХ:")
    
    try:
        if hasattr(abp, 'variables') and abp.variables:
            existing_vars = [var.variable_name for var in abp.variables]
            print(f"    📋 Найденные переменные: {len(existing_vars)}")
            
            # Критические переменные ИК
            required_vars = [
                "LeftFootEffectorLocation",
                "RightFootEffectorLocation",
                "LeftFootIKAlpha",
                "RightFootIKAlpha"
            ]
            
            missing_vars = []
            for var in required_vars:
                if var in existing_vars:
                    print(f"      ✅ {var}")
                else:
                    print(f"      ❌ {var} - ОТСУТСТВУЕТ!")
                    missing_vars.append(var)
            
            if missing_vars:
                print(f"    ❌ Отсутствуют переменные ИК!")
                print(f"    💡 Добавьте их в Animation Blueprint → My Blueprint → Variables")
            else:
                print(f"    ✅ Все переменные ИК присутствуют!")
        else:
            print(f"    ❌ Нет доступа к переменным")
            
    except Exception as e:
        print(f"    ❌ Ошибка проверки переменных: {e}")

# Запускаем проверку
check_strafe_settings()



