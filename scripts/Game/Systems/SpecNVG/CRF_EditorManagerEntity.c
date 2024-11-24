modded class SCR_EditorManagerEntity : SCR_EditorBaseEntity
{
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		super.StartEvents(type);
		
		if(type == EEditorEventOperation.OPEN)
		{
			if(!SCR_PlayerController.GetLocalControlledEntity())
				return;
			
			PS_PlayableControllerComponent playableController = PS_PlayableControllerComponent.Cast(SCR_PlayerController.Cast(GetGame().GetPlayerController()).FindComponent(PS_PlayableControllerComponent));
			
			playableController.ZeusClose();
		}
	}
}