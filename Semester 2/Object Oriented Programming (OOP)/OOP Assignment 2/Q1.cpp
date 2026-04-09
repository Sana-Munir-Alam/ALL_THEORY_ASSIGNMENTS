#include <iostream>
#include <string>
#include <ctime>
using namespace std;

class Stop; // Forward declaration

// Base class for all Users [Student, Teacher and Staff]
class User {
    protected:
        int UserID;
        string UserName;
        double Balance;
        bool IsActive;
        Stop* AssignedStop;

    public:
        User(int ID, string Name) : UserID(ID), UserName(Name), Balance(0.0), IsActive(false), AssignedStop(nullptr) {
            // Paramatrized Constructor
        }
        // Virtual destructor
        virtual ~User() {}

        // Getters
        int GetUserID() const { return UserID; }
        string GetUserName() const { return UserName; }
        double GetBalance() const { return Balance; }
        bool GetActiveStatus() const { return IsActive; }
        Stop* GetAssignedStop() const { return AssignedStop; }

        // Setters
        void SetAssignedStop(Stop* stop) { AssignedStop = stop; }

        // Virtual function for payment (to be overridden by derived classes)
        virtual void MakePayment(double Amount) = 0;

        // Operator overloading to compare users
        bool operator==(const User& other) const {
            return UserID == other.UserID && UserName == other.UserName;
        }
};

// Derived class for Students
class Student : public User {
    private:
        static const double SEMESTER_FEE;
    public:
        Student(int ID, string Name) : User(ID, Name) {
            // Paramatrized Constructor that calls Parent Constructor
        }

        void MakePayment(double Amount) override {
            if (Amount < 0) {
                cout << "Invalid payment Amount!" << endl;
                return;
            }
            Balance += Amount;
            if (Balance >= SEMESTER_FEE) {
                IsActive = true;
                Balance -= SEMESTER_FEE;
                cout << "Student " << GetUserName() << " Fee Status: Paid" << endl;
            } else {
                cout << "Student " << GetUserName() << " Fee Status: Remaining $" << SEMESTER_FEE - Balance << endl;
            }
        }
};

const double Student::SEMESTER_FEE = 100.0;

// Derived class for Teachers
class Teacher : public User {
    private:
        static const double MONTHLY_FEE;
    public:
        Teacher(int ID, string Name) : User(ID, Name) {
            // Paramatrized Constructor that calls Parent Constructor
        }

        void MakePayment(double Amount) override {
            if (Amount < 0) {
                cout << "Invalid payment Amount!" << endl;
                return;
            }
            Balance += Amount;
            if (Balance >= MONTHLY_FEE) {
                IsActive = true;
                Balance -= MONTHLY_FEE;
                cout << "Teacher " << GetUserName() << " Fee Status: Paid" << endl;
            } else {
                cout << "Teacher " << GetUserName() << " Fee Status: Remaining $" << MONTHLY_FEE - Balance << endl;
            }
        }
};

const double Teacher::MONTHLY_FEE = 50.0;

// Derived class for Staff
class Staff : public User {
    private:
        static const double MONTHLY_FEE;

    public:
        Staff(int ID, string Name) : User(ID, Name) {
            // Paramatrized Constructor that calls Parent Constructor
        }

        void MakePayment(double Amount) override {  
            //The payment style for Staff wasn't told in question hence following Teacher payment style
            if (Amount < 0) {
                cout << "Invalid payment Amount!" << endl;
                return;
            }
            Balance += Amount;
            if (Balance >= MONTHLY_FEE) {
                IsActive = true;
                Balance -= MONTHLY_FEE;
                cout << "Staff " << GetUserName() << " Fee Status: Paid" << endl;
            } else {
                cout << "Staff " << GetUserName() << " Fee Status: Remaining $" << MONTHLY_FEE - Balance << endl;
            }
        }
};

const double Staff::MONTHLY_FEE = 30.0;

class Stop {
    private:
        int StopID;
        string StopName;
        User** AssignedUsers;
        int NumUsers;

    public:
        Stop(int ID, string Name) : StopID(ID), StopName(Name), AssignedUsers(nullptr), NumUsers(0) {}

        ~Stop() {
            delete[] AssignedUsers;
        }

        // Getters
        int GetStopID() const { return StopID; }
        string GetStopName() const { return StopName; }
        User** AssignUser() const { return AssignedUsers; }
        int GetNumUsers() const { return NumUsers; }

        void AssignUser(User* user) {
            User** newUser = new User*[NumUsers + 1];
            for (int i = 0; i < NumUsers; ++i) {
                newUser[i] = AssignedUsers[i];
            }
            newUser[NumUsers] = user;
            cout << "User " << user->GetUserName() << " Assigned to Stop: " << GetStopName() << endl;
            delete[] AssignedUsers;
            AssignedUsers = newUser;
            NumUsers++;
            user->SetAssignedStop(this);
        }

        void RecordAttendance(User* user) const {
            if (user->GetActiveStatus()) {
                time_t now = time(0);
                cout << "Attendance recorded for " << user->GetUserName() << " at " << ctime(&now);
            } else {
                cout << "Payment required for " << user->GetUserName() << endl;
            }
        }
};

class Route {
    private:
        int RouteID;
        Stop** Stops;
        int NumStops;
    
    public:
        Route(int ID) : RouteID(ID), Stops(nullptr), NumStops(0) {}
    
        ~Route() {
            delete[] Stops;
        }
        // Getters
        int GetRouteID() const { return RouteID; }
        Stop** GetStops() const { return Stops; }
        int GetNumStops() const { return NumStops; }
    
        void AddStop(Stop* stop) {
            Stop** newStops = new Stop*[NumStops + 1];
            for (int i = 0; i < NumStops; ++i) {
                newStops[i] = Stops[i];
            }
            newStops[NumStops] = stop;
            delete[] Stops;
            Stops = newStops;
            NumStops++;
        }
    
        // Operator overloading to compare routes
        bool operator==(const Route& other) const {
            if (RouteID != other.RouteID || NumStops != other.NumStops) return false;
            for (int i = 0; i < NumStops; ++i) {
                if (Stops[i]->GetStopID() != other.Stops[i]->GetStopID()) return false;
            }
            return true;
        }

        void DisplayRoute() const {
            cout << endl << "Route ID: " << RouteID << endl;
            cout << "Stops: ";
            for (int i = 0; i < NumStops; ++i) {
                cout << Stops[i]->GetStopName();
                if (i != NumStops - 1) cout << " -> ";
            }
            cout << endl;
        }
};

class TransportationService {
    private:
        User** Users;
        int NumUsers;
        Route** Routes;
        int NumRoutes;
        Stop** Stops;
        int NumStops;

    public:
        TransportationService() : Users(nullptr), NumUsers(0), Routes(nullptr), NumRoutes(0), Stops(nullptr), NumStops(0) {}

        ~TransportationService() {
            ClearMemory();
        }
        // User management
        void RegisterUser(User* user) {
            User** newUsers = new User*[NumUsers + 1];
            for (int i = 0; i < NumUsers; ++i) {
                newUsers[i] = Users[i];
            }
            newUsers[NumUsers] = user;
            cout << "User " << user->GetUserName() << " Registered Successfully!" << endl;
            delete[] Users;
            Users = newUsers;
            NumUsers++;
        }
        // Route management
        void AddRoute(Route* route) {
            Route** newRoutes = new Route*[NumRoutes + 1];
            for (int i = 0; i < NumRoutes; ++i) {
                newRoutes[i] = Routes[i];
            }
            newRoutes[NumRoutes] = route;
            cout << "Route " << route->GetRouteID() << " Created Successfully!" << endl;
            delete[] Routes;
            Routes = newRoutes;
            NumRoutes++;
        }
        // Stop management
        void AddStop(Stop* stop) {
            Stop** newStops = new Stop*[NumStops + 1];
            for (int i = 0; i < NumStops; ++i) {
                newStops[i] = Stops[i];
            }
            newStops[NumStops] = stop;
            cout << "Stop " << stop->GetStopName() << " Created Successfully!" << endl;
            delete[] Stops;
            Stops = newStops;
            NumStops++;
        }
        // User-stop assignment
        void AssignUserToStop(User* user, Stop* stop) {
            stop->AssignUser(user);
        }
        // Attendance recording
        void RecordUserAttendance(User* user) {
            if (user->GetAssignedStop()) {
                user->GetAssignedStop()->RecordAttendance(user);
            } else {
                cout << "User " << user->GetUserName() 
                    << " not assigned to any stop!" << endl;
            }
        }
        // System status
        void DisplaySystemStatus() const {
            cout << endl << "===== Transportation System Status =====";
            cout << endl << "Registered Users: " << NumUsers;
            cout << endl << "Available Routes: " << NumRoutes;
            cout << endl << "Bus Stops: " << NumStops << endl;
        }

        // Getters
        User** GetUsers() const { return Users; }
        int GetNumUsers() const { return NumUsers; }
        Route** GetRoutes() const { return Routes; }
        int GetNumRoutes() const { return NumRoutes; }
        Stop** GetStops() const { return Stops; }
        int GetNumStops() const { return NumStops; }

    private:
        void ClearMemory() {
            for (int i = 0; i < NumUsers; ++i) {
                delete Users[i];
            }
            delete[] Users;
            
            for (int i = 0; i < NumRoutes; ++i) {
                delete Routes[i];
            }
            delete[] Routes;
            for (int i = 0; i < NumStops; ++i) {
                delete Stops[i];
            }
            delete[] Stops;
        }
};

int main() {
    TransportationService campusTransport;
    cout << "Sana Munir Alam 24K-0573" << endl << endl;
    cout << "~===== FAST Transportation System =====~" << endl;

    // 1. Register Users
    cout << endl << "=== Registering Users ===" << endl;
    campusTransport.RegisterUser(new Student(1, "Mustafa"));
    campusTransport.RegisterUser(new Teacher(2, "Dr. Ali"));
    campusTransport.RegisterUser(new Staff(3, "Ahmed"));

    // 2. Create Stops
    cout << endl << "=== Creating Bus Stops ===" << endl;
    campusTransport.AddStop(new Stop(101, "Main Campus Gate"));
    campusTransport.AddStop(new Stop(102, "Faculty Housing"));
    campusTransport.AddStop(new Stop(103, "Student Hostels"));

    // 3. Create Routes
    cout << endl << "=== Creating Bus Routes ===" << endl;
    Route* morningRoute = new Route(1001);
    morningRoute->AddStop(campusTransport.GetStops()[0]); // Main Campus Gate
    morningRoute->AddStop(campusTransport.GetStops()[1]); // Faculty Housing
    morningRoute->AddStop(campusTransport.GetStops()[2]); // Student Hostels
    campusTransport.AddRoute(morningRoute);

    // 4. Assign Users to Stops
    cout << endl << "=== Assigning Users to Stops ===" << endl;
    campusTransport.AssignUserToStop(campusTransport.GetUsers()[0], campusTransport.GetStops()[0]); // Mustafa -> Main Campus Gate
    campusTransport.AssignUserToStop(campusTransport.GetUsers()[1], campusTransport.GetStops()[1]); // Dr. Ali -> Faculty Housing
    campusTransport.AssignUserToStop(campusTransport.GetUsers()[2], campusTransport.GetStops()[2]); // Ahmed -> Student Hostels

    // 5. Process Payments
    cout << endl << "=== Processing Payments ===" << endl;
    campusTransport.GetUsers()[0]->MakePayment(120.0); // Mustafa pays $120
    campusTransport.GetUsers()[1]->MakePayment(60.0);  // Dr. Ali pays $60
    campusTransport.GetUsers()[2]->MakePayment(40.0);  // Ahmed pays $40

    // 6. Record Attendance
    cout << endl << "=== Recording Attendance ===" << endl;
    campusTransport.RecordUserAttendance(campusTransport.GetUsers()[0]); // Mustafa
    campusTransport.RecordUserAttendance(campusTransport.GetUsers()[1]); // Dr. Ali
    campusTransport.RecordUserAttendance(campusTransport.GetUsers()[2]); // Ahmed

    // 7. Display System Status
    cout << endl << "=== System Status ===" << endl;
    campusTransport.DisplaySystemStatus();
    morningRoute->DisplayRoute();

    // 8. Operator Overloading Demo
    Route* anotherRoute = new Route(1001);
    anotherRoute->AddStop(campusTransport.GetStops()[0]);
    anotherRoute->AddStop(campusTransport.GetStops()[1]);
    anotherRoute->AddStop(campusTransport.GetStops()[2]);

    cout << endl << "=== Operator Overloading Demo ===" << endl;
    if (*morningRoute == *anotherRoute) {
        cout << "The two routes are the same!" << endl;
    } else {
        cout << "The two routes are different!" << endl;
    }

    delete anotherRoute;
    return 0;
}
