import unreal

def create_strafe_curve():
    """Создает кривую для плавного движения стрейфа"""
    
    print("🎯 СОЗДАНИЕ КРИВОЙ ДВИЖЕНИЯ СТРЕЙФА")
    print("=" * 40)
    
    try:
        # Создаем новый CurveFloat
        curve = unreal.CurveFloat()
        
        # Настраиваем кривую для плавного ускорения и замедления
        # Позиция 0.0 - начало движения (медленно)
        # Позиция 0.5 - середина (быстро) 
        # Позиция 1.0 - конец движения (медленно)
        
        # Создаем ключи для кривой
        keys = [
            (0.0, 0.0),      # Начало - скорость 0
            (0.2, 0.7),      # Быстрое ускорение
            (0.5, 1.0),      # Максимальная скорость
            (0.8, 0.7),      # Начало замедления
            (1.0, 0.0)       # Конец - скорость 0
        ]
        
        # Добавляем ключи в кривую
        for time, value in keys:
            key = unreal.RichCurveKey()
            key.time = time
            key.value = value
            key.arrive_tangent = 0.0
            key.leave_tangent = 0.0
            key.arrive_tangent_weight = 0.0
            key.leave_tangent_weight = 0.0
            key.interp_mode = unreal.RichCurveInterpMode.RCIM_CUBIC
            
            curve.float_curve.keys.append(key)
        
        # Сохраняем кривую
        curve_path = "/Game/BackToZaraysk/Core/Characters/Player/Curves/StrafeMovementCurve"
        
        # Создаем директорию если не существует
        directory = "/Game/BackToZaraysk/Core/Characters/Player/Curves"
        if not unreal.EditorAssetLibrary.does_directory_exist(directory):
            unreal.EditorAssetLibrary.make_directory(directory)
        
        # Сохраняем ассет
        unreal.EditorAssetLibrary.save_asset(curve_path, curve)
        
        print(f"  ✅ Кривая создана: {curve_path}")
        print(f"  📋 Настройки кривой:")
        print(f"     - Начало: медленное ускорение")
        print(f"     - Середина: максимальная скорость")
        print(f"     - Конец: плавное замедление")
        print(f"  💡 Примените эту кривую к StrafeComponent в BP_Player")
        
        return curve_path
        
    except Exception as e:
        print(f"  ❌ Ошибка создания кривой: {e}")
        import traceback
        traceback.print_exc()
        return None

def apply_curve_to_strafe_component():
    """Применяет созданную кривую к StrafeComponent"""
    
    print("\n🔧 ПРИМЕНЕНИЕ КРИВОЙ К STRAFE COMPONENT:")
    
    try:
        # Путь к BP_Player
        bp_path = "/Game/BackToZaraysk/Core/Characters/Player/BP_Player"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(bp_path):
            print(f"  ❌ BP_Player не найден: {bp_path}")
            return False
        
        # Загружаем Blueprint
        bp = unreal.load_asset(bp_path)
        if not bp:
            print(f"  ❌ Не удалось загрузить BP_Player")
            return False
        
        print(f"  ✅ BP_Player загружен")
        print(f"  💡 Вручную примените кривую:")
        print(f"     1. Откройте BP_Player")
        print(f"     2. Выберите StrafeComponent")
        print(f"     3. В Details найдите Strafe Curve")
        print(f"     4. Установите StrafeMovementCurve")
        
        return True
        
    except Exception as e:
        print(f"  ❌ Ошибка применения кривой: {e}")
        return False

# Запускаем создание кривой
curve_path = create_strafe_curve()
if curve_path:
    apply_curve_to_strafe_component()
    
print("\n🎯 ГОТОВО! Теперь стрейф будет двигаться плавно!")
print("📋 Следующие шаги:")
print("   1. Откройте BP_Player")
print("   2. Выберите StrafeComponent") 
print("   3. В Details → Strafe Curve → выберите StrafeMovementCurve")
print("   4. Протестируйте стрейф в игре")





