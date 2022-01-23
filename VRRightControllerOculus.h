#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>

class VRRightControllerOculus : public IEntityComponent
{
public:
	VRRightControllerOculus() {};
	~VRRightControllerOculus() {};
	CCamera mainCamera;
	IEntity* rightHand;
	static float fTrigger;
	static void ReflectType(Schematyc::CTypeDesc<VRRightControllerOculus>& desc) {
		desc.SetGUID("{213C671B-0D63-4E8F-9EB3-236E3242D49C}"_cry_guid);
		desc.SetLabel("VRRightController Oculus");
		desc.SetEditorCategory("IndigoAffect");
		desc.SetDescription("This component will set tracking for the right Oculus controller.");
		desc.SetComponentFlags({ IEntityComponent::EFlags::Singleton, IEntityComponent::EFlags::Transform });
		desc.AddMember(&VRRightControllerOculus::xRot, 'xro', "xRot", "xRot", "X Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightControllerOculus::zRot, 'zro', "zRot", "zRot", "Z Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightControllerOculus::yRot, 'yro', "yRot", "yRot", "Y Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightControllerOculus::xtr, 'xtr', "xTran", "xTran", "X Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightControllerOculus::ztr, 'ztr', "zTran", "zTran", "Z Translation offset of the left hand for testing.", 0.0F);
		desc.AddMember(&VRRightControllerOculus::ytr, 'ytr', "yTran", "yTran", "Y Translation offset of the left hand for testing.", 0.0F);
	}
	float yRot = 0.0;
	float xRot = 0.0;
	float zRot = 0.0;
	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	static Ang3 getAngles();

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
private:
	//bool isPointing = false;
	//bool isFist = false;
	//bool isGrabbed = false;
	//bool stable = false;
	//FragmentID point;
	//FragmentID pointStable;
	//FragmentID idle;
	//FragmentID grabStable;
	//FragmentID grab;
	//FragmentID fistStable;
	//FragmentID fist;
	//
	//IActionPtr fistAction;
	//IActionPtr fistStableAction;
	//IActionPtr pointAction;
	//IActionPtr pointStableAction;
	//IActionPtr grabAction;
	//IActionPtr grabStableAction;
	//IActionPtr idleAction;
	//IActionController* mAction;
	bool isIdle = false;
	Cry::DefaultComponents::CAdvancedAnimationComponent* m_pAnimationComponent = nullptr;

};

