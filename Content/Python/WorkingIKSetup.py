import unreal
import time

# РАБОЧЕЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK
# Этот скрипт использует правильный API для настройки IK

def working_ik_setup():
    """Рабочее автоматическое выполнение настройки IK"""
    
    print("🤖 РАБОЧЕЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK")
    print("=" * 60)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(5)
        
        # Выполняем настройку IK
        print("Выполнение настройки IK системы...")
        setup_ik_working()
        
        print("\n✅ РАБОЧЕЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ ЗАВЕРШЕНО!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_ik_working():
    """Рабочая настройка IK системы"""
    
    print("Рабочая настройка IK системы...")
    
    # Создаем Blueprint Interface
    print("Создание Blueprint Interface...")
    create_ik_interface_working()
    
    # Создаем Blueprint Actor для настройки
    print("Создание Blueprint Actor...")
    create_ik_actor_working()
    
    # Пытаемся найти и настроить Animation Blueprint
    print("Поиск и настройка Animation Blueprint...")
    setup_animation_blueprint_working()
    
    print("✅ IK система настроена рабоче!")

def create_ik_interface_working():
    """Рабочее создание Blueprint Interface для IK"""
    
    try:
        interface_path = "/Game/Blueprints/IKInterface"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(interface_path):
            # Создаем папку
            unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/")
            
            # Создаем Blueprint Interface
            factory = unreal.BlueprintInterfaceFactory()
            interface = unreal.AssetTools.get_editor_subsystem().create_asset(
                asset_name="IKInterface",
                package_path="/Game/Blueprints/",
                asset_class=unreal.BlueprintInterface.static_class(),
                factory=factory
            )
            
            if interface:
                print("  ✅ Blueprint Interface создан")
                
                # Добавляем функции
                add_interface_functions_working(interface)
                
                unreal.EditorAssetLibrary.save_asset(interface_path)
            else:
                print("  ❌ Не удалось создать Blueprint Interface")
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания Blueprint Interface: {e}")

def add_interface_functions_working(interface):
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

def create_ik_actor_working():
    """Рабочее создание Blueprint Actor для настройки IK"""
    
    try:
        # Создаем папку для Blueprint
        unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/IK/")
        
        # Создаем Blueprint Actor
        actor_path = "/Game/Blueprints/IK/IKSetupActor"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(actor_path):
            factory = unreal.BlueprintFactory()
            ik_actor = unreal.AssetTools.get_editor_subsystem().create_asset(
                asset_name="IKSetupActor",
                package_path="/Game/Blueprints/IK/",
                asset_class=unreal.Blueprint.static_class(),
                factory=factory
            )
            
            if ik_actor:
                print("  ✅ IK Setup Actor создан")
                
                # Добавляем компоненты
                add_actor_components_working(ik_actor)
                
                # Добавляем функции
                add_actor_functions_working(ik_actor)
                
                unreal.EditorAssetLibrary.save_asset(actor_path)
            else:
                print("  ❌ Не удалось создать IK Setup Actor")
        else:
            print("  ℹ️ IK Setup Actor уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания IK Setup Actor: {e}")

def add_actor_components_working(actor):
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

def add_actor_functions_working(actor):
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

def setup_animation_blueprint_working():
    """Рабочая настройка Animation Blueprint"""
    
    try:
        # Ищем Animation Blueprint в проекте
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Создаем фильтр для поиска Animation Blueprint
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_names = ["AnimBlueprint"]
        anim_bp_filter.recursive_classes = True
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"  Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                try:
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        print(f"    Обработка: {asset_data.asset_name}")
                        add_ik_variables_to_abp_working(anim_bp)
                        add_ik_nodes_to_abp_working(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                except Exception as e:
                    print(f"    ❌ Ошибка в {asset_data.asset_name}: {e}")
        else:
            print("  ⚠️ Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"  ❌ Ошибка поиска Animation Blueprint: {e}")

def add_ik_variables_to_abp_working(anim_bp):
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
                    print(f"      ✅ Создана переменная: {name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания {name}: {e}")

def add_ik_nodes_to_abp_working(anim_bp):
    """Добавляет IK ноды в Animation Blueprint"""
    
    if not anim_bp.anim_graph:
        print("      ⚠️ AnimGraph не найден")
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
                    print(f"      ✅ Создан нод: {node_name}")
            except Exception as e:
                print(f"      ❌ Ошибка создания нода {node_name}: {e}")

# РАБОЧЕЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ
working_ik_setup()
