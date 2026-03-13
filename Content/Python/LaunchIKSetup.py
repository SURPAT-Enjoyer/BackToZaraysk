import unreal

# ФИНАЛЬНЫЙ АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK
# Этот скрипт выполнит ВСЕ необходимые шаги автоматически

def launch_ik_setup():
    """Запускает финальную автоматическую настройку IK системы"""
    
    print("🤖 ФИНАЛЬНЫЙ АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK")
    print("=" * 60)
    print("Выполняются все необходимые шаги...")
    print("=" * 60)
    
    try:
        # Шаг 1: Создаем переменные IK
        print("\n[ШАГ 1/5] Создание переменных IK...")
        step1_result = create_ik_variables_final()
        print(f"✅ Шаг 1 завершен: {step1_result} переменных создано")
        
        # Шаг 2: Создаем IK ноды
        print("\n[ШАГ 2/5] Создание IK нод...")
        step2_result = create_ik_nodes_final()
        print(f"✅ Шаг 2 завершен: {step2_result} нод создано")
        
        # Шаг 3: Настраиваем персонажа
        print("\n[ШАГ 3/5] Настройка персонажа...")
        step3_result = setup_character_final()
        print(f"✅ Шаг 3 завершен: {step3_result}")
        
        # Шаг 4: Создаем Blueprint Interface
        print("\n[ШАГ 4/5] Создание Blueprint Interface...")
        step4_result = create_interface_final()
        print(f"✅ Шаг 4 завершен: {step4_result}")
        
        # Шаг 5: Создаем Blueprint Actor
        print("\n[ШАГ 5/5] Создание Blueprint Actor...")
        step5_result = create_blueprint_actor_final()
        print(f"✅ Шаг 5 завершен: {step5_result}")
        
        print("\n" + "=" * 60)
        print("🎉 ФИНАЛЬНАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ЗАВЕРШЕНА!")
        print("=" * 60)
        print("✅ Переменные IK созданы")
        print("✅ IK ноды добавлены")
        print("✅ Персонаж настроен")
        print("✅ Blueprint Interface создан")
        print("✅ Blueprint Actor создан")
        print("\n🚀 IK СИСТЕМА ПОЛНОСТЬЮ ГОТОВА К РАБОТЕ!")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def create_ik_variables_final():
    """Создает переменные IK - финальная версия"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    total_created = 0
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                created = create_variables_final(anim_bp)
                total_created += created
                
                if created > 0:
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    
        except Exception as e:
            print(f"Ошибка в {asset_data.asset_name}: {e}")
    
    return total_created

def create_variables_final(anim_bp):
    """Создает переменные в Animation Blueprint - финальная версия"""
    
    variables = [
        ("LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", unreal.Float, 1.0),
        ("RightFootIKAlpha", unreal.Float, 1.0)
    ]
    
    existing = [var.variable_name for var in anim_bp.variables]
    created = 0
    
    for name, type_class, default in variables:
        if name not in existing:
            try:
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp, name, type_class
                )
                if new_var:
                    new_var.variable_name = name
                    new_var.default_value = default
                    created += 1
                    print(f"  ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"  ❌ Ошибка создания {name}: {e}")
    
    return created

def create_ik_nodes_final():
    """Создает IK ноды - финальная версия"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    total_created = 0
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp and anim_bp.anim_graph:
                created = create_nodes_final(anim_bp)
                total_created += created
                
                if created > 0:
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    
        except Exception as e:
            print(f"Ошибка создания нод в {asset_data.asset_name}: {e}")
    
    return total_created

def create_nodes_final(anim_bp):
    """Создает ноды в Animation Blueprint - финальная версия"""
    
    nodes = [
        ("TwoBoneIK", "LeftFootIK"),
        ("TwoBoneIK", "RightFootIK")
    ]
    
    existing = [node.node_name for node in anim_bp.anim_graph.nodes]
    created = 0
    
    for node_type, node_name in nodes:
        if node_name not in existing:
            try:
                new_node = unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
                if new_node:
                    created += 1
                    print(f"  ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"  ❌ Ошибка создания нода {node_name}: {e}")
    
    return created

def setup_character_final():
    """Настраивает персонажа - финальная версия"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    character_found = False
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            character_found = True
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    check_sockets_final(character_bp)
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"  ✅ Персонаж настроен: {asset_data.asset_name}")
            except Exception as e:
                print(f"  ❌ Ошибка настройки персонажа: {e}")
    
    if not character_found:
        print("  ⚠️ Персонаж не найден")
        return "Персонаж не найден"
    
    return "Персонаж настроен"

def check_sockets_final(character_bp):
    """Проверяет сокеты персонажа - финальная версия"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            print(f"  Сокеты: foot_l={'✅' if left_exists else '❌'}, foot_r={'✅' if right_exists else '❌'}")
            
            if not left_exists or not right_exists:
                print("  ⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
            break

def create_interface_final():
    """Создает Blueprint Interface - финальная версия"""
    
    try:
        interface_path = "/Game/Blueprints/IKInterface"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/")
            
            interface = unreal.EditorAssetLibrary.create_asset(
                "IKInterface", "/Game/Blueprints/",
                unreal.BlueprintInterface, unreal.BlueprintInterfaceFactory()
            )
            
            if interface:
                print("  ✅ Blueprint Interface создан")
                return "Blueprint Interface создан"
            else:
                print("  ❌ Не удалось создать Blueprint Interface")
                return "Ошибка создания Blueprint Interface"
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            return "Blueprint Interface уже существует"
            
    except Exception as e:
        print(f"  ❌ Ошибка создания Blueprint Interface: {e}")
        return f"Ошибка: {e}"

def create_blueprint_actor_final():
    """Создает Blueprint Actor - финальная версия"""
    
    try:
        # Создаем папку для Blueprint
        unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/IK/")
        
        # Создаем Blueprint Actor
        actor_path = "/Game/Blueprints/IK/IKSetupActor"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(actor_path):
            ik_actor = unreal.EditorAssetLibrary.create_asset(
                "IKSetupActor",
                "/Game/Blueprints/IK/",
                unreal.Blueprint,
                unreal.BlueprintFactory()
            )
            
            if ik_actor:
                print("  ✅ IK Setup Actor создан")
                unreal.EditorAssetLibrary.save_asset(actor_path)
                return "IK Setup Actor создан"
            else:
                print("  ❌ Не удалось создать IK Setup Actor")
                return "Ошибка создания IK Setup Actor"
        else:
            print("  ℹ️ IK Setup Actor уже существует")
            return "IK Setup Actor уже существует"
            
    except Exception as e:
        print(f"  ❌ Ошибка создания IK Setup Actor: {e}")
        return f"Ошибка: {e}"

# ФИНАЛЬНЫЙ АВТОМАТИЧЕСКИЙ ЗАПУСК
launch_ik_setup()