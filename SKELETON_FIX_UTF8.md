# Fix for "skeletons are not compatible" Error in ABP_Player

## 🚨 **PROBLEM**
When trying to connect climbing animations from Free Animation Library to ABP_Player State Machine, you get:
```
"skeletons are not compatible"
```

## 🔍 **CAUSE**
The animations from Free Animation Library may be created for a different version of the `SK_Mannequin` skeleton or have incompatible settings.

## ✅ **SOLUTION**

### **Method 1: Quick Fix (RECOMMENDED)**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/QuickSkeletonFix.py').read())
```

### **Method 2: Full Fix with UTF-8 Encoding**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/FixSkeletonCompatibility_UTF8.py').read())
```

### **Method 3: Create New Compatible Animations**

1. **Open Unreal Editor**
2. **Press `Ctrl + Shift + P`** for Python console
3. **Copy and paste this command:**
```python
exec(open('Content/Python/CreateClimbingAnimations_UTF8.py').read())
```

---

## 🎯 **WHAT THE SCRIPTS DO**

### **QuickSkeletonFix.py:**
- ✅ **Fixes ABP_Player skeleton** to correct version
- ✅ **Creates simple climbing animations** with correct skeleton
- ✅ **Fast execution** - takes only a few seconds
- ✅ **UTF-8 compatible** - no encoding errors

### **FixSkeletonCompatibility_UTF8.py:**
- ✅ **Fixes skeleton** of existing climbing animations
- ✅ **Checks ABP_Player** for correct skeleton
- ✅ **Creates backup copies** of original animations
- ✅ **UTF-8 compatible** - no encoding errors

### **CreateClimbingAnimations_UTF8.py:**
- ✅ **Creates new animations** with correct skeleton
- ✅ **Creates Blend Space** for Mantle animations
- ✅ **Sets up Root Motion** for all animations
- ✅ **UTF-8 compatible** - no encoding errors

---

## 📁 **CREATED ANIMATIONS**

After running the scripts, these animations will be created:

```
Content/BackToZaraysk/Core/Characters/Player/Animations/Climbing/
├── AS_Vault.uasset (vaulting)
├── AS_Mantle_1M.uasset (mantling 1m)
├── AS_Mantle_2M.uasset (mantling 2m)
├── AS_LedgeClimb_Up.uasset (climbing to ledge)
├── AS_LedgeClimb_Idle.uasset (waiting on ledge)
├── AS_LedgeClimb_Down.uasset (descending from ledge)
└── BS_Mantle.uasset (Blend Space for mantling)
```

---

## 🎮 **USING IN ABP_PLAYER**

### **1. Open ABP_Player:**
```
Content Browser → BackToZaraysk → Core → Characters → Player → ABP_Player
```

### **2. Create State Machine:**
```
1. Right Click → State Machines → Add New State Machine
2. Name: "ClimbingStateMachine"
3. Drag to Main Graph
```

### **3. Create States:**
```
ClimbingStateMachine:
├── Entry → Locomotion (Default)
├── Locomotion (existing movement)
├── Vaulting (new)
├── Mantling (new)
└── LedgeClimbing (new)
```

### **4. Connect Animations:**

#### **Vaulting State:**
```
Double Click Vaulting:
├── Drag AS_Vault from Content Browser
├── Connect: AS_Vault → Output Pose
├── Enable Root Motion: ✅
└── Root Motion Mode: Root Motion From Everything
```

#### **Mantling State:**
```
Double Click Mantling:
├── Drag BS_Mantle from Content Browser
├── Connect: BS_Mantle → Output Pose
├── Input: ClimbingHeight
├── Enable Root Motion: ✅
└── Play Rate: ClimbingSpeed / 100.0
```

#### **LedgeClimbing State:**
```
Double Click LedgeClimbing:
├── Drag AS_LedgeClimb_Up from Content Browser
├── Connect: AS_LedgeClimb_Up → Output Pose
├── Enable Root Motion: ✅
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

---

## 🔧 **MANUAL FIX (if scripts don't work)**

### **1. Check ABP_Player Skeleton:**
```
1. Open ABP_Player
2. In Details panel find Target Skeleton
3. Should be: SK_Mannequin
```

### **2. Check Animation Skeletons:**
```
1. Open climbing animation
2. In Details panel find Skeleton
3. Should be: SK_Mannequin
```

### **3. Fix Incompatible Animations:**
```
1. Right Click on animation → Reimport
2. Select correct Skeleton: SK_Mannequin
3. Click Import
```

### **4. Create New Animations:**
```
1. Right Click in Content Browser → Animation → Animation Sequence
2. Target Skeleton: SK_Mannequin
3. Create simple climbing animation
4. Enable Root Motion: ✅
```

---

## 🧪 **TESTING**

### **1. Compile and Save:**
```
1. Compile ABP_Player
2. Save All
3. Check for errors
```

### **2. Test in Game:**
```
1. Play → New Editor Window
2. Approach obstacle
3. Press Space for climbing
4. Observe animation transitions
```

### **3. Debug Information:**
```
In game should appear:
bIsClimbing: true
bIsVaulting: true (for vaulting)
bIsMantling: true (for mantling)
bIsLedgeClimbing: true (for ledge climbing)
```

---

## ✅ **RESULT**

After fixing:

✅ **Error "skeletons are not compatible" resolved**
✅ **Climbing animations connected to ABP_Player**
✅ **State Machine works correctly**
✅ **Transitions between states set up**
✅ **Root Motion enabled for all animations**
✅ **System ready for use in game**

---

## 🚀 **NEXT STEPS**

1. **Test** climbing animations in game
2. **Set up** transitions between states
3. **Add** sound effects
4. **Optimize** performance
5. **Create** additional animations if needed

**Skeleton compatibility problem solved!** 🎯





