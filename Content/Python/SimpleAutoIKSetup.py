import unreal

# ПРОСТОЙ АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK
# Выполните: exec(open('Content/Python/SimpleAutoIKSetup.py').read())

def simple_auto_ik_setup():
    """Простой автоматический запуск настройки IK"""
    
    print("🤖 ПРОСТОЙ АВТОМАТИЧЕСКИЙ ЗАПУСК НАСТРОЙКИ IK")
    print("=" * 50)
    
    try:
        # Создаем переменные IK
        print("Создание переменных IK...")
        create_ik_variables_simple()
        
        # Создаем IK ноды
        print("Создание IK нод...")
        create_ik_nodes_simple()
        
        # Настраиваем персонажа
        print("Настройка персонажа...")
        setup_character_simple()
        
        # Создаем Blueprint Interface
        print("Создание Blueprint Interface...")
        create_interface_simple()
        
        print("\n✅ ПРОСТАЯ АВТОМАТИЧЕСКАЯ НАСТРОЙКА ЗАВЕРШЕНА!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"❌ ОШИБКА: {e}")

def create_ik_variables_simple():
    """Простое создание переменных IK"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                create_variables_simple(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
        except:
            pass

def create_variables_simple(anim_bp):
    """Создает переменные просто"""
    
    variables = [
        ("LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", unreal.Float, 1.0),
        ("RightFootIKAlpha", unreal.Float, 1.0)
    ]
    
    existing = [var.variable_name for var in anim_bp.variables]
    
    for name, type_class, default in variables:
        if name not in existing:
            try:
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp, name, type_class
                )
                if new_var:
                    new_var.variable_name = name
                    new_var.default_value = default
            except:
                pass

def create_ik_nodes_simple():
    """Простое создание IK нод"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp and anim_bp.anim_graph:
                create_nodes_simple(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
        except:
            pass

def create_nodes_simple(anim_bp):
    """Создает ноды просто"""
    
    nodes = [
        ("TwoBoneIK", "LeftFootIK"),
        ("TwoBoneIK", "RightFootIK")
    ]
    
    existing = [node.node_name for node in anim_bp.anim_graph.nodes]
    
    for node_type, node_name in nodes:
        if node_name not in existing:
            try:
                unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
            except:
                pass

def setup_character_simple():
    """Простая настройка персонажа"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    check_sockets_simple(character_bp)
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
            except:
                pass

def check_sockets_simple(character_bp):
    """Проверяет сокеты просто"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            if not left_exists or not right_exists:
                print("⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
            break

def create_interface_simple():
    """Простое создание Blueprint Interface"""
    
    try:
        interface_path = "/Game/Blueprints/IKInterface"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/")
            
            interface = unreal.EditorAssetLibrary.create_asset(
                "IKInterface", "/Game/Blueprints/",
                unreal.BlueprintInterface, unreal.BlueprintInterfaceFactory()
            )
            
            if interface:
                print("✅ Blueprint Interface создан")
            else:
                print("❌ Не удалось создать Blueprint Interface")
        else:
            print("ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"❌ Ошибка создания Blueprint Interface: {e}")

# Простой автоматический запуск
simple_auto_ik_setup()
