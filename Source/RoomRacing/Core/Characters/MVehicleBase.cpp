// Fill out your copyright notice in the Description page of Project Settings.


#include "MVehicleBase.h"

#include <iostream>

#include "MovieSceneSequenceID.h"
#include "SNodePanel.h"
#include "Components/ArrowComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AMVehicleBase::AMVehicleBase()
{ 
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BodyMeshC = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMeshC"));
	
	RootComponent = BodyMeshC;
	
	if(UStaticMesh* CarMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), NULL, TEXT("/Script/Engine.StaticMesh'/Game/Assets/CarUpper.CarUpper'"))))
	{
		BodyMeshC->SetStaticMesh(CarMesh);
	}

	AC_Engine = CreateDefaultSubobject<UAudioComponent>(TEXT("AC_Engine"));
	AC_Engine->SetupAttachment(BodyMeshC);
	AC_Boost = CreateDefaultSubobject<UAudioComponent>(TEXT("AC_Boost"));
	AC_Boost->SetupAttachment(BodyMeshC);
	AC_Boost->SetAutoActivate(false);
	
	
	//ToDo Add mesh here
	ArrowC_FR = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowC_FR"));
	ArrowC_FR->SetupAttachment(BodyMeshC, FName("WheelFR"));
	ArrowC_FR->SetRelativeRotation(FRotator(-90, 0, 0));
	ArrowC_FL = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowC_FL"));
	ArrowC_FL->SetupAttachment(BodyMeshC, FName("WheelFL"));
	ArrowC_FL->SetRelativeRotation(FRotator(-90, 0, 0));
	ArrowC_BL = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowC_BL"));
	ArrowC_BL->SetupAttachment(BodyMeshC, FName("WheelBL"));
	ArrowC_BL->SetRelativeRotation(FRotator(-90, 0, 0));
	ArrowC_BR = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowC_BR"));
	ArrowC_BR->SetupAttachment(BodyMeshC, FName("WheelBR"));
	ArrowC_BR->SetRelativeRotation(FRotator(-90, 0, 0));

	WheelSceneFR = CreateDefaultSubobject<USceneComponent>(TEXT("WheelSceneFR"));
	WheelSceneFR->SetupAttachment(ArrowC_FR);
	WheelSceneFL = CreateDefaultSubobject<USceneComponent>(TEXT("WheelSceneFL"));
	WheelSceneFL->SetupAttachment(ArrowC_FL);
	WheelSceneBR = CreateDefaultSubobject<USceneComponent>(TEXT("WheelSceneBR"));
	WheelSceneBR->SetupAttachment(ArrowC_BR);
	WheelSceneBL = CreateDefaultSubobject<USceneComponent>(TEXT("WheelSceneBL"));
	WheelSceneBL->SetupAttachment(ArrowC_BL);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("USpringArmComponent"));
	SpringArmComponent->SetupAttachment(BodyMeshC);
	SpringArmComponent->TargetArmLength = 600;
	SpringArmComponent->SetRelativeRotation(FRotator(-10, 0 ,0));
	SpringArmComponent->SetRelativeLocation(FVector(0,0,130));

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponents"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	//Shift these lines to postInitialize
	BodyMeshC->SetSimulatePhysics(true);
	BodyMeshC->SetMassOverrideInKg(NAME_None, 1152);
}

// Called when the game starts or when spawned
void AMVehicleBase::BeginPlay()
{
	Super::BeginPlay();

	
		// Initialize tire load to 1/4th of the car's weight assuming evenly distributed
		float initialLoad = BodyMeshC->GetMass() * 9.8 / 4; // Using 9.8 for gravity. Adjust if your game uses a different value.
		for(int i = 0; i < 4; i++)
		{
			TireLoad[i] = initialLoad;
		}

	//put them all in container
	WheelArrowComponentHolder = {ArrowC_FR, ArrowC_FL, ArrowC_BL, ArrowC_BR};
	//putting wheels into container
	WheelSceneComponentHolder = {WheelSceneFR, WheelSceneFL, WheelSceneBL, WheelSceneBR};
	//RestLength is axle length
	MinLength = RestLength - SpringTravelLength;
	MaxLength = RestLength + SpringTravelLength;
	const FName TraceTag("MyTraceTag");
	LineTraceCollisionQuery.TraceTag = TraceTag;
	//Create visual debugging
	LineTraceCollisionQuery.bDebugQuery = true;
	//Ignore collisions with itself 
	LineTraceCollisionQuery.AddIgnoredActor(this);

	if (SW_Boost)
	AC_Boost->SetSound(SW_Boost);
	else
		UE_LOG(LogTemp, Warning, TEXT("Undefined Boost Sound"));
	if (SW_Engine)
		AC_Engine->SetSound(SW_Engine);
	else
		UE_LOG(LogTemp, Warning, TEXT("Undefined Boost Sound"));	
}

void AMVehicleBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

// Called every frame
void AMVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AC_Engine->SetPitchMultiplier(UKismetMathLibrary::MapRangeClamped(GetVehicleSpeed(),0,100,1,4));
	for(int WheelArrowIndex=0; WheelArrowIndex<4; WheelArrowIndex++)
	{
		UpdateVehicleForce(WheelArrowIndex, DeltaTime);
	}

	////////////WARNING LOGS////////////////////////////////////////////
	
	UE_LOG(LogTemp, Warning, TEXT("driftfrictionmultiplier %f"), DriftFrictionMultiplier);
	UE_LOG(LogTemp, Warning, TEXT("IsDrifting:%hhd"), bIsDrifting);
	UE_LOG(LogTemp, Warning, TEXT("IsHandBreakPressed %hhd"), bHandbrakeApplied);
	bool bIsAboveThreshold = VehicleSpeed > DriftStartSpeedThreshold;
	UE_LOG(LogTemp, Warning, TEXT("VehicleSpeed > DriftStartSpeedThreshold: %s"), bIsAboveThreshold ? TEXT("True") : TEXT("False"));
	bool bIsAboveSteeringThreshold = FMath::Abs(CurrentSteeringAngle) > SteeringAngleThreshold;
	UE_LOG(LogTemp, Warning, TEXT("FMath::Abs(CurrentSteeringAngle) > SteeringAngleThreshold: %s"), bIsAboveSteeringThreshold ? TEXT("True") : TEXT("False"));

	

	UE_LOG(LogTemp, Warning, TEXT("driftfrictionmultiplier %f"), DriftFrictionMultiplier);
	// UE_LOG(LogTemp, Warning, TEXT("Current Friction: %lf"), currentFrictionConst);
	// UE_LOG(LogTemp, Warning, TEXT("isDrifting?: %d"), bIsDrifting);
	// UE_LOG(LogTemp, Warning, TEXT("locallinearvelocity.y %f"), locallinearvelocity.Y);
	// UE_LOG(LogTemp, Warning, TEXT("dragfrictionconst %f"), DragFrictionConst);
	// UE_LOG(LogTemp, Warning, TEXT("lateralfriction vector X=%f, Y=%f, Z=%f"), lateralFrictionVector.X, lateralFrictionVector.Y, lateralFrictionVector.Z);
	// UE_LOG(LogTemp, Warning, TEXT("combinedfriction %f"), combinedFriction);
	// UE_LOG(LogTemp, Warning, TEXT("lateralfrictionmultipler %f"), lateralFrictionMultiplier);
	//
	// UE_LOG(LogTemp, Warning, TEXT("Right Vector: %s"), *BodyMeshC->GetRightVector().ToString());


}

float AMVehicleBase::GetVehicleSpeed() const
{
	return BodyMeshC->GetPhysicsLinearVelocity().Size()*0.036;
}


void AMVehicleBase::UpdateVehicleForce(int WheelArrowIndex, float DeltaTime)
{
	// Wheel arrow index selects the correct wheel to move and check collisions.
	if(!WheelArrowComponentHolder.IsValidIndex(WheelArrowIndex))
		return;

	auto wheelArrowC = WheelArrowComponentHolder[WheelArrowIndex];
	FHitResult outHit;
	FVector startTraceLoc = wheelArrowC->GetComponentLocation();
	FVector endTraceLoc = wheelArrowC->GetForwardVector() * (MaxLength + WheelRadius) + startTraceLoc;

	GetWorld()->LineTraceSingleByChannel(outHit, startTraceLoc, endTraceLoc, ECC_Camera, LineTraceCollisionQuery, FCollisionResponseParams());
	float PreviousSpringLength = SpringLength[WheelArrowIndex];

	FVector WorldLinearVelocity = BodyMeshC->GetPhysicsLinearVelocity();
	FVector localLinearVelocity = UKismetMathLibrary::InverseTransformDirection(BodyMeshC->GetComponentTransform(), WorldLinearVelocity);
	locallinearvelocity = localLinearVelocity;
	if(outHit.IsValidBlockingHit())
	{
		float currentSpringLength = outHit.Distance - WheelRadius;
		SpringLength[WheelArrowIndex] = UKismetMathLibrary::FClamp(currentSpringLength, MinLength, MaxLength);
		float springVelocity = (PreviousSpringLength - SpringLength[WheelArrowIndex]) / DeltaTime;
		float springForce = (RestLength - SpringLength[WheelArrowIndex]) * SpringForceConst;
		float damperForce = springVelocity * DamperForceConst;
		FVector upwardForce = GetActorUpVector() * (springForce + damperForce);
		BodyMeshC->AddForceAtLocation(upwardForce, wheelArrowC->GetComponentLocation());
	}
	else
	{
		SpringLength[WheelArrowIndex] = MaxLength;
	}

	CurrentSteeringAngle = UKismetMathLibrary::MapRangeClamped(RightAxisValue, -1, 1, MaxSteeringAngle * -1, MaxSteeringAngle);

	
	if(WheelArrowIndex < 2)
	{
		WheelSceneComponentHolder[WheelArrowIndex]->SetRelativeRotation(FRotator(0, 0, CurrentSteeringAngle));
	}
	

	if(WorldLinearVelocity.SizeSquared() > 1)
	{
		BodyMeshC->AddTorqueInDegrees(FVector(0, 0, CurrentSteeringAngle * turningtorqueMultiplier), NAME_None, true);
	}

	// Compute lateral friction.
	VehicleSpeed = WorldLinearVelocity.Size();
	
	if (bHandbrakeApplied && VehicleSpeed > DriftStartSpeedThreshold && FMath::Abs(CurrentSteeringAngle) > SteeringAngleThreshold) {
		bIsDrifting = true;
		DriftTimer = 0.0f;
	}
	if (bIsDrifting)
	{
		// If the drift conditions aren't being met, start or extend the drift timer
		if (FMath::Abs(CurrentSteeringAngle) <= SteeringAngleThreshold) {
			DriftTimer += DeltaTime;
		} else {
			DriftTimer = 0.0f; // Reset the timer if you're back to a sufficient steering angle
		}
    
		// If the timer exceeds a certain threshold, end the drift
		if (DriftTimer >= MaxTimeWithoutDrift) {
			bIsDrifting = false;
		}
	}
	DriftFrictionMultiplier = (bIsDrifting && WheelArrowIndex >= 2) ? IsDriftingFrictionMultiplier : NotDriftingFrictionMultiplier;
	
	if(WheelArrowIndex < 2) // Assuming indexes 0 and 1 are the front wheels
		{
		currentFrictionConst = FrontWheelFrictionConst;
		}
	else
	{
		currentFrictionConst = BackWheelFrictionConst * DriftFrictionMultiplier;
	}
	if(UKismetMathLibrary::Abs(localLinearVelocity.Y) > 2)
	{
		lateralFrictionVector = BodyMeshC->GetRightVector() * localLinearVelocity.Y * currentFrictionConst * -1 * lateralFrictionMultiplier;
	}



	
	// Compute drag friction.
	dragFrictionVector = -WorldLinearVelocity.GetSafeNormal() * WorldLinearVelocity.Size() * DragFrictionConst;

	// Combine frictions.
	combinedFriction = lateralFrictionVector + dragFrictionVector;

	// Apply combined forces.
	FVector netForce = GetActorForwardVector() * ForwardAxisValue * ForwardForceConst + combinedFriction;

	if(BrakeApplied)
	{
		netForce = WorldLinearVelocity * -1 * BrakeConst;
	}

	if(bBoost)
	{
		BodyMeshC->AddImpulse(BodyMeshC->GetForwardVector() * BoostForceConst, NAME_None, true);
	}
	BodyMeshC->AddForce(netForce);

	WheelSceneComponentHolder[WheelArrowIndex]->SetRelativeLocation(FVector(SpringLength[WheelArrowIndex], 0, 0));
	WheelSceneComponentHolder[WheelArrowIndex]->GetChildComponent(0)->AddLocalRotation(FRotator((-1 * 360 * localLinearVelocity.X * DeltaTime) / (2 * 3.14 * WheelRadius), 0, 0));
}

void AMVehicleBase::MoveForward(float Value)
{
	ForwardAxisValue = Value;
}

void AMVehicleBase::MoveRight(float Value)
{
	if(BodyMeshC->GetPhysicsLinearVelocity().Size() * 0.036 > 3)
		RightAxisValue = UKismetMathLibrary::FInterpTo(RightAxisValue, Value, GetWorld()->GetDeltaSeconds(), 5);
	else
		RightAxisValue = 0;
}

void AMVehicleBase::HandbrakePressed()
{
	bHandbrakeApplied = true;
}
void AMVehicleBase::HandbrakeReleased()
{
	bHandbrakeApplied = false;
}

	
	//Slowly add axis value, to have a smoother turn
	//
	// UKismetMathLibrary::FInterpTo(RightAxisValue, Value, GetWorld()->GetDeltaSeconds(), 5);
	// RightAxisValue = Value;

void AMVehicleBase::BrakePressed()
{
	BrakeApplied = true;
}

void AMVehicleBase::BrakeReleased()
{
	BrakeApplied = false;
}

void AMVehicleBase::OnBoostPressed()
{
	bBoost = true;
	AC_Boost->Activate();
}

void AMVehicleBase::OnBoostReleased()
{
	bBoost = false;
	AC_Boost->Deactivate();
}

// Called to bind functionality to input
void AMVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AMVehicleBase::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AMVehicleBase::MoveRight);
	PlayerInputComponent->BindAction(FName("Brake"), IE_Pressed, this, &AMVehicleBase::BrakePressed);
	PlayerInputComponent->BindAction(FName("Brake"), IE_Released, this, &AMVehicleBase::BrakeReleased);
	PlayerInputComponent->BindAction(FName("Boost"), IE_Pressed, this, &AMVehicleBase::OnBoostPressed);
	PlayerInputComponent->BindAction(FName("Boost"), IE_Released, this, &AMVehicleBase::OnBoostReleased);
	PlayerInputComponent->BindAction(FName("Handbrake"), IE_Pressed, this, &AMVehicleBase::HandbrakePressed);
	PlayerInputComponent->BindAction(FName("Handbrake"), IE_Released, this, &AMVehicleBase::HandbrakeReleased);

}

