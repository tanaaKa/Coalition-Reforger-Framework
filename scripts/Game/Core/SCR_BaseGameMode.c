modded class SCR_BaseGameMode
{
	// SERVER METHODS
	override void OnGameStart()
	{
		super.OnGameStart();
		
		// Setup and connect to the db
		
	}
	
	override void OnPlayerConnected(int playerId)
	{
		super.OnPlayerConnected(playerId);
		
		// Verify user has account and if not, tell them to register for stats
		
	}
	
	// GLOBAL METHODS 
	override void OnGameModeStart()
	{	
		super.OnGameModeStart();
		if (Replication.IsClient) return;
		
		// Create mission entry if players > 10 (QA testing)
		
	}
}