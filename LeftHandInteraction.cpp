#include "StdAfx.h"
#include "LeftHandInteraction.h"
#include "RightHandInteraction.h"
#include <DefaultComponents/Input/InputComponent.h>
#include <DefaultComponents/Physics/RigidBodyComponent.h>
#include <CrySchematyc/Reflection/TypeDesc.h>
#include <CrySchematyc/Utils/EnumFlags.h>
#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/IEnvRegistrar.h>
#include <CrySchematyc/Env/Elements/EnvComponent.h>
#include <CrySchematyc/Env/Elements/EnvFunction.h>
#include <CrySchematyc/Env/Elements/EnvSignal.h>
#include <CrySchematyc/ResourceTypes.h>
#include <CrySchematyc/MathTypes.h>
#include <CrySchematyc/Utils/SharedString.h>
#include <CryCore/StaticInstanceList.h>
#include <CrySystem/VR/IHMDDevice.h>
#include <CrySystem/VR/IHMDManager.h>
#include <CryEntitySystem/IEntitySystem.h>
#include <iostream>
#include <math.h>
#include "VRMath.h"
#include <CryMath/Cry_Matrix34.h>
#include <CryPhysics/physinterface.h>
#include <CryPhysics/RayCastQueue.h>
#include <vector>
#include "VRMovementComponent.h"
#include "VRMovementComponentOculus.h"
#include "VRGrabbable.h"
#include "VRLeftController.h"
#include "DefaultComponents/Geometry/AdvancedAnimationComponent.h"
#include <DefaultComponents/Physics/BoxPrimitiveComponent.h>

//PLACE IN CORRECT SPOT ... INITIALIZE?
struct Point {
	double x;
	double y;
};
int LeftHandInteraction::counter = 0;
bool LeftHandInteraction::grabbed = false;
IEntity* LeftHandInteraction::currentChild = NULL;
IEntity* LeftHandInteraction::LeftHandEntity = NULL;
IRenderNode* LeftHandInteraction::pItemRenderNode = NULL;
EntityId LeftHandInteraction::LastEntity = -1;
int LeftHandInteraction::constraintid = 0;
bool LeftHandInteraction::isMinimized = false;
static void RegisterLeftHandInteractionComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(LeftHandInteraction));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterLeftHandInteractionComponent)

bool LeftHandInteraction::getColliderStatus() {
	return isMinimized;
}
/*set to true to minimize
  set to false make large
*/
bool LeftHandInteraction::setColliderStatus(bool a) {
	Cry::DefaultComponents::CBoxPrimitiveComponent* box = LeftHandEntity->GetOrCreateComponent<Cry::DefaultComponents::CBoxPrimitiveComponent>();
	if (!a) {//if not minimized
		box->m_size = VRMovementComponent::largeCollider; //
		isMinimized = false;
	}
	else { //if minimized
		box->m_size = Vec3(0, 0, 0); //
		isMinimized = true;
		CryLog("Minimize...");
	}
	const SEntityEvent& boxChangeEvent = SEntityEvent(ENTITY_EVENT_COMPONENT_PROPERTY_CHANGED);
	box->SendEvent(boxChangeEvent);
	return isMinimized;
}
void LeftHandInteraction::Initialize() {
	// Create a new IEntityTriggerComponent instance, responsible for registering our entity in the proximity grid
	IEntityTriggerComponent* pTriggerComponent = m_pEntity->CreateComponent<IEntityTriggerComponent>();
	// Listen to area events in a 2m^3 box around the entity
	const Vec3 triggerBoxSize = Vec3(.07, .07, .07);
	// Create an axis aligned bounding boxVec3 Transform = Vec3(LeftXTrans, LeftYTrans, LeftZTrans);, ensuring that we listen to events around the entity translation
	const AABB triggerBounds = AABB(triggerBoxSize * .5f, triggerBoxSize * .5f);
	// Now set the trigger bounds on the trigger component
	pTriggerComponent->SetTriggerBounds(triggerBounds);

}

void GrabObject(IEntity* lefthand, IEntity* GrabbableEntity, Quat angleOffset) {
	IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("LeftHandPlacement");
	pe_params_pos pp;
	pp.pos = reference2->GetWorldPos();
	pp.q = angleOffset;
	pp.scale = 1.f;
	const Vec3 constraintAxis(0, 0, 0);
	// Set the minimum rotation angle along the specified axis (in our case, up) to be 0 degrees
	const float minRotationForAxis = 0.f;
	// Set the maximum rotation angle along the specified axis (in our case, up) to be 45 degrees
	const float maxRotationForAxis = DEG2RAD(.1f);
	// Disallow rotation on the other axes
	const float minRotationOtherAxis = 0.1f;
	// Disallow rotation on the other axes
	const float maxRotationOtherAxis = 0.1f;
	// Define the constraint
	pe_action_add_constraint constraint;
	// Create a constraint using world coordinates
	constraint.flags = world_frames | constraint_no_tears;
	// Constrain the entity to the specified point
	constraint.pt[0] = pp.pos;

	//constraint.qframe[0] = constraint.qframe[1] = angleOffset;
	//constraint.qframe[0] = Quat(Ang3(180, 0, 0));
	// Set the rotational frame for the constraint, based on the specified axis
	//constraint.qframe[0] = constraint.qframe[1] = Quat::CreateRotationV0V1(Vec3(0, 0, 0), constraintAxis);
	// Now apply the constraint limits defined above
	// Note that if the max<=min (or the limits are left unused), the constraint is treated as a fully free constraint where only position is enforced.
	// To create a fully locked constraint, see the constraint_no_rotation flag.
	constraint.flags = constraint_no_rotation;
	//constraint.xlimits[0] = minRotationForAxis;
	//constraint.xlimits[1] = maxRotationForAxis;
	//constraint.yzlimits[0] = minRotationOtherAxis;
	//constraint.yzlimits[1] = maxRotationOtherAxis;
	// Constrain the entity around the world, alternatively we can apply another IPhysicalEntity pointer
	constraint.pBuddy = lefthand->GetPhysicalEntity();
	// Now add the constraint, and store the identifier. This can be used with pe_update_constraint to remove it later on.
	GrabbableEntity->GetPhysicalEntity()->SetParams(&pp, 1);
	LeftHandInteraction::constraintid = GrabbableEntity->GetPhysicalEntity()->Action(&constraint);
}

EntityId LeftHandInteraction::getNearestObject(std::map<EntityId, IEntity*> nearhand, IEntity* hand) {
	std::map<EntityId, IEntity*>::iterator it;
	f32 distance = 100;
	EntityId closest = -1;
	Vec3 handLocation = hand->GetWorldPos();
	//VRGrabbable* component = NULL;
	//CryLog("size is: "+ToString(nearhand.size()));
	for (it = nearhand.begin(); it != nearhand.end(); it++) {
		std::string s;
		s = it->second->GetName();
		if (s == "RightHandPlacement") {
			continue;
		}
		Vec3 grabPoint = it->second->GetWorldPos();
		f32 newDistance = handLocation.GetSquaredDistance(grabPoint);
		if (newDistance < distance) {
			distance = newDistance;
			closest = it->first;
		}
	}
	//if (closest != -1)
	//	component = gEnv->pEntitySystem->GetEntity(closest)->GetComponent<VRGrabbable>();
	//if (component != NULL && RightHandInteraction::LastEntityx != EntityId(component->getParent()))
	//	RightHandInteraction::unHighlight(RightHandInteraction::LastEntityx);
	//if (component != NULL) {
	//	RightHandInteraction::currentChildX = component;
	//	IEntity* f = component->getParent(); //get child parent for highlighting
		//RightHandInteraction::highlight(component, EntityId(f));
		//CryLog("Nearest..... is " + ToString(closest) + "," + ToString(distance));
	//}
	return closest;
}
void LeftHandInteraction::ProcessEvent(const SEntityEvent& event) {
	if ((LeftHandInteraction::grabbed && VRMovementComponent::leftTrigger < .7) && (LeftHandInteraction::grabbed && VRMovementComponentOculus::leftTriggerOculus < .7)) {
		pe_action_update_constraint f;
		f.idConstraint = LeftHandInteraction::constraintid;
		f.bRemove = 1;
		pe_action_set_velocity setVel;
		pe_status_dynamics d;
		gEnv->pEntitySystem->FindEntityByName("LeftHand")->GetPhysicalEntity()->GetStatus(&d);
		setVel.v = d.v;
		setVel.w = VRLeftController::angVel;

		LeftHandInteraction::grabbed = false;
		IEntity* object = LeftHandInteraction::currentChild->GetOrCreateComponent<VRGrabbable>()->getParent();
		object->GetPhysicalEntity()->Action(&f);
		object->GetPhysicalEntity()->Action(&setVel);
		LeftHandInteraction::currentChild = NULL;
		LeftHandInteraction::setColliderStatus(false);

	}
	if (event.event == ENTITY_EVENT_ENTERAREA)
	{
		EntityId e = static_cast<EntityId>(event.nParam[0]);
		VRGrabbable* proxyObjComponentGrabability = gEnv->pEntitySystem->GetEntity(e)->GetOrCreateComponent<VRGrabbable>();
		if (proxyObjComponentGrabability->isComponentActivated()) {
			if (nearHand.size() < 15 && !grabbed) {
				nearHand[e] = gEnv->pEntitySystem->GetEntity(e);
			}
			if (RightHandInteraction::LastEntityx != e && LeftHandInteraction::LastEntity != e && e != VRMovementComponent::leftHand && e != VRMovementComponent::rightHand
				&& e != VRMovementComponentOculus::leftHand && e != VRMovementComponentOculus::rightHand && !LeftHandInteraction::grabbed) {//checks if the entity is unique

				IEntity* Handentity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
				EntityId nearestEntity = getNearestObject(nearHand, Handentity);
				VRGrabbable entityComponent = *gEnv->pEntitySystem->GetEntity(nearestEntity)->GetComponent<VRGrabbable>();
				IEntity* parent = entityComponent.getParent();
				if (LastEntity != -1) {
					VRGrabbable lastEntityComponent = *gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity)->GetComponent<VRGrabbable>();
					IEntity* lastParent = lastEntityComponent.getParent();

					if (LeftHandInteraction::pItemRenderNode && parent->GetId() != lastParent->GetId()) {
						LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
						CryLog("Regular unighlight1");
						if (RightHandInteraction::LastEntityx != -1 && parent->GetId() == gEnv->pEntitySystem->GetEntity(RightHandInteraction::LastEntityx)->GetComponent<VRGrabbable>()->getParent()->GetId() && 
							RightHandInteraction::pItemRenderNode) {
							RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 120.0f, 192.0f, 55.0f);
							CryLog("Equalizing1...");
						}


						LeftHandInteraction::pItemRenderNode = NULL;
					}
				}
				LeftHandInteraction::LastEntity = nearestEntity;
				// DEBUG: Highlight the entity.
				  // NOTE: There is no method to undo the highlight at present.
				if (LeftHandInteraction::pItemRenderNode = parent->GetRenderNode())
				{
					// NOTE: Colour order is actually ALPHA, BLUE, GREEN, RED.
					LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 56.0f, 192.0f, 255.0f);
				}
			}
		}
		///if trigger pushed. Make that entity a child
	}
	else if (event.event == ENTITY_EVENT_LEAVEAREA) {
		VRGrabbable* component = (gEnv->pEntitySystem->GetEntity(static_cast<EntityId>(event.nParam[0]))->GetOrCreateComponent<VRGrabbable>());
		IEntity* currentParentComponent = gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity);

		if (component->isComponentActivated() && currentParentComponent != NULL) {
			currentParentComponent = currentParentComponent->GetOrCreateComponent<VRGrabbable>()->getParent();//last entity parent
			EntityId erasedEntity = EntityId(component->getParent()); //erased entity parent
			nearHand.erase(static_cast<EntityId>(event.nParam[0]));
			IEntity* Handentity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
			if (currentParentComponent->GetId() != erasedEntity) {
				if (LeftHandInteraction::pItemRenderNode) {
					LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
					LeftHandInteraction::pItemRenderNode = NULL;
				}
			}
			LeftHandInteraction::LastEntity = getNearestObject(nearHand, Handentity);
		}
	}
	else if (event.event == ENTITY_EVENT_UPDATE) {
		if (VRMovementComponent::leftHand != -1 && VRMovementComponent::leftHand && (LeftHandInteraction::LeftHandEntity == NULL || LeftHandInteraction::LeftHandEntity)) {
			LeftHandInteraction::LeftHandEntity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
		}
		else if (VRMovementComponentOculus::leftHand != -1 && VRMovementComponentOculus::leftHand && (LeftHandInteraction::LeftHandEntity == NULL || LeftHandInteraction::LeftHandEntity)) {
			LeftHandInteraction::LeftHandEntity = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::leftHand);
		}
		//grab entity
		if ((VRMovementComponent::leftTrigger > .7 || VRMovementComponentOculus::leftTriggerOculus > .7) && LeftHandInteraction::LastEntity != -1 && LeftHandInteraction::currentChild == NULL && LeftHandInteraction::LeftHandEntity != NULL) {
			LeftHandInteraction::currentChild = gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity);
			VRGrabbable* proxyObjComponentGrabability = LeftHandInteraction::currentChild->GetOrCreateComponent<VRGrabbable>();
			if (proxyObjComponentGrabability->getParent()) {
				IEntity* parent = proxyObjComponentGrabability->getParent();
				IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("LeftHandPlacement");
				reference2->SetPos(proxyObjComponentGrabability->getEntityTransformOffset(false));
				reference2->SetRotation(Quat(Ang3(0, 0, 0)));
				if (VRMovementComponent::leftHand != -1) {
					IEntity* refreshedHand = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
					refreshedHand->SetRotation(Quat(Matrix33::CreateRotationXYZ(Ang3(0, 0, 0))));
					GrabObject(refreshedHand, parent, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHand->GetRotation()), true));
					setColliderStatus(proxyObjComponentGrabability->minimizeBox);// if true must minimize
				}
				else
				{
					IEntity* refreshedHandOculus = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::leftHand);
					refreshedHandOculus->SetRotation(Quat(Ang3(0, 0, 0)));
					GrabObject(refreshedHandOculus, parent, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHandOculus->GetRotation()), true));
					setColliderStatus(proxyObjComponentGrabability->minimizeBox);
				}
				LeftHandInteraction::grabbed = true;
				nearHand.clear();
			}
		}
	}
}

Cry::Entity::EventFlags LeftHandInteraction::GetEventMask() const {
	return (ENTITY_EVENT_ENTERAREA) | (ENTITY_EVENT_LEAVEAREA) | (ENTITY_EVENT_UPDATE);
}


