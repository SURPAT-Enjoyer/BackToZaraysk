import unreal
import time

def fix_animinstance_class():
    """Исправляет класс AnimInstance для ABP_Player"""
    
    print("🎭 ИСПРАВЛЕНИЕ КЛАССА ANIMINSTANCE")
    print("=" * 40)
    
    try:
        # Ждем загрузки
        time.sleep(1)
        
        # Находим ABP_Player
        abp_paths = [
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1"
        ]
        
        abp_found = False
        for abp_path in abp_paths:
            if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
                print(f"✅ Найден ABP_Player: {abp_path}")
                
                anim_bp = unreal.load_asset(abp_path)
                if anim_bp:
                    print(f"📂 ABP_Player загружен")
                    
                    # Проверяем и исправляем класс
                    check_and_fix_class(anim_bp, abp_path)
                    abp_found = True
                    break
        
        if not abp_found:
            print("❌ ABP_Player не найден по стандартным путям")
            find_all_player_abp()
            
    except Exception as e:
        print(f"❌ Ошибка: {e}")
        import traceback
        traceback.print_exc()

def find_all_player_abp():
    """Находит все Player Animation Blueprint"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            player_abps = [asset for asset in anim_bp_assets if "Player" in asset.asset_name]
            
            if player_abps:
                print(f"📋 Найдено Player Animation Blueprint: {len(player_abps)}")
                for asset in player_abps:
                    print(f"  - {asset.asset_name}")
                    
                    # Пытаемся исправить каждый найденный ABP
                    anim_bp = unreal.load_asset(asset.asset_name)
                    if anim_bp:
                        check_and_fix_class(anim_bp, asset.asset_name)
            else:
                print("❌ Player Animation Blueprint не найдены")
        else:
            print("❌ Animation Blueprint не найдены в проекте")
            
    except Exception as e:
        print(f"❌ Ошибка поиска: {e}")

def check_and_fix_class(anim_bp, asset_path):
    """Проверяет и исправляет класс AnimInstance"""
    
    print(f"\n🔍 Проверка: {asset_path}")
    
    try:
        # Проверяем текущий класс
        current_class = None
        if hasattr(anim_bp, 'parent_class'):
            current_class = anim_bp.parent_class
            if current_class:
                current_class_name = current_class.get_name()
                print(f"  Текущий класс: {current_class_name}")
                
                # Проверяем, нужен ли наш кастомный класс
                if "BTZBaseCharacterAnimInstance" in current_class_name:
                    print(f"  ✅ Уже используется правильный класс!")
                    return True
                else:
                    print(f"  ❌ Используется неправильный класс!")
                    print(f"     Нужно: UBTZBaseCharacterAnimInstance")
                    
                    # Пытаемся найти и установить правильный класс
                    return try_set_correct_class(anim_bp, asset_path)
            else:
                print(f"  ❌ Не удалось получить текущий класс")
                return False
        else:
            print(f"  ❌ Нет доступа к parent_class")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки класса: {e}")
        return False

def try_set_correct_class(anim_bp, asset_path):
    """Пытается установить правильный класс"""
    
    print(f"  🔧 Попытка установки правильного класса...")
    
    try:
        # Ищем наш кастомный класс
        target_class_names = [
            "/Script/BackToZaraysk.UBTZBaseCharacterAnimInstance",
            "/Script/BackToZaraysk.BTZBaseCharacterAnimInstance"
        ]
        
        for class_name in target_class_names:
            try:
                target_class = unreal.load_class(None, class_name)
                if target_class:
                    print(f"    ✅ Найден класс: {target_class.get_name()}")
                    
                    # Устанавливаем новый родительский класс
                    anim_bp.parent_class = target_class
                    print(f"    ✅ Установлен класс: {target_class.get_name()}")
                    
                    # Сохраняем изменения
                    unreal.EditorAssetLibrary.save_asset(asset_path)
                    print(f"    💾 Файл сохранен")
                    
                    return True
                    
            except Exception as e:
                print(f"    ⚠️ Не удалось загрузить класс {class_name}: {e}")
                continue
        
        # Если автоматическое исправление не сработало
        print(f"    ❌ Автоматическое исправление не удалось")
        print_manual_instructions()
        return False
        
    except Exception as e:
        print(f"    ❌ Ошибка установки класса: {e}")
        print_manual_instructions()
        return False

def print_manual_instructions():
    """Выводит инструкции для ручного исправления"""
    
    print(f"\n💡 РУЧНОЕ ИСПРАВЛЕНИЕ:")
    print(f"   1. Откройте Animation Blueprint в редакторе")
    print(f"   2. Нажмите F4 или кнопку 'Class Settings'")
    print(f"   3. В 'Parent Class' выберите 'UBTZBaseCharacterAnimInstance'")
    print(f"   4. Сохраните файл (Ctrl+S)")
    print(f"   5. Перезапустите редактор")

# Запускаем исправление
fix_animinstance_class()
