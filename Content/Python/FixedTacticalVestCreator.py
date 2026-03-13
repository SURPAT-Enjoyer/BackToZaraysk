import unreal

print("=== CREATING TACTICAL VEST ASSETS ===")

# Create directories
editor_asset_lib = unreal.EditorAssetLibrary()
icon_path = "/Game/BackToZaraysk/Core/Items/Icons"
equipment_path = "/Game/BackToZaraysk/Core/Items/Equipment"

if not editor_asset_lib.does_directory_exist(icon_path):
    editor_asset_lib.make_directory(icon_path)
    print("Created directory: " + icon_path)

if not editor_asset_lib.does_directory_exist(equipment_path):
    editor_asset_lib.make_directory(equipment_path)
    print("Created directory: " + equipment_path)

# 1. Create black texture icon (simplified approach)
print("\n1. Creating black texture icon...")
try:
    # Create texture using the simple factory
    texture = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "T_VestIcon_Black_3x3",
        icon_path,
        unreal.Texture2D,
        unreal.Texture2DFactoryNew()
    )
    
    if texture:
        print("SUCCESS: Texture created")
        editor_asset_lib.save_asset("/Game/BackToZaraysk/Core/Items/Icons/T_VestIcon_Black_3x3")
    else:
        print("ERROR: Failed to create texture")
except Exception as e:
    print("ERROR creating texture: " + str(e))

# 2. Create Data Asset
print("\n2. Creating Data Asset...")
try:
    item_data_class = unreal.load_class(None, "/Script/BackToZaraysk.TacticalVestItemData")
    if item_data_class:
        data_asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "DA_TacticalVest",
            equipment_path,
            item_data_class,
            unreal.DataAssetFactory()
        )
        
        if data_asset:
            print("SUCCESS: Data Asset created")
            
            # Set properties
            data_asset.set_editor_property("display_name", unreal.Text("Tactical Vest"))
            data_asset.set_editor_property("size_in_cells_x", 3)
            data_asset.set_editor_property("size_in_cells_y", 3)
            data_asset.set_editor_property("attach_socket_name", "spine_03")
            
            # Try to load mesh
            vest_mesh = unreal.load_asset("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall")
            if vest_mesh:
                data_asset.set_editor_property("equipped_mesh", vest_mesh)
                print("SUCCESS: Mesh loaded")
            else:
                print("WARNING: Mesh not found - will need manual setup")
            
            # Try to load icon
            icon_asset = unreal.load_asset("/Game/BackToZaraysk/Core/Items/Icons/T_VestIcon_Black_3x3")
            if icon_asset:
                data_asset.set_editor_property("icon", icon_asset)
                print("SUCCESS: Icon set")
            
            editor_asset_lib.save_asset("/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest")
            print("SUCCESS: Data Asset saved")
        else:
            print("ERROR: Failed to create Data Asset")
    else:
        print("ERROR: TacticalVestItemData class not found")
except Exception as e:
    print("ERROR creating Data Asset: " + str(e))

# 3. Create Blueprint
print("\n3. Creating Blueprint...")
try:
    parent_class = unreal.load_class(None, "/Script/BackToZaraysk.TacticalVest")
    if parent_class:
        blueprint_factory = unreal.BlueprintFactory()
        blueprint_factory.set_editor_property("parent_class", parent_class)
        
        blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            "BP_TacticalVest",
            equipment_path,
            unreal.Blueprint,
            blueprint_factory
        )
        
        if blueprint:
            print("SUCCESS: Blueprint created")
            
            # Configure Blueprint
            try:
                bp_generated_class = blueprint.generated_class()
                cdo = unreal.get_default_object(bp_generated_class)
                
                # Enable physics
                cdo.set_editor_property("b_enable_physics", True)
                print("SUCCESS: Physics enabled")
                
                # Set item class
                if item_data_class:
                    cdo.set_editor_property("item_class", item_data_class)
                    print("SUCCESS: ItemClass set")
                
                # Try to set mesh in component
                vest_mesh = unreal.load_asset("/Game/Insurgent_2/Mesh/Separate_Parts/SK_ChestRigSmall")
                if vest_mesh:
                    mesh_component = cdo.get_editor_property("mesh")
                    if mesh_component:
                        mesh_component.set_editor_property("skeletal_mesh", vest_mesh)
                        print("SUCCESS: Mesh set in component")
                
                editor_asset_lib.save_asset("/Game/BackToZaraysk/Core/Items/Equipment/BP_TacticalVest")
                print("SUCCESS: Blueprint saved")
                
            except Exception as e:
                print("WARNING: Error configuring Blueprint: " + str(e))
        else:
            print("ERROR: Failed to create Blueprint")
    else:
        print("ERROR: TacticalVest class not found")
except Exception as e:
    print("ERROR creating Blueprint: " + str(e))

print("\n=== ASSET CREATION COMPLETE ===")
print("Check Content Browser for:")
print("- /Game/BackToZaraysk/Core/Items/Icons/T_VestIcon_Black_3x3")
print("- /Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest")
print("- /Game/BackToZaraysk/Core/Items/Equipment/BP_TacticalVest")
print("\nIf some assets are missing, create them manually:")
print("1. Right-click in Content Browser -> Create -> Texture2D (for icon)")
print("2. Right-click -> Create -> Miscellaneous -> Data Asset -> TacticalVestItemData")
print("3. Right-click -> Create -> Blueprint Class -> TacticalVest")



