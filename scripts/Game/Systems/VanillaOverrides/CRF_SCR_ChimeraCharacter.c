modded class SCR_ChimeraCharacter
{
	[RplProp(onRplName: "OnVisibleUpdated")]
	bool m_bIsListening = false;
	
	void SetInvisible()
	{
		OnVisibleUpdated();
	}
	
	void SetListening(bool input)
	{
		m_bIsListening = input;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVisibleUpdated()
	{
		ClearFlags(EntityFlags.VISIBLE | EntityFlags.TRACEABLE);
	}
}