// Fill out your copyright notice in the Description page of Project Settings.


#include "DataTables.h"

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

    row->InTangent = transform(row->InTangent);
    row->OutTangent = transform(row->OutTangent);
    // auto& in = row->InTangent;
    // in = FVector{in.X, in.Y, in.Z} * 100.f;

    // auto& out = row->OutTangent;
    // out = FVector{out.X, out.Y, out.Z} * 100.f;
}