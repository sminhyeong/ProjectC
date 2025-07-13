// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemStruct.h"
#include "InventoryManager.generated.h"

class AItemBase;
class UInventoryWidget;
class UNetworkManager;

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

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void C2S_TryAddItem(int32 NewItemID, int32 NewItemCount, int32 UserID);
	void C2S_TryAddItem_Implementation(int32 NewItemID, int32 NewItemCount, int32 UserID);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void C2S_TrySubtractItem(int32 SubItemID, int32 SubItemCount, int32 UserID);
	void C2S_TrySubtractItem_Implementation(int32 SubItemID, int32 SubItemCount, int32 UserID);

	UFUNCTION(BlueprintCallable)
	bool CheckInventoryHasSpace(EItemCategory ItemCategory);

	// DB로부터 정보를 받아와서 Parse후 InventoryManager에 저장하는 함수
	UFUNCTION(BlueprintCallable)
	void UpdateInventoryData(int32 UseId);

	// InventoryManager에 저장된 정보로 Widget을 갱신하는 함수
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void S2C_UpdateInventoryWidget(UInventoryWidget* InventoryWindow);
	void S2C_UpdateInventoryWidget_Implementation(UInventoryWidget* InventoryWindow);

	// 서버에서 인벤토리 확인 후 가득 찼다면 이 함수 호출
	UFUNCTION(BlueprintNativeEvent)
	void InventoryIsFull();
	void InventoryIsFull_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	UNetworkManager* GetNetworkManager_Inventory();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// DB로부터 정보를 받아와서 InventoryManager로 가져오는 함수
	UFUNCTION()
	TArray<FAccountItemInfo> GetInventoryDataToDB(int32 UseId);

	// 받아온 정보를 InventoryManager에 Parse해서 저장하는 함수
	UFUNCTION(Client, Reliable)
	void S2C_ParseInventoryData(const TArray<FAccountItemInfo>& InventoryItems);
	void S2C_ParseInventoryData_Implementation(const TArray<FAccountItemInfo>& InventoryItems);

private:
	UPROPERTY(Replicated, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> WeaponList;

	UPROPERTY(Replicated, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> ArmorList;

	UPROPERTY(Replicated, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TArray<FItemAllInfo> ConsumeList;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryWidget> InventoryWindowWidget;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNetworkManager> NetworkManager;

	UPROPERTY(EditAnywhere)
	int32 MaxItemPerCategory{ 20 };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UDataTable> DT_ItemArtInfo;

};
