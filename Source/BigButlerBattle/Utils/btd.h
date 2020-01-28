// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace btd
{
    FORCEINLINE FVector SwapXY(const FVector& v)
    {
        return FVector{v.Y, v.X, v.Z}; 
    }

    FORCEINLINE FVector SwapY(const FVector& v)
    {
        return FVector{v.X, -v.Y, v.Z};
    }

    FORCEINLINE static float GetAngleBetween(FVector Vector1, FVector Vector2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
    }
    
	FORCEINLINE static float GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
    }

    /*
     Shuffles the given array (out parameter) based on the given FRandomStream.
    */
    template<typename T>
    FORCEINLINE static void ShuffleArray(TArray<T>& Arr, const FRandomStream& Stream)
    {
        if (!Arr.Num())
            return;

        int LastIndex = Arr.Num() - 1;
        for (int i = 0; i < LastIndex; ++i)
        {
            int Index = Stream.RandRange(0, LastIndex);
            if (i != Index)
            {
                Arr.Swap(i, Index);
            }
        }
    }
}