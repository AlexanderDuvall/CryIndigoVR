#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class VRRightController : public IEntityComponent
{
public:
	VRRightController() {};
	~VRRightController() {};
	CCamera mainCamera;
	IEntity* rightHand;
	static void ReflectType(Schematyc::CTypeDesc<VRRightController>& desc) {
		desc.SetGUID("{DCDBB3DF-20CA-4415-9687-9F440D77B98D}"_cry_guid);
		desc.SetLabel("VRRightController OpenVR");
		desc.SetEditorCategory("IndigoAffect");
		desc.SetDescription("This component will set controller tracking for the right OpenVR controller");
		desc.SetComponentFlags({ IEntityComponent::EFlags::Singleton, IEntityComponent::EFlags::Transform });
		desc.AddMember(&VRRightController::xRot, 'xro', "xRot", "xRot", "X Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightController::zRot, 'zro', "zRot", "zRot", "Z Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightController::yRot, 'yro', "yRot", "yRot", "Y Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightController::xtr, 'xtr', "xTran", "xTran", "X Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightController::ztr, 'ztr', "zTran", "zTran", "Z Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightController::ytr, 'ytr', "yTran", "yTran", "Y Translation offset of the left hand for testing.", 0.0F);

	}
	float yRot = 0.0;
	float xRot = 0.0;
	float zRot = 0.0;
	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;

	bool bGripPressed = false;
	bool trigger = false;
	static Vec3 angVel;

	float xtr = 0.0;
	float ytr = 0;
	float ztr = 0;
	static float angx;
	static float angy;
	static float angz;
	void MoveLiving(IPhysicalEntity&, Matrix34);
	static Ang3 getAngles();
private:
	bool isPointing = false;
	bool isFist = false;
	bool isGrabbed = false;
	bool stable = false;
	FragmentID point;
	FragmentID pointStable;
	FragmentID idle;
	FragmentID grabStable;
	FragmentID grab;
	FragmentID fistStable;
	FragmentID fist;

	IActionPtr fistAction;
	IActionPtr fistStableAction;
	IActionPtr pointAction;
	IActionPtr pointStableAction;
	IActionPtr grabAction;
	IActionPtr grabStableAction;
	IActionPtr idleAction;
	IActionController* mAction;
	bool isIdle = false;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent = nullptr;

};

