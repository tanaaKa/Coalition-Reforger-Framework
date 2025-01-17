modded enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	CRF_SpectatorMenu
}

class CRF_SpectatorMenuUI: ChimeraMenuBase
{
	protected ref array<RplId> m_aEntityIcons = {};
	protected ref array<Widget> m_aSpectatorWidgets = {};
	protected ref array<ref CRF_SpectatorLabelIconCharacter> m_aSpectatorIcons = {};
	protected CRF_Gamemode m_Gamemode;
	protected SCR_ChatPanel m_ChatPanel;
	protected SCR_MapEntity m_MapEntity;
	protected bool m_bIsMapOpened = false;
	protected Widget m_wRoot;
	protected FrameWidget m_wMapFrame;
	
	override void OnMenuOpen()
	{
		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		m_wRoot = GetRootWidget();
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		m_Gamemode = CRF_Gamemode.GetInstance();
		m_wMapFrame = FrameWidget.Cast(m_wRoot.FindAnyWidget("MapFrame"));
		GetGame().GetInputManager().AddActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
		GetGame().GetInputManager().AddActionListener("GadgetMap", EActionTrigger.DOWN, Action_ToggleMap);
		GetGame().GetInputManager().AddActionListener("ManualCameraTeleport", EActionTrigger.DOWN, Action_ManualCameraTeleport);
		GetGame().GetInputManager().AddActionListener("ShowScoreboard", EActionTrigger.DOWN, OnShowPlayerList);
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		if (m_MapEntity)
			GetGame().GetInputManager().ActivateContext("MapContext");
		
		foreach(RplId entityID: m_Gamemode.m_aCharacters)
		{
			if(!Replication.FindItem(entityID))
			{
				int index = m_aEntityIcons.Find(entityID);
				if(index == -1)
					continue;
				
				m_aEntityIcons.RemoveOrdered(index);
				delete m_aSpectatorWidgets.Get(index);
				m_aSpectatorWidgets.RemoveOrdered(index);
				m_aSpectatorIcons.RemoveOrdered(index);
				continue;
			}
			
			IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
			if(m_aEntityIcons.Find(entityID) != -1)
				continue;
			
			Widget spectatorIconWidget = GetGame().GetWorkspace().CreateWidgets("{68625BAD23CEE68F}UI/Spectator/SpectatorLabelIconCharacter.layout", FrameWidget.Cast(GetRootWidget().FindAnyWidget("IconsFrame")));
			CRF_SpectatorLabelIconCharacter spectatorIcon = CRF_SpectatorLabelIconCharacter.Cast(spectatorIconWidget.FindHandler(CRF_SpectatorLabelIconCharacter));
			spectatorIcon.SetEntity(entity, "Spine3");
			m_aEntityIcons.Insert(entityID);
			m_aSpectatorIcons.Insert(spectatorIcon);
			m_aSpectatorWidgets.Insert(spectatorIconWidget);
		}
		
		UpdateIcons();
		
		if (m_ChatPanel)
        	m_ChatPanel.OnUpdateChat(tDelta);
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_NotificationSenderComponent));
		sender.SetKillFeedTypeDeadLocal();
	}
	
	override void OnMenuInit()
	{		
		m_wRoot = GetRootWidget();
		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();
		
		m_wRoot.FindAnyWidget("MapFrame").SetVisible(false);
	}
	
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
		GetGame().GetInputManager().RemoveActionListener("GadgetMap", EActionTrigger.DOWN, Action_ToggleMap);
		GetGame().GetInputManager().RemoveActionListener("ManualCameraTeleport", EActionTrigger.DOWN, Action_ManualCameraTeleport);
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard", EActionTrigger.DOWN, OnShowPlayerList);
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_NotificationSenderComponent));
		sender.SetKillFeedTypeNoneLocal();
	}
	
	// Add player list to spectator
	protected static void OnShowPlayerList()
	{
		ArmaReforgerScripted.OpenPlayerList();
	}
	
	// Teleporting players camera
	void Action_ManualCameraTeleport()
	{
		MoveCamera(GetCursorWorldPos());
	}
	
	void MoveCamera(vector worldPosition)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (camera)
		{
			SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
			if (teleportComponent)
			{
				teleportComponent.TeleportCamera(worldPosition, true, false);
			}
		}
	}
	
	vector GetCursorWorldPos()
	{
		ArmaReforgerScripted game = GetGame();
		WorkspaceWidget workspace = game.GetWorkspace();
		BaseWorld world = game.GetWorld();
		
		vector worldPos = "0 0 0";
		
		// If map is open return map cursor world position
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity && mapEntity.IsOpen())
		{
			float worldX, worldY;
			mapEntity.GetMapCursorWorldPosition(worldX, worldY);
			worldPos[0] = worldX;
			worldPos[2] = worldY;
			worldPos[1] = world.GetSurfaceY(worldPos[0], worldPos[2]);
			
			return worldPos;
		}
		
		
		vector cursorPos = GetCursorPos();
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(cursorPos[0], cursorPos[1], outDir, world, -1);
		outDir *= 1000.0;
	
		autoptr TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.ANY_CONTACT | TraceFlags.WORLD | TraceFlags.ENTS | TraceFlags.OCEAN; 
		
		float traceDis = world.TraceMove(trace, null);
		worldPos = startPos + outDir * traceDis;
		
		return worldPos;
	}
	
	vector GetCursorPos()
	{
		int cursorX, cursorY;
		if (GetGame().GetInputManager() && GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			WidgetManager.GetMousePos(cursorX, cursorY);
			return Vector(GetGame().GetWorkspace().DPIUnscale(cursorX), GetGame().GetWorkspace().DPIUnscale(cursorY), 0);
		} else {
			return Vector(GetGame().GetWorkspace().DPIUnscale(GetGame().GetWorkspace().GetWidth()/2.0), GetGame().GetWorkspace().DPIUnscale(GetGame().GetWorkspace().GetHeight()/2.0), 0);
		}
	}

	//From RL
	RadioTransceiver GetVoNTransiver()
	{
		IEntity entity = GetGame().GetPlayerController().GetControlledEntity();
		ref array<IEntity> items = {};
		SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent)).GetItems(items);
		IEntity radioEntity;
		foreach(IEntity item: items)
		{
			if(item.FindComponent(BaseRadioComponent))
				radioEntity = item;
		}
		BaseRadioComponent radio = BaseRadioComponent.Cast(radioEntity.FindComponent(BaseRadioComponent));
		radio.SetPower(true);
		RadioTransceiver transiver = RadioTransceiver.Cast(radio.GetTransceiver(0));
		transiver.SetFrequency(1);
		return transiver;
	}
	
	void Action_VONon()
	{
		GetGame().GetCallqueue().Remove(LobbyVoNDisableDelayed);
		SCR_VoNComponent von = SCR_VoNComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(SCR_VoNComponent));
		von.SetTransmitRadio(GetVoNTransiver());
		von.SetCommMethod(ECommMethod.SQUAD_RADIO);
		von.SetCapture(true);
	}
	
	//From reforger lobby <3
	void Action_VONOff()
	{
		GetGame().GetCallqueue().CallLater(LobbyVoNDisableDelayed, 400);
	}
	
	void LobbyVoNDisableDelayed()
	{
		SCR_VoNComponent von = SCR_VoNComponent.Cast(GetGame().GetPlayerController().GetControlledEntity().FindComponent(SCR_VoNComponent));
		von.SetCommMethod(ECommMethod.DIRECT);
		von.SetCapture(false);
	}
	
	void UpdateIcons()
	{
		foreach(CRF_SpectatorLabelIconCharacter spectatorIcon: m_aSpectatorIcons)
		{
			spectatorIcon.Update();
		}
	}
	
	void Action_OnChatToggleAction()
	{
		if (!m_ChatPanel)
			return;
		
		// Frame delay
		GetGame().GetCallqueue().CallLater(OpenChatWrap, 5);
	}
	
	void OpenChatWrap()
	{
		if (!m_ChatPanel.IsOpen())
		{
			SCR_ChatPanelManager.GetInstance().OpenChatPanel(m_ChatPanel);
		}
	}
	void Action_Exit()
	{
		// For some strange reason players all the time accidentally exit game, maybe jus open pause menu
		//GameStateTransitions.RequestGameplayEndTransition();  
		//Close();
		GetGame().GetCallqueue().CallLater(OpenPauseMenuWrap, 0); //  Else menu auto close itself
	}
	void OpenPauseMenuWrap()
	{
		ArmaReforgerScripted.OpenPauseMenu();
	}
	
	//More RL code
	void Action_ToggleMap()
	{
		if (!m_MapEntity.IsOpen()) 
			OpenMap();
		else 
			CloseMap();
	}
	
	void OpenMap()
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (camera)
			camera.SetInputEnabled(false);
		
		m_wMapFrame.SetVisible(true);
		
		// WUT?
		SCR_WidgetHelper.RemoveAllChildren(GetRootWidget().FindAnyWidget("ToolMenuHoriz"));
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;
		
		MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(EMapEntityMode.FULLSCREEN, configComp.GetEditorMapConfig(), m_wMapFrame);
		m_MapEntity.OpenMap(mapConfigFullscreen);
	}
	
	void CloseMap()
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(GetGame().GetCameraManager().CurrentCamera());
		if (camera)
			camera.SetInputEnabled(true);
		
		m_wMapFrame.SetVisible(false);
		
		m_MapEntity.CloseMap();
	}
} 