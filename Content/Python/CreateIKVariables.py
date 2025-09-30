import unreal

def create_ik_variables_in_abp():
    """Создает необходимые переменные для IK в Animation Blueprint"""
    
    print("=== Создание переменных IK в Animation Blueprint ===")
    
    # Находим все Animation Blueprint в проекте
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
            
            # Создаем переменные
            create_variables(anim_bp)
            
            # Сохраняем изменения
            unreal.EditorAssetLibrary.save_asset(asset_path)
            print(f"Переменные созданы и сохранены для: {asset_path}")
            
        except Exception as e:
            print(f"Ошибка при обработке {asset_path}: {e}")

def create_variables(anim_bp):
    """Создает переменные IK в Animation Blueprint"""
    
    # Список переменных для создания
    variables_to_create = [
        {
            "name": "LeftFootEffectorLocation",
            "type": unreal.Vector,
            "default_value": unreal.Vector(0, 0, 0),
            "description": "Позиция эффектора для левой ноги"
        },
        {
            "name": "RightFootEffectorLocation", 
            "type": unreal.Vector,
            "default_value": unreal.Vector(0, 0, 0),
            "description": "Позиция эффектора для правой ноги"
        },
        {
            "name": "LeftFootIKAlpha",
            "type": unreal.Float,
            "default_value": 1.0,
            "description": "Сила IK для левой ноги"
        },
        {
            "name": "RightFootIKAlpha",
            "type": unreal.Float,
            "default_value": 1.0,
            "description": "Сила IK для правой ноги"
        }
    ]
    
    # Получаем существующие переменные
    existing_vars = [var.variable_name for var in anim_bp.variables]
    
    for var_info in variables_to_create:
        var_name = var_info["name"]
        
        if var_name not in existing_vars:
            try:
                # Создаем переменную
                new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                    anim_bp,
                    var_name,
                    var_info["type"]
                )
                
                if new_var:
                    new_var.variable_name = var_name
                    new_var.default_value = var_info["default_value"]
                    print(f"✓ Создана переменная: {var_name}")
                else:
                    print(f"✗ Не удалось создать переменную: {var_name}")
                    
            except Exception as e:
                print(f"✗ Ошибка создания переменной {var_name}: {e}")
        else:
            print(f"→ Переменная уже существует: {var_name}")

def create_ik_functions():
    """Создает Blueprint функции для IK"""
    
    print("\n=== Создание Blueprint функций для IK ===")
    
    # Находим Animation Blueprint
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    anim_bp_assets = asset_registry.get_assets_by_class("AnimBlueprint", True)
    
    for asset_data in anim_bp_assets:
        asset_path = asset_data.asset_name
        print(f"Создание функций в: {asset_path}")
        
        try:
            anim_bp = unreal.load_asset(asset_path)
            if anim_bp:
                create_ik_functions_in_abp(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_path)
                print(f"Функции созданы в: {asset_path}")
                
        except Exception as e:
            print(f"Ошибка создания функций в {asset_path}: {e}")

def create_ik_functions_in_abp(anim_bp):
    """Создает функции IK в Animation Blueprint"""
    
    # Создаем функцию SetLeftFootIK
    try:
        left_foot_func = unreal.EditorAnimBlueprintUtils.add_function(
            anim_bp,
            "SetLeftFootIK"
        )
        if left_foot_func:
            print("✓ Создана функция: SetLeftFootIK")
    except Exception as e:
        print(f"Ошибка создания SetLeftFootIK: {e}")
    
    # Создаем функцию SetRightFootIK
    try:
        right_foot_func = unreal.EditorAnimBlueprintUtils.add_function(
            anim_bp,
            "SetRightFootIK"
        )
        if right_foot_func:
            print("✓ Создана функция: SetRightFootIK")
    except Exception as e:
        print(f"Ошибка создания SetRightFootIK: {e}")

def main():
    """Основная функция"""
    print("=== Автоматическая настройка IK системы ===")
    
    try:
        # Создаем переменные
        create_ik_variables_in_abp()
        
        # Создаем функции
        create_ik_functions()
        
        print("\n=== Настройка завершена! ===")
        print("Теперь в Animation Blueprint доступны:")
        print("- LeftFootEffectorLocation (Vector)")
        print("- RightFootEffectorLocation (Vector)")
        print("- LeftFootIKAlpha (Float)")
        print("- RightFootIKAlpha (Float)")
        print("- SetLeftFootIK (Function)")
        print("- SetRightFootIK (Function)")
        
    except Exception as e:
        print(f"Общая ошибка: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()
