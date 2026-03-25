import unreal
import time

# АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK ПРИ ЗАПУСКЕ РЕДАКТОРА
# Этот скрипт будет выполнен автоматически при запуске

def execute_ik_setup_on_startup():
    """Автоматическое выполнение настройки IK при запуске"""
    
    print("🤖 АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK ПРИ ЗАПУСКЕ")
    print("=" * 60)
    
    try:
        # Ждем полной загрузки редактора
        print("Ожидание полной загрузки редактора...")
        time.sleep(10)
        
        # Выполняем настройку IK
        print("Выполнение настройки IK системы...")
        setup_ik_system_automatically()
        
        print("\n✅ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ ЗАВЕРШЕНО!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_ik_system_automatically():
    """Автоматическая настройка IK системы"""
    
    print("Настройка IK системы...")
    
    # Создаем переменные IK
    print("Создание переменных IK...")
    create_ik_variables()
    
    # Создаем IK ноды
    print("Создание IK нод...")
    create_ik_nodes()
    
    # Настраиваем персонажа
    print("Настройка персонажа...")
    setup_character()
    
    # Создаем Blueprint Interface
    print("Создание Blueprint Interface...")
    create_ik_interface()
    
    print("✅ IK система настроена автоматически!")

def create_ik_variables():
    """Создает переменные IK"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                add_ik_variables_to_abp(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
        except Exception as e:
            print(f"Ошибка в {asset_data.asset_name}: {e}")

def add_ik_variables_to_abp(anim_bp):
    """Добавляет переменные IK в Animation Blueprint"""
    
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
                    print(f"  ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"  ❌ Ошибка создания {name}: {e}")

def create_ik_nodes():
    """Создает IK ноды"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp and anim_bp.anim_graph:
                add_ik_nodes_to_abp(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
        except Exception as e:
            print(f"Ошибка создания нод в {asset_data.asset_name}: {e}")

def add_ik_nodes_to_abp(anim_bp):
    """Добавляет IK ноды в Animation Blueprint"""
    
    nodes = [
        ("TwoBoneIK", "LeftFootIK"),
        ("TwoBoneIK", "RightFootIK")
    ]
    
    existing = [node.node_name for node in anim_bp.anim_graph.nodes]
    
    for node_type, node_name in nodes:
        if node_name not in existing:
            try:
                new_node = unreal.EditorAnimBlueprintUtils.add_node(
                    anim_bp.anim_graph, node_type, node_name
                )
                if new_node:
                    print(f"  ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"  ❌ Ошибка создания нода {node_name}: {e}")

def setup_character():
    """Настраивает персонажа"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    check_character_sockets(character_bp)
                    add_ik_variables_to_character(character_bp)
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"  ✅ Персонаж настроен: {asset_data.asset_name}")
            except Exception as e:
                print(f"  ❌ Ошибка настройки персонажа: {e}")

def check_character_sockets(character_bp):
    """Проверяет сокеты персонажа"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            print(f"  Сокеты: foot_l={'✅' if left_exists else '❌'}, foot_r={'✅' if right_exists else '❌'}")
            
            if not left_exists or not right_exists:
                print("  ⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
            break

def add_ik_variables_to_character(character_bp):
    """Добавляет IK переменные к персонажу"""
    
    variables = [
        ("bEnableFootIK", unreal.Bool, True),
        ("FootIKStrength", unreal.Float, 1.0),
        ("FootIKBlendSpeed", unreal.Float, 8.0)
    ]
    
    existing = [var.variable_name for var in character_bp.variables]
    
    for name, type_class, default in variables:
        if name not in existing:
            try:
                new_var = unreal.EditorBlueprintUtils.add_variable_to_blueprint(
                    character_bp, name, type_class
                )
                if new_var:
                    new_var.variable_name = name
                    new_var.default_value = default
                    print(f"  ✅ Добавлена переменная: {name}")
            except Exception as e:
                print(f"  ❌ Ошибка создания {name}: {e}")

def create_ik_interface():
    """Создает Blueprint Interface для IK"""
    
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
                
                # Добавляем функции
                add_interface_functions(interface)
                
                unreal.EditorAssetLibrary.save_asset(interface_path)
            else:
                print("  ❌ Не удалось создать Blueprint Interface")
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания Blueprint Interface: {e}")

def add_interface_functions(interface):
    """Добавляет функции к Blueprint Interface"""
    
    functions = [
        ("SetLeftFootIK", [unreal.FunctionParameter(name="Offset", type=unreal.EdGraphPinType(pin_category="float"))]),
        ("SetRightFootIK", [unreal.FunctionParameter(name="Offset", type=unreal.EdGraphPinType(pin_category="float"))])
    ]
    
    for func_name, params in functions:
        try:
            unreal.BlueprintEditorLibrary.add_function_to_interface(interface, func_name, params)
            print(f"    ✅ Добавлена функция: {func_name}")
        except Exception as e:
            print(f"    ❌ Ошибка добавления функции {func_name}: {e}")

# АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ ПРИ ЗАПУСКЕ
execute_ik_setup_on_startup()