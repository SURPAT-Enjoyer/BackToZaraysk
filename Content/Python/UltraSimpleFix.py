# -*- coding: utf-8 -*-
import unreal

def ultra_simple_fix():
    """Ultra simple fix - just create animations"""
    
    print("ULTRA SIMPLE SKELETON FIX")
    print("=" * 30)
    
    try:
        # Load skeleton
        skeleton = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin")
        
        if not skeleton:
            print("ERROR: Could not load skeleton")
            return
        
        print("SUCCESS: Skeleton loaded")
        
        # Create folder
        folder = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(folder):
            unreal.EditorAssetLibrary.make_directory(folder)
            print("Created folder: Climbing")
        
        # Create animations using EditorAssetLibrary
        animations = ["AS_Vault", "AS_Mantle", "AS_LedgeClimb"]
        
        for anim_name in animations:
            asset_path = f"{folder}/{anim_name}.{anim_name}"
            
            if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                print(f"EXISTS: {anim_name}")
                continue
            
            print(f"Creating: {anim_name}")
            
            # Create animation using EditorAssetLibrary
            try:
                # Create empty animation sequence
                new_anim = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
                    asset_name=anim_name,
                    package_path=folder,
                    asset_class=unreal.AnimationSequence,
                    factory=unreal.AnimationSequenceFactoryNew()
                )
                
                if new_anim:
                    # Set skeleton
                    new_anim.set_editor_property("target_skeleton", skeleton)
                    
                    # Set basic properties
                    new_anim.set_editor_property("sequence_length", 2.0)
                    new_anim.set_editor_property("enable_root_motion", True)
                    
                    # Save
                    unreal.EditorAssetLibrary.save_asset(asset_path)
                    print(f"SUCCESS: {anim_name} created")
                else:
                    print(f"ERROR: Could not create {anim_name}")
                    
            except Exception as e:
                print(f"ERROR creating {anim_name}: {e}")
        
        print("\nDONE!")
        print("Check Content Browser for created animations")
        
    except Exception as e:
        print(f"ERROR: {e}")

if __name__ == "__main__":
    ultra_simple_fix()





