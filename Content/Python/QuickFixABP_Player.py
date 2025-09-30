import unreal
import time

def quick_fix_abp_player():
    """Быстрое исправление ABP_Player"""
    
    print("🔧 БЫСТРОЕ ИСПРАВЛЕНИЕ ABP_PLAYER ИК")
    print("=" * 40)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # Находим ABP_Player
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            print(f"Найден ABP_Player: {abp_path}")
            
            # Загружаем Animation Blueprint
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
                print("ABP_Player загружен успешно")
                
                # Добавляем переменные IK
                add_ik_variables_to_abp(anim_bp)
                
                # Сохраняем
                unreal.EditorAssetLibrary.save_asset(abp_path)
                print("✅ ABP_Player исправлен и сохранен!")
                
            else:
                print("❌ Не удалось загрузить ABP_Player")
        else:
            print(f"❌ ABP_Player не найден по пути: {abp_path}")
            
            # Ищем все Animation Blueprint
            print("Поиск всех Animation Blueprint...")
            find_all_animation_blueprints()
            
    except Exception as e:
        print(f"❌ Ошибка: {e}")
        import traceback
        traceback.print_exc()

def find_all_animation_blueprints():
    """Находит все Animation Blueprint в проекте"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"Найдено Animation Blueprint: {len(anim_bp_assets)}")
            for asset_data in anim_bp_assets:
                print(f"  - {asset_data.asset_name}")
        else:
            print("Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"Ошибка поиска: {e}")

def add_ik_variables_to_abp(anim_bp):
    """Добавляет переменные IK в Animation Blueprint"""
    
    print("Добавление переменных IK...")
    
    variables_to_add = [
        ("LeftFootEffectorLocation", "Vector"),
        ("RightFootEffectorLocation", "Vector"),
        ("LeftFootIKAlpha", "Float"),
        ("RightFootIKAlpha", "Float")
    ]
    
    for var_name, var_type in variables_to_add:
        try:
            # Проверяем, существует ли переменная
            if hasattr(anim_bp, 'variables'):
                existing_vars = [var.variable_name for var in anim_bp.variables] if anim_bp.variables else []
                
                if var_name not in existing_vars:
                    # Создаем переменную
                    if var_type == "Vector":
                        new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                            anim_bp, var_name, unreal.Vector
                        )
                    elif var_type == "Float":
                        new_var = unreal.EditorAnimBlueprintUtils.add_variable(
                            anim_bp, var_name, unreal.Float
                        )
                    
                    if new_var:
                        new_var.variable_name = var_name
                        print(f"  ✅ Создана переменная: {var_name}")
                    else:
                        print(f"  ❌ Не удалось создать: {var_name}")
                else:
                    print(f"  ℹ️ Переменная уже существует: {var_name}")
            else:
                print("  ⚠️ Нет доступа к переменным Animation Blueprint")
                
        except Exception as e:
            print(f"  ❌ Ошибка создания {var_name}: {e}")

# Запускаем быстрое исправление
quick_fix_abp_player()
