// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SearchPatrolLocation.generated.h"

/**
 * 
 */
UCLASS()
class RPG_API UBTTask_SearchPatrolLocation : public UBTTaskNode
{
	GENERATED_BODY()
public:

	UBTTask_SearchPatrolLocation();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
