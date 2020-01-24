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

    static FVector SwapY(const FVector& v)
    {
        return FVector{v.X, -v.Y, v.Z};
    }

    static float GetAngleBetween(FVector Vector1, FVector Vector2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
    }
    
	static float GetAngleBetweenNormals(FVector Normal1, FVector Normal2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
    }

    /**
     * @brief Swaps the values of two elements.
     * Attempts to invoke move semantics on two values to swap the contents of the values.
     * @param v1 The first value
     */
    template<typename T>
    static void Swap(T& v1, T& v2)
    {
        T temp{std::move(v1)};
        v1 = std::move(v2);
        v2 = std::move(temp);
    }
}