class CRF_GamemodeComponentClass: SCR_BaseGameModeComponentClass {}

class CRF_GamemodeComponent: SCR_BaseGameModeComponent
{
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Globally Used Functions/Variables
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	protected ref RandomGenerator m_RNG = new RandomGenerator;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	static CRF_GamemodeComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return CRF_GamemodeComponent.Cast(gameMode.FindComponent(CRF_GamemodeComponent));
		else
			return null;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		// Only run on in-game post init
		// Is the the right way to do this? WHO KNOWS !
		if (!GetGame().InPlayMode()) return;
		
		GetGame().GetInputManager().AddActionListener("SwitchSpectatorUI", EActionTrigger.DOWN, UpdateHUDVisible);
		GetGame().GetCallqueue().CallLater(AddMsgAction, 0, false);
			
		if (Replication.IsServer())
		{
			GetGame().GetCallqueue().CallLater(UpdatePlayerGearScriptsArray, m_RNG.RandInt(10000, 20000), true);
			
			m_Logging = CRF_LoggingServerComponent.Cast(this.FindComponent(CRF_LoggingServerComponent));
			GetGame().GetCallqueue().CallLater(WaitTillGameStart, 1000, true);
		} 
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void WaitTillGameStart()
	{
		if(CRF_Gamemode.GetInstance().m_GamemodeState != CRF_GamemodeState.GAME)
			return;
 		
		m_SafeStartEnabled = !CRF_Gamemode.GetInstance().m_bSafestartInstantlyEnabled;
		Replication.BumpMe();//Broadcast m_SafeStartEnabled change
		
		GetGame().GetCallqueue().Remove(WaitTillGameStart);		
		GetGame().GetCallqueue().CallLater(ToggleSafeStartServer, 1000, false, CRF_Gamemode.GetInstance().m_bSafestartInstantlyEnabled);
	}
	
	void OnGamemodeStateChanged()
	{}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// GearScripts Functions/Variables
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	const int HEADGEAR    = 0;
	const int SHIRT       = 1;
	const int ARMOREDVEST = 2;
	const int PANTS       = 3;
	const int BOOTS       = 4;
	const int BACKPACK    = 5;
	const int VEST        = 6;
	const int HANDWEAR    = 7;
	const int HEAD        = 8;
	const int EYES        = 9;
	const int EARS        = 10;
	const int FACE        = 11;	
	const int NECK        = 12;
	const int EXTRA1      = 13;
	const int EXTRA2      = 14;
	const int WAIST       = 15;
	const int EXTRA3      = 16;
	const int EXTRA4      = 17;
	
	const ref array<EWeaponType> WEAPON_TYPES_THROWABLE = {EWeaponType.WT_FRAGGRENADE, EWeaponType.WT_SMOKEGRENADE};
	
	const ref TStringArray m_aLeadershipRolesUGL = {"_COY_P", "_PL_P", "_SL_P", "_FO_P", "_JTAC_P", "_1SG_P", "_PSG_P"};
	const ref TStringArray m_aLeadershipRolesCarbine = {"_MO_P", "_IndirectLead_P", "_LogiLead_P", "_VehLead_P"};
		
	const ref TStringArray m_aSquadLevelRolesUGL = {"_TL_P", "_Gren_P", "_RTO_P"};
	const ref TStringArray m_aSquadLevelRolesRifle = {"_Rifleman_P", "_Demo_P", "_AAT_P", "_AAR_P"};
	const ref TStringArray m_aSquadLevelRolesCarbine = {"_Medic_P"};
		
	const ref TStringArray m_aInfantrySpecialtiesRolesRifle = {"_AHAT_P", "_AMAT_P", "_AHMG_P", "_AMMG_P", "_AAA_P", "_DroneOp_P"};
		
	const ref TStringArray m_aVehicleSpecialtiesRolesCarbine = {"_VehDriver_P", "_VehGunner_P", "_VehLoader_P", "_LogiRunner_P", "_IndirectGunner_P", "_IndirectLoader_P"};
	const ref TStringArray m_aVehicleSpecialtiesRolesPistol = {"_Pilot_P", "_CrewChief_P"};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

	// A array we use primarily for replication of m_mAllPlayerGearScriptsMap to clients.
	[RplProp(onRplName: "UpdateLocalPlayerGearScriptsMap")]
	protected ref array<string> m_aAllPlayerGearScriptsArray = new array<string>;
	
	// A hashmap that is modified only on each client by a .BumpMe from the authority.
	protected ref map<string, string> m_mAllPlayerGearScriptsMap = new map<string, string>;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	override void OnControllableSpawned(IEntity entity)
	{
		super.OnControllableSpawned(entity);
		
		if(RplSession.Mode() == RplMode.Client)
			return;
		
		GetGame().GetCallqueue().CallLater(CheckWorldValid, 100, false, entity);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void CheckWorldValid(IEntity entity)
	{
		if(!GetGame().GetWorld())
		{
			GetGame().GetCallqueue().CallLater(CheckWorldValid, 100, false, entity);
			return;
		}
		
		AddGearToEntity(entity, entity.GetPrefabData().GetPrefabName());
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	ResourceName GetGearScriptResource(string factionKey)
	{
		if (!GetGearScriptSettings(factionKey))
		{
			PrintFormat("NO GEARSCRIPT ASSIGNED TO: %1", factionKey);
			return "";
		};
		
		return GetGearScriptSettings(factionKey).m_rGearScript;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	CRF_GearScriptContainer GetGearScriptSettings(string factionKey)
	{
		CRF_GearScriptContainer gearScriptContainer;
		
		switch(factionKey)
		{
			case "BLUFOR" : {gearScriptContainer = CRF_Gamemode.GetInstance().m_BLUFORGearScriptSettings;   break;}
			case "OPFOR"  : {gearScriptContainer = CRF_Gamemode.GetInstance().m_OPFORGearScriptSettings;    break;}
			case "INDFOR" : {gearScriptContainer = CRF_Gamemode.GetInstance().m_INDFORGearScriptSettings;   break;}
			case "CIV"    : {gearScriptContainer = CRF_Gamemode.GetInstance().m_CIVILIANGearScriptSettings; break;}
		}
		
		return gearScriptContainer;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Functions to replicate and store values to each clients m_mAllPlayerGearScriptsMap
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	override protected void OnGameEnd()
	{
		super.OnGameEnd();
		
		//--- Server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		GetGame().GetCallqueue().Remove(UpdatePlayerGearScriptsArray);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	string ReturnPlayerGearScriptsMapValue(int playerID, string key)
	{
		// Get the players key
		key = string.Format("%1%2", playerID, key);
		return m_mAllPlayerGearScriptsMap.Get(key);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetPlayerGearScriptsMapValue(string value, int playerID, string key)
	{
		// Get the players key
		key = string.Format("%1%2", playerID, key);
		m_mAllPlayerGearScriptsMap.Set(key, value);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdatePlayerGearScriptsArray()
	{
		// Create a temp array so we arent broadcasting for each change to m_aAllPlayerGearScriptsArray.
		protected ref array<string> tempPlayerArray = new array<string>;

		// Fill tempPlayerArray with all keys and values in m_mAllPlayerGearScriptsMap.
		for (int i = 0; i < m_mAllPlayerGearScriptsMap.Count(); i++)
		{
			string key = m_mAllPlayerGearScriptsMap.GetKey(i);
			string value = m_mAllPlayerGearScriptsMap.Get(key);
			
			tempPlayerArray.Insert(string.Format("%1~%2", key, value));
		};

		// Replicate m_aAllPlayerGearScriptsArray to all clients.
		m_aAllPlayerGearScriptsArray = tempPlayerArray;
		Replication.BumpMe();
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateLocalPlayerGearScriptsMap()
	{
		// Fill m_mAllPlayerGearScriptsMap with all keys and values from authorities m_mAllPlayerGearScriptsMap.
		foreach (string playerKeyAndValueToSplit : m_aAllPlayerGearScriptsArray)
		{
			array<string> playerKeyAndValueArray = {};
			playerKeyAndValueToSplit.Split("~", playerKeyAndValueArray, false);
			m_mAllPlayerGearScriptsMap.Set(playerKeyAndValueArray[0], playerKeyAndValueArray[1]);
		};
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Functions to for Gear Script
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddGearToEntity(IEntity entity, ResourceName prefabName)
	{
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// CHECKS & DEFINING KEY VARIABLES
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
		
		ResourceName ResourceNameToScan = prefabName;	
		
		if(!ResourceNameToScan.Contains("CRF_GS_") || !entity)
			return;
		
		string factionKey;
		
		switch(true)
		{
			case(ResourceNameToScan.Contains("BLUFOR")) : {factionKey = "BLUFOR";   break;}
			case(ResourceNameToScan.Contains("OPFOR"))  : {factionKey = "OPFOR";    break;}
			case(ResourceNameToScan.Contains("INDFOR")) : {factionKey = "INDFOR";   break;}
			case(ResourceNameToScan.Contains("CIV"))    : {factionKey = "CIV";      break;}
		}
		
		ResourceName gearScriptResourceName = GetGearScriptResource(factionKey);		
		CRF_GearScriptContainer gearScriptSettings = GetGearScriptSettings(factionKey);
		
		if(gearScriptResourceName.IsEmpty() || !gearScriptSettings)
			return;
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// GET COMPONENTS
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		SCR_CharacterInventoryStorageComponent inventory = SCR_CharacterInventoryStorageComponent.Cast(entity.FindComponent(SCR_CharacterInventoryStorageComponent));
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if(!inventory || !inventoryManager)
		{
			Print(string.Format("CRF GEAR SCRIPT ERROR: %1 DOESN'T HAVE COMPONENTS WE NEED!", entity), LogLevel.ERROR);
			return;
		}
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// GET ROLE
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		array<string> value = {};
		ResourceNameToScan.Split("_", value, true);
		
		string role = "_" + value[3] + "_" + value[4];
		
		role.Split(".", value, true);
		role = value[0];

		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// CLEAR CHARACTER
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		array<IEntity> items = {};
		array<IEntity> itemsRoot = {};
		inventoryManager.GetAllItems(items, inventory);
		inventoryManager.GetAllRootItems(itemsRoot);
		
		items.InsertAll(itemsRoot);
		
		foreach(IEntity item : items)
			SCR_EntityHelper.DeleteEntityAndChildren(item);
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// CHECK CLOTHING/WEAPONS/ITEMS
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	
		CRF_GearScriptConfig gearConfig = CRF_GearScriptConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(BaseContainerTools.LoadContainer(gearScriptResourceName).GetResource().ToBaseContainer()));
		
		array<Managed> weaponSlotComponentArray = {};
		entity.FindComponents(WeaponSlotComponent, weaponSlotComponentArray);
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
        spawnParams.Transform[3] = entity.GetOrigin();
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// CLOTHING
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		if(gearConfig.m_DefaultFactionGear)
			foreach(CRF_Clothing clothing : gearConfig.m_DefaultFactionGear.m_DefaultClothing)
				UpdateClothingSlot(clothing.m_ClothingPrefabs, clothing.m_sClothingType, role, spawnParams, inventory, inventoryManager);
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// CUSTOM GEAR
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		bool isLeader = false;
		bool isSquad = false;
		bool isInfSpec = false;
		bool isVehSpec = false;
		
		if(gearConfig.m_FactionWeapons)
		{
			switch(true)
			{
				// Leadership --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				case(m_aLeadershipRolesUGL.Contains(role))             : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "RifleUGL", "",    true);  isLeader = true;  break;}
				case(m_aLeadershipRolesCarbine.Contains(role))         : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Carbine",  "",    true);  isLeader = true;  break;}	
				// Squad Level -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				case(m_aSquadLevelRolesUGL.Contains(role))             : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "RifleUGL", "",    false); isSquad = true;   break;}
				case(m_aSquadLevelRolesCarbine.Contains(role))         : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Carbine",  "",    false); isSquad = true;   break;}
				case(m_aSquadLevelRolesRifle.Contains(role))           : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "",    false); isSquad = true;   break;}
				case(role == "_AT_P")                                  : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "AT",  false); isSquad = true;   break;}
				case(role == "_AR_P")                                  : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "AR",       "",    true); isSquad = true;   break;}
				// Infantry Specialties ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				case(m_aInfantrySpecialtiesRolesRifle.Contains(role))  : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "",    false); isInfSpec = true; break;}
				case(role == "_HAT_P")                                 : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "HAT", false); isInfSpec = true; break;}
				case(role == "_MAT_P")                                 : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "MAT", false); isInfSpec = true; break;}
				case(role == "_HMG_P")                                 : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "HMG",      "",    true);  isInfSpec = true; break;}
				case(role == "_MMG_P")                                 : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "MMG",      "",    true);  isInfSpec = true; break;}
				case(role == "_AA_P")                                  : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "AA",  false); isInfSpec = true; break;}
				case(role == "_Sniper_P")                              : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Sniper",   "",    true);  isInfSpec = true; break;}
				case(role == "_Spotter_P")                             : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "RifleUGL", "",    false); isInfSpec = true; break;}
				// Vehicle Specialties -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				case(m_aVehicleSpecialtiesRolesCarbine.Contains(role)) : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Carbine",  "",    false); isVehSpec = true; break;}
				case(m_aVehicleSpecialtiesRolesPistol.Contains(role))  : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "",         "",    true);  isVehSpec = true; break;}
				// Default -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
				default                                                : {AddWeapons(spawnParams, inventory, inventoryManager, weaponSlotComponentArray, gearConfig, "Rifle",    "",    false); break;}
			}
		} else
			Print(string.Format("CRF GEAR SCRIPT ERROR: NO WEAPONS SET: %1", gearScriptResourceName), LogLevel.ERROR);
			
		if(gearConfig.m_CustomFactionGear)
		{
			switch(true)
			{
				case(isLeader)  : {UpdateLeadershipCustomGear(gearConfig.m_CustomFactionGear.m_LeadershipCustomGear, role, spawnParams, inventory, inventoryManager);                   break;}
				case(isSquad)   : {UpdateSquadLevelCustomGear(gearConfig.m_CustomFactionGear.m_SquadLevelCustomGear, role, spawnParams, inventory, inventoryManager);                   break;}
				case(isInfSpec) : {UpdateInfantrySpecialtiesCustomGear(gearConfig.m_CustomFactionGear.m_InfantrySpecialtiesCustomGear, role, spawnParams, inventory, inventoryManager); break;}
				case(isVehSpec) : {UpdateVehicleSpecialtiesCustomGear(gearConfig.m_CustomFactionGear.m_VehicleSpecialtiesCustomGear, role, spawnParams, inventory, inventoryManager);   break;}
			}
		} else
			Print(string.Format("CRF GEAR SCRIPT ERROR: NO CUSTOM GEAR SET: %1", gearScriptResourceName), LogLevel.ERROR);
		
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		// ITEMS
		//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		if(gearConfig.m_DefaultFactionGear)
		{
			//Who we give Leadership Radios
			if(gearScriptSettings.m_bEnableLeadershipRadios && (m_aLeadershipRolesUGL.Contains(role) || m_aLeadershipRolesCarbine.Contains(role) || role == "_Spotter_P" || role == "_Pilot_P" || role == "_CrewChief_P" || role == "_Medic_P"))
				AddInventoryItem(gearScriptSettings.m_rLeadershipRadiosPrefab, 1, spawnParams, inventory, inventoryManager);
			
			//Who we give GI Radios
			if(gearScriptSettings.m_bEnableGIRadios && !(m_aLeadershipRolesUGL.Contains(role) || m_aLeadershipRolesCarbine.Contains(role) || role == "_Spotter_P" || role == "_Pilot_P" || role == "_CrewChief_P"))
				AddInventoryItem(gearScriptSettings.m_rGIRadiosPrefab, 1, spawnParams, inventory, inventoryManager);
			
			//Who we give RTO Radios
			if(gearScriptSettings.m_bEnableRTORadios && role == "_RTO_P")
				AddInventoryItem(gearScriptSettings.m_rRTORadiosPrefab, 1, spawnParams, inventory, inventoryManager);
			
			//Who we give Leadership Binos
			if(gearConfig.m_DefaultFactionGear.m_bEnableLeadershipBinoculars && (m_aLeadershipRolesUGL.Contains(role) || m_aLeadershipRolesCarbine.Contains(role) || role == "_Spotter_P" || role == "_TL_P"))
				AddInventoryItem(gearConfig.m_DefaultFactionGear.m_sLeadershipBinocularsPrefab, 1, spawnParams, inventory, inventoryManager);
			
			//Who we give Assistant Binos/Extra magazines
			if(role == "_AAR_P" || role == "_AMMG_P" || role == "_AHMG_P" || role == "_AMAT_P" || role == "_AHAT_P" || role == "_AAA_P" || role == "_AAT_P")
			{
				array<ref CRF_Spec_Magazine_Class> magazineArray = {};
		
				switch(role)
				{
					case "_AAR_P"  : {if(!gearConfig.m_FactionWeapons.m_AR  || !gearConfig.m_FactionWeapons.m_AR.m_Weapon)  {return;} magazineArray = gearConfig.m_FactionWeapons.m_AR.m_MagazineArray;  break;}
					case "_AMMG_P" : {if(!gearConfig.m_FactionWeapons.m_MMG || !gearConfig.m_FactionWeapons.m_MMG.m_Weapon) {return;} magazineArray = gearConfig.m_FactionWeapons.m_MMG.m_MagazineArray; break;}
					case "_AHMG_P" : {if(!gearConfig.m_FactionWeapons.m_HMG || !gearConfig.m_FactionWeapons.m_HMG.m_Weapon) {return;} magazineArray = gearConfig.m_FactionWeapons.m_HMG.m_MagazineArray; break;}
					case "_AMAT_P" : {if(!gearConfig.m_FactionWeapons.m_MAT || !gearConfig.m_FactionWeapons.m_MAT.m_Weapon) {return;} magazineArray = gearConfig.m_FactionWeapons.m_MAT.m_MagazineArray; break;}
					case "_AHAT_P" : {if(!gearConfig.m_FactionWeapons.m_HAT || !gearConfig.m_FactionWeapons.m_HAT.m_Weapon) {return;} magazineArray = gearConfig.m_FactionWeapons.m_HAT.m_MagazineArray; break;}
					case "_AAA_P"  : {if(!gearConfig.m_FactionWeapons.m_AA  || !gearConfig.m_FactionWeapons.m_AA.m_Weapon)  {return;} magazineArray = gearConfig.m_FactionWeapons.m_AA.m_MagazineArray;  break;}
					case "_AAT_P"  : {if(!gearConfig.m_FactionWeapons.m_AT  || !gearConfig.m_FactionWeapons.m_AT.m_Weapon)  {return;} magazineArray = gearConfig.m_FactionWeapons.m_AT.m_MagazineArray;  break;}
				}
		
				foreach(ref CRF_Spec_Magazine_Class magazine : magazineArray)
					AddInventoryItem(magazine.m_Magazine, magazine.m_AssistantMagazineCount, spawnParams, inventory, inventoryManager, role, true);
				
				if(gearConfig.m_DefaultFactionGear.m_bEnableAssistantBinoculars)
					AddInventoryItem(gearConfig.m_DefaultFactionGear.m_sAssistantBinocularsPrefab, 1, spawnParams, inventory, inventoryManager, role);
			}
			
			//Who we give extra medical items
			if(role == "_Medic_P" || role == "_MO_P")
			{	
				foreach(CRF_Inventory_Item item : gearConfig.m_DefaultFactionGear.m_DefaultMedicMedicalItems)
					AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role);
			}
			
			//What everyone gets
			foreach(CRF_Inventory_Item item : gearConfig.m_DefaultFactionGear.m_DefaultInventoryItems)
				AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role, gearConfig.m_DefaultFactionGear.m_bEnableMedicFrags);
		} else 
			Print(string.Format("CRF GEAR SCRIPT ERROR: NO DEFAULT GEAR SET: %1", gearScriptResourceName), LogLevel.ERROR);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateClothingSlot(array<ResourceName> clothingArray, string clothingStr, string role, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		if(clothingArray.IsEmpty() || clothingStr.IsEmpty())
			return;
		
		int slotInt = ConvertClothingStringToInt(clothingStr);
		
		if(slotInt == -1)
			return;
		
		array<IEntity> removedItems = {};
		IEntity previousClothing = inventory.Get(slotInt);
		ResourceName clothing = clothingArray.GetRandomElement();
		
		if (previousClothing != null)
		{
			BaseInventoryStorageComponent oldStorage = BaseInventoryStorageComponent.Cast(previousClothing.FindComponent(BaseInventoryStorageComponent));
			if(oldStorage)
			{
				array<IEntity> outItems = {};
				oldStorage.GetAll(outItems);
				foreach(IEntity item : outItems)
				{
					if(!item || !InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent)) || SCR_EquipmentStorageComponent.Cast(item.FindComponent(SCR_EquipmentStorageComponent)) ||  SCR_UniversalInventoryStorageComponent.Cast(item.FindComponent(SCR_UniversalInventoryStorageComponent)) || BaseInventoryStorageComponent.Cast(item.FindComponent(BaseInventoryStorageComponent)))
						continue;
					
					inventoryManager.TryRemoveItemFromStorage(item, oldStorage);
					removedItems.Insert(item);
				}
			};
			
			inventoryManager.TryRemoveItemFromStorage(previousClothing, inventory);
			SCR_EntityHelper.DeleteEntityAndChildren(previousClothing);
		};
		
		if(!clothing.IsEmpty())
		{
			ref IEntity resourceSpawned = GetGame().SpawnEntityPrefab(Resource.Load(clothing), GetGame().GetWorld(), spawnParams);
			inventoryManager.TryReplaceItem(resourceSpawned, inventory, slotInt);
			
			if (!inventoryManager.Contains(resourceSpawned))
			{
				Print("-------------------------------------------------------------------------------------------------------------", LogLevel.ERROR);
				Print(string.Format("CRF GEAR SCRIPT ERROR: UNABLE TO INSERT CLOTHING: %1", clothing), LogLevel.ERROR);
				Print(string.Format("CRF GEAR SCRIPT ERROR: INTO ENTITY: %1", inventoryManager.GetOwner().GetPrefabData().GetPrefabName()), LogLevel.ERROR);
				Print(" ", LogLevel.ERROR);
				Print("CRF GEAR SCRIPT ERROR: INVALID CLOTHING ITEM!", LogLevel.ERROR);
				Print("-------------------------------------------------------------------------------------------------------------", LogLevel.ERROR);
				SCR_EntityHelper.DeleteEntityAndChildren(resourceSpawned);
			};
		}
		
		foreach(IEntity oldItem : removedItems)
			InsertInventoryItem(oldItem, inventory, inventoryManager, role);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void AddInventoryItem(ResourceName item, int itemAmmount, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager, string role = "", bool enableMedicFrags = false, bool isAssistant = false)
	{	
		if(item.IsEmpty() || itemAmmount <= 0)
			return;
		
		for(int i = 1; i <= itemAmmount; i++)
		{
			ref IEntity resourceSpawned = GetGame().SpawnEntityPrefab(Resource.Load(item), GetGame().GetWorld(), spawnParams);
			
			if(!resourceSpawned)
				continue;
			
			bool isThrowable = (WeaponComponent.Cast(resourceSpawned.FindComponent(WeaponComponent)) && WEAPON_TYPES_THROWABLE.Contains(WeaponComponent.Cast(resourceSpawned.FindComponent(WeaponComponent)).GetWeaponType()));
			
			if(!enableMedicFrags && (role == "_Medic_P" || role == "_MO_P") && (isThrowable && WeaponComponent.Cast(resourceSpawned.FindComponent(WeaponComponent)).GetWeaponType() == EWeaponType.WT_FRAGGRENADE))
			{
				SCR_EntityHelper.DeleteEntityAndChildren(resourceSpawned);
				continue;
			};
			
			if(inventoryManager.CanInsertItem(resourceSpawned, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT))
			{
				BaseInventoryStorageComponent storageComp = inventoryManager.FindStorageForItem(resourceSpawned, EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT);
				inventoryManager.EquipAny(storageComp, resourceSpawned, -1);
				continue;
			};
			
			InsertInventoryItem(resourceSpawned, inventory, inventoryManager, role, isAssistant, isThrowable);
			
			if(isThrowable)
			{
				CharacterGrenadeSlotComponent grenadeSlot = CharacterGrenadeSlotComponent.Cast(inventoryManager.GetOwner().FindComponent(CharacterGrenadeSlotComponent));
				
				if(grenadeSlot && !grenadeSlot.GetWeaponEntity())
				{
					if(WeaponComponent.Cast(resourceSpawned.FindComponent(WeaponComponent)).GetWeaponType() == EWeaponType.WT_FRAGGRENADE)
					{
						grenadeSlot.SetWeapon(resourceSpawned);
					} else {
						if (WeaponComponent.Cast(resourceSpawned.FindComponent(WeaponComponent)).GetWeaponType() == EWeaponType.WT_SMOKEGRENADE && !grenadeSlot.GetWeaponEntity())
							grenadeSlot.SetWeapon(resourceSpawned);
					};
				};
			};
		}
	}
	
	protected void InsertInventoryItem(IEntity item, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager, string role = "", bool isAssistant = false, bool isThrowable = false)
	{
		if(!item) 
			return;
		
		TIntArray clothingIDs = FilterItemToClothing(item, role, isAssistant, isThrowable);
		
		// Try and insert into select clothing at first
		foreach(int clothingID : clothingIDs)
		{			
			IEntity clothing = inventory.Get(clothingID);

			if(!clothing || inventoryManager.Contains(item))
				continue;
				
			BaseInventoryStorageComponent clothingStorage = BaseInventoryStorageComponent.Cast(clothing.FindComponent(BaseInventoryStorageComponent));
			
			if(!clothingStorage)
				continue;
			
			bool successfulInsert = inventoryManager.TryInsertItemInStorage(item, clothingStorage);
			
			if(!successfulInsert)
				inventoryManager.InsertItemCRF(item, clothingStorage, null, null, false);
		};
			
		// if we cant do select clothing, just slap it in wherever
		if(!inventoryManager.Contains(item))
			inventoryManager.TryInsertItem(item);
			
		if (!inventoryManager.Contains(item) || !InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent)))
		{
			Print("-------------------------------------------------------------------------------------------------------------", LogLevel.ERROR);
			Print(string.Format("CRF GEAR SCRIPT ERROR: UNABLE TO INSERT ITEM: %1", item.GetPrefabData().GetPrefabName()), LogLevel.ERROR);
			Print(string.Format("CRF GEAR SCRIPT ERROR: INTO ENTITY: %1", inventoryManager.GetOwner().GetPrefabData().GetPrefabName()), LogLevel.ERROR);
			Print(" ", LogLevel.ERROR);
			Print("CRF GEAR SCRIPT ERROR: NOT ENOUGH SPACE IN INVENTORY/INVALID INVENTORY ITEM!", LogLevel.ERROR);
			Print("-------------------------------------------------------------------------------------------------------------", LogLevel.ERROR);
			SCR_EntityHelper.DeleteEntityAndChildren(item);
		};
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void AddWeapons(EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager, array<Managed> weaponSlotComponentArray, CRF_GearScriptConfig gearConfig, string weaponType, string atType, bool givePistol)
	{	
		//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		ref CRF_Spec_Weapon_Class specWeaponToSpawn;
		ref CRF_Weapon_Class weaponToSpawn;
		//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		foreach(int i, Managed weaponSlotCompManaged : weaponSlotComponentArray)
		{
			WeaponSlotComponent weaponSlotComponent = WeaponSlotComponent.Cast(weaponSlotCompManaged);
			
			//First Primary
			if(weaponType != "" && weaponSlotComponent.GetWeaponSlotIndex() == 0)
			{
				switch(weaponType)
				{
					case "Rifle"    : {weaponToSpawn = SelectRandomWeapon(gearConfig.m_FactionWeapons.m_Rifle);    break;}
					case "RifleUGL" : {weaponToSpawn = SelectRandomWeapon(gearConfig.m_FactionWeapons.m_RifleUGL); break;}
					case "Carbine"  : {weaponToSpawn = SelectRandomWeapon(gearConfig.m_FactionWeapons.m_Carbine);  break;}
					case "AR"       : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_AR;                       break;}
					case "MMG"      : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_MMG;                      break;}
					case "HMG"      : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_HMG;                      break;}
					case "Sniper"   : {weaponToSpawn = gearConfig.m_FactionWeapons.m_Sniper;                       break;}
				}
				
				if(weaponToSpawn && weaponToSpawn.m_Weapon)
					SpawnWeapon(weaponSlotComponent, weaponToSpawn, spawnParams, inventory, inventoryManager);
				
				if(specWeaponToSpawn && specWeaponToSpawn.m_Weapon)
					SpawnSpecWeapon(weaponSlotComponent, specWeaponToSpawn, spawnParams, inventory, inventoryManager);
			}
			
			//Second Primary
			if(atType != "" && weaponSlotComponent.GetWeaponSlotIndex() == 1)
			{
				switch(atType)
				{
					case "AT"  : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_AT;  break;}
					case "MAT" : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_MAT; break;}
					case "HAT" : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_HAT; break;}	
					case "AA"  : {specWeaponToSpawn = gearConfig.m_FactionWeapons.m_AA;  break;}
				}
				
				if(specWeaponToSpawn || specWeaponToSpawn.m_Weapon)	
					SpawnSpecWeapon(weaponSlotComponent, specWeaponToSpawn, spawnParams, inventory, inventoryManager);
			}
			
			//Pistol
			if(givePistol == true && weaponSlotComponent.GetWeaponSlotIndex() == 2)
			{
				weaponToSpawn = SelectRandomWeapon(gearConfig.m_FactionWeapons.m_Pistol);  
				
				if(weaponToSpawn && weaponToSpawn.m_Weapon)
					SpawnWeapon(weaponSlotComponent, weaponToSpawn, spawnParams, inventory, inventoryManager);
			}
		}
	}
	
	protected void SpawnWeapon(WeaponSlotComponent weaponSlotComponent, CRF_Weapon_Class weaponToSpawn, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{		
		IEntity weaponSpawned = GetGame().SpawnEntityPrefab(Resource.Load(weaponToSpawn.m_Weapon), GetGame().GetWorld(), spawnParams);
		
		if(weaponSpawned)
		{
			array<ResourceName> weaponsAttachments = weaponToSpawn.m_Attachments; 
	
			foreach(ref CRF_Magazine_Class magazine : weaponToSpawn.m_MagazineArray)
				AddInventoryItem(magazine.m_Magazine, magazine.m_MagazineCount, spawnParams, inventory, inventoryManager);
			
			InsertWeapon(weaponSpawned, weaponSlotComponent, weaponsAttachments, spawnParams, inventory, inventoryManager);
		};
	}
	
	protected void SpawnSpecWeapon(WeaponSlotComponent weaponSlotComponent, CRF_Spec_Weapon_Class specWeaponToSpawn, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{		
		IEntity specWeaponSpawned = GetGame().SpawnEntityPrefab(Resource.Load(specWeaponToSpawn.m_Weapon), GetGame().GetWorld(), spawnParams);
		
		if(specWeaponSpawned)
		{
			array<ResourceName> specWeaponsAttachments = specWeaponToSpawn.m_Attachments; 
	
			foreach(ref CRF_Spec_Magazine_Class specMagazine : specWeaponToSpawn.m_MagazineArray)
				AddInventoryItem(specMagazine.m_Magazine, specMagazine.m_MagazineCount, spawnParams, inventory, inventoryManager);
			
			InsertWeapon(specWeaponSpawned, weaponSlotComponent, specWeaponsAttachments, spawnParams, inventory, inventoryManager);
		};
	}
	
	protected void InsertWeapon(IEntity weaponSpawned, WeaponSlotComponent weaponSlotComponent, array<ResourceName> weaponsAttachments, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{					
		// Use SetSlotWeapon() instead of weaponSlotComponent.SetWeapon() due to replication issues.
		ChimeraCharacter.Cast(inventoryManager.GetOwner()).GetCharacterController().GetWeaponManagerComponent().SetSlotWeapon(weaponSlotComponent, weaponSpawned);
		
		if(!weaponsAttachments || weaponsAttachments.IsEmpty())
			return;
		
		array<AttachmentSlotComponent> attatchmentSlotArray = {};
		weaponSlotComponent.GetAttachments(attatchmentSlotArray);
		
		foreach(ResourceName attachment : weaponsAttachments)
		{
			foreach(AttachmentSlotComponent attachmentSlot : attatchmentSlotArray)
			{
				IEntity attachmentSpawned = GetGame().SpawnEntityPrefab(Resource.Load(attachment), GetGame().GetWorld(), spawnParams);
				
				if(attachmentSlot.CanSetAttachment(attachmentSpawned))
				{
					delete attachmentSlot.GetAttachedEntity();
					attachmentSlot.SetAttachment(attachmentSpawned);
					break;
				}
				delete attachmentSpawned;
			} 
		}
	}
	
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateLeadershipCustomGear(array<ref CRF_Leadership_Custom_Gear> customGearArray, string role, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		switch(role)
		{
			case "_COY_P"          : {role = "Company Commander"; break;}
			case "_PL_P"           : {role = "Platoon Leader";    break;}
			case "_MO_P"           : {role = "Medical Officer";   break;}
			case "_SL_P"           : {role = "Squad Lead";        break;}
			case "_FO_P"           : {role = "Forward Observer";  break;}
			case "_JTAC_P"         : {role = "JTAC";              break;}
			case "_VehLead_P"      : {role = "Vehicle Lead";      break;}
			case "_IndirectLead_P" : {role = "Indirect Lead";     break;}
			case "_LogiLead_P"     : {role = "Logi Lead";         break;}
			case "_1SG_P"          : {role = "First Sergeant";    break;}
			case "_PSG_P"          : {role = "Platoon Sergeant";  break;}
		}	
		
		foreach(ref CRF_Leadership_Custom_Gear customGear : customGearArray)
		{	
			if(customGear.m_sRoleToOverride != role)
				continue;
				
			foreach(CRF_Clothing clothing : customGear.m_CustomClothing)
				UpdateClothingSlot(clothing.m_ClothingPrefabs, clothing.m_sClothingType, role, spawnParams, inventory, inventoryManager);
					
			foreach(CRF_Inventory_Item item : customGear.m_AdditionalInventoryItems)
				AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateSquadLevelCustomGear(array<ref CRF_Squad_Level_Custom_Gear> customGearArray, string role, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		switch(role)
		{
			case "_TL_P"       : {role = "Team Lead";                    break;}
			case "_Gren_P"     : {role = "Grenadier";                    break;}
			case "_RTO_P"      : {role = "Radio Telephone Operator";     break;}
			case "_Rifleman_P" : {role = "Rifleman";                     break;}
			case "_Demo_P"     : {role = "Rifleman Demo";                break;}
			case "_AT_P"       : {role = "Rifleman AntiTank";            break;}
			case "_AAT_P"      : {role = "Assistant Rifleman AntiTank";  break;}
			case "_AR_P"       : {role = "Automatic Rifleman";           break;}
			case "_AAR_P"      : {role = "Assistant Automatic Rifleman"; break;}
			case "_Medic_P"    : {role = "Medic";                        break;}
		}
		
		foreach(ref CRF_Squad_Level_Custom_Gear customGear : customGearArray)
		{		
			if(customGear.m_sRoleToOverride != role)
				continue;
				
			foreach(CRF_Clothing clothing : customGear.m_CustomClothing)
				UpdateClothingSlot(clothing.m_ClothingPrefabs, clothing.m_sClothingType, role, spawnParams, inventory, inventoryManager);
					
			foreach(CRF_Inventory_Item item : customGear.m_AdditionalInventoryItems)
				AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateInfantrySpecialtiesCustomGear(array<ref CRF_Infantry_Specialties_Custom_Gear> customGearArray, string role, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		switch(role)
		{
			case "_HAT_P"     : {role = "Heavy AntiTank";              break;}
			case "_AHAT_P"    : {role = "Assistant Heavy AntiTank";    break;}
			case "_MAT_P"     : {role = "Medium AntiTank";             break;}
			case "_AMAT_P"    : {role = "Assistant Medium AntiTank";   break;}
			case "_HMG_P"     : {role = "Heavy MachineGun";            break;}
			case "_AHMG_P"    : {role = "Assistant Heavy MachineGun";  break;}
			case "_MMG_P"     : {role = "Medium MachineGun";           break;}
			case "_AMMG_P"    : {role = "Assistant Medium MachineGun"; break;}
			case "_AA_P"      : {role = "Anit-Air";                    break;}
			case "_AAA_P"     : {role = "Assistant Anit-Air";          break;}
			case "_Sniper_P"  : {role = "Sniper";                      break;}
			case "_Spotter_P" : {role = "Spotter";                     break;}
			case "_DroneOp_P" : {role = "Drone Operator";              break;}
		}
		
		foreach(ref CRF_Infantry_Specialties_Custom_Gear customGear : customGearArray)
		{	
			if(customGear.m_sRoleToOverride != role)
				continue;
				
			foreach(CRF_Clothing clothing : customGear.m_CustomClothing)
				UpdateClothingSlot(clothing.m_ClothingPrefabs, clothing.m_sClothingType, role, spawnParams, inventory, inventoryManager);
					
			foreach(CRF_Inventory_Item item : customGear.m_AdditionalInventoryItems)
				AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdateVehicleSpecialtiesCustomGear(array<ref CRF_Vehicle_Specialties_Custom_Gear> customGearArray, string role, EntitySpawnParams spawnParams, SCR_CharacterInventoryStorageComponent inventory, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		switch(role)
		{
			case "_VehDriver_P"      : {role = "Vehicle Driver";  break;}
			case "_VehGunner_P"      : {role = "Vehicle Gunner";  break;}
			case "_VehLoader_P"      : {role = "Vehicle Loader";  break;}
			case "_Pilot_P"          : {role = "Pilot";           break;}
			case "_CrewChief_P"      : {role = "Crew Chief";      break;}
			case "_LogiRunner_P"     : {role = "Logi Runner";     break;}
			case "_IndirectGunner_P" : {role = "Indirect Gunner"; break;}
			case "_IndirectLoader_P" : {role = "Indirect Loader"; break;}
		}
		
		foreach(ref CRF_Vehicle_Specialties_Custom_Gear customGear : customGearArray)
		{
			if(customGear.m_sRoleToOverride != role)
				continue;
				
			foreach(CRF_Clothing clothing : customGear.m_CustomClothing)
				UpdateClothingSlot(clothing.m_ClothingPrefabs, clothing.m_sClothingType, role, spawnParams, inventory, inventoryManager);
					
			foreach(CRF_Inventory_Item item : customGear.m_AdditionalInventoryItems)
				AddInventoryItem(item.m_sItemPrefab, item.m_iItemCount, spawnParams, inventory, inventoryManager, role);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	int ConvertClothingStringToInt(string clothingStr)
	{
		int slotInt = -1;
		
		// All the arrays belong to us
		switch(clothingStr)
		{
			case "HEADGEAR"     : {slotInt = HEADGEAR;    break;}
			case "SHIRT"        : {slotInt = SHIRT;       break;}
			case "ARMOREDVEST"  : {slotInt = ARMOREDVEST; break;}
			case "PANTS"        : {slotInt = PANTS;       break;}
			case "BOOTS"        : {slotInt = BOOTS;       break;}
			case "BACKPACK"     : {slotInt = BACKPACK;    break;}
			case "VEST"         : {slotInt = VEST;        break;}
			case "HANDWEAR"     : {slotInt = HANDWEAR;    break;}
			case "HEAD"         : {slotInt = HEAD;        break;}
			case "EYES"         : {slotInt = EYES;        break;}
			case "EARS"         : {slotInt = EARS;        break;}
			case "FACE"         : {slotInt = FACE;        break;}
			case "NECK"         : {slotInt = NECK;        break;}
			case "EXTRA1"       : {slotInt = EXTRA1;      break;}
			case "EXTRA2"       : {slotInt = EXTRA2;      break;}
			case "WAIST"        : {slotInt = WAIST;       break;}
			case "EXTRA3"       : {slotInt = EXTRA3;      break;}
			case "EXTRA4"       : {slotInt = EXTRA4;      break;}
		};
		
		return slotInt;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected CRF_Weapon_Class SelectRandomWeapon(array<ref CRF_Weapon_Class> weaponArray)
	{
		if(!weaponArray || weaponArray.IsEmpty())
			return null; 
								
		ref CRF_Weapon_Class weaponToSpawn = weaponArray.GetRandomElement();
								
		if(!weaponToSpawn)
			return null; 
								
		return weaponToSpawn;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	TIntArray FilterItemToClothing(IEntity item, string role = "", bool isAssistant = false, bool isThrowable = false)
	{
		ref array<int> clothingIDs = {};
		
		// Any magazine
		if(MagazineComponent.Cast(item.FindComponent(MagazineComponent)) || InventoryMagazineComponent.Cast(item.FindComponent(InventoryMagazineComponent)))
			clothingIDs = {VEST, ARMOREDVEST, BACKPACK, PANTS, SHIRT};
		else // Any Non-magazine
			clothingIDs = {SHIRT, PANTS, VEST, ARMOREDVEST, BACKPACK};
			
		// Any medical item
		if((role == "_Medic_P" || role == "_MO_P") && SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent)))
			clothingIDs = {BACKPACK, VEST, ARMOREDVEST};
		
		// Any pistol ammo
		if((InventoryMagazineComponent.Cast(item.FindComponent(InventoryMagazineComponent)) && InventoryMagazineComponent.Cast(item.FindComponent(InventoryMagazineComponent)).GetAttributes().GetCommonType() == ECommonItemType.RHS_PISTOL_AMMO) || isThrowable)
			clothingIDs = {PANTS, VEST, ARMOREDVEST, BACKPACK};
		
		// Any radio
		if(BaseRadioComponent.Cast(item.FindComponent(BaseRadioComponent)))
			clothingIDs = {PANTS, SHIRT, VEST, ARMOREDVEST, BACKPACK};
		
		// Any Assistant Mags
		if(isAssistant && MagazineComponent.Cast(item.FindComponent(MagazineComponent)))
			clothingIDs = {BACKPACK, VEST, ARMOREDVEST};
		
		// Check if item is explosives related
		SCR_DetonatorGadgetComponent detonator = SCR_DetonatorGadgetComponent.Cast(item.FindComponent(SCR_DetonatorGadgetComponent));
		SCR_ExplosiveChargeComponent explosives = SCR_ExplosiveChargeComponent.Cast(item.FindComponent(SCR_ExplosiveChargeComponent));
		SCR_MineWeaponComponent mine = SCR_MineWeaponComponent.Cast(item.FindComponent(SCR_MineWeaponComponent));
		SCR_RepairSupportStationComponent engTool = SCR_RepairSupportStationComponent.Cast(item.FindComponent(SCR_RepairSupportStationComponent));
		SCR_HealSupportStationComponent medTool = SCR_HealSupportStationComponent.Cast(item.FindComponent(SCR_HealSupportStationComponent));
		if(detonator || explosives || mine || engTool || medTool)
			clothingIDs = {BACKPACK, VEST, ARMOREDVEST};
		
		return clothingIDs;
	}	
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Admin Menu Functions/Variables
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	private Widget m_wSavedHintWidget;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Admin Messaging
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddMsgAction()
	{
		SCR_ChatPanelManager chatPanelManager = SCR_ChatPanelManager.GetInstance();
		ChatCommandInvoker invoker = chatPanelManager.GetCommandInvoker("admin");
		invoker.Insert(SendAdminMessage_Callback);
		ChatCommandInvoker invoker2 = chatPanelManager.GetCommandInvoker("a");
		invoker2.Insert(SendAdminMessage_Callback);
		ChatCommandInvoker invoker3 = chatPanelManager.GetCommandInvoker("r");
		invoker3.Insert(ReplyAdminMessage_Callback);
		ChatCommandInvoker invoker4 = chatPanelManager.GetCommandInvoker("reply");
		invoker4.Insert(ReplyAdminMessage_Callback);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminMessage_Callback(SCR_ChatPanel panel, string data)
	{
		CRF_ClientComponent.GetInstance().SendAdminMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReplyAdminMessage_Callback(SCR_ChatPanel panel, string data)
	{
		if(!SCR_Global.IsAdmin())
			return;
		
		CRF_ClientComponent.GetInstance().ReplyAdminMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminMessage(string data)
	{
		Rpc(RpcAsk_SendAdminMessage, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_SendAdminMessage(string data)
	{
		if(!SCR_Global.IsAdmin())
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		chatComponent.ShowMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ReplyAdminMessage(string data, int playerID)
	{
		Rpc(RpcAsk_ReplyAdminMessage, data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_ReplyAdminMessage(string data, int playerID)
	{
		if(GetGame().GetPlayerController().GetPlayerId() != playerID)
			return;
		
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		
		chatComponent.ShowMessage(string.Format("Admin: %1", data));

		LogAdminAction(string.Format("Reply to %1: %2", GetGame().GetPlayerManager().GetPlayerName(playerID), data), playerID, false);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Respawn
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SpawnGroupServer(int playerId, string prefab, vector spawnLocation, int groupID)
	{
		Rpc(Respawn, playerId, prefab, spawnLocation, groupID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void Respawn(int playerId, string prefab, vector spawnLocation, int groupID)
	{
//		Rpc(RpcAsk_CloseMap, playerId);

		if(prefab.IsEmpty())
		{
			switch(SCR_GroupsManagerComponent.GetInstance().FindGroup(groupID).m_faction)
			{
				case "BLUFOR" : {prefab = "{268EAF6C56517778}Prefabs/Characters/Factions/BLUFOR/US_Army/BLUFOR_AMG.et"; break;}
				case "OPFOR"  : {prefab = "{FC0904D71EF8DB6A}Prefabs/Characters/Factions/OPFOR/CRF_GS_OPFOR_Rifleman_P.et";   break;}
				case "INDFOR" : {prefab = "{A303C25424BC7149}Prefabs/Characters/Factions/INDFOR/CRF_GS_INDFOR_Rifleman_P.et"; break;}
				case "CIV"    : {prefab = "{71EF8F2C5207403C}Prefabs/Characters/Factions/CIV/CRF_GS_CIV_Rifleman_P.et";       break;}
			}
		}

		Resource resource = Resource.Load(prefab);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
		vector finalSpawnLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(finalSpawnLocation, spawnLocation, 3);

		CRF_Gamemode.GetInstance().RespawnPlayer(playerId, prefab, finalSpawnLocation, groupID);
		LogAdminAction(string.Format("%1 was respawned to %2", GetGame().GetPlayerManager().GetPlayerName(playerId), SCR_GroupsManagerComponent.GetInstance().FindGroup(groupID).m_faction), playerId, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetPlayerGroup(SCR_AIGroup group, int playerID)
	{
		SCR_PlayerFactionAffiliationComponent.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID).FindComponent(SCR_PlayerFactionAffiliationComponent)).RequestFaction(group.GetFaction());
		SCR_GroupsManagerComponent.GetInstance().AddPlayerToGroup(group.GetGroupID(), playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Gear
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SetPlayerGear(int playerID, string prefab)
	{	
		IEntity entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);

		AddGearToEntity(entity, prefab);
		SetPlayerGearScriptsMapValue(prefab, playerID, "GSR"); // GSR = Gear Script Resource
		
		LogAdminAction(string.Format("%1's gear was set to %2", GetGame().GetPlayerManager().GetPlayerName(playerID), prefab.Substring(prefab.LastIndexOf("/") + 1, prefab.LastIndexOf(".") - prefab.LastIndexOf("/") - 1)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void AddItem(int playerID, string prefab)
	{
		if(playerID == 0 || prefab.IsEmpty())
			return;
		
		IEntity entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		SCR_InventoryStorageManagerComponent entityInventoryManager = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
        spawnParams.Transform[3] = entity.GetOrigin();
		ref IEntity resourceSpawned = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
		if(!entityInventoryManager.TryInsertItem(resourceSpawned))
			delete resourceSpawned;
		
		LogAdminAction(string.Format("%2 was added to %1's inventory", GetGame().GetPlayerManager().GetPlayerName(playerID), prefab.Substring(prefab.LastIndexOf("/") + 1, prefab.LastIndexOf(".") - prefab.LastIndexOf("/") - 1)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Teleport
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void TeleportPlayers(int playerID1, int playerID2)
	{
		Rpc(Rpc_TeleportPlayers, playerID1, playerID2);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_TeleportPlayers(int playerID1, int playerID2)
	{
		if(SCR_PlayerController.GetLocalPlayerId() != playerID1)
			return;
		
		IEntity entity2 = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID2);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
	    spawnParams.TransformMode = ETransformMode.WORLD;
		vector teleportLocation = vector.Zero;
		SCR_WorldTools.FindEmptyTerrainPosition(teleportLocation, entity2.GetOrigin(), 3);
	    spawnParams.Transform[3] = teleportLocation;
	
		SCR_Global.TeleportPlayer(playerID1, teleportLocation);
		
		LogAdminAction(string.Format("%1 was teleported to %2", GetGame().GetPlayerManager().GetPlayerName(playerID1), GetGame().GetPlayerManager().GetPlayerName(playerID2)), playerID1, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Hints
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintAll(string data)
	{
		Rpc(Rpc_SendHintAll, data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintAll(string data)
	{
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintPlayer(string data, int playerID)
	{
		Rpc(Rpc_SendHintPlayer, data, playerID);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintPlayer(string data, int playerID)
	{
		if(playerID == 0 || SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;
		
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendHintFaction(string data, string factionKey)
	{
		Rpc(Rpc_SendHintFaction, data, factionKey);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void Rpc_SendHintFaction(string data, string factionKey)
	{
		if(factionKey.IsEmpty() && !SCR_FactionManager.SGetLocalPlayerFaction() && (SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction()).GetFactionKey() != factionKey))
			return;
		
		SendAdminHint(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void SendAdminHint(string data)
	{
		Widget widget;
		widget = GetGame().GetWorkspace().CreateWidgets("{43FC66BA3D85E9C7}UI/layouts/Hint/hint.layout");
		
		if (!widget)
			return;
		
		if(m_wSavedHintWidget)
			delete m_wSavedHintWidget;
		
		m_wSavedHintWidget = widget;
		
		CRF_Hint hint = CRF_Hint.Cast(widget.FindHandler(CRF_Hint));
		hint.ShowHint(data, 8000);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Heal
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayer(int playerID)
	{
		IEntity PlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(PlayerEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return;
		
		damageComponent.FullHeal();
		damageComponent.SetHealthScaled(1);
		
		LogAdminAction(string.Format("%1's was healed", GetGame().GetPlayerManager().GetPlayerName(playerID)), playerID, true);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void HealPlayerVehicle(int playerID)
	{
		IEntity PlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		
		IEntity VehicleEntity = SCR_CompartmentAccessComponent.GetVehicleIn(PlayerEntity);
		if (!VehicleEntity)
			return;
			
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.Cast(VehicleEntity.FindComponent(SCR_DamageManagerComponent));
		if (!damageComponent)
			return;
		
		damageComponent.FullHeal();
		damageComponent.SetHealthScaled(1);
		
		LogAdminAction(string.Format("%1's vehicle was repaired", GetGame().GetPlayerManager().GetPlayerName(playerID)), playerID, true);
	}

	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Log Admin Actions
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	void SendAdminMessage(string data)
	void LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		Rpc(RpcAsk_LogAdminAction, data, playerID, sendToPlayer);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_LogAdminAction(string data, int playerID, bool sendToPlayer)
	{
		if (sendToPlayer)
		{
			if(GetGame().GetPlayerController().GetPlayerId() != playerID && !SCR_Global.IsAdmin())
				return;
		} else {
			if(!SCR_Global.IsAdmin())
				return;
		}

		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		SCR_ChatComponent chatComponent = SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
		if (!chatComponent)
			return;
		chatComponent.ShowMessage(data);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Safestart Functions/Variables
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	[RplProp(onRplName: "OnSafeStartChange")]
	protected bool m_SafeStartEnabled = false;
	ref ScriptInvoker m_OnSafeStartChange = new ScriptInvoker();
	
	[RplProp()]
	protected string m_sServerWorldTime;
	
	[RplProp()]
	protected ref array<string> m_aFactionsStatusArray;
	protected ref array<SCR_Faction> m_aPlayedFactionsArray = new array<SCR_Faction> ;
	
	[RplProp(onRplName: "ShowMessage")]
	protected string m_sMessageContent;
	protected string m_sStoredMessageContent;
	
	[RplProp()]
	protected bool m_bKillRedundantUnitsBool;
	
	protected int m_iTimeSafeStartBegan;
	protected int m_iTimeMissionEnds;
	protected int m_iSafeStartTimeRemaining;
	
	protected bool m_bBluforReady = false;
	protected bool m_bOpforReady = false;
	protected bool m_bIndforReady = false;
	
	protected bool m_bAdminForcedReady = false;
	
	protected int m_iPlayedFactionsCount;
	protected ref map<IEntity,bool> m_mPlayersWithEHsMap = new map<IEntity,bool>;
	
	protected int m_mPlayablesCount = 0;
	
	protected SCR_PopUpNotification m_PopUpNotification = null;

	bool m_bHUDVisible = true;
	CRF_LoggingServerComponent m_Logging;
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// Ready Up functions
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void UpdateHUDVisible()
	{	
		m_bHUDVisible = !m_bHUDVisible;
	};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	TStringArray GetWhosReady() {
		return m_aFactionsStatusArray;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void UpdatePlayedFactions() 
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		SCR_SortedArray<SCR_Faction> outFaction = new SCR_SortedArray<SCR_Faction>;
		factionManager.GetSortedFactionsList(outFaction);
		
		if (!outFaction || outFaction.IsEmpty()) return;
		
		array<SCR_Faction> outArray = new array<SCR_Faction>;
		outFaction.ToArray(outArray);
		
		m_aPlayedFactionsArray.Clear();
		string bluforString = "#Coal_SS_No_Faction";
		string opforString = "#Coal_SS_No_Faction"; 
		string indforString = "#Coal_SS_No_Faction";

		foreach(SCR_Faction faction : outArray) {
			if (faction.GetPlayerCount() == 0 || faction.GetFactionLabel() == EEditableEntityLabel.FACTION_NONE) 
				continue;
			
			m_aPlayedFactionsArray.Insert(faction);
			
			Color factionColor = faction.GetFactionColor();
			float rg = Math.Max(factionColor.R(), factionColor.G());
			float rgb = Math.Max(rg, factionColor.B());
			
			switch (true) {
				case(!m_bBluforReady && rgb == factionColor.B()) : {bluforString = "#Coal_SS_Faction_Not_Ready"; break;};
				case(m_bBluforReady && rgb == factionColor.B())  : {bluforString = "#Coal_SS_Faction_Ready";     break;};
				case(!m_bOpforReady && rgb == factionColor.R())  : {opforString = "#Coal_SS_Faction_Not_Ready";  break;};
				case(m_bOpforReady && rgb == factionColor.R())   : {opforString = "#Coal_SS_Faction_Ready";      break;};
				case(!m_bIndforReady && rgb == factionColor.G()) : {indforString = "#Coal_SS_Faction_Not_Ready"; break;};
				case(m_bIndforReady && rgb == factionColor.G())  : {indforString = "#Coal_SS_Faction_Ready";     break;};
			};
		};
		
		m_aFactionsStatusArray = {bluforString, opforString, indforString};
		m_iPlayedFactionsCount = 0;
		
		foreach (string factionString : m_aFactionsStatusArray)
		{
			if (factionString == "#Coal_SS_No_Faction")
				continue;
			
			m_iPlayedFactionsCount = m_iPlayedFactionsCount + 1;
		}
		
		Replication.BumpMe();
	}
	
	//Call from server
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ToggleSideReady(string setReady, string playerName, bool adminForced) {
		if (!GetSafestartStatus()) return;
		
		// If it's an admin-forced action
		if (adminForced)
		{
			if(m_bAdminForcedReady) 
			{
				m_bBluforReady = false;
				m_bOpforReady = false;
				m_bIndforReady = false;
				m_bAdminForcedReady = false;
			
				m_sMessageContent = string.Format("An Admin (%1) Has Force Unreadied All Sides!", playerName);
				Replication.BumpMe();
				ShowMessage();
				return;
			};
		
			m_bBluforReady = true;
			m_bOpforReady = true;
			m_bIndforReady = true;
			m_bAdminForcedReady = true;
			
			m_sMessageContent = string.Format("An Admin (%1) Has Force Readied All Sides!", playerName);
			Replication.BumpMe();
			ShowMessage();
			return;
		}
		
		if(m_bAdminForcedReady) 
			return;
			
		switch (setReady)
		{
			case("Blufor") : {
				m_bBluforReady = !m_bBluforReady; 
				if (m_bBluforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Readied_Blufor - %1", playerName);
				if (!m_bBluforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Unreadied_Blufor - %1", playerName);
				break;
			};
			case("Opfor")  : {
				m_bOpforReady = !m_bOpforReady;
				if (m_bOpforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Readied_Opfor - %1", playerName);
				if (!m_bOpforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Unreadied_Opfor - %1", playerName);
				break;
			};
			case("Indfor") : {
				m_bIndforReady = !m_bIndforReady; 
				if (m_bIndforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Readied_Indfor - %1", playerName);
				if (!m_bIndforReady) m_sMessageContent = string.Format("#Coal_SS_Faction_Unreadied_Indfor - %1", playerName);
				break;
			};
		};
		Replication.BumpMe();
		ShowMessage();
	}
	
	//Call from server
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void CheckStartCountDown()
	{
		int factionsReadyCount = 0;
		foreach(string factionCheckReadyString : m_aFactionsStatusArray) {
			if (factionCheckReadyString != "#Coal_SS_Faction_Ready") 
				continue;
			factionsReadyCount = factionsReadyCount + 1;
		};
		
		if (factionsReadyCount == 0 && m_iPlayedFactionsCount == 0 || factionsReadyCount != m_iPlayedFactionsCount && m_iSafeStartTimeRemaining == 35) return;
				
		if (factionsReadyCount != m_iPlayedFactionsCount && m_iSafeStartTimeRemaining != 35) {
			m_sMessageContent = "#Coal_SS_Countdown_Cancelled";
			Replication.BumpMe();
			m_iSafeStartTimeRemaining = 35;
			return;
		};
		
		if (factionsReadyCount == m_iPlayedFactionsCount) {
			m_iSafeStartTimeRemaining = m_iSafeStartTimeRemaining - 5;
			m_sMessageContent = string.Format("#Coal_SS_Countdown_Started %1 Seconds!", m_iSafeStartTimeRemaining);
			if (m_iSafeStartTimeRemaining == 0) {
				ToggleSafeStartServer(false);
				m_sMessageContent = "#Coal_SS_Game_Live";
			};
		};
		Replication.BumpMe();
		ShowMessage();
	};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SafeStart functions
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	string GetServerWorldTime()
	{
		return m_sServerWorldTime;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	bool GetSafestartStatus()
	{
		return m_SafeStartEnabled;
	}
	
	void OnSafeStartChange() 
	{
		m_OnSafeStartChange.Invoke(m_SafeStartEnabled);
	}
	
	//Call from server
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void ToggleSafeStartServer(bool status) 
	{
		if (status) { // Turn on safestart
			if (m_SafeStartEnabled) return;
			
			m_iTimeSafeStartBegan = GetGame().GetWorld().GetWorldTime();
			m_SafeStartEnabled = true;
			m_iSafeStartTimeRemaining = 35;
			
			GetGame().GetCallqueue().Remove(UpdateMissionEndTimer);
			GetGame().GetCallqueue().Remove(CheckPlayersAlive);
			
			GetGame().GetCallqueue().CallLater(CheckStartCountDown, 5000, true);
			GetGame().GetCallqueue().CallLater(UpdateServerWorldTime, 1000, true);
			GetGame().GetCallqueue().CallLater(ActivateSafeStartEHs, 5000, true);
			GetGame().GetCallqueue().CallLater(UpdatePlayedFactions, 1000, true);
			
			Replication.BumpMe();//Broadcast m_SafeStartEnabled change
			
		} else { // Turn off safestart
			if (!m_SafeStartEnabled) return;	
			
			UpdatePlayedFactions();
			
			DeleteEmptySlots();
			m_bKillRedundantUnitsBool = true;
			m_bAdminForcedReady = false;
			m_bBluforReady = false;
			m_bOpforReady = false;
			m_bIndforReady = false;
			
			GetGame().GetCallqueue().Remove(CheckStartCountDown);
			GetGame().GetCallqueue().Remove(UpdateServerWorldTime);
			GetGame().GetCallqueue().Remove(ActivateSafeStartEHs);
			GetGame().GetCallqueue().Remove(UpdatePlayedFactions);
			
			GetGame().GetCallqueue().CallLater(CheckPlayersAlive, 5000, true);
			
			if (CRF_Gamemode.GetInstance().m_iTimeLimitMinutes > 0) {
				m_iTimeMissionEnds = GetGame().GetWorld().GetWorldTime() + (CRF_Gamemode.GetInstance().m_iTimeLimitMinutes * 60000);
				GetGame().GetCallqueue().CallLater(UpdateMissionEndTimer, 1000, true);
			} else {
				m_sServerWorldTime = "N/A";
			};
			
			Replication.BumpMe();//Broadcast change
			
			DisableSafeStartEHs();
			
			// Send notification message 
			m_Logging.GameStarted();
			
			// Use CallLater to delay the call for the removal of EHs so the changes so m_SafeStartEnabled can propagate.
			GetGame().GetCallqueue().CallLater(DisableSafeStartEHs, 1500);
			
			// Even longer delay just in case there's any edge cases we didnt anticipate.
			GetGame().GetCallqueue().CallLater(DisableSafeStartEHs, 12500);
			
			GetGame().GetCallqueue().CallLater(DelayChangeSafeStartDisabled, 250);
			
			// Update logging component since game is now live
			CRF_MDB_LoggingServerComponent logCom = CRF_MDB_LoggingServerComponent.GetInstance();
			logCom.m_iPlayerCount = GetGame().GetPlayerManager().GetPlayerCount();
			SCR_FactionManager scrFM = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			logCom.m_iBluforCount = scrFM.GetFactionPlayerCount(GetGame().GetFactionManager().GetFactionByKey("BLUFOR"));
			logCom.m_iOpforCount = scrFM.GetFactionPlayerCount(GetGame().GetFactionManager().GetFactionByKey("OPFOR"));
			logCom.m_iIndforCount = scrFM.GetFactionPlayerCount(GetGame().GetFactionManager().GetFactionByKey("INDFOR"));
			logCom.m_iCivCount = scrFM.GetFactionPlayerCount(GetGame().GetFactionManager().GetFactionByKey("CIV"));
		}
	};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void DelayChangeSafeStartDisabled() {
		m_SafeStartEnabled = false;
		Replication.BumpMe();//Broadcast m_SafeStartEnabled change
	};
	
	//Call from server
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void UpdateServerWorldTime()
	{
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float millis = m_iTimeSafeStartBegan - currentTime;
  		int totalSeconds = (millis / 1000);
		
		m_sServerWorldTime = SCR_FormatHelper.FormatTime(totalSeconds);
		
		Replication.BumpMe();
	};
	
	//Call from server
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void UpdateMissionEndTimer()
	{
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float millis = m_iTimeMissionEnds - currentTime;
  		int totalSeconds = (millis / 1000);
		
		m_sServerWorldTime = SCR_FormatHelper.FormatTime(totalSeconds);
		
		if (totalSeconds == 0) {
			GetGame().GetCallqueue().Remove(UpdateMissionEndTimer);
			m_sServerWorldTime = "Mission Time Expired!";
		};
		
		Replication.BumpMe();
	};
	
	// Why are these two methods done this way? It should just be one wtf
	void DeleteEmptySlots()
	{
		if(CRF_Gamemode.GetInstance().m_bDeleteJIPSlots) 
			if (m_SafeStartEnabled) 
				GetGame().GetCallqueue().CallLater(DeleteEmptySlotsSlowly, 125, true);
	}
	
	void DeleteEmptySlotsSlowly()
	{
		CRF_Gamemode gamemode = CRF_Gamemode.GetInstance();
		if (gamemode.m_bDeleteJIPSlots && !gamemode.m_bRespawnEnabled) {
			for (int i = 0; i < gamemode.m_aEntitySlots.Count(); i++) {
				if (gamemode.m_aSlots.Get(i) == 0 || gamemode.m_aSlots.Get(i) == -1) {
					//Print("Removing Entity");
					gamemode.RemovePlayableEntity(gamemode.m_aEntitySlots.Get(i));
					return;
				}
			}
			GetGame().GetCallqueue().Remove(DeleteEmptySlotsSlowly);
		}
		
	}
	
	// Called from server to all clients
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void ShowMessage()
	{	
		if (m_sMessageContent == m_sStoredMessageContent)
			return;
		
		m_PopUpNotification = SCR_PopUpNotification.GetInstance();
		
		m_sStoredMessageContent = m_sMessageContent;
		
		if (m_sMessageContent == "All Blufor Players Have Been Eliminated!" || m_sMessageContent == "All Opfor Players Have Been Eliminated!" || m_sMessageContent == "All Indfor Players Have Been Eliminated!") 
		{
			m_PopUpNotification.PopupMsg(m_sMessageContent, 30);
			return;
		};

		if (m_sMessageContent == "#Coal_SS_Game_Live")
			m_PopUpNotification.PopupMsg(m_sMessageContent, 8, "#Coal_SS_SafeStart_Started_Subtext");
		else
			m_PopUpNotification.PopupMsg(m_sMessageContent, 2.5, "#Coal_SS_Countdown_Started_Subtext");
	};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	void CheckPlayersAlive()
	{
		foreach(SCR_Faction faction : m_aPlayedFactionsArray) {
			Color factionColor = faction.GetFactionColor();
			float rg = Math.Max(factionColor.R(), factionColor.G());
			float rgb = Math.Max(rg, factionColor.B());
			
			switch (true) {
				case(rgb == factionColor.B() && faction.GetPlayerCount() == 0 && m_aFactionsStatusArray[0] != "#Coal_SS_No_Faction") : { m_sMessageContent = "All Blufor Players Have Been Eliminated!"; break;};
				case(rgb == factionColor.R() && faction.GetPlayerCount() == 0 && m_aFactionsStatusArray[1] != "#Coal_SS_No_Faction") : { m_sMessageContent = "All Opfor Players Have Been Eliminated!";  break;};
				case(rgb == factionColor.G() && faction.GetPlayerCount() == 0 && m_aFactionsStatusArray[2] != "#Coal_SS_No_Faction") : { m_sMessageContent = "All Indfor Players Have Been Eliminated!";  break;};
			};
		};
		
		Replication.BumpMe();
		ShowMessage();
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// SafeStart EHs
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
	protected void ActivateSafeStartEHs()
	{	
		array<int> outPlayers = {};
		GetGame().GetPlayerManager().GetPlayers(outPlayers);
		
		foreach (int playerID : outPlayers)
		{
			IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!controlledEntity) 
				continue;
			
			EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(controlledEntity.FindComponent(EventHandlerManagerComponent));
			if (!eventHandler) 
				continue;
		
			CharacterControllerComponent charComp = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
			if (!charComp) 
				continue;
			
			bool alreadyHasEventHandlers = m_mPlayersWithEHsMap.Get(controlledEntity);
		
			if (!alreadyHasEventHandlers) {
				charComp.SetSafety(true, true);
				eventHandler.RegisterScriptHandler("OnProjectileShot", this, OnWeaponFired);
				eventHandler.RegisterScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
				m_mPlayersWithEHsMap.Set(controlledEntity, true);
			};
		};
	}
	
	protected void DisableSafeStartEHs()
	{	
		for (int i = 0; i < m_mPlayersWithEHsMap.Count(); i++)
		{
			IEntity controlledEntityKey = m_mPlayersWithEHsMap.GetKey(i);
			
			if(!controlledEntityKey)
				continue;
			
			CharacterControllerComponent charComp = CharacterControllerComponent.Cast(controlledEntityKey.FindComponent(CharacterControllerComponent));
			if (!charComp) 
				continue;
			
			charComp.SetSafety(false, false);
			
			EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(controlledEntityKey.FindComponent(EventHandlerManagerComponent));
			if (!eventHandler) 
				continue;
			
			eventHandler.RemoveScriptHandler("OnProjectileShot", this, OnWeaponFired);
			eventHandler.RemoveScriptHandler("OnGrenadeThrown", this, OnGrenadeThrown);
			
			m_mPlayersWithEHsMap.Set(controlledEntityKey, false);
		};
	};
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void OnWeaponFired(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{		
		// Get projectile and delete it
		delete entity;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	protected void OnGrenadeThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (!weapon)
			return;
		
		// Get grenade and delete it
		delete entity;
	}
}