// Fill out your copyright notice in the Description page of Project Settings.


#include "MVehicleBase.h"

#include "MovieSceneSequenceID.h"
#include "Components/ArrowComponent.h"
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
	//put them all in container
	WheelArrowComponentHolder = {ArrowC_FR, ArrowC_FL, ArrowC_BL, ArrowC_BR};
	//RestLength is axle length
	MinLength = RestLength - SpringTravelLength;
	MaxLength = RestLength + SpringTravelLength;
	const FName TraceTag("MyTraceTag");
	LineTraceCollisionQuery.TraceTag = TraceTag;
	//Create visual debugging
	LineTraceCollisionQuery.bDebugQuery = true;
	//Ignore collisions with itself 
	LineTraceCollisionQuery.AddIgnoredActor(this);
	
}

void AMVehicleBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

// Called every frame
void AMVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for(int WheelArrowIndex=0; WheelArrowIndex<4; WheelArrowIndex++)
	{
		UpdateVehicleForce(WheelArrowIndex, DeltaTime);
	}

}

void AMVehicleBase::UpdateVehicleForce(int WheelArrowIndex, float DeltaTime)
{
	//Wheel arrow index selects the correct wheel to move and check collisions.
	if(!WheelArrowComponentHolder.IsValidIndex(WheelArrowIndex))
		return;

	//Store the indexed arrow into the variable "arrow"
	auto wheelArrowC = WheelArrowComponentHolder[WheelArrowIndex];
	//Type FHitResult is a variable that stores the collisions and their types into the variable outHit
	FHitResult outHit;
	FVector startTraceLoc = wheelArrowC->GetComponentLocation();
	FVector endTraceLoc = wheelArrowC->GetForwardVector()*(MaxLength + WheelRadius) + startTraceLoc;
	//Create the trace using LineTraceSingleByChannel
	GetWorld()->LineTraceSingleByChannel(outHit, startTraceLoc, endTraceLoc, ECC_Camera, LineTraceCollisionQuery, FCollisionResponseParams());
	float PreviousSpringLength = SpringLength[WheelArrowIndex];
	//these lines of code are used to obtain the local linear velocity of a physics-enabled component (BodyMeshC).
	//This is useful for scenarios where you need to understand how a component is moving relative to its own orientation and position,
	//which can be different from its motion in the global world coordinate system.
	FVector WorldLinearVelocity = BodyMeshC->GetPhysicsLinearVelocity();
	FVector localLinearVelocity = UKismetMathLibrary::InverseTransformDirection(BodyMeshC->GetComponentTransform(), WorldLinearVelocity);
	if(outHit.IsValidBlockingHit())
	{
		float currentSpringLength = outHit.Distance - WheelRadius;
		SpringLength[WheelArrowIndex] = UKismetMathLibrary::FClamp(currentSpringLength, MinLength, MaxLength);
		float springVelocity = (PreviousSpringLength - SpringLength[WheelArrowIndex])/DeltaTime;
		float springForce = (RestLength - SpringLength[WheelArrowIndex])*SpringForceConst;
		float damperForce = springVelocity* DamperForceConst;
		FVector upwardForce = GetActorUpVector()*(springForce + damperForce);
		BodyMeshC->AddForceAtLocation(upwardForce, wheelArrowC->GetComponentLocation());
	}

	else
	{
		SpringLength[WheelArrowIndex] = MaxLength;
	}

	float CurrentSteeringAngle = UKismetMathLibrary::MapRangeClamped(RightAxisValue, -1, 1, MaxSteeringAngle*-1, MaxSteeringAngle);
	if(WorldLinearVelocity.SizeSquared() > 1) //If car is moving, add rotational torque
	{
		BodyMeshC->AddTorqueInDegrees(FVector(0, 0, CurrentSteeringAngle), NAME_None, true);
	}

	FVector frictionVector = FVector::ZeroVector;
	if(UKismetMathLibrary::Abs(localLinearVelocity.Y>2))
	{
		frictionVector = BodyMeshC->GetRightVector()*localLinearVelocity.Y*FrictionConst*-1;
	}

	FVector netForce = GetActorForwardVector()*ForwardAxisValue*ForwardForceConst + frictionVector; //forward force + friction force

	if(BrakeApplied)
	{
		netForce = WorldLinearVelocity*-1*BrakeConst;
	}
	BodyMeshC->AddForce(netForce);
}

void AMVehicleBase::MoveForward(float Value)
{
	ForwardAxisValue = Value;
}

void AMVehicleBase::MoveRight(float Value)
{
	//Slowly add axis value, to have a smoother turn
	UKismetMathLibrary::FInterpTo(RightAxisValue, Value, GetWorld()->GetDeltaSeconds(), 5);
	RightAxisValue = Value;
}

void AMVehicleBase::BrakePressed()
{
	BrakeApplied = true;
}

void AMVehicleBase::BrakeReleased()
{
	BrakeApplied = false;
}

// Called to bind functionality to input
void AMVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AMVehicleBase::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AMVehicleBase::MoveRight);
	PlayerInputComponent->BindAction(FName("Brake"), IE_Pressed, this, &AMVehicleBase::BrakePressed);
	PlayerInputComponent->BindAction(FName("Brake"), IE_Released, this, &AMVehicleBase::BrakeReleased);

}



