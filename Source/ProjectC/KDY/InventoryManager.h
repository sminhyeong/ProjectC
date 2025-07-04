// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemStruct.h"
#include "InventoryManager.generated.h"

class AItemBase;
class UInventoryWidget;

UCLASS()
class PROJECTC_API AInventoryManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInventoryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void C2S_AddItem(FRPGItemData NewItemData);
	bool C2S_AddItem_Validate(FRPGItemData NewItemData);
	void C2S_AddItem_Implementation(FRPGItemData NewItemData);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void S2C_UpdateInventory(EItemCategory UpdateListCategory);
	void S2C_UpdateInventory_Implementation(EItemCategory UpdateListCategory);

	// 서버에서 인벤토리 확인 후 가득 찼다면 이 함수 호출
	UFUNCTION(BlueprintNativeEvent)
	void InventoryIsFull();
	void InventoryIsFull_Implementation();

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FRPGItemData> WeaponList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FRPGItemData> ArmorList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FRPGItemData> ConsumeList;

	UPROPERTY(EditAnywhere)
	int32 MaxItemPerCategory{ 20 };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryWidget> InventoryWidget;

};
