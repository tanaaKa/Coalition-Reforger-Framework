[ComponentEditorProps(category: "Game Mode Component", description: "")]
class CRF_SearchAndRescueGameModeComponentClass: SCR_BaseGameModeComponentClass
{
	
}
class CRF_SearchAndRescueGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("360", "auto", "The amount of time between marker updates in seconds.")]
	int m_timeBetweenPings;
	
	[Attribute("false", "auto", "Set the character to unconcious state.")]
	bool m_setUnconcious;
	
	[Attribute("transponder", "auto", "The entity that being tracked.")]
	string m_searchForUnit;
	
	[Attribute("Transponder Signal", "auto", "The text of the marker on the map.")]
	string m_markerText;
	
	[Attribute("BLUFOR", "auto", "Faction key for the searching side.")]
	string m_searcherFactionKey;
	
	[Attribute("{42A502E3BB727CEB}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_HeliPilot.et", "auto", "The visual prefab of the transponder. Must be moveable by the player.")]
	string pilotPrefab;
	
	CRF_SafestartGameModeComponent m_safestart;
		
	[RplProp(onRplName: "updatePilotPos")]
	vector m_sPilotPos;
	vector m_sStoredPilotPos;
	
	IEntity m_ePilotEntity;
	

	//------------------------------------------------------------------------------------------------
	
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode()) 
			return;
		
		GetGame().GetCallqueue().CallLater(WaitTillGameStart, 1000, true);
		
	}
	
	void WaitTillGameStart()
	{
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning()) 
		{
			IEntity transponderEntity = GetGame().GetWorld().FindEntityByName(m_searchForUnit);
			
			// Spawn pilot entity
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = transponderEntity.GetOrigin();
			
			m_ePilotEntity = GetGame().SpawnEntityPrefab(Resource.Load(pilotPrefab),GetGame().GetWorld(),spawnParams);
			
			// Knock the pilot uncon if needed
			if (m_setUnconcious && Replication.IsServer()) 
			{	
				SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_ePilotEntity.FindComponent(SCR_CharacterControllerComponent));
				characterController.SetUnconscious(true);
				SCR_CharacterDamageManagerComponent damangeMangerController = SCR_CharacterDamageManagerComponent.Cast(m_ePilotEntity.FindComponent(SCR_CharacterDamageManagerComponent));
				damangeMangerController.EnableDamageHandling(false)
				
			}
			
			GetGame().GetCallqueue().Remove(WaitTillGameStart);
			GetGame().GetCallqueue().CallLater(WaitTillSafeStartEnds, 1000, true);
		}
		return;
	}
	
	void WaitTillSafeStartEnds()
	{
		m_safestart = CRF_SafestartGameModeComponent.GetInstance();
		if (!m_safestart.GetSafestartStatus())
		{	
			GetGame().GetCallqueue().Remove(WaitTillSafeStartEnds);
			if (Replication.IsServer())
			{
				GetGame().GetCallqueue().CallLater(transponderInit, 1000, true);
			}
		}
		return;

	}
	
	void transponderInit()
	{
		// Update the transponder position to the entity being searched for across all the clients
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning()) 
		{
			
			m_sPilotPos = m_ePilotEntity.GetOrigin();
			
			Replication.BumpMe();
			updatePilotPos();
		}
		return;
	}
	
	// Function called by the server sent to the clients
	void updatePilotPos()
	{	
			if (m_sPilotPos == m_sStoredPilotPos)
			return;
	
			IEntity transponder = GetGame().GetWorld().FindEntityByName(m_searchForUnit);	
			// Update tranponder location on the client
			transponder.SetOrigin(m_sPilotPos);
				
			m_sStoredPilotPos = m_sPilotPos;
		
			// Create marker on transponder entity if in the search faction
			CRF_GameModePlayerComponent gameModePlayerComponent = CRF_GameModePlayerComponent.GetInstance();
			if (!gameModePlayerComponent) 
				return;
			
			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (!factionManager)
			return;
			
			Faction faction = factionManager.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			if (!faction)
			return;
	        
	        if (faction.GetFactionKey() == m_searcherFactionKey)  
			{
				gameModePlayerComponent.AddScriptedMarker(m_searchForUnit, "0 0 0", m_timeBetweenPings, m_markerText, "{428583D4284BC412}UI/Textures/Editor/EditableEntities/Waypoints/EditableEntity_Waypoint_SearchAndDestroy.edds", 50, ARGB(255, 0, 0, 225));
			}    
	};
	
}