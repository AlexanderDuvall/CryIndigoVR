#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>
class VRMovementComponentOculus : public IEntityComponent
{
public:
	VRMovementComponentOculus() {};
	~VRMovementComponentOculus() {};
	static EntityId leftHand;
	static EntityId rightHand;
	static bool canMove;
	static float leftTriggerOculus;
	static float rightTriggerOculus;
	static void ReflectType(Schematyc::CTypeDesc<VRMovementComponentOculus>& desc) {
		desc.SetGUID("{CB232C5A-2544-4428-ADEE-29F9A7160CED}"_cry_guid);
		desc.SetEditorCategory("IndigoAffect");
		desc.SetLabel("VRMovementComponentOculus");
		desc.AddMember(&VRMovementComponentOculus::m_speed, 'spe', "Speed", "Speed", "How fast the VR object will go.", 3.0F);
		desc.SetComponentFlags({ IEntityComponent::EFlags::Singleton, IEntityComponent::EFlags::Transform });
	}
	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	Cry::Audio::DefaultComponents::CListenerComponent* m_pAudioListenerComponent = nullptr;

	float m_speed = 1.0F;
	CCamera mainCamera;
private:
	IEntity* CameraEntity;
};

