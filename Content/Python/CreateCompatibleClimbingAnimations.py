import unreal
import time

def create_compatible_climbing_animations():
    """Создает совместимые анимации лазания для ABP_Player"""
    
    print("🎭 СОЗДАНИЕ СОВМЕСТИМЫХ АНИМАЦИЙ ЛАЗАНИЯ")
    print("=" * 50)
    print("Создаем анимации, совместимые с скелетом персонажа...")
    
    try:
        time.sleep(1)
        
        # Основной скелет проекта
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print(f"❌ Не найден основной скелет: {target_skeleton_path}")
            return
        
        print(f"✅ Основной скелет найден: {target_skeleton_path}")
        
        # Создаем папку для анимаций лазания
        climbing_animations_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(climbing_animations_path):
            unreal.EditorAssetLibrary.make_directory(climbing_animations_path)
            print(f"📁 Создана папка: {climbing_animations_path}")
        
        # Создаем анимации лазания
        climbing_animations = [
            {
                "name": "AS_Vault",
                "description": "Vault animation - перепрыгивание через препятствие",
                "length": 1.5,
                "keys": 45
            },
            {
                "name": "AS_Mantle_1M",
                "description": "Mantle 1M animation - взбирание на препятствие 1 метр",
                "length": 2.0,
                "keys": 60
            },
            {
                "name": "AS_Mantle_2M",
                "description": "Mantle 2M animation - взбирание на препятствие 2 метра",
                "length": 2.5,
                "keys": 75
            },
            {
                "name": "AS_LedgeClimb_Up",
                "description": "Ledge climb up animation - взбирание на край",
                "length": 2.0,
                "keys": 60
            },
            {
                "name": "AS_LedgeClimb_Idle",
                "description": "Ledge climb idle animation - ожидание на краю",
                "length": 1.0,
                "keys": 30
            },
            {
                "name": "AS_LedgeClimb_Down",
                "description": "Ledge climb down animation - спуск с края",
                "length": 1.5,
                "keys": 45
            }
        ]
        
        created_count = 0
        for anim_data in climbing_animations:
            anim_path = f"{climbing_animations_path}/{anim_data['name']}.{anim_data['name']}"
            
            if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"🎬 Создаем: {anim_data['name']} - {anim_data['description']}")
                
                if create_climbing_animation(anim_path, target_skeleton, anim_data):
                    created_count += 1
                    print(f"  ✅ Создано: {anim_data['name']}")
                else:
                    print(f"  ❌ Не удалось создать: {anim_data['name']}")
            else:
                print(f"  ⚠️ Анимация уже существует: {anim_data['name']}")
        
        print(f"\n🎯 ИТОГО СОЗДАНО: {created_count} анимаций")
        
        # Создаем Blend Space для Mantle
        create_mantle_blend_space(climbing_animations_path, target_skeleton)
        
        # Проверяем созданные анимации
        verify_created_animations(climbing_animations_path)
        
    except Exception as e:
        print(f"❌ Ошибка создания анимаций: {e}")
        import traceback
        traceback.print_exc()

def create_climbing_animation(anim_path, target_skeleton, anim_data):
    """Создает анимацию лазания"""
    try:
        # Создаем новую анимацию
        factory = unreal.AnimationSequenceFactory()
        factory.target_skeleton = target_skeleton
        
        new_anim = factory.factory_create_new(anim_path)
        
        if new_anim:
            # Устанавливаем настройки анимации
            new_anim.set_editor_property("sequence_length", anim_data["length"])
            new_anim.set_editor_property("number_of_sampled_keys", anim_data["keys"])
            
            # Включаем Root Motion
            new_anim.set_editor_property("enable_root_motion", True)
            new_anim.set_editor_property("root_motion_root_lock", unreal.AnimationRootMotionRootLock.UNLOCKED)
            
            # Устанавливаем компрессию
            new_anim.set_editor_property("compression_scheme", unreal.AnimationCompressionLibrary.get_default_animation_compression())
            
            # Сохраняем
            unreal.EditorAssetLibrary.save_asset(anim_path)
            return True
        
        return False
        
    except Exception as e:
        print(f"  ❌ Ошибка создания анимации {anim_path}: {e}")
        return False

def create_mantle_blend_space(climbing_animations_path, target_skeleton):
    """Создает Blend Space для анимаций Mantle"""
    print("\n🎨 СОЗДАНИЕ BLEND SPACE ДЛЯ MANTLE")
    print("-" * 40)
    
    try:
        # Создаем Blend Space 1D
        factory = unreal.BlendSpaceFactory1D()
        factory.target_skeleton = target_skeleton
        
        blend_space_path = f"{climbing_animations_path}/BS_Mantle.BS_Mantle"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(blend_space_path):
            print(f"🎬 Создаем Blend Space: BS_Mantle")
            
            blend_space = factory.factory_create_new(blend_space_path)
            
            if blend_space:
                # Настраиваем Blend Space
                blend_space.set_editor_property("b_snap_to_grid", True)
                blend_space.set_editor_property("b_show_grid", True)
                
                # Настраиваем ось
                axis_settings = blend_space.get_editor_property("axis_settings")
                axis_settings[0].set_editor_property("axis_name", "Height")
                axis_settings[0].set_editor_property("min_value", 0.0)
                axis_settings[0].set_editor_property("max_value", 200.0)
                axis_settings[0].set_editor_property("num_grid_divisions", 4)
                
                # Добавляем анимации
                add_animation_to_blend_space(blend_space, f"{climbing_animations_path}/AS_Mantle_1M.AS_Mantle_1M", 50.0)
                add_animation_to_blend_space(blend_space, f"{climbing_animations_path}/AS_Mantle_2M.AS_Mantle_2M", 150.0)
                
                # Сохраняем
                unreal.EditorAssetLibrary.save_asset(blend_space_path)
                print(f"  ✅ Создан Blend Space: BS_Mantle")
            else:
                print(f"  ❌ Не удалось создать Blend Space")
        else:
            print(f"  ⚠️ Blend Space уже существует: BS_Mantle")
            
    except Exception as e:
        print(f"❌ Ошибка создания Blend Space: {e}")

def add_animation_to_blend_space(blend_space, anim_path, height_value):
    """Добавляет анимацию в Blend Space"""
    try:
        anim_sequence = unreal.load_asset(anim_path)
        if anim_sequence:
            # Создаем sample point
            sample_point = unreal.BlendSample()
            sample_point.set_editor_property("animation", anim_sequence)
            sample_point.set_editor_property("sample_value", height_value)
            sample_point.set_editor_property("rate_scale", 1.0)
            
            # Добавляем в Blend Space
            blend_samples = blend_space.get_editor_property("blend_samples")
            blend_samples.append(sample_point)
            blend_space.set_editor_property("blend_samples", blend_samples)
            
            print(f"  ✅ Добавлена анимация: {anim_path} (Height: {height_value})")
            
    except Exception as e:
        print(f"  ❌ Ошибка добавления анимации {anim_path}: {e}")

def verify_created_animations(climbing_animations_path):
    """Проверяет созданные анимации"""
    print("\n🔍 ПРОВЕРКА СОЗДАННЫХ АНИМАЦИЙ")
    print("-" * 40)
    
    expected_animations = [
        "AS_Vault",
        "AS_Mantle_1M", 
        "AS_Mantle_2M",
        "AS_LedgeClimb_Up",
        "AS_LedgeClimb_Idle",
        "AS_LedgeClimb_Down",
        "BS_Mantle"
    ]
    
    for anim_name in expected_animations:
        anim_path = f"{climbing_animations_path}/{anim_name}.{anim_name}"
        
        if unreal.EditorAssetLibrary.does_asset_exist(anim_path):
            asset = unreal.load_asset(anim_path)
            if asset:
                skeleton = asset.get_skeleton()
                skeleton_name = skeleton.get_name() if skeleton else "Unknown"
                print(f"✅ {anim_name}: {skeleton_name}")
            else:
                print(f"❌ {anim_name}: Не удалось загрузить")
        else:
            print(f"❌ {anim_name}: Не найден")

def main():
    """Основная функция"""
    print("🚀 ЗАПУСК СОЗДАНИЯ СОВМЕСТИМЫХ АНИМАЦИЙ ЛАЗАНИЯ")
    print("=" * 60)
    
    # Создаем совместимые анимации
    create_compatible_climbing_animations()
    
    print("\n🎯 СОЗДАНИЕ АНИМАЦИЙ ЗАВЕРШЕНО!")
    print("Теперь можно использовать совместимые анимации в ABP_Player")
    print("\n📋 СЛЕДУЮЩИЕ ШАГИ:")
    print("1. Откройте ABP_Player")
    print("2. Создайте State Machine для лазания")
    print("3. Подключите созданные анимации:")
    print("   - AS_Vault для перепрыгивания")
    print("   - BS_Mantle для взбирания")
    print("   - AS_LedgeClimb_* для взбирания и спуска")
    print("4. Настройте переходы с переменными bIsVaulting, bIsMantling, bIsLedgeClimbing")

if __name__ == "__main__":
    main()





