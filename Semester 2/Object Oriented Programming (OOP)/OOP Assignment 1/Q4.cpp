#include <iostream>
#include <string>
#include <ctime>
using namespace std;

class Stop; // Forward declaration

class Student {
    private:
        int StudentID;
        string StudentName;
        double Balance;
        bool IsActive;
        Stop* AssignedStop;
        static const double SEMESTER_FEE;

    public:
        Student(int ID, string Name) : StudentID(ID), StudentName(Name), Balance(0.0), IsActive(false), AssignedStop(nullptr) {
            // Paramatrized Constructor
        }
        // Setters
        void SetAssignedStop(Stop* stop) { 
            AssignedStop = stop; 
        }
        // Getters
        int GetStudentID() const { return StudentID; }
        string GetStudentName() const { return StudentName; }
        double GetBalance() const { return Balance; }
        bool GetActiveStatus() const { return IsActive; }
        Stop* GetAssignedStop() const { return AssignedStop; }

        void PaySemesterFee(double amount) {
            Balance += amount;
            if (Balance >= SEMESTER_FEE) {
                IsActive = true;
                Balance -= SEMESTER_FEE;
                cout << "Student " << GetStudentName() << " Fee Status: Paid" << endl;
            }else{
                cout << "Student " << GetStudentName() << " Fee Status: Remaining $" << SEMESTER_FEE - Balance << endl;
            }
        }
};

const double Student::SEMESTER_FEE = 100.0;

class Stop {
    private:
        int StopID;
        string StopName;
        Student** AssignedStudents;
        int NumStudents;

    public:
        Stop(int ID, string Name) : StopID(ID), StopName(Name), AssignedStudents(nullptr), NumStudents(0) {}

        ~Stop() {
            delete[] AssignedStudents;
        }

        // Getters
        int GetStopID() const { return StopID; }
        string GetStopName() const { return StopName; }
        Student** GetAssignedStudents() const { return AssignedStudents; }
        int GetNumStudents() const { return NumStudents; }

        void AssignStudent(Student* student) {
            Student** newStudents = new Student*[NumStudents + 1];
            for (int i = 0; i < NumStudents; ++i) {
                newStudents[i] = AssignedStudents[i];
            }
            newStudents[NumStudents] = student;
            cout << "Student " << student->GetStudentName() << " Assigned to Stop: " << GetStopName() << endl;
            delete[] AssignedStudents;
            AssignedStudents = newStudents;
            NumStudents++;
            student->SetAssignedStop(this);
        }

        void RecordAttendance(Student* student) const {
            if (student->GetActiveStatus()) {
                time_t now = time(0);
                cout << "Attendance recorded for " << student->GetStudentName() 
                    << " at " << ctime(&now);
            } else {
                cout << "Payment required for " << student->GetStudentName() << endl;
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
        Student** Students;
        int NumStudents;
        Route** Routes;
        int NumRoutes;
        Stop** Stops;
        int NumStops;

    public:
        TransportationService() : Students(nullptr), NumStudents(0), Routes(nullptr), NumRoutes(0), Stops(nullptr), NumStops(0) {}

        ~TransportationService() {
            ClearMemory();
        }
        // Student management
        void RegisterStudent(Student* student) {
            Student** newStudents = new Student*[NumStudents + 1];
            for (int i = 0; i < NumStudents; ++i) {
                newStudents[i] = Students[i];
            }
            newStudents[NumStudents] = student;
            cout << "Student " << student->GetStudentName() << " Registered Successfully!" << endl;
            delete[] Students;
            Students = newStudents;
            NumStudents++;
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
        // Student-stop assignment
        void AssignStudentToStop(Student* student, Stop* stop) {
            stop->AssignStudent(student);
        }
        // Attendance recording
        void RecordStudentAttendance(Student* student) {
            if (student->GetAssignedStop()) {
                student->GetAssignedStop()->RecordAttendance(student);
            } else {
                cout << "Student " << student->GetStudentName() 
                    << " not assigned to any stop!" << endl;
            }
        }
        // System status
        void DisplaySystemStatus() const {
            cout << endl << "===== Transportation System Status =====";
            cout << endl << "Registered Students: " << NumStudents;
            cout << endl << "Available Routes: " << NumRoutes;
            cout << endl << "Bus Stops: " << NumStops << endl;
        }

        // Getters
        Student** GetStudents() const { return Students; }
        int GetNumStudents() const { return NumStudents; }
        Route** GetRoutes() const { return Routes; }
        int GetNumRoutes() const { return NumRoutes; }
        Stop** GetStops() const { return Stops; }
        int GetNumStops() const { return NumStops; }

    private:
        void ClearMemory() {
            for (int i = 0; i < NumStudents; ++i) {
                delete Students[i];
            }
            delete[] Students;
            
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

    cout << "~===== FAST Transportation System =====~" << endl;

    // 1. Register Students
    cout << endl << "=== Registering Students ===" << endl;
    campusTransport.RegisterStudent(new Student(1, "Mustafa"));
    campusTransport.RegisterStudent(new Student(2, "Arafat"));
    campusTransport.RegisterStudent(new Student(3, "Umar"));

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

    // 4. Assign Students to Stops
    cout << endl << "=== Assigning Students to Stops ===" << endl;
    campusTransport.AssignStudentToStop(campusTransport.GetStudents()[0], campusTransport.GetStops()[0]); // Mustafa -> Main Campus Gate
    campusTransport.AssignStudentToStop(campusTransport.GetStudents()[1], campusTransport.GetStops()[2]); // Arafat -> Student Hostels
    campusTransport.AssignStudentToStop(campusTransport.GetStudents()[2], campusTransport.GetStops()[1]); // Umar -> Faculty Housing

    // 5. Process Semester Fee Payments
    cout << endl << "=== Processing Semester Fee Payments ===" << endl;
    campusTransport.GetStudents()[0]->PaySemesterFee(120.0); // Mustafa pays $120
    campusTransport.GetStudents()[1]->PaySemesterFee(80.0);  // Arafat pays $80
    campusTransport.GetStudents()[2]->PaySemesterFee(150.0); // Umar pays $150

    // 6. Record Attendance
    cout << endl << "=== Recording Attendance ===" << endl;
    campusTransport.RecordStudentAttendance(campusTransport.GetStudents()[0]); // Mustafa
    campusTransport.RecordStudentAttendance(campusTransport.GetStudents()[1]); // Arafat
    campusTransport.RecordStudentAttendance(campusTransport.GetStudents()[2]); // Umar

    // 7. Display System Status
    cout << endl << "=== System Status ===" << endl;
    campusTransport.DisplaySystemStatus();
    morningRoute->DisplayRoute();

    return 0;
}
