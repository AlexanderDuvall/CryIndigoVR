#include "StdAfx.h"
#include "VRMovementComponent.h"
#include "VRRightController.h"
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
#include "RightHandInteraction.h"
#include "PauseMenuController.h"
//PLACE IN CORRECT SPOT ... INITIALIZE?
float VRRightController::angx = 0;
float VRRightController::angy = 0;
float VRRightController::angz = 0;
Vec3 VRRightController::angVel = Vec3(0, 0, 0);
struct Point {
	double x;
	double y;
};
float VRMovementComponent::rightTrigger = 0.f;
EntityId VRMovementComponent::rightHand = -1;
void PhysicalizeRightController(IEntity& entity) {
	SEntityPhysicalizeParams physParams;
	// Set the physics type to PE_LIVING
	physParams.type = PE_LIVING;
	// Set the mass to 10 kilograms
	physParams.mass = 20;
	physParams.density = 1;
	// Living entities have to set the SEntityPhysicalizeParams::pPlayerDimensions field
	pe_player_dimensions playerDimensions;
	// Prefer usage of a cylinder instead of capsule
	playerDimensions.bUseCapsule = 1;
	// Specify the size of our cylinder0
	playerDimensions.sizeCollider = Vec3(0.001f, 0.001f, 0.001f);
	// Keep pivot at the player's feet (defined in player geometry) 
	playerDimensions.heightPivot = 0.f;
	// Offset collider upwards
	playerDimensions.heightCollider = 0.f;
	physParams.pPlayerDimensions = &playerDimensions;
	// Living entities have to set the SEntityPhysicalizeParams::pPlayerDynamics field
	pe_player_dynamics playerDynamics;
	// Mass needs to be repeated in the pe_player_dynamics structure
	playerDynamics.mass = physParams.mass;
	physParams.pPlayerDynamics = &playerDynamics;
	// Now physicalize the entity
	entity.Physicalize(physParams);
}
Ang3 VRRightController::getAngles() {
	return Ang3(VRRightController::angx, VRRightController::angy, VRRightController::angz);
}

void VRRightController::MoveLiving(IPhysicalEntity& physicalEntity, Matrix34 world)
{
	pe_action_move move;
	// Apply movement request directly to velocity
	move.iJump = 1;	// Apply the impulse one unit upwards
	// This can be combined with pe_action_impulse::point to apply the impulse on a specific point
	move.dir = Vec3(world.GetTranslation().x, world.GetTranslation().y, world.GetTranslation().z * 1.2f) * 20.0f;
	// Apply the impulse on the entity
	physicalEntity.Action(&move, 1);
}


static void RegisterVRRightControllerComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(VRRightController));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterVRRightControllerComponent)


void VRRightController::Initialize() {
	Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
	rightHand = rigid->GetEntity();
	VRMovementComponent::rightHand = rightHand->GetId();
	PhysicalizeRightController(*rightHand);
	mainCamera = gEnv->pSystem->GetViewCamera();

	m_pAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	m_pAnimationComponent->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/vrrighthand.adb");
	m_pAnimationComponent->SetCharacterFile("characters/Hands/right_glove/scifi_R_animated_.cdf");
	m_pAnimationComponent->SetControllerDefinitionFile("Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml");
	m_pAnimationComponent->SetDefaultScopeContextName("FirstPersonCharacter");
	// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	m_pAnimationComponent->SetAnimationDrivenMotion(true);
	// Load the character and Mannequin data from file
	m_pAnimationComponent->LoadFromDisk();
	// Acquire fragment and tag identifiers to avoid doing so each update
	idle = m_pAnimationComponent->GetFragmentId("Idle");
	point = m_pAnimationComponent->GetFragmentId("Point");
	pointStable = m_pAnimationComponent->GetFragmentId("PointStable");
	grabStable = m_pAnimationComponent->GetFragmentId("GrabStable");
	grab = m_pAnimationComponent->GetFragmentId("Grab");
	fistStable = m_pAnimationComponent->GetFragmentId("FistStable");
	fist = m_pAnimationComponent->GetFragmentId("Fist");

	fistAction = new TAction<SAnimationContext>(2, fist);
	fistStableAction = new TAction<SAnimationContext>(2, fistStable);

	pointAction = new TAction<SAnimationContext>(2, point);
	pointStableAction = new TAction<SAnimationContext>(2, pointStable);

	grabAction = new TAction<SAnimationContext>(2, grab);
	grabStableAction = new TAction<SAnimationContext>(2, grabStable);

	idleAction = new TAction<SAnimationContext>(2, idle);
	mAction = m_pAnimationComponent->GetActionController();
	//mAction->Queue(*idleAction);
	m_pAnimationComponent->ResetCharacter();
	m_pAnimationComponent->QueueFragmentWithId(idle);
	isIdle = true;
}
void VRRightController::ProcessEvent(const SEntityEvent& event) {
	bool animEvent = event.event == Cry::Entity::EEvent::AnimationEvent;
	bool holdend = false;
	bool generalEnd = false;
	bool RightHoldFistEnd = false;
	bool RightGeneralFistEnd = false;
	bool RightGeneralPointEnd = false;
	bool RightHoldPointEnd = false;
	bool RightHoldGrabEnd = false;
	bool RightGeneralGrabEnd = false;

	if (animEvent) {
		const AnimEventInstance* anEvent = reinterpret_cast <const AnimEventInstance*> (event.nParam[0]);
		string const param = static_cast<string>(anEvent->m_CustomParameter);
		RightHoldFistEnd = param == "RightHoldFistEnd";
		RightGeneralFistEnd = param == "RightGeneralFistEnd";
		RightGeneralPointEnd = param == "RightGeneralPointEnd";
		RightHoldPointEnd = param == "RightHoldPointEnd";
		RightHoldGrabEnd = param == "RightHoldGrabEnd";
		RightGeneralGrabEnd = param == "RightGeneralGrabEnd";
		if (param == "RightHoldEnd") {
			holdend = true;
			CryLog((param));
		}
		else if (param == "RightGeneralEnd") {
			generalEnd = true;
			CryLog((param));
		}
		CryLog((param));

	}
	VRRightController::angx = xRot;
	VRRightController::angy = yRot;
	VRRightController::angz = zRot;
	if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
	{
		if (IHmdDevice* pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
		{
			if (pDevice->GetClass() == EHmdClass::eHmdClass_OpenVR) // Check, if the connected device is an Oculus device)
			{
				const IHmdController* pController = pDevice->GetController();
				// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
				if (pController->IsConnected(eHmdController_OpenVR_2))
				{
					if (RightHandInteraction::grabbedx) {
						isFist = false;
						isPointing = false;
						if ((!RightGeneralGrabEnd && RightHoldGrabEnd) || (RightGeneralGrabEnd && !stable) ) {
							m_pAnimationComponent->QueueCustomFragment(*grabStableAction);
							isIdle = false;
							stable = true;
							isGrabbed = true;
						}
						else if (!isGrabbed) {
							//mAction->Queue(*grabAction);
							//m_pAnimationComponent->ResetCharacter();
							m_pAnimationComponent->QueueCustomFragment(*grabAction);
							stable = false;
							isIdle = false;
							isGrabbed = true;
						}
					}
					else if ((bGripPressed && trigger && !isFist) || (isFist && (RightHoldFistEnd || RightGeneralFistEnd)) && !PauseMenuController::paused) {
						isFist = true;
						isPointing = false;
						if ((!RightGeneralFistEnd && RightHoldFistEnd) || (RightGeneralFistEnd && !stable)) {
							m_pAnimationComponent->QueueCustomFragment(*fistStableAction);
							isIdle = false;
							stable = true;
						}
						else {
							//mAction->Queue(*fistAction);
							//m_pAnimationComponent->ResetCharacter();
							m_pAnimationComponent->QueueCustomFragment(*fistAction);
							stable = false;
							isIdle = false;
						}
					}
					else if ((bGripPressed && !trigger && !isPointing) || (isPointing && (RightHoldPointEnd || RightGeneralPointEnd))) {
						isFist = false;
						isPointing = true;
						if ((!RightGeneralPointEnd && RightHoldPointEnd) || (RightGeneralPointEnd && !stable) || PauseMenuController::paused) {
							//mAction->Queue(*pointStableAction);
							m_pAnimationComponent->QueueCustomFragment(*pointStableAction);
							isIdle = false;
							stable = true;
						}
						else {
							//mAction->Queue(*pointAction);
							//m_pAnimationComponent->ResetCharacter();
							m_pAnimationComponent->QueueCustomFragment(*pointAction);
							isIdle = false;
							stable = false;
						}
					}
					else if (!bGripPressed && (isPointing || isFist || stable || isGrabbed || holdend || generalEnd) && !isIdle) {
						isPointing = false;
						stable = false;
						isFist = false;
						isGrabbed = false;
						//mAction->Queue(*idleAction);
						//reset hand
						//m_pAnimationComponent->ResetCharacter();
						m_pAnimationComponent->QueueFragmentWithId(idle);
						//m_pAnimationComponent->QueueCustomFragment(*idleAction);
						isIdle = true;
					}

					IEntity* body = gEnv->pEntitySystem->FindEntityByName("HMD Cam");
					// Get the current tracking state
					HmdTrackingState cState = pController->GetLocalTrackingState(eHmdController_OpenVR_2);
					// either use orientation and position directly, or build a TransformationMatrix from them
					//col2 = controller rotation
					//col3 = controller position
					Vec3 a = cState.pose.position;
					Ang3 CameraAngles = mainCamera.GetAngles();
					float y = a.y;
					float x = a.x;
					float Camangle = float(CameraAngles.x * 57.324 + 90);
					float controllerAngle = float(VRtoPolar(x, y) * (180 / 3.14));
					float camOffset = Camangle;
					float camTheta = Camangle - camOffset;
					float controllerTheta = (controllerAngle + camTheta);
					Point newControllerPolar = VRtoRect(controllerTheta);
					float resultantVector = float(std::abs(sqrt(pow(x, 2) + pow(y, 2))));
					angVel = cState.pose.angularVelocity;

					Vec3 calcHandLocale = Vec3(resultantVector * float(newControllerPolar.x) + xtr, float(resultantVector * newControllerPolar.y) + ytr, cState.pose.position.z + ztr);
					Quat quat = cState.pose.orientation;
					Ang3 newcol = Ang3::GetAnglesXYZ(quat);
					newcol.RangePI();
					newcol += Ang3(xRot, yRot, zRot);
					newcol = Ang3(newcol.x, newcol.y, newcol.z);
					Quat xnxx = Quat(newcol);
					//x - yaw
					//y - pitch
					//z - roll

					Matrix34 newLocale = Matrix34::Create(Vec3(1, 1, 1), xnxx, calcHandLocale); //hand locale
					// You could not apply the values to an entity. For example, to position the entity 'pEntity' relative to the entity 'pReference':
					// reference from player body to get correct location.
					Matrix34 m = body->GetWorldTM() * newLocale; //world locale
					Matrix34 impulse = m - rightHand->GetWorldTM();
					MoveLiving(*rightHand->GetPhysicalEntity(), impulse);
					rightHand->SetRotation(Quat(m));
					VRMovementComponent::rightTrigger = pController->GetTriggerValue(eHmdController_OpenVR_2, eKI_Motion_OpenVR_Trigger);
					bGripPressed = pController->IsButtonPressed(eHmdController_OpenVR_2, eKI_Motion_OpenVR_Grip);
					trigger = pController->GetTriggerValue(eHmdController_OpenVR_2, eKI_Motion_OpenVR_Trigger) >= .7 ? true : false;

				}
			}
		}
	}
}

Cry::Entity::EventFlags VRRightController::GetEventMask() const {
	return (Cry::Entity::EEvent::Update | Cry::Entity::EEvent::AnimationEvent);
}



