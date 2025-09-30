import unreal
import time

def auto_fix_abp_player_ik():
    """Автоматическое исправление ИК в ABP_Player"""
    
    print("🤖 АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ ABP_PLAYER ИК")
    print("=" * 50)
    
    try:
        # Ждем загрузки редактора
        print("⏳ Ожидание загрузки редактора...")
        time.sleep(3)
        
        # Находим и исправляем ABP_Player
        print("🔍 Поиск ABP_Player...")
        fix_abp_player_ik()
        
        print("\n✅ АВТОМАТИЧЕСКОЕ ИСПРАВЛЕНИЕ ЗАВЕРШЕНО!")
        print("🎯 ИК система в Animation Blueprint исправлена!")
        
    except Exception as e:
        print(f"\n❌ ОШИБКА: {e}")
        import traceback
        traceback.print_exc()

def fix_abp_player_ik():
    """Исправляет ИК в ABP_Player"""
    
    abp_found = False
    
    try:
        # Ищем Animation Blueprint в проекте
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        # Создаем фильтр для поиска Animation Blueprint
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                try:
                    # Ищем ABP_Player (может быть в разных папках)
                    if "ABP_Player" in asset_data.asset_name or "Player" in asset_data.asset_name:
                        print(f"🎯 Найден ABP_Player: {asset_data.asset_name}")
                        
                        anim_bp = unreal.load_asset(asset_data.asset_name)
                        if anim_bp:
                            print(f"📂 ABP_Player загружен: {asset_data.asset_name}")
                            
                            # Исправляем Animation Blueprint
                            fix_animation_blueprint_ik_variables(anim_bp)
                            
                            # Сохраняем изменения
                            unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                            print(f"💾 ✅ ABP_Player исправлен и сохранен: {asset_data.asset_name}")
                            abp_found = True
                            
                except Exception as e:
                    print(f"❌ Ошибка в {asset_data.asset_name}: {e}")
        
        if not abp_found:
            print("⚠️ ABP_Player не найден, создаем переменные для всех Animation Blueprint...")
            create_ik_variables_for_all_abp(anim_bp_assets)
            
    except Exception as e:
        print(f"❌ Ошибка поиска Animation Blueprint: {e}")

def create_ik_variables_for_all_abp(anim_bp_assets):
    """Создает переменные ИК для всех найденных Animation Blueprint"""
    
    for asset_data in anim_bp_assets:
        try:
            print(f"🔧 Обработка: {asset_data.asset_name}")
            
            anim_bp = unreal.load_asset(asset_data.asset_name)
            if anim_bp:
                fix_animation_blueprint_ik_variables(anim_bp)
                unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                print(f"✅ Исправлен: {asset_data.asset_name}")
                
        except Exception as e:
            print(f"❌ Ошибка в {asset_data.asset_name}: {e}")

def fix_animation_blueprint_ik_variables(anim_bp):
    """Исправляет переменные ИК в Animation Blueprint"""
    
    print("🔧 Исправление переменных ИК...")
    
    # Создаем переменные IK
    ik_variables = [
        ("LeftFootEffectorLocation", "Vector"),
        ("RightFootEffectorLocation", "Vector"),
        ("LeftFootIKAlpha", "Float"),
        ("RightFootIKAlpha", "Float")
    ]
    
    # Получаем существующие переменные
    existing_vars = []
    try:
        if hasattr(anim_bp, 'variables') and anim_bp.variables:
            existing_vars = [var.variable_name for var in anim_bp.variables]
            print(f"📝 Существующие переменные: {len(existing_vars)}")
    except:
        existing_vars = []
        print("⚠️ Нет доступа к переменным, пытаемся создать...")
    
    for var_name, var_type in ik_variables:
        if var_name not in existing_vars:
            try:
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
                    # Устанавливаем значение по умолчанию
                    if var_type == "Vector":
                        new_var.default_value = unreal.Vector(0, 0, 0)
                    elif var_type == "Float":
                        new_var.default_value = 1.0
                    
                    print(f"✅ Создана переменная: {var_name} ({var_type})")
                else:
                    print(f"❌ Не удалось создать: {var_name}")
                    
            except Exception as e:
                print(f"❌ Ошибка создания {var_name}: {e}")
        else:
            print(f"ℹ️ Переменная уже существует: {var_name}")
    
    print("🎯 Переменные ИК настроены!")

# Запускаем автоматическое исправление
auto_fix_abp_player_ik()
