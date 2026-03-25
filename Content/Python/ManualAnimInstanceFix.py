import unreal
import time

def manual_animinstance_fix():
    """Ручное исправление класса AnimInstance с подробными инструкциями"""
    
    print("🎭 РУЧНОЕ ИСПРАВЛЕНИЕ КЛАССА ANIMINSTANCE")
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
                abp_found = True
                break
        
        if abp_found:
            print(f"\n📋 ИНСТРУКЦИИ ДЛЯ РУЧНОГО ИСПРАВЛЕНИЯ:")
            print(f"=" * 50)
            print(f"1. Откройте Animation Blueprint:")
            print(f"   - Найдите файл в Content Browser")
            print(f"   - Дважды кликните для открытия")
            print(f"")
            print(f"2. Откройте Class Settings:")
            print(f"   - В верхней панели найдите кнопку 'Class Settings'")
            print(f"   - Или нажмите F4")
            print(f"")
            print(f"3. Установите правильный Parent Class:")
            print(f"   - В разделе 'Parent Class' найдите выпадающий список")
            print(f"   - Найдите и выберите 'UBTZBaseCharacterAnimInstance'")
            print(f"   - Если не найдено, найдите 'BTZBaseCharacterAnimInstance'")
            print(f"")
            print(f"4. Сохраните изменения:")
            print(f"   - Нажмите Ctrl+S")
            print(f"   - Или File → Save")
            print(f"")
            print(f"5. Проверьте результат:")
            print(f"   - В Class Settings должно отображаться:")
            print(f"     'Parent Class: BTZBaseCharacterAnimInstance'")
            print(f"")
            print(f"✅ После этого ИК система будет работать правильно!")
            
        else:
            print("❌ ABP_Player не найден")
            print("🔍 Поиск всех Animation Blueprint...")
            
            # Ищем все Animation Blueprint
            asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
            anim_bp_filter = unreal.ARFilter()
            anim_bp_filter.class_paths = [unreal.AnimBlueprint.static_class()]
            anim_bp_assets = asset_registry.get_assets(anim_bp_filter)
            
            if anim_bp_assets:
                print(f"📋 Найдено Animation Blueprint: {len(anim_bp_assets)}")
                for asset_data in anim_bp_assets:
                    if "Player" in asset_data.asset_name or "Character" in asset_data.asset_name:
                        print(f"  - {asset_data.asset_name}")
                        
                print(f"\n💡 Выберите подходящий Animation Blueprint и следуйте инструкциям выше")
            else:
                print("❌ Animation Blueprint не найдены")
            
    except Exception as e:
        print(f"❌ Ошибка: {e}")

def check_current_animinstance_class():
    """Проверяет текущий класс AnimInstance"""
    
    print("\n🔍 ПРОВЕРКА ТЕКУЩЕГО КЛАССА ANIMINSTANCE:")
    
    try:
        # Находим ABP_Player
        abp_paths = [
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
            "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1"
        ]
        
        for abp_path in abp_paths:
            if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
                print(f"📂 Проверка: {abp_path}")
                
                anim_bp = unreal.load_asset(abp_path)
                if anim_bp:
                    if hasattr(anim_bp, 'parent_class'):
                        parent_class = anim_bp.parent_class
                        if parent_class:
                            class_name = parent_class.get_name()
                            print(f"  Текущий класс: {class_name}")
                            
                            if "BTZBaseCharacterAnimInstance" in class_name:
                                print(f"  ✅ Используется правильный класс!")
                                return True
                            else:
                                print(f"  ❌ Используется неправильный класс!")
                                print(f"     Нужно: UBTZBaseCharacterAnimInstance")
                                return False
                        else:
                            print(f"  ❌ Не удалось получить родительский класс")
                            return False
                    else:
                        print(f"  ❌ Нет доступа к parent_class")
                        return False
                break
        else:
            print("❌ ABP_Player не найден")
            return False
            
    except Exception as e:
        print(f"❌ Ошибка проверки: {e}")
        return False

# Запускаем проверку и инструкции
print("🔍 Проверка текущего состояния...")
if not check_current_animinstance_class():
    print("\n" + "="*50)
    manual_animinstance_fix()
else:
    print("\n✅ Класс AnimInstance уже настроен правильно!")
