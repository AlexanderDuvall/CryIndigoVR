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
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
//PLACE IN CORRECT SPOT ... INITIALIZE?
struct Point {
	double x;
	double y;
};
bool LeftHandInteraction::grabbed = false;
IEntity* LeftHandInteraction::currentChild = NULL;
IEntity* LeftHandInteraction::LeftHandEntity = NULL;
IRenderNode* LeftHandInteraction::pItemRenderNode = NULL;
EntityId LeftHandInteraction::LastEntity = -1;
int LeftHandInteraction::constraintid = 0;
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

void LeftHandInteraction::Initialize() {
	// Create a new IEntityTriggerComponent instance, responsible for registering our entity in the proximity grid
	IEntityTriggerComponent* pTriggerComponent = m_pEntity->CreateComponent<IEntityTriggerComponent>();
	// Listen to area events in a 2m^3 box around the entity
	const Vec3 triggerBoxSize = Vec3(.07, .07, .07);
	// Create an axis aligned bounding boxVec3 Transform = Vec3(xTrans, yTrans, zTrans);, ensuring that we listen to events around the entity translation
	const AABB triggerBounds = AABB(triggerBoxSize * .5f, triggerBoxSize * .5f);
	// Now set the trigger bounds on the trigger component
	pTriggerComponent->SetTriggerBounds(triggerBounds);

}
/*
Grab an object by repositioning it through a physical constraint. Quat changes too.
*/
void GrabObject(IEntity* lefthand, IEntity* GrabbableEntity, Quat angleOffset) {
	IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("LeftHandPlacement");
	pe_params_pos pp;
	pp.pos = reference2->GetWorldPos();
	pp.q = angleOffset;
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
	constraint.xlimits[0] = minRotationForAxis;
	constraint.xlimits[1] = maxRotationForAxis;
	constraint.yzlimits[0] = minRotationOtherAxis;
	constraint.yzlimits[1] = maxRotationOtherAxis;
	// Constrain the entity around the world, alternatively we can apply another IPhysicalEntity pointer
	constraint.pBuddy = lefthand->GetPhysicalEntity();
	// Now add the constraint, and store the identifier. This can be used with pe_update_constraint to remove it later on.
	GrabbableEntity->GetPhysicalEntity()->SetParams(&pp);
	LeftHandInteraction::constraintid = GrabbableEntity->GetPhysicalEntity()->Action(&constraint); // save constraint id for destruction later.
}

void LeftHandInteraction::ProcessEvent(const SEntityEvent& event) {
	//let go of object here
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
		LeftHandInteraction::currentChild->GetPhysicalEntity()->Action(&f);
		LeftHandInteraction::currentChild->GetPhysicalEntity()->Action(&setVel);
		LeftHandInteraction::currentChild = NULL;
	}
	//activates if entity is inside invisible area (proxy trigger)
	if (event.event == ENTITY_EVENT_ENTERAREA)
	{
		EntityId e = static_cast<EntityId>(event.nParam[0]);
		VRGrabbable* proxyObjComponentGrabability = gEnv->pEntitySystem->GetEntity(e)->GetComponent<VRGrabbable>();
		//HighLights Entity of LATEST object in the proxy box
		if (RightHandInteraction::LastEntityx != e && LeftHandInteraction::LastEntity != e && e != VRMovementComponent::leftHand && e != VRMovementComponent::rightHand
			&& e != VRMovementComponentOculus::leftHand && e != VRMovementComponentOculus::rightHand && !LeftHandInteraction::grabbed && (proxyObjComponentGrabability && proxyObjComponentGrabability != NULL)) {
			if (LeftHandInteraction::pItemRenderNode) {
				LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
				LeftHandInteraction::pItemRenderNode = NULL;
			}
			LeftHandInteraction::LastEntity = e;
			// DEBUG: Highlight the entity.
			  // NOTE: There is no method to undo the highlight at present.
			if (LeftHandInteraction::pItemRenderNode = gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity)->GetRenderNode())
			{
				// NOTE: Colour order is actually ALPHA, BLUE, GREEN, RED.
				LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 56.0f, 192.0f, 255.0f);
			}
		}
		///if trigger pushed. Make that entity a child
	}
	//turns off Silhouettes of highlighted object when leaving proxy area
	else if (event.event == ENTITY_EVENT_LEAVEAREA) {
		if (LeftHandInteraction::LastEntity == static_cast<EntityId>(event.nParam[0])) {
			if (LeftHandInteraction::pItemRenderNode) {
				LeftHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
				LeftHandInteraction::pItemRenderNode = NULL;
			}
			LeftHandInteraction::LastEntity = -1;
		}
	}
	else if (event.event == ENTITY_EVENT_UPDATE) {
		//set left hand entity of either oculus or openvr device
		if (VRMovementComponent::leftHand != -1 && VRMovementComponent::leftHand && (LeftHandInteraction::LeftHandEntity == NULL || LeftHandInteraction::LeftHandEntity)) {
			LeftHandInteraction::LeftHandEntity = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
		}
		else if (VRMovementComponentOculus::leftHand != -1 && VRMovementComponentOculus::leftHand && (LeftHandInteraction::LeftHandEntity == NULL || LeftHandInteraction::LeftHandEntity)) {
			LeftHandInteraction::LeftHandEntity = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::leftHand);
		}
		//grab entity here
		if ((VRMovementComponent::leftTrigger > .7 || VRMovementComponentOculus::leftTriggerOculus > .7) && LeftHandInteraction::LastEntity != -1 && LeftHandInteraction::currentChild == NULL && LeftHandInteraction::LeftHandEntity != NULL) {
			LeftHandInteraction::currentChild = gEnv->pEntitySystem->GetEntity(LeftHandInteraction::LastEntity);
			VRGrabbable* proxyObjComponentGrabability = LeftHandInteraction::currentChild->GetComponent<VRGrabbable>();
			IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("LeftHandPlacement"); // REPOSITION REFERENCE 2 PLACEMENT ... Will be deprecated.
			reference2->SetPos(proxyObjComponentGrabability->getEntityTransformOffset(false));
			//grabs selected entity for respective oculus or openvr hand
			if (VRMovementComponent::leftHand != -1) {
				IEntity* refreshedHand = gEnv->pEntitySystem->GetEntity(VRMovementComponent::leftHand);
				refreshedHand->SetRotation(Quat(Ang3(0, 0, 0)));
				GrabObject(refreshedHand, currentChild, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHand->GetRotation()), true));
			}
			else
			{
				IEntity* refreshedHandOculus = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::leftHand);
				refreshedHandOculus->SetRotation(Quat(Ang3(0, 0, 0)));
				GrabObject(refreshedHandOculus, currentChild, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHandOculus->GetRotation()), true));
			}
			//self explanatory...
			LeftHandInteraction::grabbed = true;
		}

	}
}

Cry::Entity::EventFlags LeftHandInteraction::GetEventMask() const {
	return (ENTITY_EVENT_ENTERAREA) | (ENTITY_EVENT_LEAVEAREA) | (ENTITY_EVENT_UPDATE);
}


