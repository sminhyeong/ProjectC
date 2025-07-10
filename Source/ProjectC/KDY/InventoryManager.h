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

	UFUNCTION(BlueprintCallable)
	bool TryAddItem(int32 NewItemID, int32 NewItemCount);

	UFUNCTION(BlueprintCallable)
	bool CheckInventoryHasSpace(EItemCategory ItemCategory);

	// DB로부터 정보를 받아와서 Parse후 InventoryManager에 저장하는 함수
	UFUNCTION(BlueprintCallable)
	void UpdateInventoryData();

	// InventoryManager에 저장된 정보로 Widget을 갱신하는 함수
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void S2C_UpdateInventoryWidget();
	void S2C_UpdateInventoryWidget_Implementation();

	// 서버에서 인벤토리 확인 후 가득 찼다면 이 함수 호출
	UFUNCTION(BlueprintNativeEvent)
	void InventoryIsFull();
	void InventoryIsFull_Implementation();

private:
	// DB로부터 정보를 받아와서 InventoryManager로 가져오는 함수
	UFUNCTION()
	TArray<FAccountItemInfo> GetInventoryDataToDB();

	// 받아온 정보를 InventoryManager에 Parse해서 저장하는 함수
	UFUNCTION(Client, Reliable)
	void S2C_ParseInventoryData(const TArray<FAccountItemInfo>& InventoryItems);
	void S2C_ParseInventoryData_Implementation(const TArray<FAccountItemInfo>& InventoryItems);

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> WeaponList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> ArmorList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> ConsumeList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryWidget> InventoryWidget;

	UPROPERTY(EditAnywhere)
	int32 MaxItemPerCategory{ 20 };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> DT_ItemArtInfo;

};
