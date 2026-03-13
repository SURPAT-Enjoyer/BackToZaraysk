# -*- coding: utf-8 -*-

import unreal

def find_data_asset_path():
    """Найти путь к Data Asset для жилета"""
    
    print("=== ПОИСК ПУТИ К DATA ASSET ЖИЛЕТА ===")
    
    # Получаем Asset Registry
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    
    # Ищем все Data Asset
    data_asset_assets = asset_registry.get_assets_by_class("DataAsset", True)
    
    print(f"Найдено {len(data_asset_assets)} Data Asset в проекте:")
    
    for asset in data_asset_assets:
        asset_name = asset.get_asset().get_name()
        asset_path = asset.get_asset().get_path_name()
        asset_class = asset.get_asset().get_class().get_name()
        
        # Если это наш Data Asset
        if "tactical" in asset_name.lower() or "vest" in asset_name.lower() or "DA_" in asset_name:
            print(f"   🎯 НАЙДЕН: {asset_name}")
            print(f"      Полный путь: {asset_path}")
            print(f"      Класс: {asset_class}")
            
            # Генерируем правильный путь для LoadObject
            # Убираем .uasset и добавляем .ClassName
            path_parts = asset_path.split('.')
            if len(path_parts) >= 2:
                base_path = path_parts[0]  # /Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest
                asset_name_only = path_parts[1]  # DA_TacticalVest
                
                # Правильный путь для LoadObject
                load_object_path = f"{base_path}.{asset_name_only}.{asset_name_only}"
                print(f"      Путь для LoadObject: {load_object_path}")
                
                # Тестируем загрузку
                test_load_object(load_object_path, asset_name)
            
            print()

def test_load_object(load_path, asset_name):
    """Тестировать загрузку Data Asset по пути"""
    
    print(f"      🧪 Тестирование загрузки: {load_path}")
    
    try:
        loaded_asset = unreal.EditorAssetLibrary.load_asset(load_path)
        if loaded_asset:
            print(f"      ✅ Загрузка успешна: {loaded_asset.get_name()}")
            print(f"      ✅ Класс: {loaded_asset.get_class().get_name()}")
            
            # Проверяем EquippedMesh
            try:
                equipped_mesh = unreal.EditorAssetLibrary.get_editor_property(loaded_asset, "EquippedMesh")
                if equipped_mesh:
                    print(f"      ✅ EquippedMesh установлен: {equipped_mesh.get_name()}")
                else:
                    print(f"      ❌ EquippedMesh не установлен")
            except:
                print(f"      ⚠️ Не удалось проверить EquippedMesh")
                
        else:
            print(f"      ❌ Загрузка не удалась")
            
    except Exception as e:
        print(f"      ❌ Ошибка при загрузке: {e}")

def main():
    find_data_asset_path()

if __name__ == "__main__":
    main()


