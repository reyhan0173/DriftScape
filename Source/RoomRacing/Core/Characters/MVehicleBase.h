// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"

#include "GameFramework/Pawn.h"

#include "MVehicleBase.generated.h"

class USpringArmComponent;
class UArrowComponent;

UCLASS()
class ROOMRACING_API AMVehicleBase : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BodyMeshC;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UArrowComponent* ArrowC_FR;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UArrowComponent* ArrowC_FL;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UArrowComponent* ArrowC_BR;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UArrowComponent* ArrowC_BL;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCameraComponent* CameraComponent;

	//Exposed Variable
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float RestLength = 50;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float SpringTravelLength = 20;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float WheelRadius = 30;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float SpringForceConst = 50000;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float DamperForceConst = 5000;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float ForwardForceConst = 100000;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float MaxSteeringAngle = 30;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float FrictionConst = 500;
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float BrakeConst = 1000;


	// Sets default values for this pawn's properties
	AMVehicleBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

protected:
	UPROPERTY(BlueprintReadOnly)
	bool BrakeApplied;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	TArray<UArrowComponent*> WheelArrowComponentHolder;
	float MinLength;
	float MaxLength;
	FCollisionQueryParams LineTraceCollisionQuery;
	float SpringLength[4] = {0,0,0,0};
	float ForwardAxisValue;
	float RightAxisValue;

	

private:
	void UpdateVehicleForce(int WheelArrowIndex, float DeltaTime);
	void MoveForward(float Value);
	void MoveRight(float Value);
	void BrakePressed();
	void BrakeReleased();
	
};


