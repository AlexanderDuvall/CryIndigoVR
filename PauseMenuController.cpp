#include "StdAfx.h"
#include "PauseMenuController.h"
#include "VRLeftController.h"
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

//PLACE IN CORRECT SPOT ... INITIALIZE?
struct Point {
	double x;
	double y;
};
bool PauseMenuController::paused = false;


static void RegisterPauseMenuComponent(Schematyc::IEnvRegistrar& registrar)
{
	Schematyc::CEnvRegistrationScope scope = registrar.Scope(IEntity::GetEntityScopeGUID());
	{
		Schematyc::CEnvRegistrationScope componentScope = scope.Register(SCHEMATYC_MAKE_ENV_COMPONENT(PauseMenuController));
		// Functions	
		{

		}
	}
}
CRY_STATIC_AUTO_REGISTER_FUNCTION(&RegisterPauseMenuComponent)
//asdf

void PauseMenuController::Initialize() {
	//Cry::DefaultComponents::CRigidBodyComponent* rigid = m_pEntity->GetOrCreateComponent <Cry::DefaultComponents::CRigidBodyComponent>();
	//righthand = rigid->GetEntity();
}
void PauseMenuController::ProcessEvent(const SEntityEvent& event) {
	mainBody = gEnv->pEntitySystem->FindEntityByName("HMD Cam");
	settingsMenu = gEnv->pEntitySystem->FindEntityByName("UI Settings");
	mainMenu = gEnv->pEntitySystem->FindEntityByName("UI PlaneBase");
	confirmMenu = gEnv->pEntitySystem->FindEntityByName("UI Confirm");
	InformationMenu = gEnv->pEntitySystem->FindEntityByName("UI Information");
	LevelsMenu = gEnv->pEntitySystem->FindEntityByName("UI Levels");

	backgroundPanel = gEnv->pEntitySystem->FindEntityByName("BackgroundPanelMain");
	uiBall = gEnv->pEntitySystem->FindEntityByName("UI_BALL");
	ICVar* mainMenuVar = gEnv->pConsole->GetCVar("ui_showMainMenu"); // will be 1 if showing.
	ICVar* settingsVar = gEnv->pConsole->GetCVar("ui_showSettings"); // will be 1 if showing.
	//ICVar* informationVar = gEnv->pConsole->GetCVar("ui_showInformation "); // will be 1 if showing.
	//ICVar* confirmVar = gEnv->pConsole->GetCVar("ui_showConfirm"); // will be 1 if showing.
	ICVar* openingSceneVar = gEnv->pConsole->GetCVar("ui_isOpeningScene");
	if (event.event == ENTITY_EVENT_UPDATE)
	{
		if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager()) // Check, if the HMD Manager exists
		{
			if (IHmdDevice* pDevice = pHmdManager->GetHmdDevice()) // Check, if a valid HMD device is connected
			{
				issteam = pDevice->GetClass() == EHmdClass::eHmdClass_OpenVR;

				const IHmdController* pController = pDevice->GetController();

				auto controllertypeLeft = issteam ? eHmdController_OpenVR_1 : eHmdController_OculusLeftHand;
				auto controllertypeRight = issteam ? eHmdController_OpenVR_2 : eHmdController_OculusRightHand;
				auto controllerButtonLeft = issteam ? eKI_Motion_OpenVR_ApplicationMenu : static_cast<EKeyId>(controllertypeLeft + eKI_Motion_OculusTouch_Y);
				auto controllerButtonRight = issteam ? eKI_Motion_OpenVR_ApplicationMenu : static_cast<EKeyId>(controllertypeLeft + eKI_Motion_OculusTouch_B);
				// Make sure the desired controller is connected (the OpenVR implementation in CRYENGINE currently supports controller ID 1 and 2)
				if (pController->IsConnected(controllertypeLeft) || pController->IsConnected(controllertypeRight))
				{
					bool bAppMenuPressed = pController->IsButtonPressed(controllertypeLeft, controllerButtonLeft) || pController->IsButtonPressed(controllertypeRight, controllerButtonRight);
					if (bAppMenuPressed == false) {
						alreadyLogged = false;
					}
					if (openingSceneVar->GetIVal() == 1) {
						paused = true;
					}
					//bring main menu up if not in opening_scene.

					if (bAppMenuPressed && !alreadyLogged && !PauseMenuController::paused && openingSceneVar->GetIVal() == 0) {
						pDevice->RecenterPose();
						Matrix34 local = mainMenu->GetLocalTM();
						Matrix34 backLocal = backgroundPanel->GetLocalTM();
						local.SetTranslation(Vec3(xPos, yPos, zPos));
						backLocal.SetTranslation(Vec3(xPos + backgroundPanelX, yPos + backgroundPanelY, zPos + backgroundPanelZ));
						alreadyLogged = true;
						mainMenu->SetLocalTM(local);
						backgroundPanel->SetLocalTM(backLocal);
						PauseMenuController::paused = true;
						mainMenuVar->Set(1);
						settingsVar->Set(0);
						if (uiBall)
							uiBall->Hide(false);
						CryLog("OpeningScene...");
					}
					//hide main menu
					else if ((bAppMenuPressed && !alreadyLogged && PauseMenuController::paused && openingSceneVar->GetIVal() == 0) || forceHide && openingSceneVar->GetIVal() == 0) {
						Matrix34 local = mainMenu->GetLocalTM();
						Matrix34 backLocal = backgroundPanel->GetLocalTM();
						//Matrix34 laserlocal = laser->GetLocalTM();
						//laserlocal.SetTranslation(Vec3(0, 0, -100));
						//laser->SetLocalTM(local);
						//laser->InvalidateTM();
						local.SetTranslation(Vec3(1, 1, -10.8));
						backLocal.SetTranslation(Vec3(0, 0, -10));
						alreadyLogged = true;
						mainMenu->SetLocalTM(local);
						mainMenu->SetLocalTM(local);
						backgroundPanel->SetLocalTM(backLocal);
						paused = false;
						//settingsMenu->InvalidateTM();
						//backgroundPanel->InvalidateTM();
						//mainMenu->InvalidateTM();
						mainMenuVar->Set(0);
						settingsVar->Set(0);
						forceHide = false;
						if (uiBall)
							uiBall->Hide(true);
						CryLog("Hiding menu if");
					}
					else {
						PauseMenuController::checkMenuStatus();
					}
				}
			}
		}
	}
}

/*
Check the Menu status and bring it up as demanded from the sandbox.
*/

void PauseMenuController::checkMenuStatus() {
	ICVar* mainMenuVar = gEnv->pConsole->GetCVar("ui_showMainMenu"); // will be 1 if showing.
	ICVar* settingsVar = gEnv->pConsole->GetCVar("ui_showSettings");
	ICVar* levelsVar = gEnv->pConsole->GetCVar("ui_showLevels");
	ICVar* informationVar = gEnv->pConsole->GetCVar("ui_showInformation"); // will be 1 if showing.
	ICVar* confirmVar = gEnv->pConsole->GetCVar("ui_showConfirm"); // will be 1 if showing.
	if (PauseMenuController::paused && settingsVar->GetIVal() == 1) {
		if (isInfoVisible && informationVar->GetIVal() == 0) {
			//back to the beginning ------------------
			Matrix34 local = confirmMenu->GetLocalTM();
			local.SetTranslation(Vec3(1, 1, -10.8));
			confirmMenu->SetLocalTM(local);
			InformationMenu->SetLocalTM(local);
			settingsMenu->SetLocalTM(local);

			settingsVar->Set(0);
			informationVar->Set(0);
			confirmVar->Set(0);
			//pull up main menu ----------------------
			local = InformationMenu->GetLocalTM();
			local.SetTranslation(Vec3(xPos, yPos, zPos));
			mainMenu->SetLocalTM(local);
			isInfoVisible = false;
			CryLog("Back to Begnning");
		}
		else if (!isInfoVisible && informationVar->GetIVal() == 1) { //check if information menu was requested
			CryLog("Information Requested");
			//hiding confirm
			Matrix34 local = confirmMenu->GetLocalTM();
			local.SetTranslation(Vec3(1, 1, -10.8));
			confirmMenu->SetLocalTM(local);
			//------------------------------------------------------------------ show information menu
			local = InformationMenu->GetLocalTM();
			local.SetTranslation(Vec3(xPos, yPos, zPos));
			InformationMenu->SetLocalTM(local);
			isInfoVisible = true;

		}
		else if (confirmVar->GetIVal() == 1 && !isConfirming && PauseMenuController::settingsVisible) {//check if confirm menu was requested
			CryLog("Confirm requested");
			isConfirming = true;
			//place confirm menu and hide settings menu.
			//hiding settings
			Matrix34 local = settingsMenu->GetLocalTM();
			local.SetTranslation(Vec3(1, 1, -10.8));
			settingsMenu->SetLocalTM(local);
			//------------------------------------------------------------------ show confirm menu
			local = confirmMenu->GetLocalTM();
			local.SetTranslation(Vec3(xPos, yPos, zPos));
			confirmMenu->SetLocalTM(local);
		}
		else if (confirmVar->GetIVal() == 0 && settingsVisible && isConfirming) {
			CryLog("Hiding Confirm and Bringing up Settings");
			isConfirming = false;
			//hide confirm and bring up settings
			//---------------- hide confirm
			Matrix34 local = settingsMenu->GetLocalTM();
			local.SetTranslation(Vec3(1, 1, -10.8));
			confirmMenu->SetLocalTM(local);
			//--------------- show settings
			local = confirmMenu->GetLocalTM();
			local.SetTranslation(Vec3(xPos, yPos, zPos));
			settingsMenu->SetLocalTM(local);
		}
		else if (settingsVar->GetIVal() == 1 && !settingsVisible) {///bring up settings.
			PauseMenuController::settingsVisible = true;
			Matrix34 local = settingsMenu->GetLocalTM();
			Matrix34 localMain = settingsMenu->GetLocalTM();
			//Matrix34 backLocalMain = backgroundPanel->GetLocalTM();
			localMain.SetTranslation(Vec3(1, 1, -10.8));
			//backLocalMain.SetTranslation(Vec3(1, 1, -10.8));
			local.SetTranslation(Vec3(xPos, yPos, zPos));
			mainMenu->SetLocalTM(localMain);
			settingsMenu->SetLocalTM(local);
			mainMenuVar->Set(1);
			settingsVar->Set(1);
			PauseMenuController::paused = true;
			isConfirming = false;
			CryLog("Bringing Up Settings");
		}
	}
	else if (!levelsVisible && paused && levelsVar->GetIVal() == 1 && LevelsMenu) { // show levels menu
		PauseMenuController::levelsVisible = true;
		Matrix34 local = LevelsMenu->GetLocalTM();
		Matrix34 localMain = LevelsMenu->GetLocalTM();
		//Matrix34 backLocalMain = backgroundPanel->GetLocalTM();
		localMain.SetTranslation(Vec3(1, 1, -10.8));
		//backLocalMain.SetTranslation(Vec3(1, 1, -10.8));
		local.SetTranslation(Vec3(xPos, yPos, zPos));
		mainMenu->SetLocalTM(localMain);
		LevelsMenu->SetLocalTM(local);
		//mainMenuVar->Set(1);
		levelsVar->Set(1);
		PauseMenuController::paused = true;
		levelsVisible = true;
		CryLog("Bringing Up Levels");
	}
	else if (paused && levelsVar->GetIVal() == 0 && levelsVisible && LevelsMenu) { //closes levels var if paused and cvar is 0.
		PauseMenuController::settingsVisible = false;
		Matrix34 local = LevelsMenu->GetLocalTM();
		Matrix34 localMain = mainMenu->GetLocalTM();
		localMain.SetTranslation(Vec3(xPos, yPos, zPos));
		local.SetTranslation(Vec3(1, 1, -10.8));
		mainMenu->SetLocalTM(localMain);
		LevelsMenu->SetLocalTM(local);
		levelsVar->Set(0);
		PauseMenuController::paused = true;
		levelsVisible = false;
		CryLog("closing levels Var");
	}
	else if (PauseMenuController::paused && settingsVar->GetIVal() == 0 && PauseMenuController::settingsVisible) {		//remove settings menu.

		PauseMenuController::settingsVisible = false;
		Matrix34 local = settingsMenu->GetLocalTM();
		Matrix34 localMain = mainMenu->GetLocalTM();
		localMain.SetTranslation(Vec3(xPos, yPos, zPos));
		local.SetTranslation(Vec3(1, 1, -10.8));
		mainMenu->SetLocalTM(localMain);
		settingsMenu->SetLocalTM(local);
		settingsVar->Set(0);
		PauseMenuController::paused = true;
		CryLog("Removing Settings");
	}
	else if (paused && mainMenuVar->GetIVal() == 0)
	{
		// hide everything
		PauseMenuController::paused = false;
		forceHide = true;
		PauseMenuController::paused = true;
		settingsVar->Set(0);
		//CryLog("Hiding Variable");
	}
}
Cry::Entity::EventFlags PauseMenuController::GetEventMask() const {
	return (ENTITY_EVENT_UPDATE);
}



