# Function to Calculate Force
def Calculation(Weight, InitialSpeedKM, DistanceM, FinalSpeed):
    # (a) Calculation
    # Constant
    g = 9.81  # Acceleration due to gravity in m/s^2
    
    # Converting Speed from km/h to m/s
    InitialSpeedMS = InitialSpeedKM * 1000 / 3600
    
    # Converting Weight to Mass
    Mass = Weight / g
    
    # Calculating deceleration
    # Formula used => v^2 = u^2 + 2as
    Acceleration = (FinalSpeed**2 - InitialSpeedMS**2) / (2 * DistanceM)
    
    # Calculating Force
    # Formula Used => F = ma
    Force = Mass * abs(Acceleration)
    
    print("\nWorking (a):")
    print("1. Initial speed (u) = 40 km/h = ", format(InitialSpeedMS, ".2f"), "m/s")
    print("2. Distance (s) = 15m")
    print("3. Calculating Mass:")
    print("\tMass = Weight / g")
    print("\tMass = ", Weight, "/", g," = ", format(Mass,".2f"),"kg")
    print("4. Final speed (v) = 0 m/s")
    print("5. Calculating Deceleration:\n\tv^2 = u^2 + 2as")
    print("\ta = (v^2 - u^2) / (2 * s)")
    print("\ta =", format(Acceleration, ".2f"), "m/s^2")
    print("6. Force (F) = ma = ", format(Force, ".2f"), "N")
    
    # (b) Calculatation
    # Calculating Time
    #Formula Used => v = u + at
    Time = (FinalSpeed - InitialSpeedMS) / Acceleration
    print("\nWorking (b):")
    print("1. Calculating Time:\n\tv = u + at")
    print("\tt = (v - u) / a")
    print("\tt = ", format(Time,".1f"),"s")
    
    # (c) Calculatation
    NewInitialSpeedMS = 2 * InitialSpeedMS
    # Calculating New Distance
    # Formula Used => v^2 = u^2 + 2as
    NewDistanceM = (FinalSpeed**2 - NewInitialSpeedMS**2) / (2 * Acceleration)
    DistanceFactor = NewDistanceM / DistanceM
    print("\nWorking (c):")
    print("1. Calculating New Distance:\n\tv^2 = u^2 + 2as")
    print("\ts = (v^2 - u^2) / (2 * a)")
    print("\ts =", format(NewDistanceM, ".2f"), "m")
    print("2. Calculating Distance Factor:")
    print("\tFactor = New Distance / Old Distance")
    print("\tFactor = ",NewDistanceM," / ", DistanceM, " = ",DistanceFactor)
    
    # (d) Calculatation
    NewInitialSpeedMS = 2 * InitialSpeedMS
    # Calculating New Distance
    #Formula Used => v = u + at
    NewTime = (FinalSpeed - NewInitialSpeedMS) / (Acceleration)
    TimeFactor = NewTime / Time
    print("\nWorking (d):")
    print("1. Calculating New Time:\n\tv = u + at")
    print("\tt = (v - u) / (a)")
    print("\tt =", format(NewTime, ".1f"), "s")
    print("2. Calculating Time Factor:")
    print("\tFactor = New Time / Old Time")
    print("\tFactor = ",NewTime," / ", Time, " = ",TimeFactor)
    
    return Force, Time, DistanceFactor, TimeFactor
    

# MAIN
print("CHAPTER 5: FORCE AND MOTION - I")
print("Question 20:")
print("A car that weighs 1.3*10^4 N is initially moving at 40 km/h when the\nbrakes are applied and the car is brought to a stop in 15m. Assuming\nthe force that stops the car is constant, find:")
print("(a) The magnitude of that force?")
print("(b) The time required for the change in speed?")
print("If the initial speed is doubled, and the car experiences the same force\nduring braking, by what factors are:")
print("(c) The stopping distance multiplied?")
print("(d) The stopping time multiplied?")

InitialSpeedKM = 40     # Initial speed in km/h
Weight = 1.3 * 10**4    # Weight of the car in Newtons
DistanceM = 15           # Stopping distance in m
FinalSpeed = 0

Force, Time, DistanceFactor, TimeFactor = Calculation(Weight, InitialSpeedKM, DistanceM, FinalSpeed)

#Displaying Final Answer
print("\nAnswer:")
print("(a) The magnitude of the stopping force:", format(Force, ".2e"), "N")
print("(b) The time required to stop:", format(Time,".1f"), "s")
print("(c) The stopping distance is multiplied by a factor of:", DistanceFactor)
print("(d) The stopping time is multiplied by a factor of:", TimeFactor)
