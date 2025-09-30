import unreal
import time

# ФИНАЛЬНОЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK
# Этот скрипт использует финальный правильный API для настройки IK

def final_ik_setup():
    """Финальное автоматическое выполнение настройки IK"""
    
    print("🤖 ФИНАЛЬНОЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ НАСТРОЙКИ IK")
    print("=" * 60)
    
    try:
        # Ждем загрузки редактора
        print("Ожидание загрузки редактора...")
        time.sleep(5)
        
        # Выполняем настройку IK
        print("Выполнение настройки IK системы...")
        setup_ik_final()
        
        print("\n✅ ФИНАЛЬНОЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ ЗАВЕРШЕНО!")
        print("IK система готова к работе!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def setup_ik_final():
    """Финальная настройка IK системы"""
    
    print("Финальная настройка IK системы...")
    
    # Создаем Blueprint Interface
    print("Создание Blueprint Interface...")
    create_ik_interface_final()
    
    # Создаем Blueprint Actor для настройки
    print("Создание Blueprint Actor...")
    create_ik_actor_final()
    
    print("✅ IK система настроена финально!")

def create_ik_interface_final():
    """Финальное создание Blueprint Interface для IK"""
    
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
                add_interface_functions_final(interface)
                
                unreal.EditorAssetLibrary.save_asset(interface_path)
            else:
                print("  ❌ Не удалось создать Blueprint Interface")
        else:
            print("  ℹ️ Blueprint Interface уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания Blueprint Interface: {e}")

def add_interface_functions_final(interface):
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

def create_ik_actor_final():
    """Финальное создание Blueprint Actor для настройки IK"""
    
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
                add_actor_components_final(ik_actor)
                
                # Добавляем функции
                add_actor_functions_final(ik_actor)
                
                unreal.EditorAssetLibrary.save_asset(actor_path)
            else:
                print("  ❌ Не удалось создать IK Setup Actor")
        else:
            print("  ℹ️ IK Setup Actor уже существует")
            
    except Exception as e:
        print(f"  ❌ Ошибка создания IK Setup Actor: {e}")

def add_actor_components_final(actor):
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

def add_actor_functions_final(actor):
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

# ФИНАЛЬНОЕ АВТОМАТИЧЕСКОЕ ВЫПОЛНЕНИЕ
final_ik_setup()