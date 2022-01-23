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
//PLACE IN CORRECT SPOT ... INITIALIZE?/
struct Point {
	double x;
	double y;
};
bool RightHandInteraction::grabbedx = false;
IEntity* RightHandInteraction::currentChildx = NULL;
IEntity* RightHandInteraction::RightHandEntityx = NULL;
IRenderNode* RightHandInteraction::pItemRenderNode;
EntityId RightHandInteraction::LastEntityx = -1;
int RightHandInteraction::constraintid = 0;

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
	// Create an axis aligned bounding boxVec3 Transform = Vec3(xTrans, yTrans, zTrans);, ensuring that we listen to events around the entity translation
	const AABB triggerBounds = AABB(triggerBoxSize * .5f, triggerBoxSize * .5f);
	// Now set the trigger bounds on the trigger component
	pTriggerComponent->SetTriggerBounds(triggerBounds);
}

void GrabRightObject(IEntity* righthand, IEntity* GrabbableEntity, Quat angleOffset) {

	IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("RightHandPlacement");
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

	//constraint.qframe[0] = constraint.qframe[1] = angleOffset;
	//constraint.qframe[0] = Quat(Ang3(180, 0, 0));
	// Set the rotational frame for the constraint, based on the specified axis
	//constraint.qframe[0] = constraint.qframe[1] = Quat::CreateRotationV0V1(Vec3(0, 0, 0), constraintAxis);
	// Now apply the constraint limits defined above
	// Note that if the max<=min (or the limits are left unused), the constraint is treated as a fully free constraint where only position is enforced.
	// To create a fully locked constraint, see the constraint_no_rotation flag.
	constraint.xlimits[0] = minRotationForAxis;
	constraint.xlimits[1] = maxRotationForAxis;
	constraint.yzlimits[0] = minRotationOtherAxis;
	constraint.yzlimits[1] = maxRotationOtherAxis;
	// Constrain the entity around the world, alternatively we can apply another IPhysicalEntity pointer
	constraint.pBuddy = righthand->GetPhysicalEntity();
	// Now add the constraint, and store the identifier. This can be used with pe_update_constraint to remove it later on.
	GrabbableEntity->GetPhysicalEntity()->SetParams(&pp);
	RightHandInteraction::constraintid = GrabbableEntity->GetPhysicalEntity()->Action(&constraint);
}

void RightHandInteraction::ProcessEvent(const SEntityEvent& event) {

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
		RightHandInteraction::currentChildx->GetPhysicalEntity()->Action(&f);
		RightHandInteraction::currentChildx->GetPhysicalEntity()->Action(&setVel);
		RightHandInteraction::currentChildx = NULL;
	}
	if (event.event == ENTITY_EVENT_ENTERAREA)
	{
		EntityId e = static_cast<EntityId>(event.nParam[0]);
		VRGrabbable* proxyObjComponentGrabability = gEnv->pEntitySystem->GetEntity(e)->GetComponent<VRGrabbable>();
		if (LeftHandInteraction::LastEntity != e && RightHandInteraction::LastEntityx != e && e != VRMovementComponent::leftHand
			&& e != VRMovementComponent::rightHand && e != 27 && !RightHandInteraction::grabbedx && (proxyObjComponentGrabability && proxyObjComponentGrabability != NULL)) {
			RightHandInteraction::LastEntityx = e;
			if (RightHandInteraction::pItemRenderNode) {
				RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
				RightHandInteraction::pItemRenderNode = NULL;
			}
			// DEBUG: Highlight the entity.
			  // NOTE: There is no method to undo the highlight at present.
			if (RightHandInteraction::pItemRenderNode = gEnv->pEntitySystem->GetEntity(RightHandInteraction::LastEntityx)->GetRenderNode())
			{
				// NOTE: Colour order is actually ALPHA, BLUE, GREEN, RED.
				RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(.53f, 120.0f, 192.0f, 55.0f);

			}
		}
		///if trigger pushed. Make that entity a child
	}
	else if (event.event == ENTITY_EVENT_LEAVEAREA) {
		if (RightHandInteraction::LastEntityx == static_cast<EntityId>(event.nParam[0])) {
			if (RightHandInteraction::pItemRenderNode) {
				RightHandInteraction::pItemRenderNode->m_nHUDSilhouettesParam = RGBA8(0.0f, 0.0f, 0.0f, 0.0f);
				RightHandInteraction::pItemRenderNode = NULL;

			}
			RightHandInteraction::LastEntityx = -1;
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
			VRGrabbable* proxyObjComponentGrabability = RightHandInteraction::currentChildx->GetComponent<VRGrabbable>();
			IEntity* reference2 = gEnv->pEntitySystem->FindEntityByName("RightHandPlacement");
			reference2->SetPos(proxyObjComponentGrabability->getEntityTransformOffset(true));
			if (VRMovementComponent::leftHand != -1) {
				IEntity* refreshedHand = gEnv->pEntitySystem->GetEntity(VRMovementComponent::rightHand);
				refreshedHand->SetRotation(Quat(Ang3(0, 0, 0)));
				GrabRightObject(refreshedHand, currentChildx, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHand->GetRotation()),false));
			}
			else {
				IEntity* refreshedHandOculus = gEnv->pEntitySystem->GetEntity(VRMovementComponentOculus::rightHand);
				refreshedHandOculus->SetRotation(Quat(Ang3(0, 0, 0)));
				GrabRightObject(refreshedHandOculus, currentChildx, proxyObjComponentGrabability->getEntityAngleOffset(Ang3(refreshedHandOculus->GetRotation()),false));
			}
			RightHandInteraction::grabbedx = true;
		}
	}
}

Cry::Entity::EventFlags RightHandInteraction::GetEventMask() const {
	return (ENTITY_EVENT_ENTERAREA) | (ENTITY_EVENT_LEAVEAREA) | (ENTITY_EVENT_UPDATE);
}