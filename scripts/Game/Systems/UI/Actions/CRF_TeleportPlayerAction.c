//------------------------------------------------------------------------------------------------
class CRF_TeleportPlayerAction : ScriptedUserAction
{	
	[Attribute("", "auto", "Object we are teleporting to", category: "CRF Teleport")]
	string m_sObjectNameToTeleportTo;
	
	[Attribute("", "auto", "Gearscript given to the teleported entity on the other side", category: "CRF Teleport")]
	string m_sGearscriptToSet;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		IEntity teleportObject = GetGame().GetWorld().FindEntityByName(m_sObjectNameToTeleportTo);
		vector teleportPosition = teleportObject.GetOrigin();
		vector teleportYawPitchRoll = teleportObject.GetYawPitchRoll();
		
		vector finalSpawnLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(finalSpawnLocation, teleportPosition, 3);
		
		SCR_Global.TeleportLocalPlayer(finalSpawnLocation, SCR_EPlayerTeleportedReason.FAST_TRAVEL);
		SCR_PlayerController.GetLocalControlledEntity().SetYawPitchRoll(teleportYawPitchRoll);
		
		CRF_GamemodeComponent.GetInstance().SetupAddGearToEntity(SCR_PlayerController.GetLocalControlledEntity(), m_sGearscriptToSet);
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{
		return false;
	}
};