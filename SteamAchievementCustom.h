#pragma once
#include "Steamworks/public/steam/steam_api.h"	
class CSteamAchievements
{
public:
#define _ACH_ID( id, name ) { id, #id, name, "", 0, 0 }
	struct Achievement_t
	{
		int m_eAchievementID;
		const char* m_pchAchievementID;
		char m_rgchName[128];
		char m_rgchDescription[256];
		bool m_bAchieved;
		int m_iIconImage;
	};
	enum ESteamAchievement
	{
		HOURS_3,
		Hours_5,
		HOURS_1O,
		HOURS_20,
		ALL_BILLBOARDS,
		ALL_PINS,
		ALL_STATUES,
		ALL_POEMS
	};
	CSteamAchievements(Achievement_t* Achievements, int NumAchievements);
	static CSteamAchievements* g_SteamAchievements;
	static Achievement_t g_Achievements[8];
	void SteamAPICallBack();
	void steamShutdown();
	bool RequestStats();
	bool SetAchievement(const char* ID);
	static void initializeSteam();

 	int m_iAppID; // Our current AppID
	bool m_bInitialized; // Have we called Request stats and received the callback?
	Achievement_t* m_pAchievements; // Achievements data
	int m_iNumAchievements; // The number of Achievements
	STEAM_CALLBACK(CSteamAchievements, OnUserStatsReceived, UserStatsReceived_t,
		m_CallbackUserStatsReceived);
	STEAM_CALLBACK(CSteamAchievements, OnUserStatsStored, UserStatsStored_t,
		m_CallbackUserStatsStored);
	STEAM_CALLBACK(CSteamAchievements, OnAchievementStored,
		UserAchievementStored_t, m_CallbackAchievementStored);
};