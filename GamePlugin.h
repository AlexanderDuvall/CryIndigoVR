#pragma once
#include "CESave.h"
#include <CrySystem/ICryPlugin.h>
#include <CryEntitySystem/IEntityClass.h>
#include <CryNetwork/INetwork.h>
class CPlayerComponent;

// The entry-point of the application
// An instance of CGamePlugin is automatically created when the library is loaded
// We then construct the local player entity and CPlayerComponent instance when OnClientConnectionReceived is first called.
class CGamePlugin
	: public Cry::IEnginePlugin
	, public ISystemEventListener
 {
public:
	CRYINTERFACE_SIMPLE(Cry::IEnginePlugin)
		CRYGENERATE_SINGLETONCLASS_GUID(CGamePlugin, "FirstPersonShooter", "{FC9BD884-49DE-4494-9D64-191734BBB7E3}"_cry_guid)

		virtual ~CGamePlugin();

	// Cry::IEnginePlugin
	virtual const char* GetCategory() const override { return "Game"; }
	virtual bool Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams) override;
	// ~Cry::IEnginePlugin

	// ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam) override;
	// ~ISystemEventListener

	 
	// Helper function to call the specified callback for every player in the game
 
	// Helper function to get the CGamePlugin instance
	// Note that CGamePlugin is declared as a singleton, so the CreateClassInstance will always return the same pointer
	static CGamePlugin* GetInstance()
	{
		return cryinterface_cast<CGamePlugin>(CGamePlugin::s_factory.CreateClassInstance().get());
	}

protected:
	// Map containing player components, key is the channel id received in OnClientConnectionReceived
	std::unordered_map<int, EntityId> m_players;
	PLUGIN_FLOWNODE_REGISTER
		PLUGIN_FLOWNODE_UNREGISTER
};
