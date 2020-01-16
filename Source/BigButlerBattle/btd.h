// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace btd
{
	static FVector SwapXY(const FVector& v)
    {
        return FVector{v.Y, v.X, v.Z}; 
    }

    static float GetAngleBetween(FVector Vector1, FVector Vector2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
    }
    
	static float GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
    }
}