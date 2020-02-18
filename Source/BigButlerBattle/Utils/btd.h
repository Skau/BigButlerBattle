// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/Object.h"

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
    * Really fast version of acos.
    * Maximum error of 0.18 rad.
    * @ref https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm
    */
    FORCEINLINE static float FastAcos(float rad) 
    {
        return (-0.69813170079773212 * rad * rad - 0.87266462599716477) * rad + 1.5707963267948966;
    }

    /*
    * Waits before calling a lambda.
    * @param Context object.
    * @param How many seconds to wait before call.
    * @param The lambda to call.
    */
    FORCEINLINE static FTimerHandle Delay(UObject* Context, float Seconds, TFunction<void(void)> lambda)
    {
        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda(lambda);
        FTimerHandle Handle;
        Context->GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, Seconds, false);
        return Handle;
    }

    /*
    * Repeats a lambda.
    * @param Context object.
    * @param How many seconds between calls.
    * @param How many iterations to call.
    * @param The lambda to call.
    */
    FORCEINLINE static void Repeat(UObject* Context, float Seconds, int Iterations, TFunction<void(void)> Lambda)
    {
        if (Iterations <= 0)
            return;

        Delay(Context, Seconds, [=]()
        {
            Lambda();
            Repeat(Context, Seconds, Iterations - 1, Lambda);
        });
    }


    /**
     * @brief Swaps the values of two elements.
     * Attempts to invoke move semantics on two values to swap the contents of the values.
     * @param v1 The first value
     */
    template<typename T>
    FORCEINLINE static void Swap(T& v1, T& v2)
    {
        T temp{ std::move(v1) };
        v1 = std::move(v2);
        v2 = std::move(temp);
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
                Swap(i, Index);
            }
        }
    }

    inline static FInputActionBinding BindActionLambda(UInputComponent* Input, FName ActionName, const EInputEvent KeyEvent, TFunction<void(void)> func)
    {
        FInputActionBinding ab{ActionName, KeyEvent};
	    ab.ActionDelegate.GetDelegateForManualSet().BindLambda(func);
	    return Input->AddActionBinding(MoveTemp(ab));
    }
}