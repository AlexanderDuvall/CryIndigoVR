#pragma once
#include"CryEntitySystem/IEntityComponent.h"
#include "CryPhysics/physinterface.h"

class PauseMenuController : public IEntityComponent
{
public:
	PauseMenuController() {};
	~PauseMenuController() {};

	static void ReflectType(Schematyc::CTypeDesc<PauseMenuController>& desc) {
		desc.SetEditorCategory("IndigoAffect");
		desc.SetGUID("{C4D1CE4D-BB0A-40B9-8949-BA5A0940BB9E}"_cry_guid);
		desc.AddMember(&PauseMenuController::xRot, 'xro', "xRot", "xRot", "X Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&PauseMenuController::zRot, 'zro', "zRot", "zRot", "Z Rotation of the left hand for testing.", 0.0F);
		desc.AddMember(&PauseMenuController::yRot, 'yro', "yRot", "yRot", "Y Rotation of the left hand for testing.", 0.0F);

		desc.AddMember(&PauseMenuController::xPos, 'xpo', "xPos", "xPos", "X Position of the pause menu", 0.0F);
		desc.AddMember(&PauseMenuController::zPos, 'zpo', "zPos", "zPos", "Z Position of the pause menu", 0.0F);
		desc.AddMember(&PauseMenuController::yPos, 'ypo', "yPos", "yPos", "Y Position of the pause menu", 0.0F);

		desc.AddMember(&PauseMenuController::backgroundPanelX, 'bpx', "BCPX", "BCPX", "Orient offset of background panel on X-axis", 0.0F);
		desc.AddMember(&PauseMenuController::backgroundPanelY, 'bpy', "BCPY", "BCPY", "Orient offset of background panel on Y-axis", 0.0F);
		desc.AddMember(&PauseMenuController::backgroundPanelZ, 'bpz', "BCPZ", "BCPZ", "Orient offset of background panel on Z-axis", 0.0F);

		desc.SetLabel("PauseMenuController");
	}

	virtual void Initialize() override;
	virtual void ProcessEvent(const SEntityEvent& event) override;
	virtual Cry::Entity::EventFlags GetEventMask() const override;
	IEntity* righthand;
	IEntity* uiBall;
	IEntity* mainBody;
	IEntity* backgroundPanel;
 	float yPos = 0.0;
	float xPos = 0.0;
	float zPos = 0.0;
	float xRot = 0.0;
	float yRot = 0.0;
	float zRot = 0.0;
	float backgroundPanelX = 0;
	float backgroundPanelY = 0;
	float backgroundPanelZ = 0;
	bool isoculus = false;
	bool issteam = false;
	static bool paused;
private:
	void checkMenuStatus( );
	bool initialHide = false;
	bool forceHide = false;
	bool alreadyLogged = false;
	bool settingsVisible = false;
	bool isConfirming = false;
	bool isInfoVisible = false;
	bool levelsVisible = false;
	IEntity* laser;
	IEntity* settingsMenu;
	IEntity* mainMenu;
	IEntity* confirmMenu;
	IEntity* InformationMenu;
	IEntity* LevelsMenu;

	const uint32 queryFlags = ent_all;
	const uint32 rayFlags = rwi_stop_at_pierceable;
};