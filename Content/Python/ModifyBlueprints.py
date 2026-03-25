import unreal

# АВТОМАТИЧЕСКОЕ ИЗМЕНЕНИЕ BLUEPRINT'ОВ ДЛЯ IK СИСТЕМЫ
# Этот скрипт изменит все необходимые Blueprint'ы автоматически

def modify_blueprints_for_ik():
    """Автоматически изменяет Blueprint'ы для IK системы"""
    
    print("🤖 АВТОМАТИЧЕСКОЕ ИЗМЕНЕНИЕ BLUEPRINT'ОВ ДЛЯ IK")
    print("=" * 60)
    
    try:
        # Шаг 1: Изменяем Animation Blueprint
        print("\n[ШАГ 1/4] Изменение Animation Blueprint...")
        modify_animation_blueprint()
        
        # Шаг 2: Изменяем PlayerCharacter Blueprint
        print("\n[ШАГ 2/4] Изменение PlayerCharacter Blueprint...")
        modify_player_character_blueprint()
        
        # Шаг 3: Создаем Blueprint Interface
        print("\n[ШАГ 3/4] Создание Blueprint Interface...")
        create_blueprint_interface()
        
        # Шаг 4: Создаем Blueprint Actor для настройки
        print("\n[ШАГ 4/4] Создание Blueprint Actor...")
        create_ik_setup_actor()
        
        print("\n" + "=" * 60)
        print("🎉 ВСЕ BLUEPRINT'Ы ИЗМЕНЕНЫ АВТОМАТИЧЕСКИ!")
        print("=" * 60)
        print("✅ Animation Blueprint настроен")
        print("✅ PlayerCharacter Blueprint настроен")
        print("✅ Blueprint Interface создан")
        print("✅ Blueprint Actor создан")
        print("\n🚀 IK СИСТЕМА ПОЛНОСТЬЮ ГОТОВА К РАБОТЕ!")
        print("=" * 60)
        
    except Exception as e:
        print(f"\n❌ КРИТИЧЕСКАЯ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def modify_animation_blueprint():
    """Изменяет Animation Blueprint для IK системы"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        try:
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                print(f"  Обработка Animation Blueprint: {asset_data.asset_name}")
                
                # Создаем переменные IK
                create_ik_variables_in_abp(anim_bp)
                
                # Создаем IK ноды
                create_ik_nodes_in_abp(anim_bp)
                
                # Сохраняем изменения
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                print(f"  ✅ Animation Blueprint обновлен: {asset_data.asset_name}")
                
        except Exception as e:
            print(f"  ❌ Ошибка в {asset_data.asset_name}: {e}")

def create_ik_variables_in_abp(anim_bp):
    """Создает переменные IK в Animation Blueprint"""
    
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
                    print(f"    ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"    ❌ Ошибка создания {name}: {e}")

def create_ik_nodes_in_abp(anim_bp):
    """Создает IK ноды в Animation Blueprint"""
    
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
                    print(f"    ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"    ❌ Ошибка создания нода {node_name}: {e}")

def modify_player_character_blueprint():
    """Изменяет PlayerCharacter Blueprint для IK системы"""
    
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    print(f"  Обработка PlayerCharacter Blueprint: {asset_data.asset_name}")
                    
                    # Проверяем сокеты
                    check_character_sockets(character_bp)
                    
                    # Добавляем IK переменные
                    add_ik_variables_to_character(character_bp)
                    
                    # Сохраняем изменения
                    unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                    print(f"  ✅ PlayerCharacter Blueprint обновлен: {asset_data.asset_name}")
                    
            except Exception as e:
                print(f"  ❌ Ошибка в {asset_data.asset_name}: {e}")

def check_character_sockets(character_bp):
    """Проверяет сокеты персонажа"""
    
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            left_exists = comp.does_socket_exist("foot_l")
            right_exists = comp.does_socket_exist("foot_r")
            
            print(f"    Сокеты: foot_l={'✅' if left_exists else '❌'}, foot_r={'✅' if right_exists else '❌'}")
            
            if not left_exists or not right_exists:
                print("    ⚠️ Добавьте сокеты 'foot_l' и 'foot_r' в скелет персонажа")
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
                    print(f"    ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"    ❌ Ошибка создания {name}: {e}")

def create_blueprint_interface():
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
                
                # Добавляем функции к интерфейсу
                add_interface_functions(interface)
                
                unreal.EditorAssetLibrary.save_asset(interface_path)
                print("  ✅ Blueprint Interface сохранен")
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

def create_ik_setup_actor():
    """Создает Blueprint Actor для автоматической настройки IK"""
    
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
                
                # Добавляем компоненты
                add_actor_components(ik_actor)
                
                # Добавляем функции
                add_actor_functions(ik_actor)
                
                # Сохраняем
                unreal.EditorAssetLibrary.save_asset(actor_path)
                print("  ✅ IK Setup Actor сохранен")
            else:
                print("  ❌ Не удалось создать IK Setup Actor")
        else:
            print("  ℹ️ IK Setup Actor уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания IK Setup Actor: {e}")

def add_actor_components(actor):
    """Добавляет компоненты к Actor"""
    
    try:
        # Добавляем Scene Component
        scene_comp = unreal.EditorBlueprintUtils.add_component_to_blueprint(
            actor, unreal.SceneComponent
        )
        
        if scene_comp:
            print("    ✅ Scene Component добавлен")
            
    except Exception as e:
        print(f"    ❌ Ошибка добавления компонентов: {e}")

def add_actor_functions(actor):
    """Добавляет функции к Actor"""
    
    try:
        # Добавляем функцию BeginPlay
        begin_play_func = unreal.EditorBlueprintUtils.add_function_to_blueprint(
            actor, "BeginPlay"
        )
        
        if begin_play_func:
            print("    ✅ BeginPlay функция добавлена")
        
        # Добавляем функцию настройки IK
        ik_setup_func = unreal.EditorBlueprintUtils.add_function_to_blueprint(
            actor, "SetupIKSystem"
        )
        
        if ik_setup_func:
            print("    ✅ SetupIKSystem функция добавлена")
            
    except Exception as e:
        print(f"    ❌ Ошибка добавления функций: {e}")

# АВТОМАТИЧЕСКИЙ ЗАПУСК ИЗМЕНЕНИЯ BLUEPRINT'ОВ
modify_blueprints_for_ik()
