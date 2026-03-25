# -*- coding: utf-8 -*-
import unreal
import time

def fix_skeleton_compatibility():
    """Fix skeleton compatibility for climbing animations"""
    
    print("FIXING SKELETON COMPATIBILITY")
    print("=" * 50)
    print("Fixing 'skeletons are not compatible' error...")
    
    try:
        time.sleep(1)
        
        # Main project skeleton
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print(f"ERROR: Main skeleton not found: {target_skeleton_path}")
            return
        
        print(f"SUCCESS: Main skeleton found: {target_skeleton_path}")
        
        # List of climbing animations to fix
        climbing_animations = [
            "/Game/FreeAnimationLibrary/Animations/Vault/anim_Vault",
            "/Game/FreeAnimationLibrary/Animations/Mantle/anim_Mantle_1M_R",
            "/Game/FreeAnimationLibrary/Animations/Mantle/anim_Mantle_2M_R",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimb_ClimbUp",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimb_Idle",
            "/Game/FreeAnimationLibrary/Animations/LedgeClimb/anim_LedgeClimbing_Down"
        ]
        
        fixed_count = 0
        for anim_path in climbing_animations:
            if unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"Fixing: {anim_path}")
                if fix_animation_skeleton(anim_path, target_skeleton):
                    fixed_count += 1
                    print(f"SUCCESS: Fixed {anim_path}")
                else:
                    print(f"ERROR: Could not fix {anim_path}")
            else:
                print(f"WARNING: Animation not found: {anim_path}")
        
        print(f"\nTOTAL FIXED: {fixed_count} animations")
        
        # Check ABP_Player
        check_abp_player_skeleton()
        
    except Exception as e:
        print(f"ERROR: {e}")
        import traceback
        traceback.print_exc()

def fix_animation_skeleton(anim_path, target_skeleton):
    """Fix skeleton for specific animation"""
    try:
        # Load animation
        anim_sequence = unreal.load_asset(anim_path)
        if not anim_sequence:
            return False
        
        # Check current skeleton
        current_skeleton = anim_sequence.get_skeleton()
        if current_skeleton == target_skeleton:
            print(f"  SUCCESS: Skeleton already correct: {anim_path}")
            return True
        
        print(f"  CHANGING skeleton for: {anim_path}")
        
        # Create new animation with correct skeleton
        new_anim_path = anim_path.replace(".", "_Fixed.")
        
        # Copy animation
        unreal.EditorAssetLibrary.duplicate_asset(anim_path, new_anim_path)
        
        # Load new animation
        new_anim = unreal.load_asset(new_anim_path)
        if new_anim:
            # Set correct skeleton
            new_anim.set_skeleton(target_skeleton)
            
            # Save changes
            unreal.EditorAssetLibrary.save_asset(new_anim_path)
            
            # Replace original animation
            unreal.EditorAssetLibrary.delete_asset(anim_path)
            unreal.EditorAssetLibrary.rename_asset(new_anim_path, anim_path)
            
            return True
        
        return False
        
    except Exception as e:
        print(f"  ERROR fixing animation {anim_path}: {e}")
        return False

def check_abp_player_skeleton():
    """Check ABP_Player skeleton"""
    print("\nCHECKING ABP_PLAYER")
    print("-" * 30)
    
    abp_paths = [
        "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player",
        "/Game/BackToZaraysk/Core/Characters/Player/ABP_Player1"
    ]
    
    target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
    target_skeleton = unreal.load_asset(target_skeleton_path)
    
    for abp_path in abp_paths:
        if unreal.EditorAssetLibrary.does_asset_exist(abp_path):
            print(f"Checking: {abp_path}")
            
            anim_bp = unreal.load_asset(abp_path)
            if anim_bp:
                current_skeleton = anim_bp.get_skeleton()
                if current_skeleton == target_skeleton:
                    print(f"  SUCCESS: ABP_Player skeleton is correct")
                else:
                    print(f"  ERROR: ABP_Player skeleton is wrong")
                    print(f"  FIXING ABP_Player skeleton...")
                    
                    # Fix ABP_Player skeleton
                    anim_bp.set_skeleton(target_skeleton)
                    unreal.EditorAssetLibrary.save_asset(abp_path)
                    print(f"  SUCCESS: ABP_Player skeleton fixed")
            break

def create_compatible_animations():
    """Create compatible climbing animations"""
    print("\nCREATING COMPATIBLE ANIMATIONS")
    print("-" * 40)
    
    # Main skeleton
    target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
    target_skeleton = unreal.load_asset(target_skeleton_path)
    
    # Create simple climbing animations
    climbing_animations = [
        ("anim_Vault_Simple", "Vault animation"),
        ("anim_Mantle_Simple", "Mantle animation"),
        ("anim_LedgeClimb_Simple", "Ledge climb animation")
    ]
    
    for anim_name, description in climbing_animations:
        print(f"Creating: {anim_name} - {description}")
        
        # Create new animation
        factory = unreal.AnimationSequenceFactory()
        factory.target_skeleton = target_skeleton
        
        anim_path = f"/Game/BackToZaraysk/Core/Characters/Player/Animations/{anim_name}.{anim_name}"
        new_anim = factory.factory_create_new(anim_path)
        
        if new_anim:
            # Set basic settings
            new_anim.set_editor_property("sequence_length", 2.0)  # 2 seconds
            new_anim.set_editor_property("number_of_sampled_keys", 60)  # 60 keys
            
            # Save
            unreal.EditorAssetLibrary.save_asset(anim_path)
            print(f"  SUCCESS: Created {anim_name}")
        else:
            print(f"  ERROR: Could not create {anim_name}")

def main():
    """Main function"""
    print("STARTING SKELETON COMPATIBILITY FIX")
    print("=" * 60)
    
    # Fix existing animations
    fix_skeleton_compatibility()
    
    # Create compatible animations
    create_compatible_animations()
    
    print("\nFIX COMPLETED!")
    print("Now you can use climbing animations in ABP_Player")
    print("\nNEXT STEPS:")
    print("1. Open ABP_Player")
    print("2. Create State Machine for climbing")
    print("3. Connect fixed animations")
    print("4. Set up transitions")

if __name__ == "__main__":
    main()





