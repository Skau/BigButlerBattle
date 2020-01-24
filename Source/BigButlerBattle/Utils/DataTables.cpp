// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTables.h"
#include "Templates/UnrealTemplate.h"
#include "btd.h"

void FBezierPoint::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
	// Super::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems); - No need, as super's implementation is empty.

    auto row = InDataTable->FindRow<FBezierPoint>(InRowName, *InDataTable->GetName());
    if (!row)
        return;

    auto& pos = row->Position;
    pos = FVector{pos.Y, pos.X, pos.Z} * 100.f;

    auto transform = [](const FVector& f)
    {
        return FVector{f.Y, f.X, f.Z} * 100.f;
    };

    /* In-tangent is multiplied by -1 because tangents are relative to the direction they're pointing.
     * So the in-tangent isn't a vector pointing from the control point, it's a negative direction vector
     * from the control point.
     */
    row->InTangent = transform(row->InTangent);
    row->OutTangent = -transform(row->OutTangent);
    // auto& in = row->InTangent;
    // in = FVector{in.X, in.Y, in.Z} * 100.f;

    // auto& out = row->OutTangent;
    // out = FVector{out.X, out.Y, out.Z} * 100.f;
}