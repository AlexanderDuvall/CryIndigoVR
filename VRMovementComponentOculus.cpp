#include "StdAfx.h"
#include "VRMovementComponentOculus.h"
#include "VRMath.h"
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
#include <CryMath/Cry_Matrix34.h>
#include <CryPhysics/physinterface.h>
struct Point {
	double x;
	double y;
};
float VRMovementComponentOculus::rightTriggerOculus = 0.f;
float VRMovementComponentOculus::leftTriggerOculus = 0.f;
//PLACE IN CORRECT SPOT ... INITIALIZE?
void PhysicalizeLivingOculus(IEntity& entity) {
	SEntityPhysicalizeParams physParams;
	// Set the physics type to PE_LIVINGtv
	physParams.type = PE_LIVING;
	// Set the mass to 90 kilograms
	physParams.mass = 100;
	// Living entities have to set the SEntityPhysicalizeParams::pPlayerDimensions field
	pe_player_dimensions playerDimensions;
	// Prefer usage of a cylinder instead of capsule
	playerDimensions.bUseCapsule = 0;
	// Specify the size of our cylinder
	playerDimensions.sizeCollider = Vec3(0, 0, 0);
	// Keep pivot at the player's feet (defined in player geometry) 
	playerDimensions.heightPivot = 0.f;
	// Offset collider upwards
	//playerDimensions.heightCollider = 1.f;
	physParams.pPlayerDimensions = &playerDimensions;
	// Living entities have to set the SEntityPhysicalizeParams::pPlayerDynamics field
	pe_player_dynamics playerDynamics;
	// Mass needs to be repeated in the pe_player_dynamics structure
	playerDynamics.mass = physParams.mass;
	physParams.pPlayerDynamics = &playerDynamics;
	// Now physicalize the entity
	entity.Physicalize(physParams);
}

void MoveLivingOculus(IPhysicalEntity& physicalEntity, double x, double y, double g)
{
	pe_action_move moveAction;
	// Apply movement request directly to velocity
	moveAction.iJump = 1;
	moveAction.dir = Vec3((float)x, (float)y, (float)g);
	physicalEntity.Action(&moveAction);
	//CryLog("moving...");
}

void resetCamera(Ang3 pastCamera) {

	const IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager();
	IHmdDevice* pDev = pHmdManager ? pHmdManager->GetHmdDevice() : nullptr;
	if (pDev && pHmdManager->IsStereoSetupOk())
	{
		const HmdTrackingState& sensorState = pDev->GetLocalTrackingState();
		if (sensorState.CheckStatusFlags(eHmdStatus_IsUsable))
		{
			//pDev->RecenterPose();
			CCamera CameraEntity = gEnv->pSystem->GetViewCamera();
			CameraEntity.SetAngles((Ang3(RAD2DEG(pastCamera.x), RAD2DEG(pastCamera.x), RAD2DEG(pastCamera.x))));
		}
	}

}

static void RegisterVrMovementComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(VRMovementComponentOculus));
		// Functions	
		{
		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterVrMovementComponent)


void VRMovementComponentOculus::Initialize() {
	Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
	IEntity* newEntity = rigid->GetEntity();
	PhysicalizeLivingOculus(*newEntity);
	mainCamera = gEnv->pSystem->GetViewCamera();
	m_pAudioListenerComponent = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();
}
void VRMovementComponentOculus::ProcessEvent(const SEntityEvent& event) {
	if (event.event == ENTITY_EVENT_UPDATE)
	{
		if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
		{
			if (IHmdDevice* pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
			{
				if (pDevice->GetClass() == EHmdClass::eHmdClass_Oculus) // Check, if the connected device is an Oculus device)
				{
					const IHmdController* pController = pDevice->GetController();
					// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
					if (pController->IsConnected(eHmdController_OculusLeftHand)) {
						//Vec2 vTouchPad = pController->IsGestureTriggered(eHmdController_OculusRightHand, eKI_Motion_OculusTouch_Gesture_ThumbUpR);
						Vec2 vTouchPad = pController->GetThumbStickValue(eHmdController_OculusLeftHand, eKI_Motion_OculusTouch_StickL_X);
						//Vec2 vTouchPad = Vec2(0, 0);
						mainCamera = gEnv->pSystem->GetViewCamera();
						Ang3 ang = mainCamera.GetAngles();
						//CryLog("x"+ToString(RAD2DEG(ang.x)));
						//CryLog("y"+ToString(RAD2DEG(ang.y)));
						//CryLog("z"+ToString(RAD2DEG(ang.z)));
						if (abs(vTouchPad.x) > 0.2 || abs(vTouchPad.y) > .2) {
							double cameraYaw = ang.x * 57.324 + 90;
							double x = vTouchPad.x;
							double y = vTouchPad.y;
							double controllerVector = sqrt(x * x + y * y);
							double StickAngle = VRtoPolar(x, y) * 180 / 3.14;
							double Direction = VRnewAngle(StickAngle, cameraYaw);
							Point p = VRtoRect(Direction);
							Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
							IEntity* newEntity = rigid->GetEntity();
							IPhysicalEntity* phys = newEntity->GetPhysicalEntity();
							double speed = m_speed * controllerVector;
							ICVar* mainMenuVar = gEnv->pConsole->GetCVar("ui_showMainMenu"); // will be 1 if showing.
							if (mainMenuVar->GetIVal() == 0) {
								if (!isMoving) {
									//resetCamera(ang);
									isMoving = true;
								}
								MoveLivingOculus(*phys, p.x * speed, p.y * speed, -9.8);
								pastMovement = newEntity->GetWorldPos();
							}
						}
						else {
							isMoving = false;
							Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
							IEntity* newEntity = rigid->GetEntity();
							IPhysicalEntity* phys = newEntity->GetPhysicalEntity(); 
							MoveLivingOculus(*phys,0, 0, -9.8);
							//newEntity->SetWorldTM(Matrix34::createpastMovement);
						}
						//Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
						//rigid->ApplyImpulse(Vec3(0, 0, 10) * m_Force);
					}
				}
			}
		}
	}
}

Cry::Entity::EventFlags VRMovementComponentOculus::GetEventMask() const {
	return (ENTITY_EVENT_UPDATE);
}



