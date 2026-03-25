import unreal
import time

def set_correct_animinstance_class():
    """Устанавливает правильный класс AnimInstance для ABP_Player"""
    
    print("🎭 УСТАНОВКА ПРАВИЛЬНОГО КЛАССА ANIMINSTANCE")
    print("=" * 50)
    
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
                    
                    # Устанавливаем правильный класс AnimInstance
                    set_animinstance_class(anim_bp)
                    
                    # Сохраняем изменения
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print(f"💾 ✅ ABP_Player обновлен и сохранен")
                    abp_found = True
                    break
        
        if not abp_found:
            print("❌ ABP_Player не найден по стандартным путям")
            print("🔍 Поиск всех Animation Blueprint...")
            find_and_fix_all_animation_blueprints()
            
    except Exception as e:
        print(f"❌ Ошибка установки класса: {e}")
        import traceback
        traceback.print_exc()

def find_and_fix_all_animation_blueprints():
    """Находит и исправляет все Animation Blueprint"""
    
    try:
        asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
        
        anim_bp_filter = unreal.ARFilter()
        anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
        
        anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
        
        if anim_bp_assets:
            print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
            
            for asset_data in anim_bp_assets:
                if "Player" in asset_data.asset_name or "Character" in asset_data.asset_name:
                    print(f"\n🔧 Исправление: {asset_data.asset_name}")
                    
                    anim_bp = unreal.load_asset(asset_data.asset_name)
                    if anim_bp:
                        set_animinstance_class(anim_bp)
                        unreal.EditorAssetLibrary.save_asset(asset_data.asset_name)
                        print(f"✅ Исправлен: {asset_data.asset_name}")
                        
    except Exception as e:
        print(f"❌ Ошибка поиска: {e}")

def set_animinstance_class(anim_bp):
    """Устанавливает правильный класс AnimInstance"""
    
    print("\n🎭 УСТАНОВКА КЛАССА ANIMINSTANCE")
    print("-" * 40)
    
    try:
        # Проверяем текущий класс
        current_class = None
        if hasattr(anim_bp, 'parent_class'):
            current_class = anim_bp.parent_class
            if current_class:
                current_class_name = current_class.get_name()
                print(f"📋 Текущий родительский класс: {current_class_name}")
                
                # Проверяем, нужен ли наш кастомный класс
                if "BTZBaseCharacterAnimInstance" in current_class_name:
                    print(f"✅ Уже используется правильный класс: {current_class_name}")
                    return
                else:
                    print(f"⚠️ Используется неправильный класс: {current_class_name}")
            else:
                print(f"❌ Не удалось получить текущий класс")
        else:
            print(f"❌ Нет доступа к parent_class")
        
        # Ищем наш кастомный класс AnimInstance
        print(f"\n🔍 Поиск UBTZBaseCharacterAnimInstance...")
        
        # Попробуем найти класс по имени
        target_class_name = "/Script/BackToZaraysk.UBTZBaseCharacterAnimInstance"
        
        try:
            # Загружаем класс
            target_class = unreal.load_class(None, target_class_name)
            if target_class:
                print(f"✅ Найден целевой класс: {target_class.get_name()}")
                
                # Устанавливаем новый родительский класс
                anim_bp.parent_class = target_class
                print(f"✅ Установлен новый родительский класс: {target_class.get_name()}")
                
                # Проверяем, что класс установлен
                if hasattr(anim_bp, 'parent_class') and anim_bp.parent_class:
                    new_class_name = anim_bp.parent_class.get_name()
                    print(f"✅ Подтверждено: новый класс = {new_class_name}")
                else:
                    print(f"❌ Не удалось подтвердить установку класса")
                    
            else:
                print(f"❌ Не удалось загрузить класс: {target_class_name}")
                print(f"\n💡 АЛЬТЕРНАТИВНЫЙ СПОСОБ:")
                print(f"   1. Откройте Animation Blueprint вручную")
                print(f"   2. В Class Settings → Parent Class")
                print(f"   3. Найдите и выберите UBTZBaseCharacterAnimInstance")
                print(f"   4. Сохраните Animation Blueprint")
                
        except Exception as e:
            print(f"❌ Ошибка установки класса: {e}")
            print(f"\n💡 РУЧНОЕ ИСПРАВЛЕНИЕ:")
            print(f"   1. Откройте Animation Blueprint")
            print(f"   2. В Class Settings → Parent Class")
            print(f"   3. Выберите UBTZBaseCharacterAnimInstance")
            print(f"   4. Сохраните файл")
            
    except Exception as e:
        print(f"❌ Ошибка установки класса AnimInstance: {e}")

def check_animinstance_class(anim_bp):
    """Проверяет класс AnimInstance"""
    
    print("\n🔍 ПРОВЕРКА КЛАССА ANIMINSTANCE:")
    
    try:
        if hasattr(anim_bp, 'parent_class'):
            parent_class = anim_bp.parent_class
            if parent_class:
                class_name = parent_class.get_name()
                print(f"  Родительский класс: {class_name}")
                
                if "BTZBaseCharacterAnimInstance" in class_name:
                    print(f"  ✅ Используется правильный кастомный AnimInstance")
                    return True
                else:
                    print(f"  ⚠️ Используется стандартный AnimInstance")
                    print(f"     Рекомендуется UBTZBaseCharacterAnimInstance")
                    return False
            else:
                print(f"  ❌ Не удалось получить родительский класс")
                return False
        else:
            print(f"  ❌ Нет доступа к parent_class")
            return False
            
    except Exception as e:
        print(f"  ❌ Ошибка проверки класса: {e}")
        return False

# Запускаем установку правильного класса
set_correct_animinstance_class()
