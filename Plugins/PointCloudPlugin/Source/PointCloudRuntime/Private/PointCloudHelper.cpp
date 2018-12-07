// Copyright 2018 Michal Cieciura. All Rights Reserved.

#include "PointCloudHelper.h"
#include "PointCloudShared.h"
#include "PointCloud.h"
#include "Async/Async.h"
#include "Async/Future.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "FileManager.h"
#include "FileHelper.h"
#include "Package.h"

#if WITH_EDITOR
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialFunction.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionMaterialFunctionCall.h"
#include "Materials/MaterialExpressionFunctionOutput.h"
#include "Materials/MaterialExpressionFunctionInput.h"
#endif

bool FPointCloudHelper::ImportAsBIN(const uint8*& Buffer, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex)
{
	bool bHasRGB;
	FMemory::Memcpy(&bHasRGB, Buffer, 1);

	ColorMode = EPointCloudColorMode::Intensity;
	
	uint32 NumPoints = 0;
	FMemory::Memcpy(&NumPoints, Buffer + 1, 4);
	
	if (NumPoints == 0)
	{
		return false;
	}

	int32 Offset = 5;
	uint32 LastPoint = NumPoints - 1;

	if (LastIndex >= FirstIndex && LastIndex > 0)
	{
		LastPoint = FMath::Min(LastPoint, LastIndex);
	}

	OutPoints.Reset(LastPoint - FirstIndex + 1);

	FVector Location;
	FVector Color;

	for (uint32 i = FirstIndex; i <= LastPoint; i++)
	{
		FPointCloudPoint Point;

		FMemory::Memcpy(&Location, Buffer + Offset, 12);
		Offset += 12;

		if (bHasRGB)
		{
			FMemory::Memcpy(&Color, Buffer + Offset, 12);
			Offset += 12;

			Point = FPointCloudPoint(Location, Color);
		}
		else
		{
			Point = FPointCloudPoint(Location);
		}
		
		OutPoints.Add(Point);
	}

	return true;
}

bool FPointCloudHelper::ImportAsLAS(const uint8*& Buffer, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex)
{
	UE_LOG(LogTemp, Error, TEXT("LAS files are not implemented yet"));
	return false;
}

bool FPointCloudHelper::ImportAsText(const FString& Filename, TArray<FPointCloudPoint>& OutPoints, EPointCloudColorMode &ColorMode, uint32 FirstIndex, uint32 LastIndex, FPointCloudFileHeader PointCloudFileHeader)
{
	// Modified FFileHelper::LoadFileToString, FFileHelper::LoadFileToStringArray and String::ParseIntoArray
	//@Todo: Add multithreading support
	TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Filename));
	if (Reader)
	{
		int64 Size = Reader->TotalSize();

		uint8* Data = (uint8*)FMemory::Malloc(Size);
		Reader->Serialize(Data, Size);
		Reader->Close();
		Reader = nullptr;
		uint8 *DataPtr = Data;

		float *TempFloats = new float[PointCloudFileHeader.SelectedColumns.Num()]();
		uint8 Iterator = 0;
		bool bEndOfLine = false;

		// Determine which values to use for Min/Max
		bool SampleChannel[] = { (PointCloudFileHeader.SelectedColumns[3] > -1), (PointCloudFileHeader.SelectedColumns[4] > -1), (PointCloudFileHeader.SelectedColumns[5] > -1) };

		// Check Data Usage
		uint8 NumExpectedColumns = 3 + (SampleChannel[0] + SampleChannel[1] + SampleChannel[2]);
		switch (NumExpectedColumns)
		{
		case 4:
			ColorMode = EPointCloudColorMode::Intensity;
			break;

		case 5:
		case 6:
			ColorMode = EPointCloudColorMode::RGB;
			break;

		default:
			ColorMode = EPointCloudColorMode::None;
			break;
		}
		
		// If the range has not been set, set it now
		if (FMath::IsNearlyZero(PointCloudFileHeader.RGBRange.X) && FMath::IsNearlyZero(PointCloudFileHeader.RGBRange.Y))
		{
			PointCloudFileHeader.RGBRange = ReadFileMinMaxColumns(DataPtr, PointCloudFileHeader.Delimiter[0], { PointCloudFileHeader.SelectedColumns[3] , PointCloudFileHeader.SelectedColumns[4] , PointCloudFileHeader.SelectedColumns[5] });
		}

		float RGBMulti = 1 / (PointCloudFileHeader.RGBRange.Y - PointCloudFileHeader.RGBRange.X);

		OutPoints.Reset();

		FirstIndex += PointCloudFileHeader.LinesToSkip;
		LastIndex += PointCloudFileHeader.LinesToSkip;

		uint8 LoadedColumns = 0;

		for (uint32 LineIndex = 0; *DataPtr != 0;)
		{
			const char* ChunkStart = (char*)DataPtr;
			while (*DataPtr != 0 && *DataPtr != '\r' && *DataPtr != '\n' && *DataPtr != PointCloudFileHeader.Delimiter[0])
			{
				DataPtr++;
			}

			// Don't parse until the first index specified
			if (LineIndex >= FirstIndex)
			{
				if (LastIndex == PointCloudFileHeader.LinesToSkip || LastIndex < FirstIndex || LineIndex <= LastIndex)
				{
					int32 ColumnIndex = PointCloudFileHeader.SelectedColumns.IndexOfByKey(Iterator++);
					
					// Is the column assigned to anything?
					if (ColumnIndex != INDEX_NONE)
					{
						LoadedColumns++;
						TempFloats[ColumnIndex] = (float)atof(ChunkStart);
					}
				}
				else
				{
					// Stop parsing after the last line specified
					break;
				}
			}

			if (*DataPtr == PointCloudFileHeader.Delimiter[0])
			{
				DataPtr++;
			}
			if (*DataPtr == '\r')
			{
				DataPtr++;
				bEndOfLine = true;
			}
			if (*DataPtr == '\n')
			{
				DataPtr++;
				bEndOfLine = true;
			}
			if (*DataPtr == 0)
			{
				bEndOfLine = true;
			}

			if (bEndOfLine)
			{
				if (LoadedColumns == NumExpectedColumns)
				{
					float R = FMath::Clamp((TempFloats[3] - PointCloudFileHeader.RGBRange.X) * RGBMulti, 0.0f, 1.0f);
					float G = FMath::Clamp((TempFloats[4] - PointCloudFileHeader.RGBRange.X) * RGBMulti, 0.0f, 1.0f);
					float B = FMath::Clamp((TempFloats[5] - PointCloudFileHeader.RGBRange.X) * RGBMulti, 0.0f, 1.0f);
					OutPoints.Emplace(TempFloats[0], TempFloats[1], TempFloats[2], R, G, B);
				}

				LineIndex++;
				Iterator = 0;
				bEndOfLine = false;
				LoadedColumns = 0;
			}
		}

		delete[] TempFloats;
		TempFloats = NULL;
		FMemory::Free(Data);
		Data = NULL;
		DataPtr = NULL;

		return true;
	}

	return false;
}

FPointCloudFileHeader FPointCloudHelper::ReadFileHeader(const FString& Filename)
{
	FPointCloudFileHeader Header;

	TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Filename));
	if (Reader)
	{
		Header.Filename = Filename;

		int64 TotalSize = Reader->TotalSize();
		int64 Size = FMath::Min(TotalSize, (int64)2048);

		uint8* Data = (uint8*)FMemory::Malloc(Size);
		Reader->Serialize(Data, Size);
		Reader->Close();
		Reader = nullptr;
		uint8 *DataPtr = Data;

		int32 UnrecognizableLines = 0;
		bool bFetchNextLine = true;

		FString Line;
		uint8 *LineStart = NULL;

		while (bFetchNextLine)
		{
			bFetchNextLine = false;

			LineStart = DataPtr;

			while (*DataPtr != 0 && *DataPtr != '\r' && *DataPtr != '\n')
			{
				DataPtr++;
			}

			FFileHelper::BufferToString(Line, LineStart, DataPtr - LineStart);
			
			// Determine Delimiter
			if (Line.Contains(","))
			{
				Header.Delimiter = ",";
			}
			else if (Line.Contains(" "))
			{
				Header.Delimiter = " ";
			}
			else if (Line.Contains("\t"))
			{
				Header.Delimiter = "\t";
			}

			Line.ParseIntoArray(Header.Columns, *Header.Delimiter);

			if (Header.Columns.Num() < 3)
			{
				// Skip line
				UnrecognizableLines++;
				bFetchNextLine = true;

				if (*DataPtr == '\r')
				{
					DataPtr++;
				}
				if (*DataPtr == '\n')
				{
					DataPtr++;
				}
			}
		}

		LineStart = NULL;

		// Check if first line is text-based
		Header.bHasDescriptions = Header.Columns.Num() > 0 && !FCString::IsNumeric(*Header.Columns[0]);

		// Skip irrelevant lines
		Header.LinesToSkip = UnrecognizableLines + Header.bHasDescriptions;

		// Estimate number of points
		uint64 ContentSize = TotalSize;
		if (Header.LinesToSkip > 0)
		{
			Size -= Line.Len() + 1;
			ContentSize -= Line.Len() + 1;

			if (*DataPtr == '\r')
			{
				DataPtr++;
			}
			if (*DataPtr == '\n')
			{
				DataPtr++;
			}
		}

		int32 LineCount = 0;
		int32 Count = 0;
		while (*DataPtr != 0)
		{
			while (*DataPtr != 0 && *DataPtr != '\r' && *DataPtr != '\n')
			{
				Count++;
				DataPtr++;
			}

			if (Count > 0)
			{
				LineCount++;
				Count = 0;
			}

			if (*DataPtr == '\r')
			{
				DataPtr++;
			}
			if (*DataPtr == '\n')
			{
				DataPtr++;
			}
		}

		Header.EstimatedPointCount = ContentSize * LineCount / Size;

		FMemory::Free(Data);
		DataPtr = NULL;
	}

	return Header;
}

FVector2D FPointCloudHelper::ReadFileMinMaxColumn(const FString& Filename, int32 ColumnIndex)
{
	return ReadFileMinMaxColumns(Filename, { ColumnIndex });
}
FVector2D FPointCloudHelper::ReadFileMinMaxColumns(const FString& Filename, TArray<int32> Columns)
{
	FPointCloudFileHeader Header = ReadFileHeader(Filename);
	FVector2D Result;

	TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*Filename));
	if (Reader)
	{
		int64 Size = Reader->TotalSize();

		uint8* Data = (uint8*)FMemory::Malloc(Size);
		Reader->Serialize(Data, Size);
		Reader->Close();
		Reader = nullptr;

		Result = ReadFileMinMaxColumns(Data, Header.Delimiter[0], Columns);

		FMemory::Free(Data);
		Data = NULL;
	}

	return Result;
}
FVector2D FPointCloudHelper::ReadFileMinMaxColumns(const uint8 *DataPtr, uint8 Delimiter, TArray<int32> Columns)
{
	FVector2D Result = FVector2D(FLT_MAX, -FLT_MAX);
	float Tmp = 0;

	uint8 Iterator = 0;
	bool bEndOfLine = false;

	for (; *DataPtr != 0;)
	{
		const char* ChunkStart = (char*)DataPtr;
		while (*DataPtr != 0 && *DataPtr != '\r' && *DataPtr != '\n' && *DataPtr != Delimiter)
		{
			DataPtr++;
		}

		if (Columns.Contains(Iterator++))
		{
			Tmp = (float)atof(ChunkStart);
			Result.X = FMath::Min(Result.X, Tmp);
			Result.Y = FMath::Max(Result.Y, Tmp);
		}

		if (*DataPtr == Delimiter)
		{
			DataPtr++;
		}
		if (*DataPtr == '\r')
		{
			DataPtr++;
			bEndOfLine = true;
		}
		if (*DataPtr == '\n')
		{
			DataPtr++;
			bEndOfLine = true;
		}

		if (bEndOfLine)
		{
			Iterator = 0;
			bEndOfLine = false;
		}
	}

	return Result;
}

void FPointCloudHelper::DensityReduction(TArray<FPointCloudPoint>& Points, const float MinDistanceBetweenPoints)
{
	if (MinDistanceBetweenPoints > 0)
	{	
		FVector CellSize = CalculateBounds(Points).GetSize() * 0.101;
		for (int32 pass = 0; pass < 2; pass++)
		{
			TArray<TArray<FPointCloudPoint*>> Sections = SplitIntoSections(Points, CellSize, 10, 2800);

			int32 NumThreads = Sections.Num();
			TFuture<bool> *Results = new TFuture<bool>[NumThreads];

			// Fire threads
			for (int32 i = 0; i < NumThreads; i++)
			{
				Results[i] = Async<bool>(EAsyncExecution::ThreadPool, [i, &Sections, &MinDistanceBetweenPoints]
				{
					TArray<FPointCloudPoint*>& Section = Sections[i];

					for (int32 j = 0; j < Section.Num(); j++)
					{
						if (!Section[j]->bEnabled)
						{
							continue;
						}

						for (int32 k = 0; k < Section.Num(); k++)
						{
							if (j == k || !Section[k]->bEnabled)
								continue;

							Section[k]->bEnabled = Section[j]->GridDistance(Section[k]) > MinDistanceBetweenPoints;
						}
					}

					return true;
				});
			}

			// Sync
			for (int32 i = 0; i < NumThreads; i++)
			{
				if (Results[i].Get()) {}
			}

			delete[] Results;

			CellSize *= 0.75;
		}
	}
}

void FPointCloudHelper::NoiseReduction(TArray<FPointCloudPoint>& Points, const float MaxDistanceBetweenPoints, const int32 MinPointDensity)
{
	if (MaxDistanceBetweenPoints <= 0 || MinPointDensity <= 0)
	{
		return;
	}

	TArray<TArray<FPointCloudPoint*>> Sections = SplitIntoSections(Points, CalculateBounds(Points).GetSize() * 0.101, 1, 2800);

	int32 NumThreads = Sections.Num();
	TFuture<bool> *Results = new TFuture<bool>[NumThreads];

	// Fire threads
	for (int32 i = 0; i < NumThreads; i++)
	{
		Results[i] = Async<bool>(EAsyncExecution::ThreadPool, [i, &Sections, &MaxDistanceBetweenPoints, &MinPointDensity]
		{
			TArray<FPointCloudPoint*>& Section = Sections[i];
			
			for (int32 j = 0; j < Section.Num(); j++)
			{				
				if (!Section[j]->bEnabled)
				{
					continue;
				}

				int32 Neighbors = 0;
				Section[j]->bEnabled = false;

				for (int32 k = 0; k < Section.Num(); k++)
				{
					if (j == k || !Section[k]->bEnabled)
						continue;

					if (Section[j]->GridDistance(Section[k]) <= MaxDistanceBetweenPoints)
					{
						// If minimum density reached, no need to check this point further
						if (++Neighbors >= MinPointDensity)
						{
							Section[j]->bEnabled = true;
							break;
						}
					}
				}
			}

			return true;
		});
	}

	// Sync
	for (int32 i = 0; i < NumThreads; i++)
	{
		if (Results[i].Get()) {}
	}

	delete[] Results;
}

FVector FPointCloudHelper::Transform(TArray<FPointCloudPoint>& Points, const EPointCloudOffset Offset, const FVector Translation, const FVector Scale, bool bUseLowPrecision)
{
	FVector OffsetTranslation;

	switch (Offset)
	{
	case EPointCloudOffset::Center:
		OffsetTranslation = CalculateBounds(Points).GetCenter();
		break;
	
	case EPointCloudOffset::FirstPoint:
		if (Points.Num() > 0)
		{
			OffsetTranslation = Points[0].Location;
		}
		break;

	default:
		OffsetTranslation = FVector::ZeroVector;
		break;
	}

	// Flips X axis
	FVector CorrectedScale = FVector(-Scale.X, Scale.Y, Scale.Z);

	for (FPointCloudPoint& Point : Points)
	{
		Point.Location = (Point.Location + Translation - OffsetTranslation) * CorrectedScale;

		if (bUseLowPrecision)
		{
			Point.Location = FVector((FFloat16)Point.Location.X, (FFloat16)Point.Location.Y, (FFloat16)Point.Location.Z);
		}
	}

	return OffsetTranslation;
}

TArray<TArray<FPointCloudPoint*>> FPointCloudHelper::SplitIntoSections(TArray<FPointCloudPoint>& Points, FVector SectionSize, int32 MinSectionCount, int32 MaxSectionCount)
{
	TArray<FPointCloudPoint*> PointsPtr;
	PointsPtr.AddUninitialized(Points.Num());
	for (int32 i = 0; i < Points.Num(); i++)
	{
		PointsPtr[i] = &Points[i];
	}

	return SplitIntoSections(PointsPtr, SectionSize, MinSectionCount, MaxSectionCount);
}
TArray<TArray<FPointCloudPoint*>> FPointCloudHelper::SplitIntoSections(TArray<FPointCloudPoint*>& Points, FVector SectionSize, int32 MinSectionCount /*= 1*/, int32 MaxSectionCount /*= 0*/)
{
	TArray<TArray<FPointCloudPoint*>> Chunks;

	if (SectionSize.X <= 0 || SectionSize.Y <= 0 || SectionSize.Z <= 0)
	{
		return Chunks;
	}

	FBox Bounds = CalculateBounds(Points);
	FVector BoundSize = Bounds.GetSize();

	int32 CellsX = FMath::CeilToInt(BoundSize.X / SectionSize.X);
	int32 CellsY = FMath::CeilToInt(BoundSize.Y / SectionSize.Y);
	int32 CellsZ = FMath::CeilToInt(BoundSize.Z / SectionSize.Z);

	FVector InvertedSectionSize = FVector::OneVector / SectionSize;

	// Initialize
	Chunks.AddDefaulted(CellsX * CellsY * CellsZ);

	// Calculate strides
	int32 StrideY = CellsX;
	int32 StrideZ = CellsX * CellsY;

	int32 NumPoints = Points.Num();
	int32 *Assignments = new int32[NumPoints];
	int32 PerBatch = 5000;
	int32 NumThreads = FMath::CeilToInt(((float)Points.Num()) / PerBatch);
	TFuture<bool> *Results = new TFuture<bool>[NumThreads];

	for (int32 i = 0; i < NumPoints; i++)
	{
		Assignments[i] = -1;
	}

	for (int32 b = 0; b < NumThreads; b++)
	{
		Results[b] = Async<bool>(EAsyncExecution::ThreadPool, [b, &PerBatch, &NumPoints, &Assignments, &Points, &Bounds, &InvertedSectionSize, &CellsX, &CellsY, &CellsZ, &StrideY, &StrideZ]
		{
			for (int32 i = 0, idx = b * PerBatch; i < PerBatch && idx < NumPoints; i++, idx++)
			{
				if (!Points[idx]->bEnabled)
				{
					continue;
				}

				FVector v = (Points[idx]->Location - Bounds.Min) * InvertedSectionSize;
				Assignments[idx] = FMath::Min((int32)v.X, CellsX - 1) + FMath::Min((int32)v.Y, CellsY - 1) * StrideY + FMath::Min((int32)v.Z, CellsZ - 1) * StrideZ;
			}
				
			return true;
		});
	}

	// Sync
	for (int32 i = 0; i < NumThreads; i++)
	{
		if (Results[i].Get()) {}
	}

	// Assign
	for (int32 i = 0; i < NumPoints; i++)
	{
		if (Assignments[i] >= 0)
		{
			Chunks[Assignments[i]].Add(Points[i]);
		}
	}

	delete[] Results;
	delete[] Assignments;
	
	// Remove Empty Sections
	MinSectionCount = FMath::Max(MinSectionCount, 1);
	for (int32 i = 0; i < Chunks.Num(); i++)
	{
		if (Chunks[i].Num() < MinSectionCount)
		{
			Chunks.RemoveAt(i--);
		}
	}

	// Further divide if needed
	if (MaxSectionCount > 0)
	{
		for (int32 i = 0; i < Chunks.Num(); i++)
		{
			if (Chunks[i].Num() > MaxSectionCount)
			{
				Chunks.Append(SplitIntoSections(Chunks[i], SectionSize * 0.5f, MinSectionCount, MaxSectionCount));
				Chunks.RemoveAt(i--);
			}
		}
	}

	return Chunks;
}

FVector FPointCloudHelper::AdjustSectionSize(TArray<FPointCloudPoint>& Points, const FVector SectionSize)
{
	FVector Out = SectionSize;

	if (Out.X <= 0 || Out.Y <= 0 || Out.Z <= 0)
	{
		FVector BoundSize = CalculateBounds(Points).GetSize();

		if (Out.X <= 0)
		{
			Out.X = FMath::Max(BoundSize.X / 30.0f, 1000.0f);
		}
		if (Out.Y <= 0)
		{
			Out.Y = FMath::Max(BoundSize.Y / 30.0f, 1000.0f);
		}
		if (Out.Z <= 0)
		{
			Out.Z = FMath::Max(BoundSize.Z / 30.0f, 1000.0f);
		}
	}

	return Out;
}

int32 FPointCloudHelper::CountEnabledPoints(const TArray<FPointCloudPoint*>& Points)
{
	int32 Count = 0;

	for (int32 i = 0; i < Points.Num(); i++)
	{
		if (Points[i]->bEnabled)
		{
			Count++;
		}
	}

	return Count;
}
int32 FPointCloudHelper::CountEnabledPoints(const TArray<FPointCloudPoint>& Points)
{
	int32 Count = 0;

	for (int32 i = 0; i < Points.Num(); i++)
	{
		if (Points[i].bEnabled)
		{
			Count++;
		}
	}

	return Count;
}

TArray<FPointCloudPoint*> FPointCloudHelper::GetEnabledPoints(const TArray<FPointCloudPoint*>& Points)
{
	TArray<FPointCloudPoint*> EnabledPoints;

	for (int32 i = 0; i < Points.Num(); i++)
	{
		if (Points[i] && Points[i]->bEnabled)
		{
			EnabledPoints.Add(Points[i]);
		}
	}

	return EnabledPoints;
}

FBox FPointCloudHelper::CalculateBounds(const TArray<FPointCloudPoint*>& Points, const FTransform& Transform)
{
	FBox BoundingBox(ForceInit);
	
	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		if (Points[Index] && Points[Index]->bEnabled)
		{
			BoundingBox += Transform.TransformPosition(Points[Index]->Location);
		}
	}

	AdjustBounds(BoundingBox);

	return BoundingBox;
}
FBox FPointCloudHelper::CalculateBounds(const TArray<FPointCloudPoint>& Points, const FTransform& Transform)
{
	FBox BoundingBox(ForceInit);

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		if (!Points[Index].bEnabled)
		{
			continue;
		}

		BoundingBox += Transform.TransformPosition(Points[Index].Location);
	}

	AdjustBounds(BoundingBox);

	return BoundingBox;
}
FBox FPointCloudHelper::CalculateBounds(const TArray<FPointCloudPoint*>& Points)
{
	FBox BoundingBox(ForceInit);

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		if (Points[Index] && Points[Index]->bEnabled)
		{
			BoundingBox += Points[Index]->Location;
		}
	}

	AdjustBounds(BoundingBox);

	return BoundingBox;
}
FBox FPointCloudHelper::CalculateBounds(const TArray<FPointCloudPoint>& Points)
{
	FBox BoundingBox(ForceInit);

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		if (!Points[Index].bEnabled)
		{
			continue;
		}

		BoundingBox += Points[Index].Location;
	}

	AdjustBounds(BoundingBox);

	return BoundingBox;
}

int32 FPointCloudHelper::CalculatePointSize(UPointCloud *PointCloud, bool bIncludeIB)
{
	// HalfPrecision only really uses 3 with the 4th being reserved for color
	int32 Size = PointCloud->UsesLowPrecision() ? 4 * sizeof(FFloat16) : sizeof(FVector) + sizeof(FColor);

	if (PointCloud->UsesSprites())
	{		
		// 4 vertices per point
		Size *= 4;

		// Index buffer
		if (bIncludeIB)
		{
			Size += 6 * sizeof(uint32);
		}
	}
	else
	{
		// Index buffer
		if (bIncludeIB)
		{
			Size += sizeof(uint32);
		}
	}

	return Size;
}

FString FPointCloudHelper::GetColorModeAsString(UPointCloud *PointCloud)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPointCloudColorMode"), true);
	return (EnumPtr && PointCloud) ? EnumPtr->GetNameStringByIndex((uint8)PointCloud->ColorMode) : FString("Invalid");
}

#if WITH_EDITOR
bool FPointCloudHelper::MaterialUsesUV(UMaterialInterface *Material)
{
	return IsValid(Material) ? MaterialUsesUV(GetTopMostMaterial(Material)) : false;
}
bool FPointCloudHelper::MaterialUsesUV(UMaterial *Material)
{
	return MaterialContainsExpressionOfClass(Material, UMaterialExpressionTextureCoordinate::StaticClass());
}

UMaterial* FPointCloudHelper::GetTopMostMaterial(UMaterialInterface *Material)
{
	UMaterial *ParentMaterial = NULL;
	UMaterialInstance *MaterialInstance = Cast<UMaterialInstance>(Material);

	if (IsValid(MaterialInstance))
	{
		ParentMaterial = GetTopMostMaterial(MaterialInstance->Parent);
	}
	else
	{
		ParentMaterial = Cast<UMaterial>(Material);
	}

	return ParentMaterial;
}

bool FPointCloudHelper::MaterialContainsExpressionOfClass(UMaterial *Material, UClass* ExpressionClass)
{
	if (!IsValid(Material) || !IsValid(ExpressionClass))
	{
		return false;
	}

	for (int32 MPIdx = 0; MPIdx < MP_MAX; MPIdx++)
	{
		if (ExpressionInputContainsExpressionOfClass(Material->GetExpressionInputForProperty(EMaterialProperty(MPIdx)), ExpressionClass))
		{
			return true;
		}
	}

	return false;
}
bool FPointCloudHelper::ExpressionInputContainsExpressionOfClass(FExpressionInput *ExpressionInput, UClass* ExpressionClass)
{
	UMaterialExpression *Expression = ExpressionInput ? ExpressionInput->Expression : NULL;
	if (!Expression)
	{
		return false;
	}

	if (Expression->GetClass() == ExpressionClass)
	{
		return true;
	}

	if (Expression->IsA(UMaterialExpressionMaterialFunctionCall::StaticClass()))
	{
		UMaterialFunction *MF = Cast<UMaterialFunction>(((UMaterialExpressionMaterialFunctionCall*)Expression)->MaterialFunction);
		if (IsValid(MF))
		{
			for (UMaterialExpression *FunctionExpression : MF->FunctionExpressions)
			{
				if (FunctionExpression->IsA(UMaterialExpressionFunctionOutput::StaticClass()))
				{
					TArray<FExpressionInput*> Inputs = FunctionExpression->GetInputs();
					for (FExpressionInput* InnerInput : Inputs)
					{
						if (ExpressionInputContainsExpressionOfClass(InnerInput, ExpressionClass))
						{
							return true;
						}
					}
				}
			}
		}
	}

	TArray<FExpressionInput*> Inputs = Expression->GetInputs();
	for (FExpressionInput* InnerInput : Inputs)
	{
		if (ExpressionInputContainsExpressionOfClass(InnerInput, ExpressionClass))
		{
			return true;
		}
	}

	return false;
}

#endif

void FPointCloudHelper::AdjustBounds(FBox &Bounds)
{
	float MinValue = 0.01f;

	if (Bounds.Min.X == Bounds.Max.X)
	{
		Bounds.Min.X -= MinValue;
		Bounds.Max.X += MinValue;
	}
	if (Bounds.Min.Y == Bounds.Max.Y)
	{
		Bounds.Min.Y -= MinValue;
		Bounds.Max.Y += MinValue;
	}
	if (Bounds.Min.Z == Bounds.Max.Z)
	{
		Bounds.Min.Z -= MinValue;
		Bounds.Max.Z += MinValue;
	}
}