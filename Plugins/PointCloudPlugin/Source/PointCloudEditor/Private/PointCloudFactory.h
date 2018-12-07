// Copyright 2018 Michal Cieciura. All Rights Reserved.

#pragma once
#include "UnrealEd.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"
#include "Factories/Factory.h"
#include "PointCloudRuntime/Public/PointCloud.h"
#include "PointCloudFactory.generated.h"

class FAssetTypeActions_PointCloud : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_PointCloud() {}

	// Begin IAssetTypeActions Interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override { return FColor(0, 128, 128); }
	virtual UClass* GetSupportedClass() const override { return UPointCloud::StaticClass(); }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::Misc; }
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	// End IAssetTypeActions Interface
};

UCLASS()
class UPointCloudFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY()
	class UPointCloudImportSettings *ImportSettings;

public:
	// Begin UFactory Interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual bool FactoryCanImport(const FString& Filename) override { return true; }
	virtual FText GetDisplayName() const override { return FText::FromString("Point Cloud"); }
	// End UFactory Interface

private:
	bool IsTextFile(FString Filename)
	{
		FString FileExtension = FPaths::GetExtension(Filename);
		return FileExtension.Equals(TEXT("xyz"), ESearchCase::IgnoreCase) || FileExtension.Equals(TEXT("csv"), ESearchCase::IgnoreCase) || FileExtension.Equals(TEXT("txt"), ESearchCase::IgnoreCase);
	}
};