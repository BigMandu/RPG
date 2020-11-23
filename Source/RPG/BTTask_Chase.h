// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_Chase.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API UBTTask_Chase : public UBTTask_MoveTo
{
	GENERATED_BODY()
public:
	UBTTask_Chase();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
