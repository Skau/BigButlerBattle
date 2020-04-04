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
    FORCEINLINE static FTimerHandle Delay(UObject* Context, const float Seconds, TFunction<void(void)> Lambda)
    {
        FTimerDelegate TimerCallback;
        TimerCallback.BindLambda(Lambda);
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
    FORCEINLINE static void Repeat(UObject* Context, const float Seconds, const int Iterations, TFunction<void(void)> Lambda)
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
     * @param V1 The first value
     * @param V2 The second value
     */
    template<typename T>
    FORCEINLINE static void Swap(T& V1, T& V2)
    {
        T Temp{ std::move(V1) };
        V1 = std::move(V2);
        V2 = std::move(Temp);
    }
    /*
     Shuffles the given array (out parameter) based on the given FRandomStream.
    */
    template<typename T>
    FORCEINLINE static void ShuffleArray(TArray<T>& Arr, const FRandomStream& Stream)
    {
        if (!Arr.Num())
            return;

        const int LastIndex = Arr.Num() - 1;
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

    template<typename T>
    inline static int sign(const T& v, float tolerance = SMALL_NUMBER)
    {
        auto zero = T{};
        return FMath::IsNearlyEqual(v, zero, tolerance) ? 0 : (v < zero + tolerance ? -1 : 1);
    }
    
    template <typename T>
    inline static auto InvLerp(const T &a, const T &b, const T &p)
    {
        /**
         * p = a + t * (b - a)
         * p - a = t * (b - a)
         * (p - a) / (b - a) = t
         */
        return (p - a) / (b - a);
    }

    // Converts a single digit integer to a char (or the last digit if multiple digits)
    inline static TCHAR ConvertIntDigitToChar(int32 Int)
    {
        return static_cast<TCHAR>(Int % 10) + '0';
    }
}