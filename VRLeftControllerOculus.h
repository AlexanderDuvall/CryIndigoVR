#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
static Matrix34 handLocale;
static Matrix34 worldLocale;
 
class VRLeftControllerOculus : public IEntityComponent
{
public:
	VRLeftControllerOculus() {};
	~VRLeftControllerOculus() {};
	CCamera mainCamera;
	IEntity* leftHand;
	bool bGripPressed = false;
	bool trigger = false; 
	bool stable = false;
	static void ReflectType(Schematyc::CTypeDesc<VRLeftControllerOculus>& desc) {
		desc.SetGUID("{36BF7CC6-A0D8-4187-8B9F-2CA66C7C4CBB}"_cry_guid);
		desc.SetLabel("VRLeftController Oculus");
		desc.SetEditorCategory("IndigoAffect");
		desc.AddMember(&VRLeftControllerOculus::xRot, 'xro', "xRot", "xRot", "X Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftControllerOculus::zRot, 'zro', "zRot", "zRot", "Z Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftControllerOculus::yRot, 'yro', "yRot", "yRot", "Y Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftControllerOculus::xtr, 'xtr', "xTran", "xTran", "X Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftControllerOculus::ytr, 'ytr', "yTran", "yTran", "Y Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftControllerOculus::ztr, 'ztr', "zTran", "zTran", "Z Translation offset of the left hand for testing.", 0.0F);
		desc.SetDescription("This component will set the left hand movement for an Oculus controller.");
	}
	float yRot = 0.0;
	float xRot = 0.0;
	float zRot = 0.0;

	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;

	static float angx;
	static float angy;
	static float angz;
	void MoveLiving(IPhysicalEntity&, Matrix34);
	static Ang3 getAngles();
	bool isPointing = false;
	bool isFist = false;
	bool isGrabbed = false;
	float xtr = 0.0;
	float ytr = 0;
	float ztr = 0;
	float ImpulseScalar = 0.0;
	Matrix34 pastLocation = Matrix34::Create(Vec3(-1, -1, -1), ZERO, Vec3(-1, -1, -1));
	Vec3 pastVR = Vec3(-1, -1, -1);
 	FragmentID point;
	FragmentID pointStable;
	FragmentID idle;
	FragmentID grabStable;
	FragmentID grab;
	FragmentID fistStable;
	FragmentID fist;
	static Vec3 angVel;
	IActionPtr fistAction;
	IActionPtr fistStableAction;
	IActionPtr pointAction;
	IActionPtr pointStableAction;
	IActionPtr grabAction;
	IActionPtr grabStableAction;
	IActionPtr idleAction;
	IActionController* mAction;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent = nullptr;
	bool isIdle = false;

};
 
