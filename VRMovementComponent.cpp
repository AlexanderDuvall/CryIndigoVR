#include "StdAfx.h"
#include "VRMovementComponent.h"
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
Cry::DefaultComponents::CRigidBodyComponent* rigid;
bool VRMovementComponent::canMove = true;
struct Point {
	double x;
	double y;
};
//PLACE IN CORRECT SPOT ... INITIALIZE?
void PhysicalizeLiving(IEntity& entity) {
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
	playerDimensions.sizeCollider = Vec3(0.f, 0.0f, .0f);
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
void MoveLiving(IPhysicalEntity& physicalEntity, double x, double y, double g)
{
	pe_action_move moveAction;
	// Apply movement request directly to velocity
	moveAction.iJump = 1;
	moveAction.dir = Vec3((float)x, (float)y, (float)g);
	physicalEntity.Action(&moveAction);
	//CryLog("moving...");
}



static void RegisterVrMovementComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(VRMovementComponent));
		// Functions	
		{
		}
	}
}

CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterVrMovementComponent)


void VRMovementComponent::Initialize() {
	rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
	IEntity* newEntity = rigid->GetEntity();
	PhysicalizeLiving(*newEntity);
	mainCamera = gEnv->pSystem->GetViewCamera();
	CameraEntity = gEnv->pEntitySystem->FindEntityByName("HMD Cam");
	m_pAudioListenerComponent = m_pEntity->GetOrCreateComponent<Cry::Audio::DefaultComponents::CListenerComponent>();
}
void VRMovementComponent::ProcessEvent(const SEntityEvent& event) {
	if (event.event == ENTITY_EVENT_UPDATE)
	{
		if (VRMovementComponent::canMove)
			if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
			{
				if (IHmdDevice* pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
				{
					if (pDevice->GetClass() == EHmdClass::eHmdClass_OpenVR) // Check, if the connected device is an Oculus device)
					{
						const IHmdController* pController = pDevice->GetController();
						// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
						if (pController->IsConnected(eHmdController_OpenVR_1))
						{
							const IHmdController* pController = pDevice->GetController();
							// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
							mainCamera = gEnv->pSystem->GetViewCamera();
							IEntity* newEntity = rigid->GetEntity();
							IPhysicalEntity* phys = newEntity->GetPhysicalEntity();
							//TODO get camera working with IRL movements so body will move as the player moves IRL.
							Vec2 vTouchPad = pController->GetThumbStickValue(eHmdController_OpenVR_1, eKI_Motion_OpenVR_TouchPad_X);
							if (abs(vTouchPad.x) > 0.2 || abs(vTouchPad.y) > .2) {
								Ang3 ang = mainCamera.GetAngles();
								double cameraYaw = ang.x * 57.324 + 90; // 90 is the offset. 
								double x = vTouchPad.x;
								double y = vTouchPad.y;
								double controllerVector = sqrt(x * x + y * y);
								double StickAngle = VRtoPolar(x, y) * 180 / 3.14;
								double Direction = VRnewAngle(StickAngle, cameraYaw);
								Point p = VRtoRect(Direction);
								double speed = m_speed * controllerVector;
								ICVar* mainMenuVar = gEnv->pConsole->GetCVar("ui_showMainMenu"); // will be 1 if showing.
								if (mainMenuVar->GetIVal() == 0)
									MoveLiving(*phys, p.x * speed, p.y * speed, -2);
							}

						}

					}


				}

			}
	}
}

Cry::Entity::EventFlags VRMovementComponent::GetEventMask() const {
	return (ENTITY_EVENT_UPDATE);
}



