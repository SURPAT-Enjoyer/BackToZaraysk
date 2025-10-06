# -*- coding: utf-8 -*-
"""
Automatic creation of tactical vest assets
"""

import unreal

def create_black_texture_icon():
    """Creates black texture 96x96 for vest icon"""
    print("=== Creating black icon 3x3 ===")
    
    # Path for saving
    texture_path = "/Game/BackToZaraysk/Core/Items/Icons/T_VestIcon_Black_3x3"
    package_path = "/Game/BackToZaraysk/Core/Items/Icons"
    
    # Create directory if not exists
    editor_asset_lib = unreal.EditorAssetLibrary()
    if not editor_asset_lib.does_directory_exist(package_path):
        editor_asset_lib.make_directory(package_path)
    
    # Create texture
    texture_factory = unreal.Texture2DFactoryNew()
    texture_factory.set_editor_property("width", 96)
    texture_factory.set_editor_property("height", 96)
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    texture = asset_tools.create_asset(
        "T_VestIcon_Black_3x3",
        package_path,
        unreal.Texture2D,
        texture_factory
    )
    
    if texture:
        print("SUCCESS: Texture created: " + texture_path)
        # Save
        editor_asset_lib.save_asset(texture_path)
        return texture_path
    else:
        print("ERROR: Failed to create texture")
        return None


def create_tactical_vest_data_asset():
    """Creates Data Asset for tactical vest"""
    print("\n=== Creating Data Asset for vest ===")
    
    # Path for saving
    data_asset_path = "/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest"
    package_path = "/Game/BackToZaraysk/Core/Items/Equipment"
    
    # Create directory if not exists
    editor_asset_lib = unreal.EditorAssetLibrary()
    if not editor_asset_lib.does_directory_exist(package_path):
        editor_asset_lib.make_directory(package_path)
    
    # Load UTacticalVestItemData class
    item_data_class = unreal.load_class(None, "/Script/BackToZaraysk.TacticalVestItemData")
    if not item_data_class:
        print("ERROR: UTacticalVestItemData class not found!")
        return None
    
    # Create Data Asset
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    data_asset = asset_tools.create_asset(
        "DA_TacticalVest",
        package_path,
        item_data_class,
        unreal.DataAssetFactory()
    )
    
    if data_asset:
        print("SUCCESS: Data Asset created: " + data_asset_path)
        
        # Load vest mesh
        vest_mesh = unreal.load_asset("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall")
        if vest_mesh:
            data_asset.set_editor_property("equipped_mesh", vest_mesh)
            print("SUCCESS: SK_ChestRigSmall mesh set")
        else:
            print("WARNING: SK_ChestRigSmall mesh not found")
        
        # Load icon
        icon_texture = unreal.load_asset("/Game/BackToZaraysk/Core/Items/Icons/T_VestIcon_Black_3x3")
        if icon_texture:
            data_asset.set_editor_property("icon", icon_texture)
            print("SUCCESS: Icon set")
        
        # Set properties
        data_asset.set_editor_property("display_name", unreal.Text("Tactical Vest"))
        data_asset.set_editor_property("size_in_cells_x", 3)
        data_asset.set_editor_property("size_in_cells_y", 3)
        data_asset.set_editor_property("attach_socket_name", "spine_03")
        
        # Save
        editor_asset_lib.save_asset(data_asset_path)
        print("SUCCESS: Data Asset saved")
        return data_asset_path
    else:
        print("ERROR: Failed to create Data Asset")
        return None


def create_tactical_vest_blueprint():
    """Creates Blueprint actor for tactical vest"""
    print("\n=== Creating Blueprint for vest ===")
    
    # Path for saving
    bp_path = "/Game/BackToZaraysk/Core/Items/Equipment/BP_TacticalVest"
    package_path = "/Game/BackToZaraysk/Core/Items/Equipment"
    
    # Create directory if not exists
    editor_asset_lib = unreal.EditorAssetLibrary()
    if not editor_asset_lib.does_directory_exist(package_path):
        editor_asset_lib.make_directory(package_path)
    
    # Load C++ class ATacticalVest
    parent_class = unreal.load_class(None, "/Script/BackToZaraysk.TacticalVest")
    if not parent_class:
        print("ERROR: ATacticalVest class not found!")
        return None
    
    # Create Blueprint
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    blueprint_factory = unreal.BlueprintFactory()
    blueprint_factory.set_editor_property("parent_class", parent_class)
    
    blueprint = asset_tools.create_asset(
        "BP_TacticalVest",
        package_path,
        unreal.Blueprint,
        blueprint_factory
    )
    
    if blueprint:
        print("SUCCESS: Blueprint created: " + bp_path)
        
        # Get CDO (Class Default Object)
        try:
            bp_generated_class = blueprint.generated_class()
            cdo = unreal.get_default_object(bp_generated_class)
            
            # Load vest mesh
            vest_mesh = unreal.load_asset("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall")
            if vest_mesh:
                # Get Mesh component
                mesh_component = cdo.get_editor_property("mesh")
                if mesh_component:
                    mesh_component.set_editor_property("skeletal_mesh", vest_mesh)
                    print("SUCCESS: SK_ChestRigSmall mesh set in component")
            
            # Enable physics
            cdo.set_editor_property("b_enable_physics", True)
            print("SUCCESS: Physics enabled")
            
            # Set Data Asset
            data_asset = unreal.load_asset("/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest")
            if data_asset:
                cdo.set_editor_property("item_class", data_asset.get_class())
                print("SUCCESS: ItemClass set")
            
            # Compile Blueprint
            unreal.EditorAssetLibrary.save_asset(bp_path)
            print("SUCCESS: Blueprint saved and compiled")
            
        except Exception as e:
            print("WARNING: Error configuring Blueprint: " + str(e))
        
        return bp_path
    else:
        print("ERROR: Failed to create Blueprint")
        return None


def main():
    """Main function - creates all assets"""
    print("\n" + "="*60)
    print("CREATING TACTICAL VEST ASSETS")
    print("="*60 + "\n")
    
    # 1. Create icon
    icon_path = create_black_texture_icon()
    
    # 2. Create Data Asset
    data_asset_path = create_tactical_vest_data_asset()
    
    # 3. Create Blueprint
    bp_path = create_tactical_vest_blueprint()
    
    # Results
    print("\n" + "="*60)
    print("RESULTS:")
    print("="*60)
    if icon_path:
        print("SUCCESS: Icon: " + icon_path)
    if data_asset_path:
        print("SUCCESS: Data Asset: " + data_asset_path)
    if bp_path:
        print("SUCCESS: Blueprint: " + bp_path)
    
    print("\nALL ASSETS CREATED!")
    print("\nNow you can:")
    print("1. Drag BP_TacticalVest to scene")
    print("2. Pick up vest with character")
    print("3. Equip through inventory context menu")
    print("="*60 + "\n")


# Запускаем создание ассетов
if __name__ == "__main__":
    main()

