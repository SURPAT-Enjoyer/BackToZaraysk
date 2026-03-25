import unreal
import time

def fix_skeleton_compatibility():
    """Исправляет несовместимость скелетов для анимаций лазания"""
    
    print("🔧 ИСПРАВЛЕНИЕ НЕСОВМЕСТИМОСТИ СКЕЛЕТОВ")
    print("=" * 50)
    print("Исправляем проблему 'skeletons are not compatible'...")
    
    try:
        time.sleep(1)
        
        # Основной скелет проекта
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print(f"❌ Не найден основной скелет: {target_skeleton_path}")
            return
        
        print(f"✅ Основной скелет найден: {target_skeleton_path}")
        
        # Список анимаций лазания для исправления
        climbing_animations = [
            "/Game/FreeAnimationLibrary/Animations/Vault/anim_Vault",
            "/Game/FreeAnimationLibrary/Animations/Mantle/anim_Mantle_1M_R",
            "/Game/FreeAnimationLibrary/Animations/Mantle/anim_Mantle_2M_R",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimb_ClimbUp",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimb_Idle",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimbing_Down",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimbing_Left",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimbing_Right",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimbing_Up"
        ]
        
        fixed_count = 0
        for anim_path in climbing_animations:
            if unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"🔧 Исправляем: {anim_path}")
                if fix_animation_skeleton(anim_path, target_skeleton):
                    fixed_count += 1
                    print(f"✅ Исправлено: {anim_path}")
                else:
                    print(f"❌ Не удалось исправить: {anim_path}")
            else:
                print(f"⚠️ Анимация не найдена: {anim_path}")
        
        print(f"\n🎯 ИТОГО ИСПРАВЛЕНО: {fixed_count} анимаций")
        
        # Проверяем ABP_Player
        check_abp_player_skeleton()
        
    except Exception as e:
        print(f"❌ Ошибка исправления: {e}")
        import traceback
        traceback.print_exc()

def fix_animation_skeleton(anim_path, target_skeleton):
    """Исправляет скелет конкретной анимации"""
    try:
        # Загружаем анимацию
        anim_sequence = unreal.load_asset(anim_path)
        if not anim_sequence:
            return False
        
        # Проверяем текущий скелет
        current_skeleton = anim_sequence.get_skeleton()
        if current_skeleton == target_skeleton:
            print(f"  ✅ Скелет уже правильный: {anim_path}")
            return True
        
        print(f"  🔄 Меняем скелет для: {anim_path}")
        
        # Создаем новую анимацию с правильным скелетом
        new_anim_path = anim_path.replace(".", "_Fixed.")
        
        # Копируем анимацию
        unreal.EditorAssetLibrary.duplicate_asset(anim_path, new_anim_path)
        
        # Загружаем новую анимацию
        new_anim = unreal.load_asset(new_anim_path)
        if new_anim:
            # Устанавливаем правильный скелет
            new_anim.set_skeleton(target_skeleton)
            
            # Сохраняем изменения
            unreal.EditorAssetLibrary.save_asset(new_anim_path)
            
            # Заменяем оригинальную анимацию
            unreal.EditorAssetLibrary.delete_asset(anim_path)
            unreal.EditorAssetLibrary.rename_asset(new_anim_path, anim_path)
            
            return True
        
        return False
        
    except Exception as e:
        print(f"  ❌ Ошибка исправления анимации {anim_path}: {e}")
        return False

def check_abp_player_skeleton():
    """Проверяет скелет ABP_Player"""
    print("\n🔍 ПРОВЕРКА ABP_PLAYER")
    print("-" * 30)
    
    abp_paths = [
        "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
        "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1"
    ]
    
    target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
    target_skeleton = unreal.load_asset(target_skeleton_path)
    
    for abp_path in abp_paths:
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            print(f"📂 Проверяем: {abp_path}")
            
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
                current_skeleton = anim_bp.get_skeleton()
                if current_skeleton == target_skeleton:
                    print(f"  ✅ Скелет ABP_Player правильный")
                else:
                    print(f"  ❌ Скелет ABP_Player неправильный")
                    print(f"  🔧 Исправляем скелет ABP_Player...")
                    
                    # Исправляем скелет ABP_Player
                    anim_bp.set_skeleton(target_skeleton)
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print(f"  ✅ Скелет ABP_Player исправлен")
            break

def create_compatible_animations():
    """Создает совместимые анимации лазания"""
    print("\n🎭 СОЗДАНИЕ СОВМЕСТИМЫХ АНИМАЦИЙ")
    print("-" * 40)
    
    # Основной скелет
    target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
    target_skeleton = unreal.load_asset(target_skeleton_path)
    
    # Создаем простые анимации лазания
    climbing_animations = [
        ("anim_Vault_Simple", "Vault animation"),
        ("anim_Mantle_Simple", "Mantle animation"),
        ("anim_LedgeClimb_Simple", "Ledge climb animation")
    ]
    
    for anim_name, description in climbing_animations:
        print(f"🎬 Создаем: {anim_name} - {description}")
        
        # Создаем новую анимацию
        factory = unreal.AnimationSequenceFactory()
        factory.target_skeleton = target_skeleton
        
        anim_path = f"/Game/BackToZaraysk/Core/Characters/Player/Animations/{anim_name}.{anim_name}"
        new_anim = factory.factory_create_new(anim_path)
        
        if new_anim:
            # Устанавливаем базовые настройки
            new_anim.set_editor_property("sequence_length", 2.0)  # 2 секунды
            new_anim.set_editor_property("number_of_sampled_keys", 60)  # 60 ключей
            
            # Сохраняем
            unreal.EditorAssetLibrary.save_asset(anim_path)
            print(f"  ✅ Создано: {anim_name}")
        else:
            print(f"  ❌ Не удалось создать: {anim_name}")

def main():
    """Основная функция"""
    print("🚀 ЗАПУСК ИСПРАВЛЕНИЯ НЕСОВМЕСТИМОСТИ СКЕЛЕТОВ")
    print("=" * 60)
    
    # Исправляем существующие анимации
    fix_skeleton_compatibility()
    
    # Создаем совместимые анимации
    create_compatible_animations()
    
    print("\n🎯 ИСПРАВЛЕНИЕ ЗАВЕРШЕНО!")
    print("Теперь можно использовать анимации лазания в ABP_Player")
    print("\n📋 СЛЕДУЮЩИЕ ШАГИ:")
    print("1. Откройте ABP_Player")
    print("2. Создайте State Machine для лазания")
    print("3. Подключите исправленные анимации")
    print("4. Настройте переходы")

if __name__ == "__main__":
    main()





