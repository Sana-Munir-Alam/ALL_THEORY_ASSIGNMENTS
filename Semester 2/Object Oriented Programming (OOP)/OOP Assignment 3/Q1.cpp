#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
using namespace std;

class Stop; // forward
class Route;
class TransportationService;

// —————— HELPERS FOR BINARY I/O ——————
void writeStringBin(ofstream &out,const string &s){
    int len = (int)s.size();
    out.write(reinterpret_cast<char*>(&len), sizeof(len));
    out.write(s.data(), len);
}
string readStringBin(ifstream &in){
    int len; in.read(reinterpret_cast<char*>(&len), sizeof(len));
    string s(len,'\0');
    in.read(&s[0], len);
    return s;
}

// Abstract User Class 
class User {
    protected:
        int UserID;
        string UserName;
        double Balance;
        bool IsActive;
        Stop* AssignedStop;
    
    public:
        User(int ID, const string& Name) : UserID(ID), UserName(Name), Balance(0.0), IsActive(false), AssignedStop(nullptr) {}
        virtual ~User() = default;

        // Getter and Settors
        int    GetUserID()       const { return UserID; }
        string GetUserName()     const { return UserName; }
        double GetBalance()      const { return Balance; }
        bool   GetActiveStatus() const { return IsActive; }
        Stop*  GetAssignedStop() const { return AssignedStop; }
        void   SetAssignedStop(Stop* stop) { AssignedStop = stop; }

        // Pure Virtual Functions
        virtual void MakePayment(double Amount) = 0;
        virtual void Login() = 0;
    
        // static template to dump this subclass
        template<typename T>
        static int SaveAll(const string& filename, const TransportationService& TS);

        // Comparison operator
        bool operator==(const User& other) const {
            return UserID == other.UserID && UserName == other.UserName;
        }
};

// Derived Classes 
class Student : public User {
    private:
        static constexpr double SEMESTER_FEE = 100.0;
    public:
        Student(int ID, const string& Name) : User(ID, Name) {}
        void MakePayment(double Amount) override {
            try {
                if (Amount < 0) {
                    throw invalid_argument("[Student::MakePayment] Invalid amount! Amount cannot be negative.");    // Throw an exception for an invalid amount
                }
                Balance += Amount;
                if (Balance >= SEMESTER_FEE) {
                    IsActive = true;
                    Balance -= SEMESTER_FEE;
                    cout << "Student " << UserName << " Fee Status: Paid" << endl;
                } else {
                    cout << "Student " << UserName
                        << " Fee Status: Remaining $"
                        << (SEMESTER_FEE - Balance) << "" << endl;
                }
            } catch (const invalid_argument& e) {
                cout << e.what() << endl;
            }
        }
        // Login style: username + password
        void Login() override {
            cout << "Student " << UserName << " logging in with Password style..." << endl;
        }
        static int SaveAll(const TransportationService& TS) {
            return User::SaveAll<Student>("student.bin", TS);
        }
};

class Teacher : public User {
    private:
        static constexpr double MONTHLY_FEE = 50.0;
    public:
        Teacher(int ID, const string& Name) : User(ID, Name) {}
        void MakePayment(double Amount) override {
            try {
                if (Amount < 0) {
                    throw invalid_argument("[Teacher::MakePayment] Invalid amount! Amount cannot be negative.");    // Throw an exception for an invalid amount
                }
                Balance += Amount;
                if (Balance >= MONTHLY_FEE) {
                    IsActive = true;
                    Balance -= MONTHLY_FEE;
                    cout << "Teacher " << UserName << " Fee Status: Paid" << endl;
                } else {
                    cout << "Teacher " << UserName
                        << " Fee Status: Remaining $"
                        << (MONTHLY_FEE - Balance) << "" << endl;
                }
            } catch (const invalid_argument& e) {
                cout << e.what() << endl;
            }
        }
        // Login style: username + OTP
        void Login() override {
            cout << "Teacher " << UserName << " logged in with OTP" << endl;
        }
        static int SaveAll(const TransportationService& TS) {
            return User::SaveAll<Teacher>("teacher.bin", TS);
        }
};

class Staff : public User {
    private:
        static constexpr double MONTHLY_FEE = 30.0;
    public:
        Staff(int ID, const string& Name) : User(ID, Name) {}

        void MakePayment(double Amount) override {
            try {
                if (Amount < 0) {
                    throw invalid_argument("[Staff::MakePayment] Invalid amount! Amount cannot be negative.");  // Throw an exception for an invalid amount
                }
                Balance += Amount;
                if (Balance >= MONTHLY_FEE) {
                    IsActive = true;
                    Balance -= MONTHLY_FEE;
                    cout << "Staff " << UserName << " Fee Status: Paid" << endl;
                } else {
                    cout << "Staff " << UserName
                        << " Fee Status: Remaining $"
                        << (MONTHLY_FEE - Balance) << "" << endl;
                }
            } catch (const invalid_argument& e) {
                cout << e.what() << endl;
            }
        }
        // Login style: PIN code
        void Login() override {
            cout << "Staff " << UserName << " logging in with PIN code..." << endl;
        }
        static int SaveAll(const TransportationService& TS) {
            return User::SaveAll<Staff>("staff.bin", TS);
        }
};

class Stop {
    private:
        int StopID;
        string StopName;
        User** AssignedUsers;
        int NumUsers;

    public:
        Stop(int ID, const string& Name) : StopID(ID), StopName(Name), AssignedUsers(nullptr), NumUsers(0) {}

        ~Stop() {
            delete[] AssignedUsers;
        }

        // Getters
        int GetStopID() const          { return StopID; }
        const string& GetStopName() const { return StopName; }
        User** GetAssignedUsers() const   { return AssignedUsers; }
        int GetNumUsers() const        { return NumUsers; }

        void AssignUser(User* user) {
            if (!user) {
                cerr << "Error: Null user cannot be assigned to stop." << endl;
                return;
            }
            try {
                User** temp = new User*[NumUsers + 1];
                for (int i = 0; i < NumUsers; ++i)
                    temp[i] = AssignedUsers[i];
                temp[NumUsers] = user;
                
                delete[] AssignedUsers;
                AssignedUsers = temp;
                ++NumUsers;
                
                user->SetAssignedStop(this);
                cout << "User " << user->GetUserName() 
                     << " Assigned to Stop: " << StopName << "" << endl;
            } 
            catch (const bad_alloc& e) {
                cerr << "Memory allocation failed in Stop::AssignUser: " << e.what() << "" << endl;
            }
        }

        void RecordAttendance(User* user) const {
            if (user->GetActiveStatus()) {
                time_t now = time(nullptr);
                cout << "Attendance recorded for " << user->GetUserName() << " at " << ctime(&now);
            } else {
                cout << "Payment required for " << user->GetUserName() << "" << endl;
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
        ~Route() { delete[] Stops; }

        int GetRouteID() const { return RouteID; }
        Stop** GetStops() const { return Stops; }
        int GetNumStops() const  { return NumStops; }

        void AddStop(Stop* stop) {
            if (!stop) {
                cerr << "Error: Cannot add null stop to route." << endl;
                return;
            }
            try {
                Stop** temp = new Stop*[NumStops + 1];
                for (int i = 0; i < NumStops; ++i)
                    temp[i] = Stops[i];
                    
                temp[NumStops] = stop;
                delete[] Stops;
                Stops = temp;
                ++NumStops;
            } 
            catch (const bad_alloc& e) {
                cerr << "Memory allocation failed in Route::AddStop: "  << e.what() << "" << endl;
            }
        }

        bool operator==(const Route& other) const {
            if (RouteID != other.RouteID || NumStops != other.NumStops) 
                return false;
            for (int i = 0; i < NumStops; ++i) {
                if (Stops[i]->GetStopID() != other.Stops[i]->GetStopID())
                    return false;
            }
            return true;
        }

        void DisplayRoute() const {
            cout << "\nRoute ID: " << RouteID << "\nStops: ";
            for (int i = 0; i < NumStops; ++i) {
                cout << Stops[i]->GetStopName();
                if (i < NumStops - 1) cout << " -> ";
            }
            cout << "" << endl;
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
            for (int i = 0; i < NumUsers; ++i) delete Users[i];
            delete[] Users;
            for (int i = 0; i < NumRoutes; ++i) delete Routes[i];
            delete[] Routes;
            for (int i = 0; i < NumStops; ++i) delete Stops[i];
            delete[] Stops;
        }

        void RegisterUser(User* user) {
            if (!user) {
                cerr << "Error: Cannot register null user." << endl;
                return;
            }
            try {
                User** temp = new User*[NumUsers + 1];
                for (int i = 0; i < NumUsers; ++i)
                    temp[i] = Users[i];
                temp[NumUsers] = user;
                delete[] Users;
                Users = temp;
                ++NumUsers;
                cout << "User " << user->GetUserName() << " Registered Successfully!" << endl;
            } 
            catch (const bad_alloc& e) {
                cerr << "Memory allocation failed in RegisterUser: " << e.what() << "" << endl;
            }
        }

        void AddRoute(Route* route) {
            if (!route) {
                cerr << "Error: Cannot add null route." << endl;
                return;
            }
            try {
                Route** temp = new Route*[NumRoutes + 1];
                for (int i = 0; i < NumRoutes; ++i)
                    temp[i] = Routes[i];
                temp[NumRoutes] = route;
                delete[] Routes;
                Routes = temp;
                ++NumRoutes;
                cout << "Route " << route->GetRouteID() << " Created Successfully!" << endl;
            } 
            catch (const bad_alloc& e) {
                cerr << "Memory allocation failed in AddRoute: " << e.what() << "" << endl;
            }
        }

        void AddStop(Stop* stop) {
            if (!stop) {
                cerr << "Error: Cannot add null stop." << endl;
                return;
            }
            try {
                Stop** temp = new Stop*[NumStops + 1];
                for (int i = 0; i < NumStops; ++i)
                    temp[i] = Stops[i]; 
                temp[NumStops] = stop;
                delete[] Stops;
                Stops = temp;
                ++NumStops;
                cout << "Stop " << stop->GetStopName() << " Created Successfully!" << endl;
            } 
            catch (const bad_alloc& e) {
                cerr << "Memory allocation failed in AddStop: " << e.what() << "" << endl;
            }
        }

        void AssignUserToStop(User* user, Stop* stop) {
            stop->AssignUser(user);
        }

        void RecordUserAttendance(User* user) {
            if (user->GetAssignedStop())
                user->GetAssignedStop()->RecordAttendance(user);
            else
                cout << "User " << user->GetUserName()
                    << " not assigned to any stop!" << endl;
        }

        void DisplaySystemStatus() const {
            cout << "\n===== Transportation System Status =====" << endl
                << "Registered Users: " << NumUsers << "" << endl
                << "Available Routes: "   << NumRoutes << "" << endl
                << "Bus Stops: "          << NumStops   << "" << endl;
        }

        int SaveAllRoutes(const string& filename = "routes.bin") const {
            ofstream out(filename, ios::binary);
            if (!out) throw runtime_error{"Could not open " + filename};
    
            // header: number of routes
            int R = NumRoutes;
            out.write(reinterpret_cast<const char*>(&R), sizeof(R));
    
            // each route: ID, #stops, then for each stop (stopID + name)
            for (int i = 0; i < NumRoutes; ++i) {
                Route* r = Routes[i];
                int rid = r->GetRouteID();
                out.write(reinterpret_cast<const char*>(&rid), sizeof(rid));
    
                int ns = r->GetNumStops();
                out.write(reinterpret_cast<const char*>(&ns), sizeof(ns));
    
                for (int j = 0; j < ns; ++j) {
                    Stop* s = r->GetStops()[j];
                    int sid = s->GetStopID();
                    out.write(reinterpret_cast<const char*>(&sid), sizeof(sid));
                    writeStringBin(out, s->GetStopName());
                }
            }
            out.close();
            if (out.fail()) throw runtime_error{"Error: Write Operation failed on " + filename};
            if (out.fail()) throw runtime_error{"Error: Unrecoverable error on " + filename};
            return 0;
        }

        // Accessors
        User** GetUsers() const   { return Users; }
        int GetNumUsers() const   { return NumUsers; }
        Route** GetRoutes() const { return Routes; }
        int GetNumRoutes() const  { return NumRoutes; }
        Stop** GetStops() const   { return Stops; }
        int GetNumStops() const   { return NumStops; }
};

// TEMPLATE OF FILING FOR USER CHILDREN
template<typename T>
int User::SaveAll(const string& filename, const TransportationService& TS) {
    ofstream out(filename, ios::binary);
    if (!out) throw runtime_error{"Could not open " + filename};

    // count how many T* objects we have
    int count = 0;
    for (int i = 0; i < TS.GetNumUsers(); ++i) {
        if (dynamic_cast<T*>(TS.GetUsers()[i])) {
            ++count;
        }
    }
    // write header
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // write each record
    for (int i = 0; i < TS.GetNumUsers(); ++i) {
        if (auto ptr = dynamic_cast<T*>(TS.GetUsers()[i])) {
            int    id   = ptr->GetUserID();
            double bal  = ptr->GetBalance();
            bool   act  = ptr->GetActiveStatus();
            int    sid  = ptr->GetAssignedStop() 
                         ? ptr->GetAssignedStop()->GetStopID() 
                         : 0;

            out.write(reinterpret_cast<const char*>(&id), sizeof(id));
            writeStringBin(out, ptr->GetUserName());
            out.write(reinterpret_cast<const char*>(&bal), sizeof(bal));
            out.write(reinterpret_cast<const char*>(&act), sizeof(act));
            out.write(reinterpret_cast<const char*>(&sid), sizeof(sid));
        }
    }
    // close + error checking
    out.close();
    if (out.fail()) throw runtime_error{"Error: Write Operation failed on " + filename};
    if (out.fail()) throw runtime_error{"Error: Unrecoverable error on " + filename};
    return 0;
}

int main() {
        try {
        TransportationService TS;
        cout << "Sana Munir Alam 24K-0573\n" << endl;
        cout << "~===== FAST Transportation System =====~" << endl;

        // Safe user allocation with cleanup of other succesfully stored user incase one fialed
        User* u[3] = {nullptr};
        try {
            u[0] = new Student(1, "Mustafa");
            u[1] = new Teacher(2, "Dr.Ali");
            u[2] = new Staff(3, "Ahmed");
        } 
        catch (const bad_alloc& e) {
            cerr << "User allocation failed: " << e.what() << "" << endl;
            for (auto& ptr : u) delete ptr;  // Cleanup any allocated users
            return 1;
        }     

        // 1. Register & Login Users
        cout << "\n=== Registering & Logging In Users ===" << endl;
        for (int i = 0; i < 3; ++i) {
            u[i]->Login();                      // each subclass’s Login()
            TS.RegisterUser(u[i]); // each subclass registration
        }

        // 2. Create Stops with safe allocation
        cout << "\n=== Creating Bus Stops ===" << endl;
        try {
            TS.AddStop(new Stop(101, "Main Campus Gate"));
            TS.AddStop(new Stop(102, "Faculty Housing"));
            TS.AddStop(new Stop(103, "Student Hostels"));
        } 
        catch (const bad_alloc& e) {
            cerr << "Stop allocation failed: " << e.what() << "" << endl;
            return 1;
        }

        // 3. Create Routes with safe allocation
        cout << "\n=== Creating Bus Routes ===" << endl;
        Route* morningRoute = nullptr;
        Route* anotherRoute = nullptr;
        try {
            morningRoute = new Route(1001);
            anotherRoute = new Route(1002);
        } 
        catch (const bad_alloc& e) {
            cerr << "Route allocation failed: " << e.what() << "" << endl;
            delete morningRoute;
            delete anotherRoute;
            return 1;
        }
        // Add stops to routes with null checks
        if (TS.GetNumStops() >= 3) {  // Ensure stops were added
            morningRoute->AddStop(TS.GetStops()[0]);
            morningRoute->AddStop(TS.GetStops()[1]);
            morningRoute->AddStop(TS.GetStops()[2]);
            
            anotherRoute->AddStop(TS.GetStops()[2]);
            anotherRoute->AddStop(TS.GetStops()[1]);
            anotherRoute->AddStop(TS.GetStops()[2]);
        }
        else {
            cerr << "Error: Insufficient stops for route creation" << endl;
            return 1;
        }
        TS.AddRoute(morningRoute);                  // Adding Route 1
        TS.AddRoute(anotherRoute);                  // Adding Route 2

        // 4. Assign Users to Stops
        cout << "\n=== Assigning Users to Stops ===" << endl;
        TS.AssignUserToStop(TS.GetUsers()[0], TS.GetStops()[2]);
        TS.AssignUserToStop(TS.GetUsers()[1], TS.GetStops()[1]);
        TS.AssignUserToStop(TS.GetUsers()[2], TS.GetStops()[0]);

        // 5. Process Payments
        cout << "\n=== Processing Payments ===" << endl;
        TS.GetUsers()[0]->MakePayment(-120.0);      // Throws an Exception
        TS.GetUsers()[1]->MakePayment(60.0);        // Payment Accepted
        TS.GetUsers()[2]->MakePayment(20.0);        // Balance Remaining

        // 6. Record Attendance
        cout << "\n=== Recording Attendance ===" << endl;
        for (int i = 0; i < TS.GetNumUsers(); ++i) {
            TS.RecordUserAttendance(TS.GetUsers()[i]);
        }
        // 7. Display System Status
        cout << "\n=== System Status For Route 1001 ===" << endl;
        TS.DisplaySystemStatus();
        morningRoute->DisplayRoute();

        cout << "\n=== System Status For Route 1002 ===" << endl; // Display System Status
        TS.DisplaySystemStatus();
        anotherRoute->DisplayRoute();    

        // 8. Operator Overloding Demo
        cout << "\n=== Operator Overloading Demo ===" << endl;
        if (*morningRoute == *anotherRoute)
            cout << "The two routes are the same!" << endl;
        else
            cout << "The two routes are different!" << endl;

        // 9. Saving Datas in File
        Student::SaveAll(TS);
        Teacher::SaveAll(TS);
        Staff::SaveAll(TS);
        TS.SaveAllRoutes();

    } catch (const exception& ex) {
        cerr << "Fatal error: " << ex.what() << "" << endl;
        return 1;
    }

    // ----- new filing demo -----
    cout << "\n=== Filing Demo ===" << endl;
    cout<<"\nEnter a Route ID to fetch: ";
    int query; 
    cin >> query;

    ifstream in("routes.bin", ios::binary);         // Open Route file for reading
    if(!in){                                        // Check to see if file opened successfully
      cerr<<"Failed opening routes.bin" << endl;          // Print error message
      return 1;                                     // Exit
    }
    int total;
    in.read(reinterpret_cast<char*>(&total), sizeof(total));
    bool found=false;
    for(int i=0; i<total && !found; ++i){
      int rid;      // Route ID
      in.read(reinterpret_cast<char*>(&rid),sizeof(rid));
      int ns;       // Number of Stops
      in.read(reinterpret_cast<char*>(&ns), sizeof(ns));
      if(rid==query){
        cout<<"Route "<<rid<<" found with "<<ns<<" stops:" << endl;
        for(int j=0;j<ns;++j){
          int sid;   in.read(reinterpret_cast<char*>(&sid), sizeof(sid));
          string nm = readStringBin(in);
          cout<<"  ["<<sid<<"] "<<nm<<"" << endl;
        }
        found=true;
      } else {
        // skip past this route's stops
        for(int j=0; j<ns; ++j){
          int sid; 
          in.read(reinterpret_cast<char*>(&sid), sizeof(sid));
          int nl;  
          in.read(reinterpret_cast<char*>(&nl),  sizeof(nl));
          in.seekg(nl, ios::cur);
        }
      }
    }
    if(!found) {
        cout<<"Route "<<query<<" does not exist." << endl;
    }
    return 0;
}
