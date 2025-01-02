modded class CLB_SpectatorMenuUI
{
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("ChatToggle", EActionTrigger.DOWN, Action_OnChatToggleAction);
		GetGame().GetInputManager().RemoveActionListener("MenuBack", EActionTrigger.DOWN, Action_Exit);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.DOWN, Action_VONon);
		GetGame().GetInputManager().RemoveActionListener("VONDirect", EActionTrigger.UP, Action_VONOff);
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_NotificationSenderComponent));
		sender.SetKillFeedTypeNoneLocal();
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		foreach(RplId entityID: m_Gamemode.m_aEntitySlots)
		{
			if(!Replication.FindItem(entityID))
				continue;
			
			IEntity entity = RplComponent.Cast(Replication.FindItem(entityID)).GetEntity();
			
			if(m_aEntityIcons.Find(entity) != -1)
				continue;
			
			Widget spectatorIconWidget = GetGame().GetWorkspace().CreateWidgets("{68625BAD23CEE68F}UI/Spectator/SpectatorLabelIconCharacter.layout", FrameWidget.Cast(GetRootWidget().FindAnyWidget("IconsFrame")));
			CLB_SpectatorLabelIconCharacter spectatorIcon = CLB_SpectatorLabelIconCharacter.Cast(spectatorIconWidget.FindHandler(CLB_SpectatorLabelIconCharacter));
			spectatorIcon.SetEntity(entity, "Spine3");
			m_aEntityIcons.Insert(entity);
			m_aSpectatorIcons.Insert(spectatorIcon);
		}
		UpdateIcons();
		
		if (m_ChatPanel)
        	m_ChatPanel.OnUpdateChat(tDelta);
		
		SCR_NotificationSenderComponent sender = SCR_NotificationSenderComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_NotificationSenderComponent));
		sender.SetKillFeedTypeDeadLocal();
	}
}