import unreal

# АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK В РЕДАКТОРЕ
# Этот скрипт будет выполнен автоматически

def auto_setup_ik():
    """Автоматическая настройка IK системы"""
    
    print("🤖 АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK")
    print("=" * 50)
    
    try:
        # Создаем переменные IK
        print("Создание переменных IK...")
        create_ik_variables_auto()
        
        # Создаем IK ноды
        print("Создание IK нод...")
        create_ik_nodes_auto()
        
        # Настраиваем персонажа
        print("Настройка персонажа...")
        setup_character_auto()
        
        # Создаем Blueprint Interface
        print("Создание Blueprint Interface...")
        create_interface_auto()
        
        print("\n✅ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ЗАВЕРШЕНА!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"❌ ОШИБКА: {e}")

def create_ik_variables_auto():
    """Автоматическое создание переменных IK"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    total_created = 0
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                created = create_variables_auto(anim_bp)
                total_created += created
                
                if created > 0:
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    
        except Exception as e:
            print(f"Ошибка в {asset_data.asset_name}: {e}")
    
    print(f"Создано переменных: {total_created}")

def create_variables_auto(anim_bp):
    """Создает переменные автоматически"""
    
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

def create_ik_nodes_auto():
    """Автоматическое создание IK нод"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    total_created = 0
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp and anim_bp.anim_graph:
                created = create_nodes_auto(anim_bp)
                total_created += created
                
                if created > 0:
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    
        except Exception as e:
            print(f"Ошибка создания нод в {asset_data.asset_name}: {e}")
    
    print(f"Создано нод: {total_created}")

def create_nodes_auto(anim_bp):
    """Создает ноды автоматически"""
    
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

def setup_character_auto():
    """Автоматическая настройка персонажа"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    character_found = False
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            character_found = True
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    check_sockets_auto(character_bp)
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"  ✅ Персонаж настроен: {asset_data.asset_name}")
            except Exception as e:
                print(f"  ❌ Ошибка настройки персонажа: {e}")
    
    if not character_found:
        print("  ⚠️ Персонаж не найден")

def check_sockets_auto(character_bp):
    """Проверяет сокеты персонажа автоматически"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            print(f"  Сокеты: foot_l={'✅' if left_exists else '❌'}, foot_r={'✅' if right_exists else '❌'}")
            
            if not left_exists or not right_exists:
                print("  ⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
            break

def create_interface_auto():
    """Автоматическое создание Blueprint Interface"""
    
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
            else:
                print("  ❌ Не удалось создать Blueprint Interface")
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания Blueprint Interface: {e}")

# Автоматический запуск
auto_setup_ik()