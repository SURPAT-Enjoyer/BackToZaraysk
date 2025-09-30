import unreal

def run_ik_setup_in_editor():
    """Запускает настройку IK внутри Unreal Editor"""
    
    print("=== НАСТРОЙКА IK В РЕДАКТОРЕ ===")
    
    # Находим все Animation Blueprint
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    print(f"Найдено Animation Blueprint: {len(anim_bp_assets)}")
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"\nОбрабатываем: {asset_path}")
        
        try:
            # Загружаем Animation Blueprint
            anim_bp = unreal.load_asset(asset_path)
            if not anim_bp:
                print(f"Не удалось загрузить: {asset_path}")
                continue
            
            # Создаем переменные IK
            create_ik_variables_in_abp(anim_bp)
            
            # Сохраняем
            unreal.EditorAssetLibrary.save_asset(asset_path)
            print(f"✅ Настройка завершена для: {asset_path}")
            
        except Exception as e:
            print(f"❌ Ошибка в {asset_path}: {e}")

def create_ik_variables_in_abp(anim_bp):
    """Создает переменные IK в Animation Blueprint"""
    
    # Список переменных для создания
    variables_to_create = [
        ("LeftFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("RightFootEffectorLocation", unreal.Vector, unreal.Vector(0, 0, 0)),
        ("LeftFootIKAlpha", unreal.Float, 1.0),
        ("RightFootIKAlpha", unreal.Float, 1.0)
    ]
    
    # Получаем существующие переменные
    existing_vars = [var.variable_name for var in anim_bp.variables]
    
    created_count = 0
    
    for var_name, var_type, default_value in variables_to_create:
        if var_name not in existing_vars:
            try:
                # Создаем переменную
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp,
                    var_name,
                    var_type
                )
                
                if new_var:
                    new_var.variable_name = var_name
                    new_var.default_value = default_value
                    created_count += 1
                    print(f"✅ Создана переменная: {var_name}")
                else:
                    print(f"❌ Не удалось создать: {var_name}")
                    
            except Exception as e:
                print(f"❌ Ошибка создания {var_name}: {e}")
        else:
            print(f"ℹ️ Переменная уже существует: {var_name}")
    
    print(f"Создано переменных: {created_count}")

def create_ik_functions_in_abp(anim_bp):
    """Создает функции IK в Animation Blueprint"""
    
    functions_to_create = [
        "SetLeftFootIK",
        "SetRightFootIK",
        "GetLeftFootIK", 
        "GetRightFootIK"
    ]
    
    created_count = 0
    
    for func_name in functions_to_create:
        try:
            # Проверяем, существует ли функция
            existing_functions = [func.function_name for func in anim_bp.functions]
            
            if func_name not in existing_functions:
                new_func = unreal.EditorAnimBlueprintUtils.add_function(
                    anim_bp,
                    func_name
                )
                
                if new_func:
                    created_count += 1
                    print(f"✅ Создана функция: {func_name}")
                else:
                    print(f"❌ Не удалось создать функцию: {func_name}")
            else:
                print(f"ℹ️ Функция уже существует: {func_name}")
                
        except Exception as e:
            print(f"❌ Ошибка создания функции {func_name}: {e}")
    
    print(f"Создано функций: {created_count}")

def setup_character_ik():
    """Настраивает IK для персонажа"""
    
    print("\n=== НАСТРОЙКА ПЕРСОНАЖА ===")
    
    # Находим Blueprint персонажа
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    blueprint_assets = asset_registry.get_assets_by_class("Blueprint", True)
    
    character_found = False
    
    for asset_data in blueprint_assets:
        if "PlayerCharacter" in str(asset_data.asset_name):
            character_found = True
            print(f"Найден персонаж: {asset_data.asset_name}")
            
            try:
                character_bp = unreal.load_asset(asset_data.asset_name)
                if character_bp:
                    check_character_sockets(character_bp)
                    
            except Exception as e:
                print(f"❌ Ошибка настройки персонажа: {e}")
    
    if not character_found:
        print("ℹ️ Персонаж не найден. Убедитесь, что Blueprint называется 'PlayerCharacter'")

def check_character_sockets(character_bp):
    """Проверяет сокеты персонажа"""
    
    # Получаем компонент скелета
    skeletal_mesh_comp = None
    for comp in character_bp.components:
        if isinstance(comp, unreal.SkeletalMeshComponent):
            skeletal_mesh_comp = comp
            break
    
    if skeletal_mesh_comp:
        # Проверяем сокеты
        left_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_l")
        right_socket_exists = skeletal_mesh_comp.does_socket_exist("foot_r")
        
        print(f"Сокет foot_l: {'✅' if left_socket_exists else '❌'}")
        print(f"Сокет foot_r: {'✅' if right_socket_exists else '❌'}")
        
        if not left_socket_exists or not right_socket_exists:
            print("ℹ️ Сокеты для ног нужно добавить вручную:")
            print("1. Откройте скелет персонажа")
            print("2. Добавьте сокет 'foot_l' к кости левой ноги")
            print("3. Добавьте сокет 'foot_r' к кости правой ноги")

def main():
    """Главная функция"""
    print("🚀 ЗАПУСК НАСТРОЙКИ IK В РЕДАКТОРЕ")
    print("=" * 50)
    
    try:
        # Настраиваем Animation Blueprint
        run_ik_setup_in_editor()
        
        # Настраиваем персонажа
        setup_character_ik()
        
        print("\n" + "=" * 50)
        print("✅ НАСТРОЙКА ЗАВЕРШЕНА!")
        print("=" * 50)
        print("Созданы переменные:")
        print("• LeftFootEffectorLocation (Vector)")
        print("• RightFootEffectorLocation (Vector)")
        print("• LeftFootIKAlpha (Float)")
        print("• RightFootIKAlpha (Float)")
        print("\nСледующие шаги:")
        print("1. Откройте Animation Blueprint")
        print("2. Добавьте Two Bone IK ноды")
        print("3. Подключите переменные к IK нодам")
        print("4. Настройте кости для IK")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
