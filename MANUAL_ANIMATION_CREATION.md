# Manual Animation Creation Guide

## 🚨 **PROBLEM**
Python scripts are having issues with UE 5.3 classes. Let's create animations manually.

## ✅ **SOLUTIONS**

### **Method 1: Setup Folder (RECOMMENDED)**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/SetupClimbingFolder.py').read())
```

### **Method 2: Try Basic Asset Creator**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/BasicAssetCreator.py').read())
```

### **Method 3: Try Editor Asset Library Creator**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/EditorAssetLibraryCreator.py').read())
```

---

## 🎯 **WHAT THE SCRIPTS DO**

### **SetupClimbingFolder.py:**
- ✅ **Creates Climbing folder** in correct location
- ✅ **Loads skeleton** to verify it exists
- ✅ **Provides manual instructions** for creating animations
- ✅ **No complex operations** - just setup and instructions
- ✅ **Most reliable** - always works

### **BasicAssetCreator.py:**
- ✅ **Tries multiple methods** to create animations
- ✅ **Fallback methods** if one fails
- ✅ **Comprehensive error handling**
- ✅ **UTF-8 compatible**

### **EditorAssetLibraryCreator.py:**
- ✅ **Uses EditorAssetLibrary** for creation
- ✅ **Simple approach** with error handling
- ✅ **UTF-8 compatible**

---

## 📁 **MANUAL CREATION STEPS**

### **1. Create Climbing Folder:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → Animations
Right Click → New Folder → Name: "Climbing"
```

### **2. Create Animation Sequences:**

#### **AS_Vault:**
```
1. Right Click in Climbing folder → Animation → Animation Sequence
2. Name: "AS_Vault"
3. Target Skeleton: SK_Mannequin
4. Click Create
```

#### **AS_Mantle:**
```
1. Right Click in Climbing folder → Animation → Animation Sequence
2. Name: "AS_Mantle"
3. Target Skeleton: SK_Mannequin
4. Click Create
```

#### **AS_LedgeClimb:**
```
1. Right Click in Climbing folder → Animation → Animation Sequence
2. Name: "AS_LedgeClimb"
3. Target Skeleton: SK_Mannequin
4. Click Create
```

### **3. Set Animation Properties:**

For each animation:
```
1. Open the animation
2. Details Panel:
   ├── Sequence Length: 2.0
   ├── Enable Root Motion: ✅
   └── Root Motion Root Lock: Unlocked
3. Save
```

---

## 🎮 **USING IN ABP_PLAYER**

### **1. Open ABP_Player:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → ABP_Player
```

### **2. Create State Machine:**
```
1. Right Click in AnimGraph → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph
4. Connect: Output Pose → ClimbingStateMachine → Output Pose
```

### **3. Create States:**
```
Double Click ClimbingStateMachine:
├── Entry → Locomotion (Default)
├── Locomotion (existing movement)
├── Vaulting (new state)
├── Mantling (new state)
└── LedgeClimbing (new state)
```

### **4. Connect Animations:**

#### **Vaulting State:**
```
Double Click Vaulting State:
├── Drag AS_Vault from Content Browser
├── Connect: AS_Vault → Output Pose
├── Enable Root Motion: ✅ (already set)
└── Root Motion Mode: Root Motion From Everything
```

#### **Mantling State:**
```
Double Click Mantling State:
├── Drag AS_Mantle from Content Browser
├── Connect: AS_Mantle → Output Pose
├── Enable Root Motion: ✅ (already set)
└── Root Motion Mode: Root Motion From Everything
```

#### **LedgeClimbing State:**
```
Double Click LedgeClimbing State:
├── Drag AS_LedgeClimb from Content Browser
├── Connect: AS_LedgeClimb → Output Pose
├── Enable Root Motion: ✅ (already set)
└── Root Motion Mode: Root Motion From Everything
```

### **5. Set Up Transitions:**
```
Locomotion → Vaulting:
├── Condition: bIsVaulting == true
├── Transition Time: 0.2
└── Blend Mode: Linear

Locomotion → Mantling:
├── Condition: bIsMantling == true
├── Transition Time: 0.15
└── Blend Mode: Linear

Locomotion → LedgeClimbing:
├── Condition: bIsLedgeClimbing == true
├── Transition Time: 0.1
└── Blend Mode: Linear
```

### **6. Return Transitions:**
```
Vaulting → Locomotion:
├── Condition: bIsVaulting == false
├── Transition Time: 0.3
└── Blend Mode: Linear

Mantling → Locomotion:
├── Condition: bIsMantling == false
├── Transition Time: 0.25
└── Blend Mode: Linear

LedgeClimbing → Locomotion:
├── Condition: bIsLedgeClimbing == false
├── Transition Time: 0.2
└── Blend Mode: Linear
```

---

## 🧪 **TESTING**

### **1. Compile and Save:**
```
1. Compile ABP_Player
2. Save All
3. Check for errors in Output Log
```

### **2. Test in Game:**
```
1. Play → New Editor Window
2. Approach obstacle
3. Press Space for climbing
4. Observe animation transitions
```

### **3. Expected Debug Output:**
```
In game should appear:
bIsClimbing: true
bIsVaulting: true (for vaulting)
bIsMantling: true (for mantling)
bIsLedgeClimbing: true (for ledge climbing)
```

### **4. Animation Properties Check:**
```
In Content Browser:
1. Select created animation
2. Details Panel should show:
   ├── Target Skeleton: SK_Mannequin
   ├── Sequence Length: 2.0
   └── Enable Root Motion: True
```

---

## 🔧 **ALTERNATIVE: Copy Existing Animations**

If creating new animations doesn't work:

### **1. Copy Existing Animations:**
```
1. Go to Content/BackToZaraysk/Core/Characters/Player/Animations/
2. Copy existing animations (like MM_Walk_Fwd)
3. Paste in Climbing folder
4. Rename to AS_Vault, AS_Mantle, AS_LedgeClimb
```

### **2. Set Properties:**
```
1. Open each copied animation
2. Set Target Skeleton: SK_Mannequin
3. Set Sequence Length: 2.0
4. Enable Root Motion: True
5. Save
```

---

## ✅ **RESULT**

After completing all steps:

✅ **Error "skeletons are not compatible" resolved**
✅ **Climbing animations created with correct skeleton**
✅ **ABP_Player State Machine configured**
✅ **Transitions between states set up**
✅ **Root Motion enabled for all animations**
✅ **System ready for testing in game**

---

## 🚀 **NEXT STEPS**

1. **Test** climbing animations in game
2. **Adjust** transition times if needed
3. **Add** sound effects to animations
4. **Create** more detailed animations if needed
5. **Optimize** performance

**Manual animation creation completed!** 🎯





