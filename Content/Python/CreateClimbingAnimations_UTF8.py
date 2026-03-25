# -*- coding: utf-8 -*-
import unreal
import time

def create_climbing_animations():
    """Create compatible climbing animations for ABP_Player"""
    
    print("CREATING COMPATIBLE CLIMBING ANIMATIONS")
    print("=" * 50)
    print("Creating animations compatible with character skeleton...")
    
    try:
        time.sleep(1)
        
        # Main project skeleton
        target_skeleton_path = "/Game/BackToZaraysk/Core/Characters/Player/Meshes/SK_Mannequin.SK_Mannequin"
        target_skeleton = unreal.load_asset(target_skeleton_path)
        
        if not target_skeleton:
            print(f"ERROR: Main skeleton not found: {target_skeleton_path}")
            return
        
        print(f"SUCCESS: Main skeleton found: {target_skeleton_path}")
        
        # Create climbing animations folder
        climbing_animations_path = "/Game/BackToZaraysk/Core/Characters/Player/Animations/Climbing"
        if not unreal.EditorAssetLibrary.does_directory_exist(climbing_animations_path):
            unreal.EditorAssetLibrary.make_directory(climbing_animations_path)
            print(f"Created folder: {climbing_animations_path}")
        
        # Create climbing animations
        climbing_animations = [
            {
                "name": "AS_Vault",
                "description": "Vault animation - jumping over obstacle",
                "length": 1.5,
                "keys": 45
            },
            {
                "name": "AS_Mantle_1M",
                "description": "Mantle 1M animation - climbing 1 meter obstacle",
                "length": 2.0,
                "keys": 60
            },
            {
                "name": "AS_Mantle_2M",
                "description": "Mantle 2M animation - climbing 2 meter obstacle",
                "length": 2.5,
                "keys": 75
            },
            {
                "name": "AS_LedgeClimb_Up",
                "description": "Ledge climb up animation - climbing to ledge",
                "length": 2.0,
                "keys": 60
            },
            {
                "name": "AS_LedgeClimb_Idle",
                "description": "Ledge climb idle animation - waiting on ledge",
                "length": 1.0,
                "keys": 30
            },
            {
                "name": "AS_LedgeClimb_Down",
                "description": "Ledge climb down animation - descending from ledge",
                "length": 1.5,
                "keys": 45
            }
        ]
        
        created_count = 0
        for anim_data in climbing_animations:
            anim_path = f"{climbing_animations_path}/{anim_data['name']}.{anim_data['name']}"
            
            if not unreal.EditorAssetLibrary.does_asset_exist(anim_path):
                print(f"Creating: {anim_data['name']} - {anim_data['description']}")
                
                if create_climbing_animation(anim_path, target_skeleton, anim_data):
                    created_count += 1
                    print(f"  SUCCESS: Created {anim_data['name']}")
                else:
                    print(f"  ERROR: Could not create {anim_data['name']}")
            else:
                print(f"  WARNING: Animation already exists: {anim_data['name']}")
        
        print(f"\nTOTAL CREATED: {created_count} animations")
        
        # Create Blend Space for Mantle
        create_mantle_blend_space(climbing_animations_path, target_skeleton)
        
        # Verify created animations
        verify_created_animations(climbing_animations_path)
        
    except Exception as e:
        print(f"ERROR creating animations: {e}")
        import traceback
        traceback.print_exc()

def create_climbing_animation(anim_path, target_skeleton, anim_data):
    """Create climbing animation"""
    try:
        # Create new animation
        factory = unreal.AnimationSequenceFactory()
        factory.target_skeleton = target_skeleton
        
        new_anim = factory.factory_create_new(anim_path)
        
        if new_anim:
            # Set animation settings
            new_anim.set_editor_property("sequence_length", anim_data["length"])
            new_anim.set_editor_property("number_of_sampled_keys", anim_data["keys"])
            
            # Enable Root Motion
            new_anim.set_editor_property("enable_root_motion", True)
            new_anim.set_editor_property("root_motion_root_lock", unreal.AnimationRootMotionRootLock.UNLOCKED)
            
            # Set compression
            new_anim.set_editor_property("compression_scheme", unreal.AnimationCompressionLibrary.get_default_animation_compression())
            
            # Save
            unreal.EditorAssetLibrary.save_asset(anim_path)
            return True
        
        return False
        
    except Exception as e:
        print(f"  ERROR creating animation {anim_path}: {e}")
        return False

def create_mantle_blend_space(climbing_animations_path, target_skeleton):
    """Create Blend Space for Mantle animations"""
    print("\nCREATING BLEND SPACE FOR MANTLE")
    print("-" * 40)
    
    try:
        # Create Blend Space 1D
        factory = unreal.BlendSpaceFactory1D()
        factory.target_skeleton = target_skeleton
        
        blend_space_path = f"{climbing_animations_path}/BS_Mantle.BS_Mantle"
        
        if not unreal.EditorAssetLibrary.does_asset_exist(blend_space_path):
            print(f"Creating Blend Space: BS_Mantle")
            
            blend_space = factory.factory_create_new(blend_space_path)
            
            if blend_space:
                # Configure Blend Space
                blend_space.set_editor_property("b_snap_to_grid", True)
                blend_space.set_editor_property("b_show_grid", True)
                
                # Configure axis
                axis_settings = blend_space.get_editor_property("axis_settings")
                axis_settings[0].set_editor_property("axis_name", "Height")
                axis_settings[0].set_editor_property("min_value", 0.0)
                axis_settings[0].set_editor_property("max_value", 200.0)
                axis_settings[0].set_editor_property("num_grid_divisions", 4)
                
                # Add animations
                add_animation_to_blend_space(blend_space, f"{climbing_animations_path}/AS_Mantle_1M.AS_Mantle_1M", 50.0)
                add_animation_to_blend_space(blend_space, f"{climbing_animations_path}/AS_Mantle_2M.AS_Mantle_2M", 150.0)
                
                # Save
                unreal.EditorAssetLibrary.save_asset(blend_space_path)
                print(f"  SUCCESS: Created Blend Space: BS_Mantle")
            else:
                print(f"  ERROR: Could not create Blend Space")
        else:
            print(f"  WARNING: Blend Space already exists: BS_Mantle")
            
    except Exception as e:
        print(f"ERROR creating Blend Space: {e}")

def add_animation_to_blend_space(blend_space, anim_path, height_value):
    """Add animation to Blend Space"""
    try:
        anim_sequence = unreal.load_asset(anim_path)
        if anim_sequence:
            # Create sample point
            sample_point = unreal.BlendSample()
            sample_point.set_editor_property("animation", anim_sequence)
            sample_point.set_editor_property("sample_value", height_value)
            sample_point.set_editor_property("rate_scale", 1.0)
            
            # Add to Blend Space
            blend_samples = blend_space.get_editor_property("blend_samples")
            blend_samples.append(sample_point)
            blend_space.set_editor_property("blend_samples", blend_samples)
            
            print(f"  SUCCESS: Added animation: {anim_path} (Height: {height_value})")
            
    except Exception as e:
        print(f"  ERROR adding animation {anim_path}: {e}")

def verify_created_animations(climbing_animations_path):
    """Verify created animations"""
    print("\nVERIFYING CREATED ANIMATIONS")
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
                print(f"SUCCESS: {anim_name}: {skeleton_name}")
            else:
                print(f"ERROR: {anim_name}: Could not load")
        else:
            print(f"ERROR: {anim_name}: Not found")

def main():
    """Main function"""
    print("STARTING CREATION OF COMPATIBLE CLIMBING ANIMATIONS")
    print("=" * 60)
    
    # Create compatible animations
    create_climbing_animations()
    
    print("\nANIMATION CREATION COMPLETED!")
    print("Now you can use compatible animations in ABP_Player")
    print("\nNEXT STEPS:")
    print("1. Open ABP_Player")
    print("2. Create State Machine for climbing")
    print("3. Connect created animations:")
    print("   - AS_Vault for vaulting")
    print("   - BS_Mantle for mantling")
    print("   - AS_LedgeClimb_* for ledge climbing")
    print("4. Set up transitions with variables bIsVaulting, bIsMantling, bIsLedgeClimbing")

if __name__ == "__main__":
    main()





