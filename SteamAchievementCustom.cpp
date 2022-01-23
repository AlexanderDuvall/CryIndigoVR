#pragma once
#include "SteamAchievementCustom.h"
CSteamAchievements* CSteamAchievements::g_SteamAchievements = NULL;
CSteamAchievements::Achievement_t CSteamAchievements::g_Achievements[8] = {
	 _ACH_ID(CSteamAchievements::HOURS_3,"Relaxing"),
	 _ACH_ID(CSteamAchievements::Hours_5, "No Stress Here"),
	 _ACH_ID(CSteamAchievements::HOURS_1O,"Enjoying the Sunshine"),
	 _ACH_ID(CSteamAchievements::HOURS_20,"My World"),
	 _ACH_ID(CSteamAchievements::ALL_BILLBOARDS,"A New Mind"),
	 _ACH_ID(CSteamAchievements::ALL_PINS,"Call me Bamford"),
	 _ACH_ID(CSteamAchievements::ALL_STATUES, "Renaissance Man"),
	 _ACH_ID(CSteamAchievements::ALL_POEMS,"You\'re a Bard"),
};
CSteamAchievements::CSteamAchievements(CSteamAchievements::Achievement_t* Achievements, int NumAchievements) :
	m_iAppID(0),
	m_bInitialized(false),
	m_CallbackUserStatsReceived(this, &CSteamAchievements::OnUserStatsReceived),
	m_CallbackUserStatsStored(this, &CSteamAchievements::OnUserStatsStored),
	m_CallbackAchievementStored(this, &CSteamAchievements::OnAchievementStored)
{
	m_iAppID = SteamUtils()->GetAppID();
	m_pAchievements = Achievements;
	m_iNumAchievements = NumAchievements;
	RequestStats();
}
void CSteamAchievements::initializeSteam() {
	bool bRet = SteamAPI_Init();
	// Create the SteamAchievements object if Steam was successfully initialized
	if (bRet)
	{
		CSteamAchievements::g_SteamAchievements = new CSteamAchievements(CSteamAchievements::g_Achievements, 8);
	}
}
bool CSteamAchievements::RequestStats()
{
	// Is Steam loaded? If not we can't get stats.
	if (NULL == SteamUserStats() || NULL == SteamUser())
	{
		return false;
	}
	// Is the user logged on?  If not we can't get stats.
	if (!SteamUser()->BLoggedOn())
	{
		return false;
	}
	// Request user stats.
	return SteamUserStats()->RequestCurrentStats();
}
bool CSteamAchievements::SetAchievement(const char* ID)
{
	// Have we received a call back from Steam yet?
	if (CSteamAchievements::m_bInitialized)
	{
		SteamUserStats()->SetAchievement(ID);
		return SteamUserStats()->StoreStats();
	}
	// If not then we can't set achievements yet
	return false;
}
void CSteamAchievements::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
			//OutputDebugString("Received stats and achievements from Steam\n");
			m_bInitialized = true;

			// load achievements
			for (int iAch = 0; iAch < m_iNumAchievements; ++iAch)
			{
				Achievement_t& ach = m_pAchievements[iAch];

				SteamUserStats()->GetAchievement(ach.m_pchAchievementID, &ach.m_bAchieved);
			}
		}
		else
		{
		}
	}
}void CSteamAchievements::OnUserStatsStored(UserStatsStored_t* pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		if (k_EResultOK == pCallback->m_eResult)
		{
		}
		else
		{
		}
	}
}
void CSteamAchievements::OnAchievementStored(UserAchievementStored_t* pCallback)
{
	// we may get callbacks for other games' stats arriving, ignore them
	if (m_iAppID == pCallback->m_nGameID)
	{
		//OutputDebugString("Stored Achievement for Steam\n");
	}
}
void CSteamAchievements::SteamAPICallBack() {
	SteamAPI_RunCallbacks();
}
void CSteamAchievements::steamShutdown() {
	SteamAPI_Shutdown();
}
