#include "StdAfx.h"
#include "VRRightControllerOculus.h"
#include "VRMovementComponentOculus.h"
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
#include <DefaultComponents/Geometry/AdvancedAnimationComponent.h>
#include "RightHandInteraction.h"
//PLACE IN CORRECT SPOT ... INITIALIZE?
float VRRightControllerOculus::angx = 0;
float VRRightControllerOculus::angy = 0;
float VRRightControllerOculus::angz = 0;
Vec3 VRRightControllerOculus::angVel = Vec3(0, 0, 0);

struct Point {
	double x;
	double y;
};
EntityId VRMovementComponentOculus::rightHand = -1;
float VRRightControllerOculus::fTrigger = 0.0;
void PhysicalizeRightControllerOculus(IEntity& entity) {
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

Ang3 VRRightControllerOculus::getAngles() {
	return Ang3(VRRightControllerOculus::angx, VRRightControllerOculus::angy, VRRightControllerOculus::angz);
}
static void RegisterVRRightControllerOculusComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(VRRightControllerOculus));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterVRRightControllerOculusComponent)
void beamRightOculus(IEntity* body, IEntity* hand) {
	Matrix34 handlocale = body->GetWorldTM();
	handlocale.AddTranslation(Vec3(.3, .3, 0));

	hand->SetWorldTM(handlocale);
 }

void VRRightControllerOculus::Initialize() {
	Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
	rightHand = rigid->GetEntity();
	VRMovementComponentOculus::rightHand = rightHand->GetId();
	PhysicalizeRightControllerOculus(*rightHand);
	mainCamera = gEnv->pSystem->GetViewCamera();

	//m_pAnimationComponent = m_pEntity->GetOrCreateComponent<Cry::DefaultComponents::CAdvancedAnimationComponent>();
	//CryLog("Initializing Right Hand");
	//m_pAnimationComponent->SetMannequinAnimationDatabaseFile("Animations/Mannequin/ADB/vrrighthand.adb");
	//m_pAnimationComponent->SetCharacterFile("characters/Hands/right_glove/scifi_R_animated_.cdf");
	//m_pAnimationComponent->SetControllerDefinitionFile("Animations/Mannequin/ADB/FirstPersonControllerDefinition.xml");
	//m_pAnimationComponent->SetDefaultScopeContextName("FirstPersonCharacter");
	//// Disable movement coming from the animation (root joint offset), we control this entirely via physics
	//m_pAnimationComponent->SetAnimationDrivenMotion(true);
	//// Load the character and Mannequin data from file
	//m_pAnimationComponent->LoadFromDisk();
	//// Acquire fragment and tag identifiers to avoid doing so each update
	//idle = m_pAnimationComponent->GetFragmentId("Idle");
	//point = m_pAnimationComponent->GetFragmentId("Point");
	//pointStable = m_pAnimationComponent->GetFragmentId("PointStable");
	//grabStable = m_pAnimationComponent->GetFragmentId("GrabStable");
	//grab = m_pAnimationComponent->GetFragmentId("Grab");
	//fistStable = m_pAnimationComponent->GetFragmentId("FistStable");
	//fist = m_pAnimationComponent->GetFragmentId("Fist");
	//
	//fistAction = new TAction<SAnimationContext>(2, fist);
	//fistStableAction = new TAction<SAnimationContext>(2, fistStable);
	//
	//pointAction = new TAction<SAnimationContext>(2, point);
	//pointStableAction = new TAction<SAnimationContext>(2, pointStable);
	//
	//grabAction = new TAction<SAnimationContext>(2, grab);
	//grabStableAction = new TAction<SAnimationContext>(2, grabStable);
	//
	//idleAction = new TAction<SAnimationContext>(2, idle);
	//mAction = m_pAnimationComponent->GetActionController();
	////mAction->Queue(*idleAction);
	//m_pAnimationComponent->ResetCharacter();
	//m_pAnimationComponent->QueueFragmentWithId(idle);
	isIdle = true;
}
void VRRightControllerOculus::MoveLiving(IPhysicalEntity& physicalEntity, Matrix34 world)
{
	pe_action_move move;
	// Apply movement request directly to velocity
	move.iJump = 1;	// Apply the impulse one unit upwards
	// This can be combined with pe_action_impulse::point to apply the impulse on a specific point
	move.dir = Vec3(world.GetTranslation().x, world.GetTranslation().y, world.GetTranslation().z * 1.2f) * 20.0f;
	// Apply the impulse on the entity
	physicalEntity.Action(&move, 1);
}
void VRRightControllerOculus::ProcessEvent(const SEntityEvent& event) {
	bool animEvent = event.event == Cry::Entity::EEvent::AnimationEvent;
	bool holdend = false;
	bool generalEnd = false;
	if (animEvent) {
		const AnimEventInstance* anEvent = reinterpret_cast <const AnimEventInstance*> (event.nParam[0]);
		string const param = static_cast<string>(anEvent->m_CustomParameter);
		if (param == "RightHoldEnd") {
			CryLog(param);
			holdend = true;
		}
		else if (param == "RightGeneralEnd") {
			generalEnd = true;
		}
	}
	VRRightControllerOculus::angx = xRot;
	VRRightControllerOculus::angy = yRot;
	VRRightControllerOculus::angz = zRot;
	if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
	{
		if (IHmdDevice* pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
		{
			if (pDevice->GetClass() == EHmdClass::eHmdClass_Oculus) // Check, if the connected device is an Oculus device)
			{
				const IHmdController* pController = pDevice->GetController();
				// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
				if (pController->IsConnected(eHmdController_OculusRightHand))
				{
					//if (RightHandInteraction::grabbedx) {
					//	isFist = false;
					//	isPointing = false;
					//	if ((!generalEnd && holdend) || (generalEnd && !stable) && isGrabbed) {
					//		m_pAnimationComponent->QueueCustomFragment(*grabStableAction);
					//		isIdle = false;
					//		stable = true;
					//	}
					//	else if (!isGrabbed) {
					//		//mAction->Queue(*grabAction);
					//		//m_pAnimationComponent->ResetCharacter();
					//		m_pAnimationComponent->QueueCustomFragment(*grabAction);
					//		stable = false;
					//		isIdle = false;
					//		isGrabbed = true;
					//	}
					//}
					//else if ((bGripPressed && trigger && !isFist) || (isFist && animEvent)) {
					//	isFist = true;
					//	isPointing = false;
					//	if ((!generalEnd && holdend) || (generalEnd && !stable)) {
					//		m_pAnimationComponent->QueueCustomFragment(*fistStableAction);
					//		isIdle = false;
					//		stable = true;
					//	}
					//	else {
					//		//mAction->Queue(*fistAction);
					//		//m_pAnimationComponent->ResetCharacter();
					//		m_pAnimationComponent->QueueCustomFragment(*fistAction);
					//		stable = false;
					//		isIdle = false;
					//	}
					//}
					//else if ((bGripPressed && !trigger && !isPointing) || (isPointing && animEvent)) {
					//	isFist = false;
					//	isPointing = true;
					//	if (animEvent) {
					//		//mAction->Queue(*pointStableAction);
					//		m_pAnimationComponent->QueueCustomFragment(*pointStableAction);
					//		isIdle = false;
					//		stable = true;
					//	}
					//	else {
					//		//mAction->Queue(*pointAction);
					//		//m_pAnimationComponent->ResetCharacter();
					//		m_pAnimationComponent->QueueCustomFragment(*pointAction);
					//		isIdle = false;
					//		stable = false;
					//	}
					//}
					//else if (!bGripPressed && (isPointing || isFist || stable || isGrabbed || holdend || generalEnd) && !isIdle) {
					//	isPointing = false;
					//	stable = false;
					//	isFist = false;
					//	isGrabbed = false;
					//	//mAction->Queue(*idleAction);
					//	//reset hand
					//	//m_pAnimationComponent->ResetCharacter();
					//	m_pAnimationComponent->QueueFragmentWithId(idle);
					//	//m_pAnimationComponent->QueueCustomFragment(*idleAction);
					//	isIdle = true;
					//}

					IEntity* body = gEnv->pEntitySystem->FindEntityByName("HMD Cam");
					// Get the current tracking state
					HmdTrackingState cState = pController->GetLocalTrackingState(eHmdController_OculusRightHand);
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
					double vec = sqrt(pow(impulse.GetTranslation().x, 2) + pow(impulse.GetTranslation().y, 2) + pow(impulse.GetTranslation().z, 2));
					if (vec >= 3.0) {
						beamRightOculus(body, rightHand);
					}
					else
					{
						MoveLiving(*rightHand->GetPhysicalEntity(), impulse);
					}				
					rightHand->SetRotation(Quat(m));
					VRMovementComponentOculus::rightTriggerOculus = pController->GetTriggerValue(eHmdController_OculusRightHand, static_cast<EKeyId>(eHmdController_OculusRightHand + eKI_Motion_OculusTouch_L1));
					bGripPressed = pController->GetTriggerValue(eHmdController_OculusRightHand, static_cast<EKeyId>(eHmdController_OculusRightHand + eKI_Motion_OculusTouch_L2)) >= .7;
					trigger = VRMovementComponentOculus::rightTriggerOculus >= .7 ? true : false;
				}
			}
		}
	}

}

Cry::Entity::EventFlags VRRightControllerOculus::GetEventMask() const {
	return (Cry::Entity::EEvent::Update | Cry::Entity::EEvent::AnimationEvent);
}



