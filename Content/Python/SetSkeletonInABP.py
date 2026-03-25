#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Скрипт для назначения скелета в Animation Blueprint в UE 5.3
Использование: exec(open('Content/Python/SetSkeletonInABP.py').read())
"""

import unreal

def set_skeleton_in_abp():
    """Назначает скелет в Animation Blueprint"""
    
    print("SET SKELETON IN ANIMATION BLUEPRINT")
    print("==================================")
    
    # Путь к ABP_Player
    abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
    
    # Путь к целевому скелету
    skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/SK_Mannequin"
    
    try:
        # Загружаем ABP
        abp_asset = unreal.EditorAssetLibrary.load_asset(abp_path)
        if not abp_asset:
            print(f"ERROR: Не удалось загрузить ABP по пути: {abp_path}")
            return
        
        print(f"SUCCESS: ABP загружен: {abp_asset.get_name()}")
        
        # Загружаем скелет
        skeleton_asset = unreal.EditorAssetLibrary.load_asset(skeleton_path)
        if not skeleton_asset:
            print(f"ERROR: Не удалось загрузить скелет по пути: {skeleton_path}")
            return
        
        print(f"SUCCESS: Скелет загружен: {skeleton_asset.get_name()}")
        
        # Получаем текущий скелет
        current_skeleton = abp_asset.get_editor_property("target_skeleton")
        print(f"INFO: Текущий скелет: {current_skeleton.get_name() if current_skeleton else 'None'}")
        
        # Назначаем новый скелет
        abp_asset.set_editor_property("target_skeleton", skeleton_asset)
        
        # Сохраняем изменения
        unreal.EditorAssetLibrary.save_asset(abp_path)
        
        print(f"SUCCESS: Скелет {skeleton_asset.get_name()} назначен в ABP_Player")
        print("INFO: Изменения сохранены")
        
    except Exception as e:
        print(f"ERROR: Ошибка при назначении скелета: {str(e)}")

def list_available_skeletons():
    """Показывает доступные скелеты в проекте"""
    
    print("\nДОСТУПНЫЕ СКЕЛЕТЫ В ПРОЕКТЕ:")
    print("=============================")
    
    try:
        # Ищем все скелеты в проекте
        skeleton_assets = unreal.EditorAssetLibrary.list_assets("/Game", recursive=True, include_folder=False)
        
        skeleton_count = 0
        for asset_path in skeleton_assets:
            if asset_path.endswith(".uasset"):
                asset = unreal.EditorAssetLibrary.load_asset(asset_path)
                if asset and isinstance(asset, unreal.Skeleton):
                    skeleton_count += 1
                    print(f"{skeleton_count}. {asset.get_name()}")
                    print(f"   Путь: {asset_path}")
        
        if skeleton_count == 0:
            print("INFO: Скелеты не найдены")
        else:
            print(f"\nINFO: Найдено скелетов: {skeleton_count}")
            
    except Exception as e:
        print(f"ERROR: Ошибка при поиске скелетов: {str(e)}")

def main():
    """Основная функция"""
    
    print("АНАЛИЗ И НАЗНАЧЕНИЕ СКЕЛЕТА В ABP")
    print("==================================")
    
    # Показываем доступные скелеты
    list_available_skeletons()
    
    print("\n" + "="*50)
    
    # Назначаем скелет
    set_skeleton_in_abp()
    
    print("\n" + "="*50)
    print("ГОТОВО! Скелет назначен в ABP_Player")

# Запускаем скрипт
if __name__ == "__main__":
    main()





