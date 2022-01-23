#include "StdAfx.h"
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
#include <CryMath/Cry_Matrix34.h>
#include <CryPhysics/physinterface.h>
#include <CryPhysics/RayCastQueue.h>
#include <vector>
#include "RightHandInteraction.h"
#include "VRRightController.h"
#include "VRRightControllerOculus.h"
#include "VRMovementComponent.h"
#include "VRMovementComponentOculus.h"
#include "VRMath.h"
#include "LeftHandInteraction.h"
#include "VRGrabbable.h"
#include "DefaultComponents/Physics/BoxPrimitiveComponent.h"
//PLACE IN CORRECT SPOT ... INITIALIZE?/
struct Point {
	double x;
	double y;
};
int RightHandInteraction::counter = 0;
bool RightHandInteraction::grabbedx = false;
IEntity* RightHandInteraction::currentChildx = NULL;
IEntity* RightHandInteraction::RightHandEntityx = NULL;
IRenderNode* RightHandInteraction::pItemRenderNode;
EntityId RightHandInteraction::LastEntityx = -1;
int RightHandInteraction::constraintid = 0;
bool RightHandInteraction::isMinimized = false;

bool RightHandInteraction::getColliderStatus() {
	return isMinimized;
}
bool RightHandInteraction::setColliderStatus(bool a) {
	if (!a) {//if not minimized
		auto box = RightHandEntityx->GetComponent<Cry::DefaultComponents::CBoxPrimitiveComponent>();
		box->m_size = VRMovementComponent::largeCollider; //
		box->CreateGeometry();
		isMinimized = false;
	}
	else { //if minimized
		auto box = RightHandEntityx->GetComponent<Cry::DefaultComponents::CBoxPrimitiveComponent>();
		box->m_size = VRMovementComponent::smallCollider; //
		box->CreateGeometry();
		isMinimized = true;
	}
	return isMinimized;
}
static void RegisterRightHandInteractionComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(RightHandInteraction));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterRightHandInteractionComponent)

void RightHandInteraction::Initialize() {
	// Create a new IEntityTriggerComponent instance, responsible for registering our entity in the proximity grid
	IEntityTriggerComponent* pTriggerComponent = m_pEntity->CreateComponent<IEntityTriggerComponent>();
	// Listen to area events in a 2m^3 box around the entity
	const Vec3 triggerBoxSize = Vec3(.07, .07, .07);
	// Create an axis aligned bounding boxVec3 Transform = Vec3(LeftXTrans, LeftYTrans, LeftZTrans);, ensuring that we listen to events around the entity translation
	const AABB triggerBounds = AABB(triggerBoxSize * .5f, triggerBoxSize * .5f);
	// Now set the trigger bounds on the trigger component
	pTriggerComponent->SetTriggerBounds(triggerBounds);
}

void GrabRightObject(IEntity* righthand, IEntity* GrabbableEntity, Quat angleOffset, VRGrabbable* vrg) {

	//GrabbableEntity->SetRotation(angleOffset);
	//GrabbableEntity->InvalidateTM();
	IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("RightHandPlacement");
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
	constraint.pBuddy = righthand->GetPhysicalEntity();
	// Now add the constraint, and store the identifier. This can be used with pe_update_constraint to remove it later on.
	GrabbableEntity->GetPhysicalEntity()->SetParams(&pp, 1);
	RightHandInteraction::constraintid = GrabbableEntity->GetPhysicalEntity()->Action(&constraint);

}



EntityId RightHandInteraction::getNearestObject(std::map<EntityId, IEntity*> nearhand, IEntity* hand) {
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
		//CryLog(ToString(newDistance) + ":" + it->second->GetName());
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

void RightHandInteraction::ProcessEvent(const SEntityEvent& event) {
	//letgo
	if ((RightHandInteraction::grabbedx && VRMovementComponent::rightTrigger < .7) && (RightHandInteraction::grabbedx && VRMovementComponentOculus::rightTriggerOculus < .7)) {
		pe_action_update_constraint f;
		f.idConstraint = RightHandInteraction::constraintid;
		f.bRemove = 1;
		pe_action_set_velocity setVel;
		pe_status_dynamics d;
		gEnv->pEntitySystem->FindEntityByName("RightHand")->GetPhysicalEntity()->GetStatus(&d);
		setVel.v = d.v;
		setVel.w = VRRightController::angVel;

		RightHandInteraction::grabbedx = false;
		IEntity* object = RightHandInteraction::currentChildx->GetOrCreateComponent<VRGrabbable>()->getParent();
		object->GetPhysicalEntity()->Action(&f);
		object->GetPhysicalEntity()->Action(&setVel);
		RightHandInteraction::currentChildx = NULL;
		RightHandInteraction::setColliderStatus(false);
	}
	if (event.event == ENTITY_EVENT_ENTERAREA)
	{
		EntityId e = static_cast<EntityId>(event.nParam[0]);
		VRGrabbable* proxyObjComponentGrabability = gEnv->pEntitySystem->GetEntity(e)->GetOrCreateComponent<VRGrabbable>();
		if (proxyObjComponentGrabability->isComponentActivated()) {
			if (nearHand.size() < 15 && !grabbedx) {
				nearHand[e] = gEnv->pEntitySystem->GetEntity(e);
			}
			if (LeftHandInteraction::LastEntity != e && RightHandInteraction::LastEntityx != e && e != VRMovementComponent::leftHand && !RightHandInteraction::grabbedx) { ///checks if the entity is unique

				IEntity* Handentity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::rightHand);
				EntityId nearestEntity = getNearestObject(nearHand, Handentity);
				VRGrabbable entityComponent = *gEnv->pEntitySystem->GetEntity(nearestEntity)->GetComponent<VRGrabbable>();
				IEntity* parent = entityComponent.getParent();
				if (LastEntityx != -1) {
					VRGrabbable lastEntityComponent = *gEnv->pEntitySystem->GetEntity(RightHandInteraction::LastEntityx)->GetComponent<VRGrabbable>();
					IEntity* lastParent = lastEntityComponent.getParent();

					if (RightHandInteraction::pItemRenderNode && parent->GetId() != lastParent->GetId()) {
						RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
						if (LeftHandInteraction::LastEntity != -1 && parent->GetId() == gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity)->GetComponent<VRGrabbable>()->getParent()->GetId() &&
							LeftHandInteraction::pItemRenderNode) {
							LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 56.0f, 192.0f, 255.0f);
							CryLog("Equalizing2...");
						}
						RightHandInteraction::pItemRenderNode = NULL;
					}
				}
				RightHandInteraction::LastEntityx = nearestEntity;
				// DEBUG: Highlight the entity.
				  // NOTE: There is no method to undo the highlight at present.
				RightHandInteraction::pItemRenderNode = parent->GetRenderNode();
				// NOTE: Colour order is actually ALPHA, BLUE, GREEN, RED.
				RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 120.0f, 192.0f, 55.0f);
			}
		}
		///if trigger pushed. Make that entity a child
	}
	else if (event.event == ENTITY_EVENT_LEAVEAREA) {
		VRGrabbable* component = (gEnv->pEntitySystem->GetEntity(static_cast<EntityId>(event.nParam[0]))->GetOrCreateComponent<VRGrabbable>()); // component that left.
		IEntity* currentParentComponent = gEnv->pEntitySystem->GetEntity(RightHandInteraction::LastEntityx); //current closest entity.

		if (component->isComponentActivated() && currentParentComponent != NULL) {
			currentParentComponent = currentParentComponent->GetOrCreateComponent<VRGrabbable>()->getParent(); //last entity parent
			EntityId erasedEntity = component->getParent()->GetId(); //erased entity parent
			nearHand.erase(static_cast<EntityId>(event.nParam[0]));
			IEntity* Handentity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::rightHand);
			if (currentParentComponent->GetId() != erasedEntity) {
				if (RightHandInteraction::pItemRenderNode) {
					RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
					RightHandInteraction::pItemRenderNode = NULL;
				}
			}
			RightHandInteraction::LastEntityx = getNearestObject(nearHand, Handentity);
		}
	}
	else if (event.event == ENTITY_EVENT_UPDATE) {
		if (VRMovementComponent::rightHand != -1 && VRMovementComponent::rightHand && RightHandInteraction::RightHandEntityx == NULL || RightHandInteraction::RightHandEntityx) {
			RightHandInteraction::RightHandEntityx = gEnv->pEntitySystem->GetEntity(VRMovementComponent::rightHand);
		}
		else if (VRMovementComponentOculus::rightHand != -1 && VRMovementComponentOculus::rightHand && RightHandInteraction::RightHandEntityx == NULL) {
			RightHandInteraction::RightHandEntityx = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::rightHand);
		}
		//grab entity
		if ((VRMovementComponent::rightTrigger > .7 || VRMovementComponentOculus::rightTriggerOculus > .7) && RightHandInteraction::LastEntityx != -1 && RightHandInteraction::currentChildx == NULL && RightHandInteraction::RightHandEntityx != NULL) {
			RightHandInteraction::currentChildx = gEnv->pEntitySystem->GetEntity(RightHandInteraction::LastEntityx);
			VRGrabbable* proxyObjComponentGrabability = RightHandInteraction::currentChildx->GetOrCreateComponent<VRGrabbable>();
			if (proxyObjComponentGrabability->getParent()) {
				IEntity* parent = proxyObjComponentGrabability->getParent();
				IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("RightHandPlacement");
				reference2->SetPos(proxyObjComponentGrabability->getEntityTransformOffset(true));
				reference2->SetRotation(Quat(Ang3(0, 0, 0)));
				if (VRMovementComponent::leftHand != -1) {
					IEntity* refreshedHand = gEnv->pEntitySystem->GetEntity(VRMovementComponent::rightHand);
					refreshedHand->SetRotation(Quat(Matrix33::CreateRotationXYZ(Ang3(0, 0, 0))));
					setColliderStatus(proxyObjComponentGrabability->minimizeBox);
					GrabRightObject(refreshedHand, parent, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHand->GetRotation()), false), proxyObjComponentGrabability);
				}
				else {
					IEntity* refreshedHandOculus = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::rightHand);
					refreshedHandOculus->SetRotation(Quat(Ang3(0, 0, 0)));
					setColliderStatus(proxyObjComponentGrabability->minimizeBox);
					GrabRightObject(refreshedHandOculus, parent, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHandOculus->GetRotation()), false), proxyObjComponentGrabability);
				}
				RightHandInteraction::grabbedx = true;
				nearHand.clear();
			}
		}
	}
}

Cry::Entity::EventFlags RightHandInteraction::GetEventMask() const {
	return (ENTITY_EVENT_ENTERAREA) | (ENTITY_EVENT_LEAVEAREA) | (ENTITY_EVENT_UPDATE);
}