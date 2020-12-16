// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyWidget.h"
#include "Components/ProgressBar.h"


float UEnemyWidget::SetHPRatio()
{
	if (EnemyMaxHP != 0.f)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("EnemyMaxHP is : %f,  EnemyHP is : %f"), EnemyMaxHP, EnemyHP);
		UE_LOG(LogTemp, Warning, TEXT("EnemyWidget::HPRatio is : %f"), (EnemyHP <= 0) ? 0.f : (EnemyHP / EnemyMaxHP));*/

		return (EnemyHP <= 0.f) ? 0.f : (EnemyHP / EnemyMaxHP);
	}
	return 0.f;
}
//
//void UEnemyWidget::NativeConstruct()
//{
//	Super::NativeConstruct();
//	HPbar = Cast<UProgressBar>(GetWidgetFromName(TEXT("EnemyHPbar")));
//	if (HPbar != nullptr)
//	{
//		UpdateHP();
//	}
//}
//
//void UEnemyWidget::UpdateHP()
//{
//	if (HPbar != nullptr)
//	{
//		HPbar->SetPercent(SetHPRatio());
//	}
//}