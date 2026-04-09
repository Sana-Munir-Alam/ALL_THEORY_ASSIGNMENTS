# Function to Calculate Force
def Calculation(Weight, InitialSpeed, Height, MaxTension):
    # (a) Calculation
    # Constant
    g = 9.81  # Acceleration due to gravity in m/s^2
    
    # Converting Weight to Mass
    Mass = Weight / g

    # (a) Calculatation
    # Calculating Acceleration
    # Formula used => T = W - F
    Acceleration = (Weight - MaxTension) / Mass
    print("\nWorking (a):")
    print("1. Calculating Mass:")
    print("\tMass = Weight / g")
    print("\tMass = ", Weight, "/", g," = ", format(Mass,".2f"),"kg")
    print("2. Calculating Acceleration:\n\tT = W - F")
    print("\tT = W - ma")
    print("\ta = (W - T) / m")
    print("\ta =", format(Acceleration, ".1f"), "m/s^2")
    
    # (b) Calculatation
    # Calculating Speed at that acceleration
    #Formula Used => v^2 = u^2 + 2as
    Speed = (InitialSpeed**2 + (2 * Acceleration * Height))**0.5
    print("\nWorking (b):")
    print("1. Initial Speed (u) = 0 m/s")
    print("2. Height (s): 6.1 m")
    print("3. Calculating Speed:\n\tv^2 = u^2 + 2as")
    print("\tv = (u^2 + 2as)^0.5")
    print("\tv =", format(Speed,".1f"),"m/s")
    
    return Acceleration, Speed
    
    
# MAIN
print("CHAPTER 5: FORCE AND MOTION - I")
print("Question 41:")
print("Using a rope that will snap if the tension in it exceeds 387 N, you need to\nlower a bundle of old roofing material weighing 449 N from 6.1 m above the ground.\nObviously if you hang the bundle on the rope, it will snap. So, you allow\nthe bundle to accelerate downward.")
print("(a) What magnitude of the bundle's acceleration will put the rope on the verge of snapping?")
print("(b) At that acceleration, with what speed would the bundle hit the ground?")

InitialSpeed = 0
Weight = 449        # Weight of the bundle in N
MaxTension = 387    # Max Tension in N
Height = 6.1        # Distance in m

Acceleration, Speed = Calculation(Weight, InitialSpeed, Height, MaxTension)

#Displaying Final Answer
print("\nAnswer:")
print(f"(a) The magnitude of the bundle's acceleration:", format(Acceleration, ".1f"), "m/s^2")
print(f"(b) The speed of the bundle when it hits the ground:", format(Speed, ".1f"), "m/s")
