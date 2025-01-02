modded class CLB_SlottingMenuUI

	override void OnMenuOpen()
	{	
		if (RplSession.Mode() == RplMode.Dedicated) {
			Close();
			return;
		}
		
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().AddActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);

		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));
		
		m_wRoot = GetRootWidget();
		m_Gamemode = CLB_Gamemode.Cast(GetGame().GetGameMode());
		
		GetGame().GetInputManager().AddActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		
		m_LocalSlottingState = m_Gamemode.m_SlottingState;
		
		TextWidget missionText = TextWidget.Cast(m_wRoot.FindAnyWidget("MissionText"));
		if(GetGame().GetMissionName())
			missionText.SetText(GetGame().GetMissionName());
		else
			missionText.SetText("Unknown Mission");
		
		if(SCR_MissionHeader.Cast(GetGame().GetMissionHeader()))
			missionText.SetText(missionText.GetText() + " | By " + SCR_MissionHeader.Cast(GetGame().GetMissionHeader()).m_sAuthor);
		else
			missionText.SetText(missionText.GetText() + " | By " + "Unkown");
		
		string currentStateName = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetTimeAndWeatherManager().GetCurrentWeatherState().GetStateName();
		TextWidget.Cast(m_wRoot.FindAnyWidget("WeatherText")).SetText("Weather: " + currentStateName);
		
		m_wPreview = ImageWidget.Cast(m_wRoot.FindAnyWidget("PreviewBorder"));
		m_wSlotting = ImageWidget.Cast(m_wRoot.FindAnyWidget("SlottingBorder"));
		m_wGame = ImageWidget.Cast(m_wRoot.FindAnyWidget("GameBorder"));
		m_wAAR = ImageWidget.Cast(m_wRoot.FindAnyWidget("AARBorder"));
		
		int gameState = CLB_Gamemode.Cast(GetGame().GetGameMode()).m_GamemodeState; 
		switch(gameState)
		{
			case 0: {m_wPreview.SetColor(Color.FromRGBA(122, 0, 0, 255));	 break;}
			case 1: {m_wSlotting.SetColor(Color.FromRGBA(122, 0, 0, 255));	 break;}
			case 2: {m_wGame.SetColor(Color.FromRGBA(122, 0, 0, 255)); 	 	 break;}
			case 3: {m_wAAR.SetColor(Color.FromRGBA(122, 0, 0, 255)); 		 break;}
		}
		
		ButtonWidget previewButton = ButtonWidget.Cast(m_wRoot.FindAnyWidget("PreviewButton"));
		ButtonWidget gameButton = ButtonWidget.Cast(m_wRoot.FindAnyWidget("GameButton"));
		ButtonWidget aarButton = ButtonWidget.Cast(m_wRoot.FindAnyWidget("AARButton"));
		ButtonWidget advanceButton = ButtonWidget.Cast(m_wRoot.FindAnyWidget("Advance"));
			
		aarButton.SetEnabled(false);
		advanceButton.SetEnabled(false);
		gameButton.SetEnabled(false);
		m_wRoot.FindAnyWidget("UnslottedPlayers").SetOpacity(0);
		m_wRoot.FindAnyWidget("SlottingPhases").SetOpacity(0);
		FrameWidget.Cast(m_wRoot.FindAnyWidget("AdvanceFrame")).SetOpacity(0);
		
		if(m_Gamemode.m_GamemodeState == CLB_GamemodeState.GAME)
			gameButton.SetEnabled(true);
		
		SCR_ButtonTextComponent.Cast(gameButton.FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(EnterGame);
		SCR_ButtonTextComponent.Cast(previewButton.FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(OpenSlottingMenu);
		SCR_ButtonTextComponent.Cast(ButtonWidget.Cast(m_wRoot.FindAnyWidget("SlotPhaseButton")).FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(AdvanceSlottingPhase);
		SCR_ButtonTextComponent.Cast(advanceButton.FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(AdvanceMenu);
		
		m_cPlayerListBoxComponent = SCR_ListBoxComponent.Cast(OverlayWidget.Cast(m_wRoot.FindAnyWidget("PlayerList")).FindHandler(SCR_ListBoxComponent));
		m_cOrbatListBoxComponent = CLB_ListboxComponent.Cast(OverlayWidget.Cast(m_wRoot.FindAnyWidget("OrbatList")).FindHandler(CLB_ListboxComponent));
		m_cUnslotPlayerListBoxComponent = CLB_ListboxComponent.Cast(OverlayWidget.Cast(m_wRoot.FindAnyWidget("UnslotPlayerList")).FindHandler(CLB_ListboxComponent));

		m_cSlotListBoxComponent = CLB_ListboxComponent.Cast(OverlayWidget.Cast(m_wRoot.FindAnyWidget("RoleList")).FindHandler(CLB_ListboxComponent));
		InitSlots();
		
		CRF_GearScriptGamemodeComponent gsComp = CRF_GearScriptGamemodeComponent.GetInstance();	
		ResourceName icon;
	
		if(m_iBluforSlots > 0)
		{
			if(gsComp)
			{	
				ResourceName gearScriptResource = gsComp.GetGearScriptResource("BLUFOR");
				if(!gearScriptResource.IsEmpty())
				{
					CRF_GearScriptConfig gearConfig = CRF_GearScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(gearScriptResource).GetResource().ToBaseContainer()));
					if(gearConfig)
					{
						if(!gearConfig.m_FactionIcon.IsEmpty())
							icon = gearConfig.m_FactionIcon;
						else
							icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("BLUFOR")).GetFactionFlag();
					};
				};
			} else {
				icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("BLUFOR")).GetFactionFlag();
			};
			
			m_wRoot.FindAnyWidget("BluforButton").SetVisible(true);
			m_wRoot.FindAnyWidget("BluforBGSelect").SetVisible(true);
				
			ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagBlufor")).LoadImageTexture(1, icon);
			m_wRoot.FindAnyWidget("BluforBGSelect").SetColor(Color.FromRGBA(34, 196, 244, 33));
		};
		
		if(m_iOpforSlots > 0)
		{
			if(gsComp)
			{	
				ResourceName gearScriptResource = gsComp.GetGearScriptResource("OPFOR");
				if(!gearScriptResource.IsEmpty())
				{
					CRF_GearScriptConfig gearConfig = CRF_GearScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(gearScriptResource).GetResource().ToBaseContainer()));
					if(gearConfig)
					{
						if(!gearConfig.m_FactionIcon.IsEmpty())
							icon = gearConfig.m_FactionIcon;
						else
							icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("OPFOR")).GetFactionFlag();
					};
				};
			} else {
				icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("OPFOR")).GetFactionFlag();
			};
			
			m_wRoot.FindAnyWidget("OpforButton").SetVisible(true);
			m_wRoot.FindAnyWidget("OpforBGSelect").SetVisible(true);	
			
			ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagOpfor")).LoadImageTexture(1, SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("OPFOR")).GetFactionFlag());
			m_wRoot.FindAnyWidget("OpforBGSelect").SetColor(Color.FromRGBA(238, 49, 47, 33));
		};
		
		if(m_iIndforSlots > 0)
		{
			if(gsComp)
			{	
				ResourceName gearScriptResource = gsComp.GetGearScriptResource("INDFOR");
				if(!gearScriptResource.IsEmpty())
				{
					CRF_GearScriptConfig gearConfig = CRF_GearScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(gearScriptResource).GetResource().ToBaseContainer()));
					if(gearConfig)
					{
						if(!gearConfig.m_FactionIcon.IsEmpty())
							icon = gearConfig.m_FactionIcon;
						else
							icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("INDFOR")).GetFactionFlag();
					};
				};
			} else {
				icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("INDFOR")).GetFactionFlag();
			};
			
			m_wRoot.FindAnyWidget("IndforButton").SetVisible(true);
			m_wRoot.FindAnyWidget("IndforBGSelect").SetVisible(true);	
				
			ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagIndfor")).LoadImageTexture(1, SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("INDFOR")).GetFactionFlag());
			m_wRoot.FindAnyWidget("IndforBGSelect").SetColor(Color.FromRGBA(0, 177, 79, 33));
		};
		
		if(m_iCivSlots > 0)
		{
			if(gsComp)
			{	
				ResourceName gearScriptResource = gsComp.GetGearScriptResource("CIV");
				if(!gearScriptResource.IsEmpty())
				{
					CRF_GearScriptConfig gearConfig = CRF_GearScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(gearScriptResource).GetResource().ToBaseContainer()));
					if(gearConfig)
					{
						if(!gearConfig.m_FactionIcon.IsEmpty())
							icon = gearConfig.m_FactionIcon;
						else
							icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("CIV")).GetFactionFlag();
					};
				};
			} else {
				icon = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("CIV")).GetFactionFlag();
			};
			
			m_wRoot.FindAnyWidget("CivButton").SetVisible(true);
			m_wRoot.FindAnyWidget("CivBGSelect").SetVisible(true);	
				
			ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagCiv")).LoadImageTexture(1, SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByKey("CIV")).GetFactionFlag());
			m_wRoot.FindAnyWidget("CivBGSelect").SetColor(Color.FromRGBA(168, 110, 207, 33));
		};	
		
		CLB_Gamemode gamemode = CLB_Gamemode.GetInstance();
		
		if (gamemode.m_iFactionOneRatio > 0 && !gamemode.m_sFactionOneKey.IsEmpty())
		{
			EditBoxWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1")).SetText(gamemode.m_iFactionOneRatio.ToString());
			TextWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1Text")).SetText(gamemode.m_sFactionOneKey);
		
			Color colorOne;
			
			switch(gamemode.m_sFactionOneKey)
			{
				case "BLU" : colorOne = Color.FromRGBA(0, 20, 255, 255); break;
				case "OPF" : colorOne = Color.FromRGBA(188, 0, 0, 255); break;
				case "IND" : colorOne = Color.FromRGBA(0, 185, 63, 255); break;
				case "CIV" : colorOne = Color.FromRGBA(137, 0, 188, 255); break;
			}
			
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1Image")).SetColor(colorOne);
		}
		
		if (gamemode.m_iFactionTwoRatio > 0 && !gamemode.m_sFactionTwoKey.IsEmpty())
		{
			EditBoxWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2")).SetText(gamemode.m_iFactionTwoRatio.ToString());
			TextWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2Text")).SetText(gamemode.m_sFactionTwoKey);
		
			Color colorTwo;
			
			switch(gamemode.m_sFactionTwoKey)
			{
				case "BLU" : colorTwo = Color.FromRGBA(0, 20, 255, 255); break;
				case "OPF" : colorTwo = Color.FromRGBA(188, 0, 0, 255); break;
				case "IND" : colorTwo = Color.FromRGBA(0, 185, 63, 255); break;
				case "CIV" : colorTwo = Color.FromRGBA(137, 0, 188, 255); break;
			}
			
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2Image")).SetColor(colorTwo);
		}

		
		if (gamemode.m_iFactionTwoRatio < 0 || gamemode.m_sFactionTwoKey.IsEmpty() || gamemode.m_iFactionOneRatio < 0 || gamemode.m_sFactionOneKey.IsEmpty())
		{
			EditBoxWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1")).SetVisible(false);
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1Image")).SetVisible(false);
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1IntImage")).SetVisible(false);
			TextWidget.Cast(m_wRoot.FindAnyWidget("RatioBox1Text")).SetVisible(false);
			EditBoxWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2")).SetVisible(false);
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2Image")).SetVisible(false);
			ImageWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2IntImage")).SetVisible(false);
			TextWidget.Cast(m_wRoot.FindAnyWidget("RatioBox2Text")).SetVisible(false);
			ImageWidget.Cast(m_wRoot.FindAnyWidget("FinalImage")).SetVisible(false);
			TextWidget.Cast(m_wRoot.FindAnyWidget("Final")).SetVisible(false);
		}
		
		if(m_iBluforSlots > 0)
		{
			m_fSelectedFaction = GetGame().GetFactionManager().GetFactionByKey("BLUFOR");
			SelectFactionBlufor();
		}
		else if(m_iOpforSlots > 0)
		{
			m_fSelectedFaction = GetGame().GetFactionManager().GetFactionByKey("OPFOR");
			SelectFactionOpfor();
		}
		else if(m_iIndforSlots > 0)
		{
			m_fSelectedFaction = GetGame().GetFactionManager().GetFactionByKey("INDFOR");
			SelectFactionIndfor();
		}
		else if(m_iCivSlots > 0)
		{
			m_fSelectedFaction = GetGame().GetFactionManager().GetFactionByKey("CIV");
			SelectFactionOpfor();
		}
		localSlotChanges = m_Gamemode.m_iSlotChanges;
		UpdateSlots();
		SCR_ButtonTextComponent.Cast(ButtonWidget.Cast(m_wRoot.FindAnyWidget("ButtonBlufor")).FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(SelectFactionBlufor);
		SCR_ButtonTextComponent.Cast(ButtonWidget.Cast(m_wRoot.FindAnyWidget("ButtonOpfor")).FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(SelectFactionOpfor);
		SCR_ButtonTextComponent.Cast(ButtonWidget.Cast(m_wRoot.FindAnyWidget("ButtonIndfor")).FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(SelectFactionIndfor);
		SCR_ButtonTextComponent.Cast(ButtonWidget.Cast(m_wRoot.FindAnyWidget("ButtonCiv")).FindHandler(SCR_ButtonTextComponent)).m_OnClicked.Insert(SelectFactionCiv);		
	}