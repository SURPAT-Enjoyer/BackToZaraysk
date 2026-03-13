import unreal

def modify_abp_for_ik():
    # Загрузить существующий ABP
    abp = unreal.load_asset("/Game/Characters/Manuequins/Animations/ABP_Manny.ABP_Manny")
    if not abp:
        print("ABP_Manny not found")
        return

    # Получить AnimGraph
    anim_graph = abp.get_anim_graph()
    if not anim_graph:
        print("No AnimGraph found")
        return

    # Очистить существующие ноды (опционально, для простоты)
    # anim_graph.clear_graph()

    # Добавить Sequence Player
    sequence_player = unreal.AnimGraphNode_SequencePlayer()
    sequence_player.node.title = "Idle Sequence"
    # Найти подходящую анимацию
    idle_anim = unreal.load_asset("/Game/Characters/Manuequins/Animations/Manny/MM_Idle.MM_Idle")
    if idle_anim:
        sequence_player.sequence = idle_anim
    anim_graph.add_node(sequence_player)

    # Добавить Variable для Left Foot Effector
    left_var = unreal.AnimGraphNode_Variable()
    left_var.node.title = "LeftFootEffectorLocation"
    left_var.variable_name = "LeftFootEffectorLocation"
    left_var.bIsGet = True
    anim_graph.add_node(left_var)

    # Добавить Variable для Right Foot Effector
    right_var = unreal.AnimGraphNode_Variable()
    right_var.node.title = "RightFootEffectorLocation"
    right_var.variable_name = "RightFootEffectorLocation"
    right_var.bIsGet = True
    anim_graph.add_node(right_var)

    # Добавить Left Two Bone IK
    left_foot_ik = unreal.AnimGraphNode_TwoBoneIK()
    left_foot_ik.node.title = "Left Foot IK"
    left_foot_ik.i_bone_a_bone_name = "thigh_l"
    left_foot_ik.i_bone_b_bone_name = "calf_l"
    left_foot_ik.i_effector_bone_name = "foot_l"
    anim_graph.add_node(left_foot_ik)

    # Добавить Right Two Bone IK
    right_foot_ik = unreal.AnimGraphNode_TwoBoneIK()
    right_foot_ik.node.title = "Right Foot IK"
    right_foot_ik.i_bone_a_bone_name = "thigh_r"
    right_foot_ik.i_bone_b_bone_name = "calf_r"
    right_foot_ik.i_effector_bone_name = "foot_r"
    anim_graph.add_node(right_foot_ik)

    # Добавить Output Pose
    output_pose = unreal.AnimGraphNode_Root()
    anim_graph.add_node(output_pose)

    # Подключить ноды
    try:
        anim_graph.connect_nodes(sequence_player, 'Pose', left_foot_ik, 'Pose')
        anim_graph.connect_nodes(left_foot_ik, 'Pose', right_foot_ik, 'Pose')
        anim_graph.connect_nodes(right_foot_ik, 'Pose', output_pose, 'Result')
        # Подключить variables к Two Bone IK
        anim_graph.connect_nodes(left_var, 'Value', left_foot_ik, 'EffectorLocation')
        anim_graph.connect_nodes(right_var, 'Value', right_foot_ik, 'EffectorLocation')
        print("Connected nodes successfully")
    except Exception as e:
        print(f"Error connecting nodes: {e}")

    # Сохранить
    unreal.EditorAssetLibrary.save_asset(abp.get_path_name())
    print("Modified ABP_Manny with Foot IK")

modify_abp_for_ik()
