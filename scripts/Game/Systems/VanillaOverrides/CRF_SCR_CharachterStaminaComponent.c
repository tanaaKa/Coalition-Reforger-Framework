modded class SCR_CharacterStaminaComponent : CharacterStaminaComponent
{
		override event void OnStaminaDrain(float pDrain)
		{
			if(GetGame().InPlayMode() && CRF_GamemodeComponent.GetInstance().GetSafestartStatus())
				AddStamina(Math.AbsFloat(pDrain));
		};
}