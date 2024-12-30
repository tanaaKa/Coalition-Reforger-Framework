modded class SCR_PlayerController
{
	bool m_bActivated = false;
	
	override protected void UpdateLocalPlayerController()
	{
		super.UpdateLocalPlayerController();
		
		m_bIsLocalPlayerController = this == GetGame().GetPlayerController();
		if (!m_bIsLocalPlayerController)
			return;

		s_pLocalPlayerController = this;
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		GetGame().GetInputManager().AddActionListener("SpecNVG", EActionTrigger.DOWN, ActivateAction);
	}
	
	void ActivateAction()
	{
		m_bActivated = !m_bActivated;
		
		if(m_bActivated)
			ActivateNVG();
		else
			DisableNVG();
	}
	
	void ActivateNVG()
	{
		SCR_ScreenEffectsManager.GetScreenEffectsDisplay().RHS_SetHDR("{0AD0A1ADEBCF893F}Assets/Items/Equipment/NVG/pvs14/data/SpecNVGFilm.emat", true)
	}
	
	override private void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		GetGame().GetInputManager().RemoveActionListener("SpecNVG", EActionTrigger.DOWN, ActivateAction);
		if(m_bActivated)
			DisableNVG();
		
		m_bActivated = false;
		
		super.OnControlledEntityChanged(from, to);
	}
	
	void ZeusClose()
	{
		GetGame().GetInputManager().RemoveActionListener("SpecNVG", EActionTrigger.DOWN, ActivateAction);
		if(m_bActivated)
			DisableNVG();
		
		m_bActivated = false;
		
	}
	
	void DisableNVG()
	{
		SCR_ScreenEffectsManager.GetScreenEffectsDisplay().RHS_SetHDR("{765A5E642D09A4B8}Common/Postprocess/HDR_Vanila.emat", false);
	}
}