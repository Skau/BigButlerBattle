// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/Object.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameModeBase.h"
#include "ButlerGameInstance.h"

/**
 * 
 */
namespace btd
{
    FORCEINLINE FVector SwapXY(const FVector& V)
    {
        return FVector{V.Y, V.X, V.Z}; 
    }

    FORCEINLINE FVector SwapY(const FVector& V)
    {
        return FVector{V.X, -V.Y, V.Z};
    }

    FORCEINLINE static float GetAngleBetween(const FVector& Vector1, const FVector& Vector2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Vector1, Vector2) / (Vector1.Size() * Vector2.Size())));
    }
    
    FORCEINLINE static float GetAngleBetweenNormals(const FVector& Normal1, const FVector& Normal2)
    {
        return FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal1, Normal2)));
    }

    /*
    * Really fast version of acos.
    * Maximum error of 0.18 rad.
    * @ref https://stackoverflow.com/questions/3380628/fast-arc-cos-algorithm
    */
    FORCEINLINE static float FastAcos(const float Rad) 
    {
        return (-0.69813170079773212 * Rad * Rad - 0.87266462599716477) * Rad + 1.5707963267948966;
    }

    /*
    * Waits before calling a lambda.
    * @param Context object.
    * @param How many seconds to wait before call.
    * @param The lambda to call.
    */
    FORCEINLINE static void Delay(UObject* Context, const float Seconds, TFunction<void()> Lambda)
    {
        if (!Context)
            return;

        FTimerDelegate TimerCallback;
        FTimerHandle Handle;
        
        TimerCallback.BindLambda(Lambda);

        Context->GetWorld()->GetTimerManager().SetTimer(Handle, TimerCallback, Seconds, false);

        if (!Context->IsA<UButlerGameInstance>())
        {
            auto GM = Cast<AMyGameModeBase>(UGameplayStatics::GetGameMode(Context->GetWorld()));
            if (GM)
            {
                GM->AddTimerHandle(Handle);
            }
        }
    }

    /*
    * Repeats a lambda.
    * @param Context object.
    * @param How many seconds between calls.
    * @param How many iterations to call.
    * @param The lambda to call.
    */
    FORCEINLINE static void Repeat(UObject* Context, const float Seconds, const int Iterations, TFunction<void()> Lambda)
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

    inline static FInputActionBinding BindActionLambda(UInputComponent* Input, const FName& ActionName, const EInputEvent KeyEvent, TFunction<void()> Func)
    {
        FInputActionBinding AB{ActionName, KeyEvent};
	    AB.ActionDelegate.GetDelegateForManualSet().BindLambda(Func);
	    return Input->AddActionBinding(MoveTemp(AB));
    }

    template<typename T>
    inline static int Sign(const T& V, float Tolerance = SMALL_NUMBER)
    {
        auto Zero = T{};
        return FMath::IsNearlyEqual(V, Zero, Tolerance) ? 0 : (V < Zero + Tolerance ? -1 : 1);
    }
    
    /** Note: The act of dividing a vector on another vector is undefined but the Unreal
     * implementation will yield a vector and not a scalar which kinda defeats the purpose
     * of this function. However, if A, B and P is on a line the resulting vector's
     * components should be equal which can be further used to get the inverse of a lerp.
     */
    template <typename T>
    inline static auto InvLerp(const T &A, const T &B, const T &P)
    {
        /**
         * p = a + t * (b - a)
         * p - a = t * (b - a)
         * (p - a) / (b - a) = t
         */
        return (P - A) / (B - A);
    }

    // Converts a single digit integer to a char (or the last digit if multiple digits)
    inline static TCHAR ConvertIntDigitToChar(const int32 Int)
    {
        return static_cast<TCHAR>(Int % 10) + '0';
    }
}