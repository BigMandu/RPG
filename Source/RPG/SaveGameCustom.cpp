// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameCustom.h"

/*
캐릭터 스텟 (Health, Stamina, Coin, Soul, Skill Damage, Damage)을 저장해야함.
또, 플레이어의 이름과 Slot이 필요함.
*/

USaveGameCustom::USaveGameCustom()
{
	SlotName = TEXT("Default");
	UserIndex = 0;
	SaveCharacterStats.SaveWeaponName = TEXT("");
	SaveCharacterStats.MapName = TEXT("");
}