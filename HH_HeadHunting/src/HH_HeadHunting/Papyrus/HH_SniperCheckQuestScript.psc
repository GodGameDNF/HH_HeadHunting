ScriptName HH_SniperCheckQuestScript Extends ReferenceAlias

Perk Property HH_Perk Auto
Static Property HH_ShotMSTT Auto
Weapon Property HH_ShotWeapon Auto
Keyword Property WeaponTypeSniper Auto
ObjectReference Property HH_ReturnMarker Auto

Actor PlayerRef

Event OnAliasInit()
	PlayerRef = GetActorRef()
	if PlayerRef.GetEquippedWeapon().HasKeyword(WeaponTypeSniper)
		PlayerRef.AddPerk(HH_Perk)
	Endif
EndEvent

Event OnItemEquipped(Form akBaseObject, ObjectReference akReference)
	if akBaseObject as Weapon
		if akBaseObject.HasKeyword(WeaponTypeSniper)
			PlayerRef.AddPerk(HH_Perk)
		Endif
	Endif
EndEvent

Event OnItemUnEquipped(Form akBaseObject, ObjectReference akReference)
	if akBaseObject as Weapon
		if akBaseObject.HasKeyword(WeaponTypeSniper)
			PlayerRef.RemovePerk(HH_Perk)
		Endif
	Endif
EndEvent