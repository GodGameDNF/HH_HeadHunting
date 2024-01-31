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
	if akBaseObject.HasKeyword(WeaponTypeSniper)
		PlayerRef.AddPerk(HH_Perk)
	else
		PlayerRef.RemovePerk(HH_Perk)
	Endif
EndEvent