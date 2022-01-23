#pragma once
#include <CryEntitySystem/IEntityComponent.h>
class VRGrabbable : public IEntityComponent
{
public:
	VRGrabbable() {};
	~VRGrabbable() {};
	Quat getEntityAngleOffset(Ang3, bool);
	Vec3 getEntityTransformOffset(bool isRightHand);
	bool isExtension();
	bool isComponentActivated();
	IEntity* getParent();
	static void ReflectType(Schematyc::CTypeDesc<VRGrabbable>& desc) {
		desc.SetGUID("{EC66FA7E-1022-401D-B92C-092845833F77}"_cry_guid);
		desc.SetEditorCategory("IndigoAffect");
		desc.SetLabel("VR Grabbable");
		desc.AddMember(&VRGrabbable::isActivated, 'isa', "is_Activated", "Is Activated", "Will Activate component", false);
		desc.AddMember(&VRGrabbable::LeftXRota, 'lxr', "Left_Grabbed_Rotation_X", "Left Grabbed Rotation X", "Add a rotation for grabbing the Object on the X-axis for the left hand", 0.0F);
		desc.AddMember(&VRGrabbable::LeftYRota, 'lyr', "Left_Grabbed_Rotation_Y", "Left Grabbed Rotation Y", "Add a rotation for grabbing the Object on the Y-axis for the left hand", 0.0F);
		desc.AddMember(&VRGrabbable::LeftZRota, 'lzr', "Left_Grabbed_Rotation_Z", "Left Grabbed Rotation Z", "Add a rotation for grabbing the Object on the Z-axis for the left hand", 0.0F);
		
		desc.AddMember(&VRGrabbable::RightXRota, 'rxr', "Right_Grabbed_Rotation_X", "Right Grabbed Rotation X", "Add a rotation for grabbing the Object on the X-axis for the rightHand", 0.0F);
		desc.AddMember(&VRGrabbable::RightYRota, 'ryr', "Right_Grabbed_Rotation_Y", "Right Grabbed Rotation Y", "Add a rotation for grabbing the Object on the Y-axis for the rightHand", 0.0F);
		desc.AddMember(&VRGrabbable::RightZRota, 'rzr', "Right_Grabbed_Rotation_Z", "Right Grabbed Rotation Z", "Add a rotation for grabbing the Object on the Z-axis for the rightHand", 0.0F);

		desc.AddMember(&VRGrabbable::LeftXTrans, 'xgt', "Grabbed_Translation_X", "Grabbed Translation X", "Add a left hand transform offset for the object when grabbed ", 0.0F);
		desc.AddMember(&VRGrabbable::LeftYTrans, 'ygt', "Grabbed_Translation_Y", "Grabbed Translation Y", "Add a left hand transform offset for the object when grabbed", 0.0F);
		desc.AddMember(&VRGrabbable::LeftZTrans, 'zgt', "Grabbed_Translation_Z", "Grabbed Translation Z", "Add a left hand transform offset for the object when grabbed", 0.0F);

		desc.AddMember(&VRGrabbable::RightXTrans, 'xrt', "Right_Grabbed_Translation_X", "Grabbed Translation X", "Add a right hand transform offset for the object when grabbed ",-1.0F);
		desc.AddMember(&VRGrabbable::RightYTrans, 'yrt', "Right_Grabbed_Translation_Y", "Grabbed Translation Y", "Add a right hand transform offset for the object when grabbed", -1.0F);
		desc.AddMember(&VRGrabbable::RightZTrans, 'zrt', "Right_Grabbed_Translation_Z", "Grabbed Translation Z", "Add a right hand transform offset for the object when grabbed", -1.0F);

		desc.AddMember(&VRGrabbable::isChildExtension, 'ice', "is_Child_Extension", "Is Child Extension", "Will add an origin at the point where the child is.", false);
		desc.AddMember(&VRGrabbable::minimizeBox, 'min', "Minimize_Collider", "Minimize Hand Collider", "Will minimize hand collider to avoid colliding with object.", false);
		desc.SetDescription("This component will determine if the selected object can be grabbed by the HMD's hands.");
	}
	float LeftYRota = 0.0;
	float LeftXRota = 0.0;
	float LeftZRota = 0.0;
	float RightXRota = 0.0;
	float RightYRota = 0.0;
	float RightZRota = 0.0;
	float LeftYTrans = 0.0;
	float LeftXTrans = 0.0;
	float LeftZTrans = 0.0;
	float RightYTrans = 0.0;
	float RightXTrans = 0.0;
	float RightZTrans = 0.0;
	bool isChildExtension = false;
	bool isActivated = false;
	bool minimizeBox = false;
	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	Ang3 FinalOffset;
	Ang3 AngleOffset;
};