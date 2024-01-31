ScriptName HH_HeadMEScript Extends ActiveMagicEffect

Event OnEffectStart(Actor akTarget, Actor akCaster)
	HH_HeadHunting.HitHead(akTarget)
EndEvent