import unreal

def create_foot_ik_animation_blueprint():
    # Создать новый Animation Blueprint
    factory = unreal.AnimationBlueprintFactory()
    factory.target_skeleton = unreal.load_asset("/Game/Characters/Manuequins/Meshes/SK_Manny.SK_Manny")
    abp = factory.factory_create_new("/Game/Characters/Manuequins/Animations/ABP_Manny_FootIK")

    # Получить AnimGraph
    anim_graph = abp.get_anim_graph()

    # Добавить Sequence Player
    sequence_player = unreal.AnimGraphNode_SequencePlayer()
    sequence_player.node.title = "Sequence Player"
    sequence_player.sequence = unreal.load_asset("/Game/Characters/Manuequins/Animations/MM_Land.MM_Land")
    anim_graph.add_node(sequence_player)

    # Добавить Left Foot IK
    left_foot_ik = unreal.AnimGraphNode_FootIK()
    left_foot_ik.node.title = "Left Foot IK"
    # Настроить effector location - это будет переменная из AnimInstance
    anim_graph.add_node(left_foot_ik)

    # Добавить Right Foot IK
    right_foot_ik = unreal.AnimGraphNode_FootIK()
    right_foot_ik.node.title = "Right Foot IK"
    anim_graph.add_node(right_foot_ik)

    # Добавить Output Pose
    output_pose = unreal.AnimGraphNode_Root()
    anim_graph.add_node(output_pose)

    # Подключить ноды
    anim_graph.connect_nodes(sequence_player, 'Pose', left_foot_ik, 'Pose')
    anim_graph.connect_nodes(left_foot_ik, 'Pose', right_foot_ik, 'Pose')
    anim_graph.connect_nodes(right_foot_ik, 'Pose', output_pose, 'Result')

    # Сохранить
    unreal.EditorAssetLibrary.save_asset(abp.get_path_name())
    print("Foot IK Animation Blueprint created at /Game/Characters/Manuequins/Animations/ABP_Manny_FootIK")

    # Назначить ABP в PlayerCharacter BP
    player_bp = unreal.load_asset("/Game/BackToZaraysk/Core/Characters/Player/BP_PlayerCharacter")
    if player_bp:
        player_bp.set_editor_property('AnimBlueprintGeneratedClass', abp.generated_class)
        unreal.EditorAssetLibrary.save_asset(player_bp.get_path_name())
        print("Assigned Foot IK ABP to PlayerCharacter")
    else:
        print("Player BP not found")

create_foot_ik_animation_blueprint()



