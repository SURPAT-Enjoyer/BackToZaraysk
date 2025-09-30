import unreal

# Быстрое исправление класса AnimInstance
try:
    print("🎭 Быстрое исправление класса AnimInstance...")
    
    # Находим ABP_Player
    abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
    
    if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
        print(f"✅ Найден ABP_Player: {abp_path}")
        
        anim_bp = unreal.load_asset(abp_path)
        if anim_bp:
            print(f"📂 ABP_Player загружен")
            
            # Проверяем текущий класс
            if hasattr(anim_bp, 'parent_class') and anim_bp.parent_class:
                current_class = anim_bp.parent_class.get_name()
                print(f"📋 Текущий класс: {current_class}")
                
                if "BTZBaseCharacterAnimInstance" in current_class:
                    print(f"✅ Уже используется правильный класс!")
                else:
                    print(f"❌ Неправильный класс! Нужно: UBTZBaseCharacterAnimInstance")
                    print(f"💡 РУЧНОЕ ИСПРАВЛЕНИЕ:")
                    print(f"   1. Откройте ABP_Player в редакторе")
                    print(f"   2. Нажмите F4")
                    print(f"   3. В 'Parent Class' выберите 'UBTZBaseCharacterAnimInstance'")
                    print(f"   4. Сохраните (Ctrl+S)")
            else:
                print(f"❌ Не удалось получить родительский класс")
                print(f"💡 РУЧНОЕ ИСПРАВЛЕНИЕ:")
                print(f"   1. Откройте ABP_Player в редакторе")
                print(f"   2. Нажмите F4")
                print(f"   3. В 'Parent Class' выберите 'UBTZBaseCharacterAnimInstance'")
                print(f"   4. Сохраните (Ctrl+S)")
        else:
            print(f"❌ Не удалось загрузить ABP_Player")
    else:
        print(f"❌ ABP_Player не найден по пути: {abp_path}")
        print(f"🔍 Поиск всех Animation Blueprint...")
        
        # Ищем все Animation Blueprint
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            player_abps = [asset for asset in anim_bp_assets if "Player" in asset.asset_name]
            
            if player_abps:
                print(f"📋 Найдено Player Animation Blueprint:")
                for asset in player_abps:
                    print(f"  - {asset.asset_name}")
            else:
                print("❌ Player Animation Blueprint не найдены")
        else:
            print("❌ Animation Blueprint не найдены в проекте")

except Exception as e:
    print(f"❌ Ошибка: {e}")
    print(f"💡 РУЧНОЕ ИСПРАВЛЕНИЕ:")
    print(f"   1. Откройте ABP_Player в редакторе")
    print(f"   2. Нажмите F4")
    print(f"   3. В 'Parent Class' выберите 'UBTZBaseCharacterAnimInstance'")
    print(f"   4. Сохраните (Ctrl+S)")

print(f"\n✅ Проверка завершена!")
