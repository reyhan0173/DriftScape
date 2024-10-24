# DriftScape
![image](https://github.com/user-attachments/assets/aec23778-097e-498d-839d-2fad9971060e)

DriftScape is an Unreal Engine game project written in C++ currently in developement. It's an arcade-style racing game with a focus on high-speed drifting.

The player utilizes a handbreak to lose friction on the back wheels of the car and begin the drift

Here's are some Code Samples of a few systems developed to achieve this.

## Collision and Spring Force Calculation

```C++
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
```
This block handles the spring and damper system for the car's suspension. It traces from the wheel down to the surface and calculates how compressed the spring is, applying a spring force based on the distance between the wheel and the ground (springForce) and a damper force to stabilize the suspension (damperForce). The total force is then applied upward at the wheel's location.

## Lateral Friction
```C++
FVector lateralFrictionVector = FVector::ZeroVector;
if(UKismetMathLibrary::Abs(localLinearVelocity.Y) > 2)
{
    lateralFrictionVector = BodyMeshC->GetRightVector() * localLinearVelocity.Y * FrictionConst * -1 * lateralFrictionMultiplier;
}
```
This snippet calculates the lateral friction that opposes the sideways motion of the car. If the car slides too much in the Y direction (sideways), this friction force is applied in the opposite direction to help stabilize the vehicle. Toying around with these values allows for a great expression in different drifting setups.


# Turning without pressing the handbreak and drifting:

https://github.com/Oblivion0173/DriftScape/assets/113329952/22ac40d8-cdd4-4d72-86c4-61b14c0d27f6

# Turning **with** pressing the handbreak and drifting:

https://github.com/Oblivion0173/DriftScape/assets/113329952/01c28c70-145b-4380-bb67-45661fe56850

# Friction Difference when inputting handbreak while turning:

https://github.com/Oblivion0173/DriftScape/assets/113329952/1bf6ef68-3be7-4c49-8979-303897a60304

