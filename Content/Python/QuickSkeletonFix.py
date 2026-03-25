# -*- coding: utf-8 -*-
import unreal

def quick_fix():
    """Quick fix for skeleton compatibility"""
    
    print("QUICK SKELETON COMPATIBILITY FIX")
    print("=" * 40)
    
    try:
        # Check ABP_Player skeleton
        abp_path = "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player"
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            print("Checking ABP_Player...")
            
            anim_bp = unreal.load_asset(abp_path)
            target_skeleton = unreal.load_asset(target_skeleton_path)
            
            if anim_bp and target_skeleton:
                current_skeleton = anim_bp.get_editor_property("target_skeleton")
                if current_skeleton != target_skeleton:
                    print("Fixing ABP_Player skeleton...")
                    anim_bp.set_editor_property("target_skeleton", target_skeleton)
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print("SUCCESS: ABP_Player skeleton fixed!")
                else:
                    print("SUCCESS: ABP_Player skeleton is correct")
            else:
                print("ERROR: Could not load ABP_Player or skeleton")
        else:
            print("ERROR: ABP_Player not found")
        
        # Create simple climbing animations
        create_simple_animations()
        
    except Exception as e:
        print(f"ERROR: {e}")

def create_simple_animations():
    """Create simple climbing animations"""
    
    print("\nCreating simple climbing animations...")
    
    try:
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print("ERROR: Could not load target skeleton")
            return
        
        # Create animations folder
        anim_folder = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(anim_folder):
            unreal.EditorAssetLibrary.make_directory(anim_folder)
            print("Created climbing animations folder")
        
        # Create simple animations
        animations = ["AS_Vault", "AS_Mantle", "AS_LedgeClimb"]
        
        for anim_name in animations:
            anim_path = f"{anim_folder}/{anim_name}.{anim_name}"
            
            if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"Creating {anim_name}...")
                
                factory = unreal.AnimationSequenceFactoryNew()
                factory.target_skeleton = target_skeleton
                
                new_anim = factory.factory_create_new(anim_path)
                if new_anim:
                    new_anim.set_editor_property("sequence_length", 2.0)
                    new_anim.set_editor_property("enable_root_motion", True)
                    unreal.EditorAssetLibrary.save_asset(anim_path)
                    print(f"SUCCESS: Created {anim_name}")
                else:
                    print(f"ERROR: Could not create {anim_name}")
            else:
                print(f"WARNING: {anim_name} already exists")
        
        print("\nSIMPLE ANIMATIONS CREATED!")
        print("You can now use these animations in ABP_Player State Machine")
        
    except Exception as e:
        print(f"ERROR creating animations: {e}")

if __name__ == "__main__":
    quick_fix()
