# Function to Calculate Force
def CalculateForce(Mass, InitialSpeedKm, DistanceCM, FinalSpeed):
    # Converting Speed from km/h to m/s
    InitialSpeedMS = InitialSpeedKm * 1000 / 3600
    
    # Converting Distance from cm to m
    DistanceM = DistanceCM / 100
    
    # Calculating deceleration
    # Formula used => v^2 = u^2 + 2as
    Acceleration = (FinalSpeed**2 - InitialSpeedMS**2) / (2 * DistanceM)
    
    # Calculating Force
    # Formula Used => F = ma
    Force = Mass * Acceleration
    
    # Display Working
    print("\nWorking:")
    print("1. Initial speed (u) = 53 km/h = ", format(InitialSpeedMS, ".2f"), "m/s")
    print("2. Distance moved by the passenger (s) = 65 cm = ", DistanceM, "m")
    print("3. Final speed (v) = 0 m/s")
    print("4. Calculating Deceleration:\n\tv^2 = u^2 + 2as")
    print("\ta = (v^2 - u^2) / (2 * s)")
    print("\ta = ", format(Acceleration, ".2f"), "m/s^2")
    print("6. Force (F) = ma = ", format(abs(Force), ".2f"), "N (Newton)\n")
    
    return Force

# MAIN
print("CHAPTER 5: FORCE AND MOTION - I")
print("Question 20:\n")
print("A car travelling at 53 km/h hits a bridge abutment. A passenger in the car moves\nforward a distance of 65cm with respect to the road while being brought to rest\nby an inflated air bag. What magnitude of force assumed constant acts on the\npassenger's upper torso, which has a mass of 41 kg?")

Mass = 41            # Mass of passenger in kg
InitialSpeedKm = 53  # Initial Speed in km/h
DistanceCM = 65      # Distance Moved by Passenger in cm
FinalSpeed = 0

# Calling Function
Force = CalculateForce(Mass, InitialSpeedKm, DistanceCM, FinalSpeed)

#Displaying Final Answer
print("\nAnswer:")
print("The magnitude of the force acting on the passenger's torso is: ", format(Force,".2e"), "N")
