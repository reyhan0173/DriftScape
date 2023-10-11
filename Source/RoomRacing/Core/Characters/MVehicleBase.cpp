// Fill out your copyright notice in the Description page of Project Settings.


#include "MVehicleBase.h"

#include "MovieSceneSequenceID.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
	
}

void AMVehicleBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

// Called every frame
void AMVehicleBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMVehicleBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

