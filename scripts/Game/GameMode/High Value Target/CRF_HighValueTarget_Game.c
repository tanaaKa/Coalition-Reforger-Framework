[ComponentEditorProps(category: "Game Mode Component", description: "")]
class CRF_HighValueTargetGameModeComponentClass: SCR_BaseGameModeComponentClass
{
	
}
class CRF_HighValueTargetGameModeComponent: SCR_BaseGameModeComponent
{
	[Attribute("360", "auto", "The amount of time between marker updates in seconds.")]
	int m_timeBetweenPings;
	
	[Attribute("false", "auto", "Set the character to unconcious state.")]
	bool m_setUnconcious;
	
	[Attribute("false", "auto", "Disable damage on the entity.")]
	bool m_disableDamage;
	
	[Attribute("transponder", "auto", "The entity that being tracked.")]
	string m_transponderEntity;
	
	[Attribute("Transponder Signal", "auto", "The text of the marker on the map.")]
	string m_markerText;
	
	[Attribute("false", "auto", "Hide the tranponder to all other faction except the one below.")]
	bool m_filterFaction;

	[Attribute("BLUFOR", "auto", "Faction key for the searching side.")]
	string m_searcherFactionKey;
	
	[Attribute("{42A502E3BB727CEB}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_HeliPilot.et", desc: "The visual prefab of the transponder.", uiwidget: "resourcePickerThumbnail", params: "et")]
	ResourceName m_hvtPrefab;
	
	[Attribute("0 0 0", "auto", "The rotation of the prefab")]
	 vector m_hvtPrefabYaw;
	
	CLB_Gamemode m_safestart;
		
	[RplProp(onRplName: "updateHvtPos")]
	vector m_sHvtPos;
	vector m_sStoredHvtPos;
	
	IEntity m_eHvtEntity;
	
	SCR_PopUpNotification m_PopUpNotification = null;
	string m_PopUpNotificationMessage

	//------------------------------------------------------------------------------------------------
	
	override protected void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode()) 
			return;
		
		GetGame().GetCallqueue().CallLater(WaitTillGameStart, 1000, true);
		
	}
	
	//------------------------------------------------------------------------------------------------
	// Spawn HVT & Set State
	//------------------------------------------------------------------------------------------------
	void WaitTillGameStart()
	{
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning()) 
		{
			GetGame().GetCallqueue().Remove(WaitTillGameStart);
			GetGame().GetCallqueue().CallLater(WaitTillSafeStartEnds, 1000, true);
			IEntity transponderEntity = GetGame().GetWorld().FindEntityByName(m_transponderEntity);
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = transponderEntity.GetOrigin();
			m_eHvtEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_hvtPrefab),GetGame().GetWorld(),spawnParams);
			
			if (Replication.IsServer())
			{
				m_eHvtEntity.SetYawPitchRoll(m_hvtPrefabYaw);
				
				SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_eHvtEntity.FindComponent(SCR_CharacterControllerComponent));
				if (!characterController)
				return;
				
				characterController.m_OnPlayerDeathWithParam.Insert(HVTKilled);
			}
			
			if (m_disableDamage && Replication.IsServer()) 
			{
				SCR_CharacterDamageManagerComponent damangeMangerController = SCR_CharacterDamageManagerComponent.Cast(m_eHvtEntity.FindComponent(SCR_CharacterDamageManagerComponent));
				if (!damangeMangerController)
					return;
				
				damangeMangerController.EnableDamageHandling(false);
			}
			
			if (m_setUnconcious && Replication.IsServer()) 
			{			
				SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_eHvtEntity.FindComponent(SCR_CharacterControllerComponent));
				if (!characterController)
					return;
				
				setHVTUnconcious();
				characterController.m_OnLifeStateChanged.Insert(setHVTUnconcious);
				
			}	
		}
		return;
	}
	
	//------------------------------------------------------------------------------------------------
	// Scripted Marker
	//------------------------------------------------------------------------------------------------	
	
	void WaitTillSafeStartEnds()
	{
		m_safestart = CLB_Gamemode.GetInstance();
		if (!m_safestart.GetSafestartStatus())
		{	
			GetGame().GetCallqueue().Remove(WaitTillSafeStartEnds);
			if (Replication.IsServer())
			{
				GetGame().GetCallqueue().CallLater(transponderInit, 1000, true);
			}
			
			CRF_ClientComponent gameModePlayerComponent = CRF_ClientComponent.GetInstance();
				if (!gameModePlayerComponent) 
					return;
				
			if (m_filterFaction)
			{
				SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
				if (!factionManager)
					return;
				
				Faction faction = factionManager.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
				if (!faction)
					return;
		        
		        if (faction.GetFactionKey() == m_searcherFactionKey)  
				{
					gameModePlayerComponent.AddScriptedMarker(m_transponderEntity, "0 0 0", m_timeBetweenPings, m_markerText, "{428583D4284BC412}UI/Textures/Editor/EditableEntities/Waypoints/EditableEntity_Waypoint_SearchAndDestroy.edds", 50, ARGB(255, 0, 0, 225));
				}
			} else {
				gameModePlayerComponent.AddScriptedMarker(m_transponderEntity, "0 0 0", m_timeBetweenPings, m_markerText, "{428583D4284BC412}UI/Textures/Editor/EditableEntities/Waypoints/EditableEntity_Waypoint_SearchAndDestroy.edds", 50, ARGB(255, 0, 0, 225));
			}
		}
		return;

	}
	
	//------------------------------------------------------------------------------------------------
	// Transponder Location Update
	//------------------------------------------------------------------------------------------------
	
	void transponderInit()
	{
		if (SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning()) 
		{
			m_sHvtPos = m_eHvtEntity.GetOrigin();
			Replication.BumpMe();
			updateHvtPos();
		}
		return;
	}
	
	void updateHvtPos()
	{	
			if (m_sHvtPos == m_sStoredHvtPos)
				return;
	
			IEntity transponder = GetGame().GetWorld().FindEntityByName(m_transponderEntity);	
			transponder.SetOrigin(m_sHvtPos);
			m_sStoredHvtPos = m_sHvtPos;
	};
	
	//------------------------------------------------------------------------------------------------
	// HVT Unconcious State
	//------------------------------------------------------------------------------------------------
	
	void setHVTUnconcious()
	{
		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(m_eHvtEntity.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
			return;
		
		SCR_CharacterDamageManagerComponent damangeMangerController = SCR_CharacterDamageManagerComponent.Cast(m_eHvtEntity.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!damangeMangerController)
			return;
		
		characterController.SetUnconscious(true);
		damangeMangerController.SetRegenScale(0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// Pop up notification
	//------------------------------------------------------------------------------------------------
	
	void HVTKilled(SCR_CharacterControllerComponent characterController, IEntity killerEntity, Instigator killer)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
	
		Faction faction = factionManager.GetPlayerFaction(killer.GetInstigatorPlayerID());
		if (!faction)
			return;
		
		Rpc(RpcAsk_PopUpNotification, faction.GetFactionKey())
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_PopUpNotification(string faction)
	{
		AudioSystem.PlaySound("{E23715DAF7FE2E8A}Sounds/Items/Equipment/Radios/Samples/Items_Radio_Turn_On.wav");
		m_PopUpNotificationMessage = "HVT Killed by " + faction;
		m_PopUpNotification = SCR_PopUpNotification.GetInstance();
		m_PopUpNotification.PopupMsg(m_PopUpNotificationMessage, 10, "");
	}
	
}