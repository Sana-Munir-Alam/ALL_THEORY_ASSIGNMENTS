#include <iostream>
#include <string>
using namespace std;

const int LEARNER = 0;
const int INTERMEDIATE = 1;
const int FULL = 2;

// Class User
class User {
    private:
        int UserID;
        string UserName;
        int UserAge;
        string ContactInfo;
        int UserLicense;
        static int NextID;
    public:
        User(string Name, int Age, string Contact, int License) : UserName(Name), UserAge(Age), ContactInfo(Contact), UserLicense(License) {
            UserID = NextID++;
        }
        void UpdateDetails(string newName, int newAge, string newContact, int newLicense) {
            UserName = newName;
            UserAge = newAge;
            ContactInfo = newContact;
            UserLicense = newLicense;
        }
        int GetUserID() const { 
            return UserID; 
        }
        string GetUserName() const { 
            return UserName; 
        }
        int GetUserLicense() const { 
            return UserLicense; 
        }
};
int User::NextID = 1000; // A static value intended that each User has a Unique ID

class Vehicle {
    private:
        string VehicleModel;
        float RentalPrice;
        int RequiredLicense;
    public:
        Vehicle(string model, float price, int License) : VehicleModel(model), RentalPrice(price), RequiredLicense(License) {
            // Paramatrized Constructor using Inline Initilisation
        }
        bool CheckEligibility(int UserLicense) const {
            return UserLicense >= RequiredLicense;
        }
        string GetModel() const { 
            return VehicleModel; 
        }
        float GetPrice() const { 
            return RentalPrice; 
        }
        int GetRequiredLicense() const {
            return RequiredLicense;
        }
};

class RentalSystem {
    private:
        User** Users;
        int UserCount;
        Vehicle** Vehicles;
        int VehicleCount;

    public:
        RentalSystem() : Users(nullptr), UserCount(0), Vehicles(nullptr), VehicleCount(0) {
            // Default Constructor
        }

        ~RentalSystem() {   // Destructor
            for (int i = 0; i < UserCount; i++) {        // First Delete Col
                delete Users[i];
            }
            delete[] Users;                             // Than Delete the Whole Array
            for (int i = 0; i < VehicleCount; i++) {    // First Delete Col
                delete Vehicles[i];
            }
            delete[] Vehicles;                          // Than Delete the Whole Array
        }

        void AddUser(User* NewUser) {
            User** Temp = new User*[UserCount + 1];     // Creating A new Array
            for (int i = 0; i < UserCount; i++) {
                Temp[i] = Users[i];                     // Storing The Old Data In The New Array
            }
            Temp[UserCount++] = NewUser;                // Storing The New User into the Array
            delete[] Users;                             // Freeing Old Array so That New Array can be stored in it's place.
            Users = Temp;
        }

        void AddVehicle(Vehicle* NewVehicle) {
            Vehicle** Temp = new Vehicle*[VehicleCount + 1];    // Creating A new Array
            for (int i = 0; i < VehicleCount; i++) {
                Temp[i] = Vehicles[i];                          // Storing The Old Data In The New Array
            }
            Temp[VehicleCount++] = NewVehicle;                  // Storing The New Veichle into the Array
            delete[] Vehicles;                                  // Freeing Old Array so That New Array can be stored in it's place.
            Vehicles = Temp;
        }

        void DisplayAvailableVehicles() const {
            cout << endl << "Available Vehicles:" << endl;
            for (int i = 0; i < VehicleCount; i++) {
                cout << i+1 << ". " << Vehicles[i]->GetModel() << " ($" << Vehicles[i]->GetPrice() << "/day) - Requires: ";
                switch(Vehicles[i]->GetRequiredLicense()) {
                    case LEARNER: cout << "Learner or above"; break;
                    case INTERMEDIATE: cout << "Intermediate or above"; break;
                    case FULL: cout << "Full License"; break;
                }
                cout << endl;
            }
        }

        User* FindUser(int id) const {
            for (int i = 0; i < UserCount; i++){
                if (Users[i]->GetUserID() == id) {
                    return Users[i];
                }
            }
            return nullptr;
        }

        bool RentVehicle(int userID, int vehicleIndex) {
            User* user = FindUser(userID);        // First Find the Registered User So That Car Rent Can Be Done
            if (!user || vehicleIndex < 0 || vehicleIndex >= VehicleCount) {
                return false;
            }
            if (Vehicles[vehicleIndex]->CheckEligibility(user->GetUserLicense())) { // If the User License Match the Car License Criteria Then...
                cout << endl << "Rental Successful!" << endl
                    << "User: " << user->GetUserName() << endl
                    << "Vehicle: " << Vehicles[vehicleIndex]->GetModel() << endl
                    << "Daily Rate: $" << Vehicles[vehicleIndex]->GetPrice() << endl;
                return true;
            }
            cout << "Rental Failed: License requirements not met!" << endl;
            return false;
        }
        int GetVehicleCount(){
            return VehicleCount;
        }
};

int main() {
    RentalSystem system;
    
    // Predefined vehicles
    system.AddVehicle(new Vehicle("Honda Civic", 95.99, LEARNER));
    system.AddVehicle(new Vehicle("Toyota Camry", 105.50, INTERMEDIATE));
    system.AddVehicle(new Vehicle("BMW X5", 220.00, FULL));
    system.AddVehicle(new Vehicle("Civic Sedan", 120.00, INTERMEDIATE));

    int choice;
    int currentUserID = -1;

    do {
        cout << endl << "Vehicle Rental System" << endl
             << "1. Register New User" << endl
             << "2. Update User Details" << endl
             << "3. View Available Vehicles" << endl
             << "4. Rent Vehicle" << endl
             << "5. Exit" << endl
             << "Enter choice: ";
        cin >> choice;

        switch(choice) {
            case 1: {
                string Name, Contact;
                int Age, License;
                cout << endl << "~------------------------~" << endl;
                cout << "Enter Name: ";
                cin.ignore();
                getline(cin, Name);
                cout << "Enter Age: ";
                cin >> Age;
                cout << "Enter Contact Number: ";
                cin.ignore();
                getline(cin, Contact);
                cout << "License type (1-Learner, 2-Intermediate, 3-Full): ";
                cin >> License;
                
                User* NewUser = new User(Name, Age, Contact, static_cast<int>(License-1));
                system.AddUser(NewUser);
                currentUserID = NewUser->GetUserID();
                cout << "Registration complete! User ID: " << currentUserID << endl;
                break;
            }
            case 2: {
                cout << endl << "~------------------------~" << endl;
                if (currentUserID == -1) {
                    cout << "No registered user!" << endl;
                    break;
                }
                string Name, Contact;
                int Age, License;
                cout << "Enter New Name: ";
                cin.ignore();
                getline(cin, Name);
                cout << "Enter New Age: ";
                cin >> Age;
                cout << "Enter New Contact Number: ";
                cin.ignore();
                getline(cin, Contact);
                cout << "New License type (1-Learner, 2-Intermediate, 3-Full): ";
                cin >> License;
                
                User* user = system.FindUser(currentUserID);
                user->UpdateDetails(Name, Age, Contact, static_cast<int>(License-1));
                cout << "Details updated successfully!" << endl;
                break;
            }
            case 3:
                cout << endl << "~------------------------~" << endl;
                system.DisplayAvailableVehicles();
                break;
            case 4: {
                cout << endl << "~------------------------~" << endl;
                if (currentUserID == -1) {
                    cout << "Register first!" << endl;
                    break;
                }
                system.DisplayAvailableVehicles();
                int userID, vehicleChoice;
                cout << "Enter your User ID: ";
                cin >> userID;
                if (system.FindUser(userID) == nullptr) {
                    cout << "User ID not found!" << endl;
                    break;
                }
                cout << "Select vehicle (1-" << system.GetVehicleCount() << "): ";
                cin >> vehicleChoice;
                system.RentVehicle(userID, vehicleChoice-1);
                break;
            }
            case 5:
                cout << "Exiting system..." << endl;
                break;
            default:
                cout << "Invalid choice!" << endl;
        }
    } while (choice != 5);

    return 0;
}
