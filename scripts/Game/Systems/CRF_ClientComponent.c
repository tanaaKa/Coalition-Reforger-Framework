[ComponentEditorProps(category: "Client Component", description: "")]
class CRF_ClientComponentClass: ScriptComponentClass
{
	
}

class CRF_ClientComponent: ScriptComponent
{
	string m_sHintText = "Type Here";
	
	ref array<string> m_aScriptedMarkers = new array<string>; 
	
	protected CRF_GamemodeComponent m_gamemodeComponent;
	
	//------------------------------------------------------------------------------------------------

	// override/static functions

	//------------------------------------------------------------------------------------------------
	
	static CRF_ClientComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return CRF_ClientComponent.Cast(GetGame().GetPlayerController().FindComponent(CRF_ClientComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner) 
	{
		super.OnPostInit(owner);
		
		if (!GetGame().InPlayMode() || RplSession.Mode() == RplMode.Dedicated) 
			return;

		
		GetGame().GetInputManager().AddActionListener("CRF_ToggleSideReady", EActionTrigger.DOWN, ToggleSideReady);
		GetGame().GetInputManager().AddActionListener("CRF_AdminForceReady", EActionTrigger.DOWN, AdminForceReady);
	
		SCR_PlayerController.Cast(owner).m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		SCR_PlayerControllerGroupComponent.Cast(SCR_PlayerController.Cast(owner).FindComponent(SCR_PlayerControllerGroupComponent)).GetOnGroupChanged().Insert(UpdateLocalPlayerGroup);
		GetGame().GetCallqueue().CallLater(WaitTillGameStart, 500, true);
	}
	
	//------------------------------------------------------------------------------------------------
	//\***********************************************************************************************
	//\***********************************************************************************************
	
	// Specific Gamemode Functions
	
	//\***********************************************************************************************
	//\***********************************************************************************************
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	// Functions for Search and Destroy
	//------------------------------------------------------------------------------------------------
	
	void Owner_ToggleBombPlanted(string sitePlanted, bool togglePlanted)
	{	
		Rpc(RpcAsk_ToggleBombPlanted, sitePlanted, togglePlanted);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ToggleBombPlanted(string sitePlanted, bool togglePlanted)
	{
		CRF_SearchAndDestroyGameModeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_SearchAndDestroyGameModeComponent)).ToggleBombPlanted(sitePlanted, togglePlanted);
	}
	
	//------------------------------------------------------------------------------------------------
	// Functions for Frontline
	//------------------------------------------------------------------------------------------------
	
	void UpdateMapMarkers(array<string> zoneStatus, array<string> zoneObjectNames, FactionKey bluforSide, FactionKey opforSide)
	{	
		RemoveALLScriptedMarkers();
		
		foreach(int i, string zoneName : zoneObjectNames)
		{
			string status = zoneStatus[i];
			string imageTexture;
			int imageColor;
			
			array<string> zoneStatusArray = {};
			status.Split(":", zoneStatusArray, false);
			
			string zoneLocked = zoneStatusArray[1];
			FactionKey zoneFactionStored = zoneStatusArray[2];
			
			switch(i)
			{
				case 0 : {imageTexture = "{21A2A457BD0E42C1}UI\Objectives\A.edds"; break;};
				case 1 : {imageTexture = "{7F4A8D140283CCCE}UI\Objectives\B.edds"; break;};
				case 2 : {imageTexture = "{8B42CA8C0F5EA4BA}UI\Objectives\C.edds"; break;};
				case 3 : {imageTexture = "{C29ADF937D98D0D0}UI\Objectives\D.edds"; break;};
				case 4 : {imageTexture = "{3692980B7045B8A4}UI\Objectives\E.edds"; break;};
			}
			
			if(zoneLocked == "Locked")
				AddScriptedMarker(zoneName, "0 0 0", 0, "", "{91427B7866707601}UI\Objectives\lock.edds", 50, ARGB(255, 142, 142, 142));
			
			switch(zoneFactionStored)
			{
				case bluforSide : {imageColor = ARGB(255, 0, 25, 225);     break;};  //Blufor
				case opforSide  : {imageColor = ARGB(255, 225, 25, 0);     break;};  //Opfor
				default         : {imageColor = ARGB(255, 225, 225, 225);  break;};  //Uncaptured
			}
			
			AddScriptedMarker(zoneName, "0 0 0", 0, "", imageTexture, 45, imageColor);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Functions for scripted markers
	//------------------------------------------------------------------------------------------------
	
	TStringArray GetScriptedMarkersArray()
	{
		return m_aScriptedMarkers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! !LOCAL! Adds a scripted marker on the users map which will follow the specified entity
	//! \param[in] markerEntityName is the name of the entity the marker will track.
	//! \param[in] markerOffset is the offset from the marker entity. (This can also be the vector pos for a static marker, simply set the "markerEntityName" param to "Static Marker").
	//! \param[in] timeDelay is the delay between marker updates.
	//! \param[in] markerText is the text that will be displayed on the map just under the image.
	//! \param[in] markerImage is the image that will be displayed on the map.
	//! \param[in] markerColor is the color the image will be.
	void AddScriptedMarker(string markerEntityName, vector markerOffset, int timeDelay, string markerText, string markerImage, int zOrder, int markerColor)
	{
		m_aScriptedMarkers.Insert(string.Format("%1||%2||%3||%4||%5||%6||%7", markerEntityName, markerOffset.ToString(), timeDelay.ToString(), markerText, markerImage, zOrder.ToString(), markerColor.ToString()));
	}

	//------------------------------------------------------------------------------------------------
	//! !LOCAL! Removes the scripted marker from the users map, must have all params be the same params that were initially put in the AddScriptedMarkers function
	void RemoveScriptedMarker(string markerEntityName, vector markerOffset, int timeDelay, string markerText, string markerImage, int zOrder, int markerColor)
	{
		m_aScriptedMarkers.RemoveItemOrdered(string.Format("%1||%2||%3||%4||%5||%6||%7", markerEntityName, markerOffset.ToString(), timeDelay.ToString(), markerText, markerImage, zOrder.ToString(), markerColor.ToString()));
	}
	
	//------------------------------------------------------------------------------------------------
	//! !LOCAL! Removes all scripted markers from the users maps
	void RemoveALLScriptedMarkers()
	{
		m_aScriptedMarkers.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//\***********************************************************************************************
	//\***********************************************************************************************
	
	// Admin Menu
	
	//\***********************************************************************************************
	//\***********************************************************************************************
	//------------------------------------------------------------------------------------------------
	
	void SendAdminMessage(string data)
	{
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		chatComponent.ShowMessage(string.Format("Message Sent: \"%1\"", data));
		data = string.Format("PlayerID: %1 | Player Name: %3 | \"%2\"", GetGame().GetPlayerController().GetPlayerId(), data, GetGame().GetPlayerManager().GetPlayerName(GetGame().GetPlayerController().GetPlayerId()));
		Rpc(RpcAsk_SendAdminMessage, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendAdminMessage(string data)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SendAdminMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReplyAdminMessage(string data)
	{
		ref array<string> dataSplit = {};
		data.Split(" ", dataSplit, false);
		int playerID;
		string toSend;
		for(int i = 0; i < dataSplit.Count(); i++)
		{
			if(dataSplit[i] == "0")
			{
				dataSplit.RemoveOrdered(i);
				playerID = 0;
				toSend = SCR_StringHelper.Join(" ", dataSplit, true);
				break;
			}
			
			if(dataSplit[i].ToInt() > 0)
			{
				playerID = dataSplit[i].ToInt();
				dataSplit.RemoveOrdered(i);
				toSend = SCR_StringHelper.Join(" ", dataSplit, true);
				break;
			}
		}	
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		if(!playerID)
		{
			chatComponent.ShowMessage("INVALID PLAYER ID");
			return;
		}
		if(!GetGame().GetPlayerManager().GetPlayerName(playerID))
		{
			chatComponent.ShowMessage("INVALID PLAYER ID");
			return;
		}
				
		chatComponent.ShowMessage(string.Format("Message Sent to %2: \"%1\"", toSend, GetGame().GetPlayerManager().GetPlayerName(playerID)));
		toSend = string.Format("\"%1\"", toSend);
		Rpc(RpcAsk_ReplyAdminMessage, toSend, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ReplyAdminMessage(string data, int playerID)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.ReplyAdminMessage(data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Respawn
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SpawnGroup(int playerID, string prefab, vector spawnLocation, int groupID)
	{
		Rpc(RpcAsk_SpawnGroup, playerID, prefab, spawnLocation, groupID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SpawnGroup(int playerID, string prefab, vector spawnLocation, int groupID)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SpawnGroupServer(playerID, prefab, spawnLocation, groupID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Gear
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ResetGear(int playerID, string prefab)
	{
		Rpc(RpcAsk_ResetGear, playerID, prefab);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ResetGear(int playerID, string prefab)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SetPlayerGear(playerID, prefab);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddItem(int playerID, string prefab)
	{
		Rpc(RpcAsk_AddItem, playerID, prefab);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_AddItem(int playerID, string prefab)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.AddItem(playerID, prefab);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teleport
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void TeleportLocalPlayer(int playerID1, int playerID2)
	{
		IEntity entity2 = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID2);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
	    spawnParams.TransformMode = ETransformMode.WORLD;
		vector teleportLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(teleportLocation, entity2.GetOrigin(), 3);
	    spawnParams.Transform[3] = teleportLocation;
	
		SCR_Global.TeleportPlayer(playerID1, teleportLocation);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void TeleportPlayers(int playerID1, int playerID2)
	{
		Rpc(RpcAsk_TeleportPlayers, playerID1, playerID2);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_TeleportPlayers(int playerID1, int playerID2)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.TeleportPlayers(playerID1, playerID2);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Hint
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintAll(string data)
	{
		Rpc(RpcAsk_SendHintAll, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendHintAll(string data)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SendHintAll(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintPlayer(string data, int playerID)
	{
		Rpc(RpcAsk_SendHintPlayer, data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendHintPlayer(string data, int playerID)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SendHintPlayer(data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintFaction(string data, string factionKey)
	{
		Rpc(RpcAsk_SendHintFaction, data, factionKey);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SendHintFaction(string data, string factionKey)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.SendHintFaction(data, factionKey);
	}
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Heal
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayer(int playerID)
	{
		Rpc(RpcAsk_HealPlayer, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_HealPlayer(int playerID)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.HealPlayer(playerID);
	}
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayerVehicle(int playerID)
	{
		Rpc(RpcAsk_HealPlayerVehicle, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_HealPlayerVehicle(int playerID)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.HealPlayerVehicle(playerID);
	}
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Log Admin Action
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		Rpc(RpcAsk_LogAdminAction, data, playerID, sendToPlayer);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		m_gamemodeComponent = CRF_GamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_GamemodeComponent));
		m_gamemodeComponent.LogAdminAction(data, playerID, sendToPlayer);
	}
	
	//------------------------------------------------------------------------------------------------
	//\***********************************************************************************************
	//\***********************************************************************************************
	
	// Gearscript
	
	//\***********************************************************************************************
	//\***********************************************************************************************
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void WaitTillGameStart()
	{
		if(!SCR_BaseGameMode.Cast(GetGame().GetGameMode()).IsRunning())
			return;
		
		GetGame().GetCallqueue().Remove(WaitTillGameStart);
		GetGame().GetCallqueue().CallLater(DelayUpdate, 650, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayUpdate()
	{
		if(SCR_PlayerController.GetLocalMainEntity().GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
		{
			OnControlledEntityChanged(SCR_PlayerController.GetLocalMainEntity(), SCR_PlayerController.GetLocalMainEntity());
			UpdateLocalPlayerGroup(SCR_GroupsManagerComponent.GetInstance().GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId()).GetGroupID());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if(!to.GetPrefabData().GetPrefabName().Contains("CRF_GS_"))
			return;
		
		Rpc(RpcAsk_UpdatePlayerGearScriptMap, to.GetPrefabData().GetPrefabName(), SCR_PlayerController.GetLocalPlayerId(), "GSR"); // GSR = Gear Script Resource
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateLocalPlayerGroup(int groupId)
	{
		if(groupId <= 0)
			return;
		
		Rpc(RpcAsk_UpdatePlayerGearScriptMap, groupId.ToString(), SCR_PlayerController.GetLocalPlayerId(), "GID"); // GID = GROUP ID    
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_UpdatePlayerGearScriptMap(string value, int playerID, string key)
	{
		CRF_GamemodeComponent.GetInstance().SetPlayerGearScriptsMapValue(value, playerID, key);
	}
	
	//------------------------------------------------------------------------------------------------
	//\***********************************************************************************************
	//\***********************************************************************************************
	
	// Safestart
	
	//\***********************************************************************************************
	//\***********************************************************************************************
	//------------------------------------------------------------------------------------------------
	
	void ToggleSideReady() 
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if(!groupManager) return;
		
		SCR_AIGroup playersGroup = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if(!playersGroup) return;
		
		string playerName = GetGame().GetPlayerManager().GetPlayerName(SCR_PlayerController.GetLocalPlayerId());
		
		if (!playerName || playerName == "") return;
		
		if (playersGroup.IsPlayerLeader(SCR_PlayerController.GetLocalPlayerId())) 
			Owner_ToggleSideReady(playerName);
	}
	
	void AdminForceReady()
	{		
		if (!SCR_Global.IsAdmin()) return;
		Rpc(RpcAsk_ToggleSideReady, "", GetGame().GetPlayerManager().GetPlayerName(SCR_PlayerController.GetLocalPlayerId()), true);
	}
	
	//------------------------------------------------------------------------------------------------
	void Owner_ToggleSideReady(string playerName)
	{	
		string setReady = "";
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		Faction faction = factionManager.GetPlayerFaction(SCR_PlayerController.GetLocalPlayerId());
			
		Color factionColor = faction.GetFactionColor();
		float rg = Math.Max(factionColor.R(), factionColor.G());
		float rgb = Math.Max(rg, factionColor.B());
			
		switch (true) {
			case(rgb == factionColor.R()) : {setReady = "Opfor";  break;};
			case(rgb == factionColor.G()) : {setReady = "Indfor"; break;};
			case(rgb == factionColor.B()) : {setReady = "Blufor"; break;};
		};
		
		if (setReady == "") return;
		Rpc(RpcAsk_ToggleSideReady, setReady, playerName, false);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_ToggleSideReady(string setReady, string playerName, bool adminForced)
	{
		CRF_GamemodeComponent.GetInstance().ToggleSideReady(setReady, playerName, adminForced);
	}
	
	//------------------------------------------------------------------------------------------------
	//\***********************************************************************************************
	//\***********************************************************************************************
	
	// Radio  Respawn
	
	//\***********************************************************************************************
	//\***********************************************************************************************
	//------------------------------------------------------------------------------------------------
	
	void SpawnGroupServer(int groupID)
	{
		Rpc(RpcAsk_SpawnGroupServer, groupID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_SpawnGroupServer(int groupID)
	{
//		CRF_RadioRespawnSystemComponent m_radioComponent = CRF_RadioRespawnSystemComponent.Cast(GetGame().GetGameMode().FindComponent(CRF_RadioRespawnSystemComponent));
//		m_radioComponent.SpawnGroupServer(groupID);
	}
}