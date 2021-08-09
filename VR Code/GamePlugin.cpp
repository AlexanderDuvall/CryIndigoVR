#include "StdAfx.h"
#include "GamePlugin.h"

#include "Components/Player.h"

#include <CrySchematyc/Env/IEnvRegistry.h>
#include <CrySchematyc/Env/EnvPackage.h>
#include <CrySchematyc/Utils/SharedString.h>

#include <IGameObjectSystem.h>
#include <IGameObject.h>
#include <CrySystem/ConsoleRegistration.h>
// Included only once per DLL module.
#include <CryCore/Platform/platform_impl.inl>
#include "C:\Users\alexa\Documents\CRYENGINE Projects\Undergrowth_Maze_VR\New folder\VRMovementComponent.h"
#include "C:\Users\alexa\Documents\CRYENGINE Projects\Undergrowth_Maze_VR\New folder\LeftHandInteraction.h"
#include "C:\Users\alexa\Documents\CRYENGINE Projects\Undergrowth_Maze_VR\New folder\RightHandInteraction.h"
CGamePlugin::~CGamePlugin()
{
	// Remove any registered listeners before 'this' becomes invalid
	if (gEnv->pGameFramework != nullptr)
	{
		gEnv->pGameFramework->RemoveNetworkedClientListener(*this);
	}

	gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	if (gEnv->pSchematyc)
	{
		gEnv->pSchematyc->GetEnvRegistry().DeregisterPackage(CGamePlugin::GetCID());
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
		gEnv->pGameFramework->AddNetworkedClientListener(*this);

		// Don't need to load the map in editor
		if (!gEnv->IsEditor())
		{
			// Load the example map in client server mode
			gEnv->pConsole->ExecuteString("map example s", false, true);
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
		m_players.clear();
	}
	break;
	}
}

bool CGamePlugin::OnClientConnectionReceived(int channelId, bool bIsReset)
{
	// Connection received from a client, create a player entity and component
	SEntitySpawnParams spawnParams;
	spawnParams.pClass = gEnv->pEntitySystem->GetClassRegistry()->GetDefaultClass();

	// Set a unique name for the player entity
	const string playerName = string().Format("Player%" PRISIZE_T, m_players.size());
	spawnParams.sName = playerName;

	// Set local player details
	if (m_players.empty() && !gEnv->IsDedicated())
	{
		spawnParams.id = LOCAL_PLAYER_ENTITY_ID;
		spawnParams.nFlags |= ENTITY_FLAG_LOCAL_PLAYER;
	}

	// Spawn the player entity
	if (IEntity* pPlayerEntity = gEnv->pEntitySystem->SpawnEntity(spawnParams))
	{
		// Set the local player entity channel id, and bind it to the network so that it can support Multiplayer contexts
		pPlayerEntity->GetNetEntity()->SetChannelId(channelId);

		// Create the player component instance
		CPlayerComponent* pPlayer = pPlayerEntity->GetOrCreateComponentClass<CPlayerComponent>();

		if (pPlayer != nullptr)
		{
			// Push the component into our map, with the channel id as the key
			m_players.emplace(std::make_pair(channelId, pPlayerEntity->GetId()));
		}
	}

	return true;
}

bool CGamePlugin::OnClientReadyForGameplay(int channelId, bool bIsReset)
{
	// Revive players when the network reports that the client is connected and ready for gameplay
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		if (IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(it->second))
		{
			if (CPlayerComponent* pPlayer = pPlayerEntity->GetComponent<CPlayerComponent>())
			{
				pPlayer->OnReadyForGameplayOnServer();
			}
		}
	}

	return true;
}

void CGamePlugin::OnClientDisconnected(int channelId, EDisconnectionCause cause, const char* description, bool bKeepClient)
{
	// Client disconnected, remove the entity and from map
	auto it = m_players.find(channelId);
	if (it != m_players.end())
	{
		gEnv->pEntitySystem->RemoveEntity(it->second);

		m_players.erase(it);
	}
}

void CGamePlugin::IterateOverPlayers(std::function<void(CPlayerComponent& player)> func) const
{
	for (const std::pair<int, EntityId>& playerPair : m_players)
	{
		if (IEntity* pPlayerEntity = gEnv->pEntitySystem->GetEntity(playerPair.second))
		{
			if (CPlayerComponent* pPlayer = pPlayerEntity->GetComponent<CPlayerComponent>())
			{
				func(*pPlayer);
			}
		}
	}
}
CRYREGISTER_SINGLETON_CLASS(CGamePlugin)


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
//Register Flownode (you will need to do this for every FlowNode once)
//The REGISTER_FLOW_NODE Macro takes 2 parameters, the name as first and the class as second parameter.
REGISTER_FLOW_NODE("IndigoAffect:changeVRMovement", CFlowStopMovement)
REGISTER_FLOW_NODE("IndigoAffect:GetGrabbedObjects", CFlowVRGrabbed)

