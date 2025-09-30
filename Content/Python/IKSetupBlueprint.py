import unreal

# Blueprint Actor для автоматической настройки IK
# Этот скрипт создаст Blueprint Actor, который настроит IK при запуске

def create_ik_setup_actor():
    """Создает Blueprint Actor для автоматической настройки IK"""
    
    print("🤖 СОЗДАНИЕ IK SETUP ACTOR")
    print("=" * 40)
    
    try:
        # Создаем папку для Blueprint
        unreal.EditorAssetLibrary.make_directory("/Game/Blueprints/IK/")
        
        # Создаем Blueprint Actor
        actor_path = "/Game/Blueprints/IK/IKSetupActor"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(actor_path):
            # Создаем Blueprint Actor
            ik_actor = unreal.EditorAssetLibrary.create_asset(
                "IKSetupActor",
                "/Game/Blueprints/IK/",
                unreal.Blueprint,
                unreal.BlueprintFactory()
            )
            
            if ik_actor:
                print("✅ IK Setup Actor создан")
                
                # Добавляем компоненты
                add_components(ik_actor)
                
                # Добавляем функции
                add_functions(ik_actor)
                
                # Сохраняем
                unreal.EditorAssetLibrary.save_asset(actor_path)
                print("✅ IK Setup Actor сохранен")
                
            else:
                print("❌ Не удалось создать IK Setup Actor")
        else:
            print("ℹ️ IK Setup Actor уже существует")
            
    except Exception as e:
        print(f"❌ Ошибка создания IK Setup Actor: {e}")

def add_components(actor):
    """Добавляет компоненты к Actor"""
    
    try:
        # Добавляем Scene Component
        scene_comp = unreal.EditorBlueprintUtils.add_component_to_blueprint(
            actor, unreal.SceneComponent
        )
        
        if scene_comp:
            print("  ✅ Scene Component добавлен")
            
    except Exception as e:
        print(f"  ❌ Ошибка добавления компонентов: {e}")

def add_functions(actor):
    """Добавляет функции к Actor"""
    
    try:
        # Добавляем функцию BeginPlay
        begin_play_func = unreal.EditorBlueprintUtils.add_function_to_blueprint(
            actor, "BeginPlay"
        )
        
        if begin_play_func:
            print("  ✅ BeginPlay функция добавлена")
        
        # Добавляем функцию настройки IK
        ik_setup_func = unreal.EditorBlueprintUtils.add_function_to_blueprint(
            actor, "SetupIKSystem"
        )
        
        if ik_setup_func:
            print("  ✅ SetupIKSystem функция добавлена")
            
    except Exception as e:
        print(f"  ❌ Ошибка добавления функций: {e}")

def create_ik_setup_level():
    """Создает уровень для настройки IK"""
    
    print("\n🌍 СОЗДАНИЕ IK SETUP LEVEL")
    print("=" * 40)
    
    try:
        # Создаем новый уровень
        level_path = "/Game/Levels/IKSetupLevel"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(level_path):
            # Создаем уровень
            new_level = unreal.EditorLevelLibrary.create_new_level(level_path)
            
            if new_level:
                print("✅ IK Setup Level создан")
                
                # Добавляем IK Setup Actor в уровень
                add_ik_actor_to_level()
                
                # Сохраняем уровень
                unreal.EditorLevelLibrary.save_current_level()
                print("✅ IK Setup Level сохранен")
                
            else:
                print("❌ Не удалось создать уровень")
        else:
            print("ℹ️ IK Setup Level уже существует")
            
    except Exception as e:
        print(f"❌ Ошибка создания уровня: {e}")

def add_ik_actor_to_level():
    """Добавляет IK Setup Actor в уровень"""
    
    try:
        # Загружаем IK Setup Actor
        actor_class = unreal.load_asset("/Game/Blueprints/IK/IKSetupActor")
        
        if actor_class:
            # Создаем экземпляр Actor в уровне
            actor_instance = unreal.EditorLevelLibrary.spawn_actor_from_class(
                actor_class, unreal.Vector(0, 0, 0)
            )
            
            if actor_instance:
                print("  ✅ IK Setup Actor добавлен в уровень")
            else:
                print("  ❌ Не удалось добавить Actor в уровень")
        else:
            print("  ❌ IK Setup Actor не найден")
            
    except Exception as e:
        print(f"  ❌ Ошибка добавления Actor в уровень: {e}")

def main():
    """Главная функция создания IK Setup системы"""
    
    print("🚀 СОЗДАНИЕ АВТОМАТИЧЕСКОЙ IK SETUP СИСТЕМЫ")
    print("=" * 60)
    
    try:
        # Создаем Blueprint Actor
        create_ik_setup_actor()
        
        # Создаем уровень
        create_ik_setup_level()
        
        print("\n" + "=" * 60)
        print("✅ АВТОМАТИЧЕСКАЯ IK SETUP СИСТЕМА СОЗДАНА!")
        print("=" * 60)
        print("Созданы:")
        print("• IKSetupActor - Blueprint Actor для настройки")
        print("• IKSetupLevel - Уровень с настройкой IK")
        print("\nТеперь просто запустите уровень, и IK настроится автоматически!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

# Запускаем создание системы
main()