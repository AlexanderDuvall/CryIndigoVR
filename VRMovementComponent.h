#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <DefaultComponents/Audio/ListenerComponent.h>
class VRMovementComponent : public IEntityComponent
{
 public:
	VRMovementComponent() {};
	~VRMovementComponent() {};
	static float leftTrigger;
	static bool canMove;
	static float rightTrigger;
	static EntityId leftHand;
	static EntityId rightHand;
	static void ReflectType(Schematyc::CTypeDesc<VRMovementComponent>& desc) {
		desc.SetGUID("{81B0BAC7-145C-45F6-A04D-AE12D2A7E619}"_cry_guid);
		desc.SetEditorCategory("IndigoAffect");
		desc.SetLabel("VRMovementComponent");
		desc.AddMember(&VRMovementComponent::m_speed, 'spe', "Speed", "Speed", "How fast the VR object will go.", 3.0F);
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

