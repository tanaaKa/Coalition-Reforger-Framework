/*
*	Logging component for COALITION games
*	Component overrides base game mode so it always runs
*
*	Note that write files seem weird because they are parsed by an external program
*	which splits strings via colons
*
*	Server only
*/
[ComponentEditorProps(category: "CRF Logging Component", description: "")]
class CRF_MDB_LoggingServerComponentClass: SCR_BaseGameModeComponentClass
{
	
}

class CRF_MDB_LoggingServerComponent: SCR_BaseGameModeComponent
{	
	string m_sMissionName;
	string playerName;
	
	int m_iPlayerCount;
	int m_iBluforCount;
	int m_iOpforCount;
	int m_iIndforCount;
	
	SCR_FactionManager m_FM;
	
	static CRF_MDB_LoggingServerComponent GetInstance() 
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return CRF_MDB_LoggingServerComponent.Cast(gameMode.FindComponent(CRF_MDB_LoggingServerComponent));
		else
			return null;
	}
	
	// Setup
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		if (Replication.IsClient() || !GetGame().InPlayMode())
			return;
		
		m_sMissionName = GetGame().GetMissionName();
		m_iPlayerCount = GetGame().GetPlayerManager().GetPlayerCount();
		
		// Connect to DB 
		EDF_MongoDbConnectionInfo connectInfo();
		connectInfo.m_sDatabaseName = "reforger";
		connectInfo.m_sProxyHost = "localhost";
		connectInfo.m_iProxyPort = "27017";
	}
	
	// Player Connected
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		if (Replication.IsClient())
			return;
		
		playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		
		
	}
	
	// Player Disconnected 
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		if (Replication.IsClient())
			return;
		
		// Get player name
		playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		
		
	}
	
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		super.OnGameModeEnd(data);
		if (Replication.IsClient())
			return;
		
		// Close connection?
		
	}
}