#pragma once
#include <CryEntitySystem/IEntityComponent.h>
#include <vector>
class RightHandInteraction : public IEntityComponent
{


public:
	static bool grabbedx;
	static IEntity* currentChildx;
	static IEntity* RightHandEntityx;
	static IRenderNode* pItemRenderNode;
	static EntityId LastEntityx;
	static int constraintid;
	RightHandInteraction() {};
	~RightHandInteraction() {};

	static void ReflectType(Schematyc::CTypeDesc<RightHandInteraction>& desc) {
		desc.SetEditorCategory("IndigoAffect");
		desc.SetGUID("{A2CE04D8-2220-4CE1-A25F-7C7E948155B7}"_cry_guid);
		desc.SetLabel("Right Hand Interaction");
		desc.SetDescription("Add object interaction to for the right hand. Component must be put on a child of the hand. \n The tracking box will be wherever the child is.");
	}
	//void RightHandInteraction::PhysicalizeChild(IEntity*);
	virtual void Initialize() override;
 	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
private:
};

