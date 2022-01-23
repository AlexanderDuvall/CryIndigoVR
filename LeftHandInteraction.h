#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <vector>
class LeftHandInteraction : public IEntityComponent
{

public:
	static int counter;
	static bool grabbed;
	static IEntity* currentChild;
	static IEntity* LeftHandEntity;
	static IRenderNode* pItemRenderNode;
	static EntityId LastEntity;
	static int constraintid;
	LeftHandInteraction() {};
	~LeftHandInteraction() {};

	static void ReflectType(Schematyc::CTypeDesc<LeftHandInteraction>& desc) {
		desc.SetEditorCategory("IndigoAffect");
		desc.SetGUID("{BD1D6F6D-3290-4FBA-8583-FB47734215CF}"_cry_guid);
		desc.SetLabel("Left Hand Interaction");
		desc.SetDescription("Add object interaction to for the left hand. Component must be put on a child of the hand. \n The tracking box will be wherever the child is.");
	}
	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
private:
	std::map<EntityId, IEntity*> nearHand;
	Vec3 closest;
	virtual EntityId getNearestObject(std::map<EntityId, IEntity*>, IEntity*);
	static bool isMinimized;
	virtual bool getColliderStatus();
	virtual bool setColliderStatus(bool);
};

