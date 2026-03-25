import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
factory = unreal.WidgetBlueprintFactory()
factory.parent_class = unreal.load_class(None, '/Script/BackToZaraysk.InventoryWidget')

package_path = '/Game/UI'
asset_name = 'WBP_Inventory'

existing = unreal.EditorAssetLibrary.does_asset_exist(f'{package_path}/{asset_name}')
if not existing:
    bp = asset_tools.create_asset(asset_name, package_path, unreal.WidgetBlueprint, factory)
    if bp:
        unreal.log('Created WBP_Inventory')
        unreal.EditorAssetLibrary.save_loaded_asset(bp)
else:
    unreal.log('WBP_Inventory already exists')







