#pragma once
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include <CryEntitySystem/IEntityComponent.h>
static Matrix34 handLocale;
static Matrix34 worldLocale;
class VRLeftController : public IEntityComponent
{
public:
	VRLeftController() {};
	~VRLeftController() {};
	CCamera mainCamera;
	bool bGripPressed = false;
	bool trigger = false;
	IEntity* leftHand;
	bool stable = false;
	static void ReflectType(Schematyc::CTypeDesc<VRLeftController>& desc) {
		desc.SetGUID("{8D8E2ABE-1314-48AE-BB39-45C0E3206A52}"_cry_guid);
		desc.SetLabel("VRLeftController OpenVR");
		desc.SetEditorCategory("IndigoAffect");
		desc.AddMember(&VRLeftController::xRot, 'xro', "xRot", "xRot", "X Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::zRot, 'zro', "zRot", "zRot", "Z Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::yRot, 'yro', "yRot", "yRot", "Y Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::xtr, 'xtr', "xTran", "xTran", "X Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::ytr, 'ytr', "yTran", "yTran", "Y Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::ztr, 'ztr', "zTran", "zTran", "Z Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRLeftController::ImpulseScalar, 'imp', "imp", "imp", "Impulse for testing", 0.0F);
		desc.SetDescription("This component will set the left hand movement for an OpenVR controller.");
	}
	static float angx;
	static float angy;
	static float angz;
	void MoveLiving(IPhysicalEntity&, Matrix34);
	static Ang3 getAngles();
	bool isPointing = false;
	bool isFist = false;
	bool isGrabbed = false;
	float yRot = 0.0;
	float xRot = 0.0;
	float zRot = 0.0;
	float xtr = 0.0;
	float ytr = 0;
	float ztr = 0;
	float ImpulseScalar = 0.0;
	Matrix34 pastLocation = Matrix34::Create(Vec3(-1, -1, -1), ZERO, Vec3(-1, -1, -1));
	Vec3 pastVR = Vec3(-1, -1, -1);
	virtual void Initialize() override;
	void resetAnimation();
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	//FragmentID point;
	//FragmentID pointStable;
	//FragmentID idle;
	//FragmentID grabStable;
	//FragmentID grab;
	//FragmentID fistStable;
	//FragmentID fist;
	//FragmentID transition;
	static Vec3 angVel;
	//IActionPtr fistAction;
	//IActionPtr tranAction;
	//IActionPtr fistStableAction;
	//IActionPtr pointAction;
	//IActionPtr pointStableAction;
	//IActionPtr grabAction;
	//IActionPtr grabStableAction;
	//IActionPtr idleAction;
	//IActionController* mAction;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent = nullptr;
	bool isIdle = false;
};

