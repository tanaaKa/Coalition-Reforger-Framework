modded class SCR_AIGroup
{
	[Attribute("0", category: "Group")]
	bool m_bIsPlayable;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if(!GetGame().InPlayMode())
			return;
		
		if(CRF_Gamemode.GetInstance().m_GamemodeState == CRF_GamemodeState.GAME)
			m_bIsPlayable = false;
		
		#ifdef WORKBENCH
		if (m_bIsPlayable)
			GetGame().GetCallqueue().CallLater(SaveAIGRoup, 500, false);
		#else
		if (RplSession.Mode() == RplMode.Dedicated && m_bIsPlayable)
			GetGame().GetCallqueue().CallLater(SaveAIGRoup, 500, false);
		#endif
	}
	
	//Saves the group on the server
	void SaveAIGRoup()
	{
		CRF_Gamemode.GetInstance().AddGroup(this);
	}
	
	override void OnEmpty()
	{
		Event_OnEmpty.Invoke(this);
		
		//--- Delete after delay, doing it directly in this event would be unsafe
//		if (m_bDeleteWhenEmpty)
//			GetGame().GetCallqueue().CallLater(SCR_EntityHelper.DeleteEntityAndChildren, 1, false, this);		
	}
}