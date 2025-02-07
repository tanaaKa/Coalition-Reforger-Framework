modded class SCR_EditorManagerEntity
{
	
	override bool CanOpen()
	{
		if (GetCurrentMode() == EEditorMode.PHOTO && SCR_PlayerController.GetLocalControlledEntity().GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
		{
			SetIsLimited(false);
			return true;
		}
		else if (!SCR_Global.IsAdmin(SCR_PlayerController.GetLocalPlayerId()))
			SetIsLimited(true);
		//--- No modes available
		if (m_Modes.IsEmpty())
			return false;
		
		if (IsLimited())
			return m_CanOpen == m_CanOpenSum;
		else
			return m_CanOpen | EEditorCanOpen.ALIVE == m_CanOpenSum;
	}
	
	void SetIsLimited(bool input)
	{
		m_bIsLimited = input;
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	override protected void ToggleServer(bool open)
	{
		if (m_bIsOpened == open || RplSession.Mode() == RplMode.Client) return;
		// Attempting to close when it's not allowed
		if (!open && !CanClose())
		{
			SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.EDITOR_CANNOT_CLOSE);
			return;
		}
		
		m_bIsOpened = open;
		Rpc(ToggleOwner, m_bIsOpened);
		
		if (m_bIsOpened)
		{
			if (m_CurrentModeEntity)
				m_CurrentModeEntity.ActivateModeServer();
			
			Event_OnOpenedServer.Invoke();
		}
		else
		{
			if (m_CurrentModeEntity)
				m_CurrentModeEntity.DeactivateModeServer();
			
			Event_OnClosedServer.Invoke();
		}
	}
	
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		super.StartEvents(type);
		
		if(type == EEditorEventOperation.OPEN)
		{	
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_PreviewMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SlottingMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_SpectatorMenu);
			GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.CRF_AARMenu);
			
		/* From CRFs OG Override of SCR_EditorManagerEntity
			if(!SCR_PlayerController.GetLocalControlledEntity())
				return;
			
			PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(SCR_PlayerController.Cast(GetGame().GetPlayerController()).FindComponent(PS_PlayableControllerComponent));
			
			playableController.ZeusClose();
		*/
		}
		else if(type == EEditorEventOperation.CLOSE)
		{	
			if(CRF_Gamemode.GetInstance())
			{
				GetGame().GetCallqueue().CallLater(OpenUI, 500, false);
			}
		}
	}
	
	void OpenUI()
	{
		CRF_Gamemode gamemode = CRF_Gamemode.GetInstance();
		if(gamemode)
			if(SCR_PlayerController.GetLocalControlledEntity().FindComponent(CRF_PlayableCharacter))
			{
				if(!CRF_PlayableCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity().FindComponent(CRF_PlayableCharacter)).IsPlayable() && SCR_PlayerController.GetLocalControlledEntity().GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
					return;
			}
			else if (SCR_PlayerController.GetLocalControlledEntity().GetPrefabData().GetPrefabName() != "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
				return;

		switch(CRF_Gamemode.GetInstance().m_GamemodeState)
		{
			case CRF_GamemodeState.INITIAL: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_PreviewMenu);	break;}
			case CRF_GamemodeState.SLOTTING: 	{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_SlottingMenu);	break;}
			case CRF_GamemodeState.GAME: 		{SCR_PlayerController.Cast(GetGame().GetPlayerController()).EnterGame(SCR_PlayerController.GetLocalPlayerId());	break;}
			case CRF_GamemodeState.AAR: 		{GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CRF_AARMenu);	break;}
		}
	}
}