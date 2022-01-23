#pragma
#include "StdAfx.h"
#include "GamePlugin.h"

#include "Player.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>
#include <CrySchematyc/Utils/SharedString.h>
#include <IGameObjectSystem.h>
#include <IGameObject.h>
#include <CrySystem/ConsoleRegistration.h>
// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>
#include "VRMovementComponent.h"
#include "LeftHandInteraction.h"
#include "RightHandInteraction.h"
#include <CryGame/SteamAchievementCustom.h>
CESave save;
CGamePlugin::~CGamePlugin()
{
	// Remove any registered listeners before 'this' becomes invalid

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	if (gEnv->pSchematyc)
	{
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(CGamePlugin::GetCID());
	}
}
void checkStatues() {
	if (save.checkElement("statue1") == "1" &&
		save.checkElement("statue2") == "1" &&
		save.checkElement("statue3") == "1" &&
		save.checkElement("statue4") == "1" &&
		save.checkElement("statue5") == "1" &&
		save.checkElement("statue6") == "1" &&
		save.checkElement("statue7") == "1" &&
		save.checkElement("statue8") == "1" &&
		save.checkElement("statue9") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("ALL_STATUES");
	}
}
void checkPoems() {
	if (save.checkElement("poem1") == "1" &&
		save.checkElement("poem1") == "1" &&
		save.checkElement("poem1") == "1" &&
		save.checkElement("poem1") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("ALL_POEMS");
	}
}
void checkPins() {
	if (save.checkElement("pin1") == "1" &&
		save.checkElement("pin1") == "1" &&
		save.checkElement("pin1") == "1" &&
		save.checkElement("pin1") == "1" &&
		save.checkElement("croquet") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("ALL_PINS");
	}
}
void checkBillboard() {
	if (save.checkElement("billboard1") == "1" &&
		save.checkElement("billboard1") == "1" &&
		save.checkElement("billboard1") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("ALL_BILLBOARDS");
	}
}
void checkTime() {
	if (save.checkElement("3hours") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("HOURS_3");
	}
	if (save.checkElement("5hours") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("HOURS_5");
	}
	if (save.checkElement("10hours") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("HOURS_10");
	}
	if (save.checkElement("20hours") == "1") {
		CSteamAchievements::g_SteamAchievements->SetAchievement("HOURS_20");
	}
}
bool CGamePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	// Register for engine system events, in our case we need ESYSTEM_EVENT_GAME_POST_INIT to load the map
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this, "CGamePlugin");
	ICVar* hideMenu = REGISTER_INT("ui_showMainMenu", 0, VF_NULL, "Will tell when to show the main menu.");
	ICVar* hideSettings = REGISTER_INT("ui_showSettings", 0, VF_NULL, "Will tell when to show the main settings.");
	ICVar* openingScene = REGISTER_INT("ui_isOpeningScene", 1, VF_NULL, "Will tell if the menu is in opening_scene. If so, wil block menu from being removed.");
	hideMenu->Set(0);
	hideSettings->Set(0);
	openingScene->Set(1);
	save.openFile();
	return true;
}

void CGamePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
		// Called when the game framework has initialized and we are ready for game logic to start
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{
		// Listen for client connection events, in order to create the local player
		//gEnv->pGameFramework->AddNetworkedClientListener(*this);

		// Don't need to load the map in editor
		if (!gEnv->IsEditor())
		{
			// Load the example map in client server mode
			//gEnv->pConsole->ExecuteString("map example s", false, true);
		}
	}
	break;

	case ESYSTEM_EVENT_REGISTER_SCHEMATYC_ENV:
	{
		// Register all components that belong to this plug-in
		auto staticAutoRegisterLambda = [](Schematyc::IEnvRegistrar& registrar)
		{
			// Call all static callback registered with the CRY_STATIC_AUTO_REGISTER_WITH_PARAM
			Detail::CStaticAutoRegistrar<Schematyc::IEnvRegistrar&>::InvokeStaticCallbacks(registrar);
		};

		if (gEnv->pSchematyc)
		{
			gEnv->pSchematyc->GetEnvRegistry().RegisterPackage(
				stl::make_unique<Schematyc::CEnvPackage>(
					CGamePlugin::GetCID(),
					"EntityComponents",
					"Crytek GmbH",
					"Components",
					staticAutoRegisterLambda
					)
			);
		}
	}
	break;

	case ESYSTEM_EVENT_LEVEL_UNLOAD:
	{
		//m_players.clear();
	}
	break;

	}
}

CRYREGISTER_SINGLETON_CLASS(CGamePlugin)
class CFlowModifySave : public CFlowBaseNode<eNCT_Singleton>
{
public:

	CFlowModifySave(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Substring, retrieves letters from defined string.")),//0
			InputPortConfig<int>("billboard1", _HELP("")),//1
			InputPortConfig<int>("billboard2", _HELP(" ")),//2
			InputPortConfig<int>("billboard3", _HELP(" ")),//3
			InputPortConfig<int>("croquet", _HELP(" ")),//4
			InputPortConfig<int>("pin1", _HELP("")),//5
			InputPortConfig<int>("pin2", _HELP("")),//6
			InputPortConfig<int>("pin3", _HELP(" ")),//7
			InputPortConfig<int>("pin4", _HELP(" ")),//8
			InputPortConfig<int>("poem1", _HELP(" ")),//9
			InputPortConfig<int>("poem2", _HELP(" ")),//10
			InputPortConfig<int>("poem3", _HELP(" ")),//11
			InputPortConfig<int>("poem4", _HELP(" ")),//12
			InputPortConfig<int>("returning", _HELP(" ")),//13
			InputPortConfig<int>("statue1", _HELP(" ")),//14
			InputPortConfig<int>("statue2", _HELP(" ")),//15
			InputPortConfig<int>("statue3", _HELP(" ")),//16
			InputPortConfig<int>("statue4", _HELP(" ")),//17
			InputPortConfig<int>("statue5", _HELP(" ")),//18
			InputPortConfig<int>("statue6", _HELP(" ")),//19
			InputPortConfig<int>("statue7", _HELP(" ")),//20
			InputPortConfig<int>("statue8", _HELP(" ")),//21
			InputPortConfig<int>("statue9", _HELP(" ")),//22
			InputPortConfig_Void("CheckTimestamp", _HELP(" ")),//23
		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("returning"),
		{ 0 }
		};
		config.sDescription = _HELP("ModifySave");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				if (IsPortActive(pActInfo, 23)) {

					//calculate timestamp stuff
				}
				const int billboard1 = GetPortInt(pActInfo, 1);
				const int billboard2 = GetPortInt(pActInfo, 2);
				const int billboard3 = GetPortInt(pActInfo, 3);
				const int croquet = GetPortInt(pActInfo, 4);
				const int pin1 = GetPortInt(pActInfo, 5);
				const int pin2 = GetPortInt(pActInfo, 6);
				const int pin3 = GetPortInt(pActInfo, 7);
				const int pin4 = GetPortInt(pActInfo, 8);
				const int poem1 = GetPortInt(pActInfo, 9);
				const int poem2 = GetPortInt(pActInfo, 10);
				const int poem3 = GetPortInt(pActInfo, 11);
				const int poem4 = GetPortInt(pActInfo, 12);
				const int returning = GetPortInt(pActInfo, 13);
				const int statue1 = GetPortInt(pActInfo, 14);
				const int statue2 = GetPortInt(pActInfo, 15);
				const int statue3 = GetPortInt(pActInfo, 16);
				const int statue4 = GetPortInt(pActInfo, 17);
				const int statue5 = GetPortInt(pActInfo, 18);
				const int statue6 = GetPortInt(pActInfo, 19);
				const int statue7 = GetPortInt(pActInfo, 20);
				const int statue8 = GetPortInt(pActInfo, 21);
				const int statue9 = GetPortInt(pActInfo, 22);
				if (statue1 == 1) { save.modifyElement("statue1", "1"); }
				if (statue2 == 1) { save.modifyElement("statue2", "1"); }
				if (statue3 == 1) { save.modifyElement("statue3", "1"); }
				if (statue4 == 1) { save.modifyElement("statue4", "1"); }
				if (statue5 == 1) { save.modifyElement("statue5", "1"); }
				if (statue6 == 1) { save.modifyElement("statue6", "1"); }
				if (statue7 == 1) { save.modifyElement("statue7", "1"); }
				if (statue8 == 1) { save.modifyElement("statue8", "1"); }
				if (statue9 == 1) { save.modifyElement("statue9", "1"); }
				if (returning == 1) { save.modifyElement("returning", "1"); }
				if (poem1 == 1) { save.modifyElement("poem1", "1"); }
				if (poem2 == 1) { save.modifyElement("poem2", "1"); }
				if (poem3 == 1) { save.modifyElement("poem3", "1"); }
				if (poem4 == 1) { save.modifyElement("poem4", "1"); }
				if (pin1 == 1) { save.modifyElement("pin1", "1"); }
				if (pin2 == 1) { save.modifyElement("pin2", "1"); }
				if (pin3 == 1) { save.modifyElement("pin3", "1"); }
				if (pin4 == 1) { save.modifyElement("pin4", "1"); }
				if (croquet == 1) { save.modifyElement("croquet", "1"); }
				if (billboard1 == 1) { save.modifyElement("billboard1", "1"); }
				if (billboard2 == 1) { save.modifyElement("billboard2", "1"); }
				if (billboard3 == 1) { save.modifyElement("billboard3", "1"); }
				checkPoems();
				checkPins();
				checkStatues();
				checkBillboard();
				checkTime();
				int a = -1;
				sscanf(save.checkElement("returning").c_str(), "%d", &a);
				ActivateOutput(pActInfo, 0, a);
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};

class CFlowStopMovement : public CFlowBaseNode<eNCT_Singleton>
{
public:

	CFlowStopMovement(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Substring, retrieves letters from defined string.")),//0
			InputPortConfig<bool>("canMove", _HELP("Setting to true will disable movement for player.")),//22
		{ 0 }
		};

		config.sDescription = _HELP("Modify if the player can move or not.");
		config.pInputPorts = in_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				VRMovementComponent::canMove = GetPortInt(pActInfo, 1);
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};

class CFlowVRGrabbed : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowVRGrabbed(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Activates node")),//0
		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig_Void("LeftGrabbed","Activates if the left hand is grabbing something"),
			OutputPortConfig<int>("LeftGrabbedEntity","Activates if the left hand is grabbing something and returns the entity id."),

			OutputPortConfig_Void("RightGrabbed","Activates if the right hand is grabbing something"),
			OutputPortConfig<int>("RightGrabbedEntity","Activates if the left hand is grabbing something and returns the entity id."),
		{ 0 }
		};
		config.sDescription = _HELP("Check if and what hand is grabbing something.");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				if (LeftHandInteraction::grabbed) {
					int l = LeftHandInteraction::currentChild->GetId();
					ActivateOutput(pActInfo, 0, 1);
					ActivateOutput(pActInfo, 1, l);
				}
				else {
					ActivateOutput(pActInfo, 1, -1);
				}
				if (RightHandInteraction::grabbedx) {
					int r = RightHandInteraction::currentChildx->GetId();
					ActivateOutput(pActInfo, 2, 1);
					ActivateOutput(pActInfo, 3, r);
				}
				else {
					ActivateOutput(pActInfo, 3, -1);
				}
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};
class CFlowVRQuickFogFix : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowVRQuickFogFix(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Activates node")),//0
		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
		{ 0 }
		};
		config.sDescription = _HELP("Turns dual rendering off then on.");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				ICVar* stereomode = gEnv->pConsole->GetCVar("r_StereoMode");
				stereomode->Set(0);//disable dual rendering
				stereomode->Set(1);//enable dual rendering
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};
class CFlowGetSettings : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowGetSettings(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Activates node to get settings from file")),//0
			InputPortConfig<int>("Preset", _HELP("")),//1
			InputPortConfig<int>("Shadows", _HELP(" ")),//2
			InputPortConfig<int>("Render", _HELP(" ")),//3
			InputPortConfig<int>("ObjectDetails", _HELP(" ")),//4
			InputPortConfig<int>("textures", _HELP("")),//5
			InputPortConfig<int>("antialiasing", _HELP("")),//6

		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<int>("Preset"),
			OutputPortConfig<int>("Shadows"),
			OutputPortConfig<int>("Render"),
			OutputPortConfig<int>("ObjectDetails"),
			OutputPortConfig<int>("Textures"),
			OutputPortConfig<int>("AntiAliasing"),
		{ 0 }
		};
		config.sDescription = _HELP("Get Settings");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				int preset = -1;
				int shadows = -1;
				int render = -1;
				int object = -1;
				int textures = -1;
				int aa = -1;
				sscanf(save.checkElement("preset").c_str(), "%d", &preset);
				sscanf(save.checkElement("shadows").c_str(), "%d", &shadows);
				sscanf(save.checkElement("render").c_str(), "%d", &render);
				sscanf(save.checkElement("objectdetails").c_str(), "%d", &object);
				sscanf(save.checkElement("textures").c_str(), "%d", &textures);
				sscanf(save.checkElement("antialiasing").c_str(), "%d", &aa);
				if (IsPortActive(pActInfo, 1)) {
					save.modifyElement("preset", std::to_string(GetPortInt(pActInfo, 1)));
					CryLog(ToString(GetPortInt(pActInfo, 1)));

				}
				if (IsPortActive(pActInfo, 2)) {
					save.modifyElement("shadows", std::to_string(GetPortInt(pActInfo, 2)));
					ICVar* shadowsVar = gEnv->pConsole->GetCVar("sys_spec_shadows");
					shadowsVar->Set(GetPortInt(pActInfo, 2));
					CryLog("Shadows:" + ToString(GetPortInt(pActInfo, 2)));

				}
				if (IsPortActive(pActInfo, 3)) {
					save.modifyElement("render", std::to_string(GetPortInt(pActInfo, 3)));
					CryLog("Render:" + ToString(GetPortInt(pActInfo, 3)));
				}
				if (IsPortActive(pActInfo, 4)) {
					save.modifyElement("objectdetails", std::to_string(GetPortInt(pActInfo, 4)));
					ICVar* objDetVar = gEnv->pConsole->GetCVar("sys_spec_ObjectDetail");
					objDetVar->Set(GetPortInt(pActInfo, 4));
					CryLog("ObjectDetails:" + ToString(GetPortInt(pActInfo, 4)));
				}
				if (IsPortActive(pActInfo, 5)) {
					save.modifyElement("textures", std::to_string(GetPortInt(pActInfo, 5)));
					CryLog("Textures:" + ToString(GetPortInt(pActInfo, 5)));
					ICVar* texturesVar = gEnv->pConsole->GetCVar("sys_spec_Texture");
					ICVar* TEXResVar = gEnv->pConsole->GetCVar("sys_spec_textureResolution");
					texturesVar->Set(GetPortInt(pActInfo, 5));
					TEXResVar->Set(GetPortInt(pActInfo, 5));
				}
				if (IsPortActive(pActInfo, 6)) {
					save.modifyElement("antialiasing", std::to_string(GetPortInt(pActInfo, 6)));
					CryLog("AntiAliasing:" + ToString(GetPortInt(pActInfo, 6)));
					ICVar* AAVar = gEnv->pConsole->GetCVar("r_AntiAliasingMode");
					AAVar->Set(GetPortInt(pActInfo, 6));
				}
				save.saveFile();

				ActivateOutput(pActInfo, 0, preset);
				ActivateOutput(pActInfo, 1, shadows);
				ActivateOutput(pActInfo, 2, render);
				ActivateOutput(pActInfo, 3, object);
				ActivateOutput(pActInfo, 4, textures);
				ActivateOutput(pActInfo, 5, aa - 1);
				save.reconstructCFG();
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};
class CFlowSetGraphics : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowSetGraphics(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Activates node to get settings from file")),//0

		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
		{ 0 }
		};
		config.sDescription = _HELP("SetGraphics");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				ICVar* shadowsVar = gEnv->pConsole->GetCVar("sys_spec_shadows");
				ICVar* objDetVar = gEnv->pConsole->GetCVar("sys_spec_ObjectDetail");
				ICVar* texturesVar = gEnv->pConsole->GetCVar("sys_spec_Texture");
				ICVar* TEXResVar = gEnv->pConsole->GetCVar("sys_spec_textureResolution");
				ICVar* AAVar = gEnv->pConsole->GetCVar("r_AntiAliasingMode");
				int shadows = -1;
				int object = -1;
				int textures = -1;
				int aa = -1;
				sscanf(save.checkElement("shadows").c_str(), "%d", &shadows);
				sscanf(save.checkElement("objectdetails").c_str(), "%d", &object);
				sscanf(save.checkElement("textures").c_str(), "%d", &textures);
				sscanf(save.checkElement("antialiasing").c_str(), "%d", &aa);

				shadowsVar->Set(shadows);
				objDetVar->Set(object);
				texturesVar->Set(textures);
				TEXResVar->Set(textures);
				AAVar->Set(aa);
			}
			break;
		}
	}
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};


class CFlowAchievements : public CFlowBaseNode<eNCT_Singleton>
{
public:
	CFlowAchievements(SActivationInfo* pActInfo) {};
	virtual void GetConfiguration(SFlowNodeConfig& config)
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig_Void("Get", _HELP("Activates node")),//0
		{ 0 }
		};
		static const SOutputPortConfig out_config[] = {
		{ 0 }
		};
		config.sDescription = _HELP("Turns dual rendering off then on.");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_APPROVED);
	}
	virtual void ProcessEvent(EFlowEvent evt, SActivationInfo* pActInfo)
	{
		switch (evt)
		{
		case eFE_Activate:
			if (IsPortActive(pActInfo, 0))
			{
				CSteamAchievements::initializeSteam();
			}
			break;
		}
	};
	virtual void GetMemoryUsage(ICrySizer* s) const
	{
		s->Add(*this);
	}
};

//Register Flownode (you will need to do this for every FlowNode once)
//The REGISTER_FLOW_NODE Macro takes 2 parameters, the name as first and the class as second parameter.
REGISTER_FLOW_NODE("IndigoAffect:ModifySave", CFlowModifySave)
REGISTER_FLOW_NODE("IndigoAffect:changeVRMovement", CFlowStopMovement)
REGISTER_FLOW_NODE("IndigoAffect:GetSettings", CFlowGetSettings)
REGISTER_FLOW_NODE("IndigoAffect:GetGrabbedObjects", CFlowVRGrabbed)
REGISTER_FLOW_NODE("IndigoAffect:TickDualRendering", CFlowVRQuickFogFix)
REGISTER_FLOW_NODE("IndigoAffect:SetGraphics", CFlowSetGraphics)

