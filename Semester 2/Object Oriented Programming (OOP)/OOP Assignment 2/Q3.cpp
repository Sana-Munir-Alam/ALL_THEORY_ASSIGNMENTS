#include <iostream>
#include <string>
using namespace std;

class Vehicle {
    protected:
        string vehicleID;
        double Speed;
        double Capacity;
        double EnergyEfficiency;
        static int TotalDeliveries;

    public:
        Vehicle(string ID, double speed, double cap, double eff) : vehicleID(ID), Speed(speed), Capacity(cap), EnergyEfficiency(eff) {}

        virtual ~Vehicle() {}

        virtual void CalculateRoute(string Destination) {
            cout << "Calculating standard route to " << Destination << endl;
        }
        virtual double EstimatedTime(double Distance) {
            return Distance / Speed;    // Using v = s/t formula
        }
        virtual void Command(string Action, int PackageID) {
            cout << "Executing " << Action << " Command for package " << PackageID << endl;
            TotalDeliveries++;
        }
        virtual void Command(string Action, int PackageID, string Urgency) {
            cout << "Executing " << Action << " Command for package " << PackageID << " with Urgency " << Urgency << endl;
            TotalDeliveries++;
        }
        friend bool operator==(const Vehicle& v1, const Vehicle& v2);
        friend Vehicle* ResolveConflict(Vehicle* v1, Vehicle* v2);

        static int getTotalDeliveries() { return TotalDeliveries; }
        string getID() const { return vehicleID; }
        double getSpeed() const { return Speed; }
        double getCapacity() const { return Capacity; }
        double getEfficiency() const { return EnergyEfficiency; }
};

int Vehicle::TotalDeliveries = 0;
// Comapring Operator Overload
bool operator==(const Vehicle& v1, const Vehicle& v2) {
    return (v1.Speed == v2.Speed) && (v1.Capacity == v2.Capacity) && (v1.EnergyEfficiency == v2.EnergyEfficiency);
}

//The vehicle with the higher score is considered superior.
// 40% wieght to speed, 20% weight to capacity and 30% weight to energy efficiency.  [My priority list on which bases the conflict will be resolved, like the important factor in the scenerio is speed than capacity and efficiency.]
Vehicle* ResolveConflict(Vehicle* v1, Vehicle* v2) {
    double score1 = v1->Speed * 0.4 + v1->Capacity * 0.3 + v1->EnergyEfficiency * 0.3;
    double score2 = v2->Speed * 0.4 + v2->Capacity * 0.3 + v2->EnergyEfficiency * 0.3;
    return (score1 > score2) ? v1 : v2;
}

class RamzanDrone : public Vehicle {
    public:
        RamzanDrone(string ID) : Vehicle(ID, 120.0, 5.0, 0.9) {
            // Paramatrized Constructor that calls Parent COnstructor and passes (speed, capacity and efficency values)
        }

        void CalculateRoute(string Destination) override {
            cout << "Drone " << vehicleID << " calculating aerial route to " << Destination << endl;
        }
        // Basic Command Speed for delivery
        void Command(string Action, int PackageID) override {
            cout << "Drone " << vehicleID << " launching for immediate iftar delivery (" << PackageID << ") \t\t\t\t[Normal Delivery]" << endl;
            TotalDeliveries++;
        }
        // Using high speed for delivery due to urgency
        void Command(string Action, int PackageID, string Urgency) override {
            cout << "Drone " << vehicleID << " activating HIGH Speed mode for " << Urgency << " delivery (" << PackageID << ") \t\t\t[Urgent Delivery]" << endl;
            TotalDeliveries++;
        }
};

class RamzanTimeShip : public Vehicle {
    public:
        RamzanTimeShip(string ID) : Vehicle(ID, 9999.0, 50.0, 0.7) {
            // Paramatrized Constructor that calls Parent Constructor and passes (speed, capacity and efficency values)
        }

        void CalculateRoute(string Destination) override {
            cout << "TimeShip " << vehicleID << " verifying historical consistency for " << Destination << endl;
        }
        // Urgency Command
        void Command(string Action, int PackageID, string Urgency) override {
            cout << "TimeShip " << vehicleID << " validating temporal coordinates for " << Urgency << " delivery (" << PackageID << ") \t[Urgent Delivery]" << endl;
            TotalDeliveries++;
        }
};

class RamzanHyperPod : public Vehicle {
    public:
        RamzanHyperPod(string ID) : Vehicle(ID, 600.0, 200.0, 0.85) {
            // Paramatrized Constructor that calls Parent Constructor and passes (speed, capacity and efficency values)
        }

        void CalculateRoute(string Destination) override {
            cout << "HyperPod " << vehicleID << " navigating underground tunnel network to " << Destination << endl;
        }
        double EstimatedTime(double Distance) override {
            return (Distance / Speed) * 0.8;
        }
};

int main() {
    cout << "Sana Munir Alam 24K-0573" << endl;
    cout << endl << "~========== Vehicle Creation ==========~" << endl;
    
    // Create vehicles using DMA
    const int NumVehicles = 3;
    Vehicle** vehicles = new Vehicle*[NumVehicles];
    vehicles[0] = new RamzanDrone("DRONE_1");
    vehicles[1] = new RamzanTimeShip("TIMESHIP_1");
    vehicles[2] = new RamzanHyperPod("HYPERPOD_1");

    cout << "Three vehicles have been created:" << endl;
    cout << " - DRONE_1 (RamzanDrone)" << endl;
    cout << " - TIMESHIP_1 (RamzanTimeShip)" << endl;
    cout << " - HYPERPOD_1 (RamzanHyperPod)" << endl << endl;

    cout << "~========== Demonstrating Polymorphism ==========~" << endl;
    for(int i = 0; i < NumVehicles; ++i) {
        cout << endl << "-> " << vehicles[i]->getID() << " is calculating route to Grand Mosque District..." << endl;
        vehicles[i]->CalculateRoute("Grand Mosque District");

        double estimatedTime = vehicles[i]->EstimatedTime(120.0);
        cout << "Estimated travel time for " << vehicles[i]->getID() << ": " << estimatedTime << " hours" << endl;
    }
    cout << endl;

    cout << "~========== Urgency Command Demonstration ==========~" << endl;
    vehicles[0]->Command("Deliver", 1001);  //Normal
    vehicles[0]->Command("Deliver", 1002, "Iftar"); //Urgent

    vehicles[1]->Command("Deliver", 1003, "Historical");    // Urgent

    cout << endl << "~========== Conflict Resolution ==========~" << endl;
    cout << "Comparing DRONE_1 vs. HYPERPOD_1 to resolve a conflict..." << endl;
    Vehicle* result = ResolveConflict(vehicles[0], vehicles[2]);

    cout << "Winner of the conflict resolution: " << result->getID() << "!" << endl << endl;

    cout << "~========== Overloading Demonstration ==========~" << endl;
    RamzanDrone* drone2 = new RamzanDrone("DRONE_2");

    cout << "Comparing DRONE_1 with DRONE_2 using the overloaded '==' operator..." << endl;
    cout << "Are the drones identical? " << (*vehicles[0] == *drone2 ? "Yes" : "No") << endl << "";

    delete drone2;
    cout << endl;

    cout << "~========== Total Deliveries ==========~" << endl;
    cout << "Total deliveries made by all vehicles: " << Vehicle::getTotalDeliveries() << endl;

    cout << endl << "~========== Cleaning Up Memory ==========~" << endl;
    for(int i = 0; i < NumVehicles; ++i) {
        cout << "Deleting " << vehicles[i]->getID() << endl;
        delete vehicles[i];
    }
    delete[] vehicles;

    cout << "All dynamically allocated memory has been cleaned up." << endl << endl;
    cout << "~========== Program Execution Complete ==========~" << endl;

    return 0;
}
