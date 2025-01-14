class CRF_PlayableCharacterClass: ScriptComponentClass
{
}

class CRF_PlayableCharacter : ScriptComponent
{
	[Attribute()]
	protected string m_sName;
	[Attribute()]
	protected bool m_bIsPlayable;
	[Attribute()]
	protected bool m_bIsLeaderOrMedic;
	[Attribute()]
	protected bool m_bIsSpecialty;
	
	protected bool m_bIsSpectator = false;
	protected SCR_PlayerController m_PlayerController;
	
	protected float m_bTimeSliceLimit = 0;
	
	bool IsPlayable()
	{
		return m_bIsPlayable;
	}
	
	string GetName()
	{
		return m_sName;
	}
	
	bool IsLeader()
	{
		return m_bIsLeaderOrMedic;
	}
	
	bool IsSpecialty()
	{
		return m_bIsSpecialty;
	}
	
	override void OnPostInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if(!GetGame().InPlayMode())
			return;
		
		if(CRF_Gamemode.GetInstance().m_GamemodeState == CRF_GamemodeState.GAME)
			m_bIsPlayable = false;

		if(m_bIsPlayable)
		{
			GetGame().GetCallqueue().CallLater(SetInitialEntity, 500, false, owner);
			GetGame().GetCallqueue().CallLater(DisableAI, 0, false, owner);
		} else {
			return;
		}
		
		m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		SetEventMask(owner, EntityEvent.FIXEDFRAME);
		
		if(owner.GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
			m_bIsSpectator = true;		
	}
	
	override void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if(!m_bIsPlayable || !GetGame().InPlayMode())
			return;
		
		super.EOnFixedFrame(owner, timeSlice);
		if(owner.GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
		{
			owner.GetPhysics().EnableGravity(false);
			owner.SetOrigin("0 10000 0");
			Physics physics = owner.GetPhysics();
			if (physics)
			{
				physics.SetVelocity("0 0 0");
				physics.SetAngularVelocity("0 0 0");
				physics.SetMass(0);
				physics.SetDamping(1, 1);
				physics.SetActive(ActiveState.INACTIVE);
			}
		}	
		if(SCR_PlayerController.GetLocalControlledEntity() != owner)
			return;
		
		if(!m_bIsSpectator)
			return;
		
		if(!SCR_ChimeraCharacter.Cast(owner).m_bIsListening && owner.GetOrigin() != "0 10000 0")
		{
			vector debugVector[4];
			debugVector[3] = "0 10000 0";
			m_PlayerController.UpdateCameraPos(debugVector);
		}
		
		if(!SCR_ChimeraCharacter.Cast(owner).m_bIsListening)
			return;
		
		vector cameraPos[4];
		GetGame().GetCameraManager().CurrentCamera().GetWorldCameraTransform(cameraPos);
		
		m_PlayerController.UpdateCameraPos(cameraPos);
	}
	
	void DisableAI(IEntity owner)
	{
		if(AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent())
			AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent().DeactivateAI();
		GetGame().GetCallqueue().CallLater(DisableAIWrap, 0, false, owner)
	}
	
	void DisableAIWrap(IEntity owner)
	{
		if(AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent())
			AIControlComponent.Cast(owner.FindComponent(AIControlComponent)).GetAIAgent().DeactivateAI();
	}
	
	void SetInitialEntity(IEntity owner)
	{
		//Logs entity on server and disables AI
		if(Replication.IsServer())
		{
			SCR_AIGroup playableGroup = SCR_AIGroup.Cast(ChimeraAIControlComponent.Cast(owner.FindComponent(ChimeraAIControlComponent)).GetControlAIAgent().GetParentGroup());
			if(playableGroup)
				CRF_Gamemode.GetInstance().AddPlayableEntity(owner);
		}
			
		//Sets location and all the physics BS on all machines
		if(owner.GetPrefabData().GetPrefabName() == "{59886ECB7BBAF5BC}Prefabs/Characters/CRF_InitialEntity.et")
		{
			owner.GetPhysics().EnableGravity(false);
			owner.SetOrigin("0 10000 0");
			Physics physics = owner.GetPhysics();
			if (physics)
			{
				physics.SetVelocity("0 0 0");
				physics.SetAngularVelocity("0 0 0");
				physics.SetMass(0);
				physics.SetDamping(1, 1);
				physics.SetActive(ActiveState.INACTIVE);
			}
		}	
	}
}