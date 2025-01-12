modded class SCR_CharacterDamageManagerComponent
{
	//Logs death of a playable character on server
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		super.OnLifeStateChanged(previousLifeState, newLifeState);
		if(newLifeState == 10 && CRF_PlayableCharacter.Cast(GetOwner().FindComponent(CRF_PlayableCharacter)).IsPlayable())
			CRF_Gamemode.GetInstance().SetDeathState(GetOwner(), true);
	}
}