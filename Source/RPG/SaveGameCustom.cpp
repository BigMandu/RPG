// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveGameCustom.h"

/*
ĳ���� ���� (Health, Stamina, Coin, Soul, Skill Damage, Damage)�� �����ؾ���.
��, �÷��̾��� �̸��� Slot�� �ʿ���.
*/

USaveGameCustom::USaveGameCustom()
{
	SlotName = TEXT("Default");
	UserIndex = 0;
	SaveCharacterStats.SaveWeaponName = TEXT("");
	SaveCharacterStats.MapName = TEXT("");
}