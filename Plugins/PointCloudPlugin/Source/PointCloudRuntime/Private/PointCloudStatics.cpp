#include "PointCloudStatics.h"
#include "PointCloudShared.h"
#include "PointCloud.h"
#include "PointCloudHelper.h"

#define IS_EXT(Ext) (FileExtension.Equals(TEXT(Ext), ESearchCase::IgnoreCase))

UPointCloud* UPointCloudStatics::LoadPointCloudFromFile(FString Filename, int32 FirstLine, int32 LastLine, FVector2D RGBRange, FPointCloudImportSettingsColumns Columns)
{
	UPointCloudImportSettings *ImportSettings = NewObject<UPointCloudImportSettings>();
	ImportSettings->LoadFileHeader(Filename);
	ImportSettings->LineRange = FPointCloudImportSettingsLineRange(FirstLine, LastLine);
	ImportSettings->FileHeader.RGBRange = RGBRange;
	ImportSettings->FileHeader.SelectedColumns = { FMath::Max(0, Columns.LocationX), FMath::Max(0, Columns.LocationY), FMath::Max(0, Columns.LocationZ), FMath::Max(0, Columns.Red), FMath::Max(0, Columns.Green), FMath::Max(0, Columns.Blue) };

	return LoadPointCloudFromFile_Full(Filename, ImportSettings, (UObject*)GetTransientPackage(), NAME_None, RF_NoFlags);
}

class UPointCloud* UPointCloudStatics::LoadPointCloudFromFile_Full(FString Filename, UPointCloudImportSettings *ImportSettings, UObject* InParent, FName InName, EObjectFlags Flags)
{
	TArray<FPointCloudPoint> Points;
	EPointCloudColorMode ColorMode = EPointCloudColorMode::None;
	bool bSuccess = false;

	FString FileExtension = FPaths::GetExtension(Filename);
	bool bIsText = IS_EXT("txt") || IS_EXT("csv") || IS_EXT("xyz");

	UPointCloud *PC = NULL;

	if (bIsText)
	{
		bSuccess = FPointCloudHelper::ImportAsText(Filename, Points, ColorMode, ImportSettings->LineRange.First, ImportSettings->LineRange.Last, ImportSettings->FileHeader);
	}
	else
	{
		TArray<uint8> Data;
		if (FFileHelper::LoadFileToArray(Data, *Filename))
		{
			Data.Add(0);
			const uint8* Buffer = &Data[0];

			if (IS_EXT("las"))
			{
				bSuccess = FPointCloudHelper::ImportAsLAS(Buffer, Points, ColorMode, ImportSettings->LineRange.First, ImportSettings->LineRange.Last);
			}
			else if (IS_EXT("bin"))
			{
				bSuccess = FPointCloudHelper::ImportAsBIN(Buffer, Points, ColorMode, ImportSettings->LineRange.First, ImportSettings->LineRange.Last);
			}
		}
	}

	if (bSuccess)
	{
			PC = NewObject<UPointCloud>(InParent, InName, Flags);
			PC->SetSettings((UPointCloudSettings*)ImportSettings);
			PC->ColorMode = ColorMode;
			PC->SetPointCloudData(Points);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Parsing Failed"));
	}

	return PC;
}

#undef IS_EXT