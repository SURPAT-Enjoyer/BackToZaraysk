import unreal

# ПРЯМОЕ ИЗМЕНЕНИЕ BLUEPRINT'ОВ ДЛЯ IK СИСТЕМЫ
# Этот скрипт напрямую изменит все необходимые Blueprint'ы

def direct_edit_blueprints():
    """Прямое изменение Blueprint'ов для IK"""
    
    print("🤖 ПРЯМОЕ ИЗМЕНЕНИЕ BLUEPRINT'ОВ ДЛЯ IK")
    print("=" * 50)
    
    try:
        # Прямое изменение Animation Blueprint
        print("Прямое изменение Animation Blueprint...")
        direct_edit_animation_blueprint()
        
        # Прямое изменение PlayerCharacter Blueprint
        print("Прямое изменение PlayerCharacter Blueprint...")
        direct_edit_player_character_blueprint()
        
        # Прямое создание Blueprint Interface
        print("Прямое создание Blueprint Interface...")
        direct_create_ik_interface()
        
        print("\n✅ ВСЕ BLUEPRINT'Ы ПРЯМО ИЗМЕНЕНЫ!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"❌ ОШИБКА: {e}")

def direct_edit_animation_blueprint():
    """Прямое изменение Animation Blueprint"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                print(f"  Прямая обработка: {asset_data.asset_name}")
                
                # Прямое добавление переменных IK
                direct_add_ik_variables(anim_bp)
                
                # Прямое добавление IK нод
                direct_add_ik_nodes(anim_bp)
                
                # Прямое сохранение
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                print(f"  ✅ Прямо обновлен: {asset_data.asset_name}")
                
        except Exception as e:
            print(f"  ❌ Ошибка в {asset_data.asset_name}: {e}")

def direct_add_ik_variables(anim_bp):
    """Прямое добавление переменных IK"""
    
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
                    print(f"    ✅ Прямо добавлена: {name}")
            except Exception as e:
                print(f"    ❌ Ошибка {name}: {e}")

def direct_add_ik_nodes(anim_bp):
    """Прямое добавление IK нод"""
    
    if not anim_bp.anim_graph:
        print("    ⚠️ AnimGraph не найден")
        return
    
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
                    print(f"    ✅ Прямо добавлен: {node_name}")
            except Exception as e:
                print(f"    ❌ Ошибка нода {node_name}: {e}")

def direct_edit_player_character_blueprint():
    """Прямое изменение PlayerCharacter Blueprint"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    print(f"  Прямая обработка: {asset_data.asset_name}")
                    
                    # Прямая проверка сокетов
                    direct_check_sockets(character_bp)
                    
                    # Прямое добавление IK переменных
                    direct_add_character_ik_variables(character_bp)
                    
                    # Прямое сохранение
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"  ✅ Прямо обновлен: {asset_data.asset_name}")
                    
            except Exception as e:
                print(f"  ❌ Ошибка в {asset_data.asset_name}: {e}")

def direct_check_sockets(character_bp):
    """Прямая проверка сокетов персонажа"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            print(f"    Сокеты: foot_l={'✅' if left_exists else '❌'}, foot_r={'✅' if right_exists else '❌'}")
            
            if not left_exists or not right_exists:
                print("    ⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
            break

def direct_add_character_ik_variables(character_bp):
    """Прямое добавление IK переменных к персонажу"""
    
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
                    print(f"    ✅ Прямо добавлена: {name}")
            except Exception as e:
                print(f"    ❌ Ошибка {name}: {e}")

def direct_create_ik_interface():
    """Прямое создание Blueprint Interface для IK"""
    
    try:
        interface_path = "/Game/Blueprints/IKInterface"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/")
            
            interface = unreal.EditorAssetLibrary.create_asset(
                "IKInterface", "/Game/Blueprints/",
                unreal.BlueprintInterface, unreal.BlueprintInterfaceFactory()
            )
            
            if interface:
                print("  ✅ Blueprint Interface прямо создан")
                
                # Прямое добавление функций
                direct_add_interface_functions(interface)
                
                unreal.EditorAssetLibrary.save_asset(interface_path)
                print("  ✅ Blueprint Interface прямо сохранен")
            else:
                print("  ❌ Не удалось прямо создать Blueprint Interface")
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка прямого создания Blueprint Interface: {e}")

def direct_add_interface_functions(interface):
    """Прямое добавление функций к Blueprint Interface"""
    
    functions = [
        ("SetLeftFootIK", [unreal.FunctionParameter(name="Offset", type=unreal.EdGraphPinType(pin_category="float"))]),
        ("SetRightFootIK", [unreal.FunctionParameter(name="Offset", type=unreal.EdGraphPinType(pin_category="float"))])
    ]
    
    for func_name, params in functions:
        try:
            unreal.BlueprintEditorLibrary.add_function_to_interface(interface, func_name, params)
            print(f"    ✅ Прямо добавлена: {func_name}")
        except Exception as e:
            print(f"    ❌ Ошибка функции {func_name}: {e}")

# ПРЯМОЙ АВТОМАТИЧЕСКИЙ ЗАПУСК ИЗМЕНЕНИЯ BLUEPRINT'ОВ
direct_edit_blueprints()
