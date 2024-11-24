modded class PS_PlayableControllerComponent : ScriptComponent
{
	bool m_bActivated = false;
	
	override void SwitchToObserver(IEntity from)
	{
		super.SwitchToObserver(from);
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
		m_bActivated = false;
		DisableNVG();
		super.OnControlledEntityChanged(from, to);
	}
	
	void ZeusClose()
	{
		GetGame().GetInputManager().RemoveActionListener("SpecNVG", EActionTrigger.DOWN, ActivateAction);
		m_bActivated = false;
		DisableNVG();
	}
	
	void DisableNVG()
	{
		SCR_ScreenEffectsManager.GetScreenEffectsDisplay().RHS_SetHDR("{765A5E642D09A4B8}Common/Postprocess/HDR_Vanila.emat", false);
	}
}