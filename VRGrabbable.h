#pragma once
#include <CryEntitySystem/IEntityComponent.h>
class VRGrabbable : public IEntityComponent
{
public:
	VRGrabbable() {};
	~VRGrabbable() {};
	Quat getEntityAngleOffset(Ang3, bool);
	Vec3 getEntityTransformOffset(bool isRightHand);
	static void ReflectType(Schematyc::CTypeDesc<VRGrabbable>& desc) {
		desc.SetGUID("{EC66FA7E-1022-401D-B92C-092845833F77}"_cry_guid);
		desc.SetEditorCategory("IndigoAffect");
		desc.SetLabel("VR Grabbable");
		desc.AddMember(&VRGrabbable::LeftXRota, 'lxr', "Left_Grabbed_Rotation_X", "Left Grabbed Rotation X", "Add a rotation for grabbing the Object on the X-axis for the left hand", 0.0F);
		desc.AddMember(&VRGrabbable::LeftYRota, 'lyr', "Left_Grabbed_Rotation_Y", "Left Grabbed Rotation Y", "Add a rotation for grabbing the Object on the Y-axis for the left hand", 0.0F);
		desc.AddMember(&VRGrabbable::LeftZRota, 'lzr', "Left_Grabbed_Rotation_Z", "Left Grabbed Rotation Z", "Add a rotation for grabbing the Object on the Z-axis for the left hand", 0.0F);
		
		desc.AddMember(&VRGrabbable::RightXRota, 'rxr', "Right_Grabbed_Rotation_X", "Right Grabbed Rotation X", "Add a rotation for grabbing the Object on the X-axis for the rightHand", 0.0F);
		desc.AddMember(&VRGrabbable::RightYRota, 'ryr', "Right_Grabbed_Rotation_Y", "Right Grabbed Rotation Y", "Add a rotation for grabbing the Object on the Y-axis for the rightHand", 0.0F);
		desc.AddMember(&VRGrabbable::RightZRota, 'rzr', "Right_Grabbed_Rotation_Z", "Right Grabbed Rotation Z", "Add a rotation for grabbing the Object on the Z-axis for the rightHand", 0.0F);

		desc.AddMember(&VRGrabbable::xTrans, 'xgt', "Grabbed_Translation_X", "Grabbed Translation X", "Add a transform offset for the object when grabbed ", 0.0F);
		desc.AddMember(&VRGrabbable::yTrans, 'ygt', "Grabbed_Translation_Y", "Grabbed Translation Y", "Add a transform offset for the object when grabbed", 0.0F);
		desc.AddMember(&VRGrabbable::zTrans, 'zgt', "Grabbed_Translation_Z", "Grabbed Translation Z", "Add a transform offset for the object when grabbed", 0.0F);
		desc.SetDescription("This component will determine if the selected object can be grabbed by the HMD's hands.");
	}
	float LeftYRota = 0.0;
	float LeftXRota = 0.0;
	float LeftZRota = 0.0;
	float RightXRota = 0.0;
	float RightYRota = 0.0;
	float RightZRota = 0.0;
	float yTrans = 0.0;
	float xTrans = 0.0;
	float zTrans = 0.0;

	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	Ang3 FinalOffset;
	Ang3 AngleOffset;


};