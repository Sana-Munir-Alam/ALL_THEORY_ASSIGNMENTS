# Problem Solution
import math

def Calculation(Mass, InitialSpeedMS, HorizontalDistanceM, AngleDegrees, RampLength, Height):
    # Constant
    g = 9.81  # Acceleration due to gravity in m/s^2

    # Convert angle to radians
    AngleRadians = math.radians(AngleDegrees)

    # Calculating Final Velocity at launch
    # Formula Used => y(i) - y(f) = (tan(θ) *x) - (g * (x^2)) / 2(v * cos(θ))^2
    LaunchSpeed = math.sqrt(
        (HorizontalDistanceM**2 * g) / (2 * (HorizontalDistanceM * math.tan(AngleRadians) + Height))
    ) / math.cos(AngleRadians)

    # Calculating the Acceleration during Ramp Phase
    # Formula Used => v^2 = u^2 + 2as 
    Acceleration = (LaunchSpeed**2 - InitialSpeedMS**2) / (2 * RampLength)
    
    # Calculating the Average Force
    # F = ma (force = mass * acceleration)
    Force = Mass * (Acceleration + (g * math.sin(AngleRadians)))
    
    print("\nWorking:")
    print("1. Calculating Launch Speed:\n\ty(i) - y(f) = (tan(θ) * x) - (g * (x^2)) / 2(v * cos(θ))^2")
    print("\tv = sqrt((g * x^2) / 2*((tan(θ) * x) + y(f)) ) / cos(θ)")
    print("\tv = sqrt((",g,"*", HorizontalDistanceM,"^2) / 2*((tan(", format(AngleRadians, ".2f"),") *", HorizontalDistanceM, ") +", Height,")) / cos(", format(AngleRadians, ".2f"),")")
    print("\tv = ", format(LaunchSpeed,".2f"), "m/s")
    print("2. Calculating the Acceleration:\n\tv^2 = u^2 + 2as")
    print("\ta = (v^2 - u^2) / (2s)")
    print("\ta = (",format(LaunchSpeed,".2f"),"^2 -",format(InitialSpeedMS,".2f"),"^2) / 2(", format(RampLength,".2f"),")")
    print("\ta =", format(Acceleration, ".2f"),"m/s^2")
    print("3. Calculate Force:\n\tF(net) = ma(ramp)")
    print("\tF = m * (a + (g * sin(θ)))")
    print("\tF =",Mass,"* (",format(Acceleration,".2f"),"+ (",g,"* sin(",format(AngleRadians, ".2f"),")))")
    print("\tF =", format(Force,".2f"),"N")
    
    return Force


# MAIN
print("CHAPTER 5: FORCE AND MOTION - I")
print("Question 68:")
print("A shot putter launches a 7.260 kg shot by pushing it along a straight line of\nlength 1.650 m and at an angle of 34.10° from the horizontal, accelerating the\nshot to the launch speed from its initial speed of 2.500 m/s (which is due to the\nathlete's preliminary motion). The shot leaves the hand at a height of 2.110 m\nand an angle of 34.10°, and it lands at a horizontal distance of 15.90 m.\nWhat is the magnitude of the athlete's average force on the shot during the\nacceleration phase?\n(Hint: Treat the motion during the acceleration phase as though it were along a ramp\nat the given angle.)")

Mass = 7.260                # Mass of the shot in kg
InitialSpeedMS = 2.500      # Initial speed in m/s
HorizontalDistanceM = 15.90 # Horizontal distance traveled in m
AngleDegrees = 34.10        # Angle of launch in degrees
RampLength = 1.650          # Length of the ramp in m
Height = 2.110              # Height of release in m

Force = Calculation(Mass, InitialSpeedMS, HorizontalDistanceM, AngleDegrees, RampLength, Height)

print("\nAnswer:")
print("The magnitude of the athlete's average force:", format(Force, ".2f"), "N")
