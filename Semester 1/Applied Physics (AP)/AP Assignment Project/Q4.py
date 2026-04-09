# Problem Solution
import math

def Calculation(Mass1, Mass2, AngleDegrees, ForceAppliedM1):
    # Constant
    g = 9.81  # Acceleration due to gravity in m/s^2
    
    # Convert angle to radians
    AngleRadians = math.radians(AngleDegrees)
    
    print("\nApplying Newton's Second Law to the x-axis of each box:")
    print("Eq 1:\t(M2 * g * sin(θ) - T = (M2 * a)")
    print("Eq 2:\t\t\t\t   F + T = (M1 * a)")
    
    # (a) Calculation
    # Calculating Acceleration (Using Simultaneous Equation)
    Acceleration = ((Mass2 * g * math.sin(AngleRadians) + ForceAppliedM1)) / (Mass1 + Mass2)
    # Calculating Tension (Putting value of accelration in Eq 2)
    Tension = (Mass1 * Acceleration) - ForceAppliedM1
    print("\nWorking (a)")
    print("1. Calculating Acceleration using Simultaneous Equation:")
    print("\ta = ((M2 * g * sin(θ) + F) / (M1 + M2)")
    print("\ta =((",Mass2,"*",g," sin(",format(AngleRadians,".2f"),") +",ForceAppliedM1,") / (",Mass1,"+",Mass2)
    print("\ta =", format(Acceleration,".2f"),"m/s^2")
    print("2. Calculating Tension: Substitute \"a\" in Eq 2")
    print("\tT = (M1 * a) - F")
    print("\tT = (",Mass1,"*",format(Acceleration,".2f"),") -",ForceAppliedM1)
    print("\tT =",format(Tension,".1f"),"N")
    
    # (b) Calculation
    # Calculating Acceleration 2 using Eq 1 when T = 0
    Acceleration2 = g * math.sin(AngleRadians)
    # Calculating Max Force
    Force = Mass1 * Acceleration2
    print("\n Working (b)")
    print("1. Calculate Acceleration wehn T = 0 through Eq 1:")
    print("\t(M2 * g * sin(θ) - T = (M2 * a)")
    print("\ta = g * sin(θ)")
    print("\ta =",g,"* sin(",format(AngleRadians,".2f"),")")
    print("\ta =", format(Acceleration2,".2f"),"m/s^2")
    print("2. Calculating Max Force:")
    print("\tF = M1 * a")
    print("\tF =",Mass1,"*",format(Acceleration2,".2f"))
    print("\tF =",format(Force,".0f"),"N")
    
    return Tension, Force

# MAIN
print("CHAPTER 5: FORCE AND MOTION - I")
print("Question 64:")
print("Figure 5–56 shows a box of mass M2 = 1.0 kg on a frictionless plane\ninclined at angle θ = 30°. It is connected by a cord of negligible\nmass to a box of mass M1 = 3.0 kg on a horizontal firctionless surface.\nThe pulley is frictionless and massless.\nIf the magnitude of horizontal force F is 2.3 N:\n(a) What is the tension in the connecting cord?\n(b) What is the largest value the magnitude of F may have without the cord becoming slack?")

Mass1 = 3.0  # Mass of box on horizontal surface in kg
Mass2 = 1.0  # Mass of box on inclined plane in kg
AngleDegrees = 30  # Angle of inclination in Degrees
ForceAppliedM1 = 2.3  # Horizontal Force Applied to Mass1 in N

Tension, Force = Calculation(Mass1, Mass2, AngleDegrees, ForceAppliedM1)

print("\nAnswer:")
print("(a) Tension in the connecting cord:",format(Tension,".1f"),"N")
print("(b) The largest value the magnitude of F may have without the cord becoming slack:",format(Force,".0f"),"N")
