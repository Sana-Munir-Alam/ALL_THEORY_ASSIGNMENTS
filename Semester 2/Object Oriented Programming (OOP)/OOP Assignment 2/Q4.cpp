#include <iostream>
using namespace std;

// Global hash function
int CalculateHash(const string& Password) {
    int hash = 5381;
    for (char c : Password) {
        hash = hash * 33 + c;
    }
    return hash;
}

class User{
    protected:
        int UserID;
        string UserName;
        string Email;
        int HashPassword;
        string* PermissionList;
        int NumPermission;
    public:
        void CopyPermissions(const string* Permission, int Count) {
            PermissionList = new string[Count];
            for(int i = 0; i < Count; i++) {
                PermissionList[i] = Permission[i];
            }
            NumPermission = Count;
        }
        User(const int& ID, const string& Name, const string& email, const string& Password, const string* Permission, int Count) : UserID(ID), UserName(Name), Email(email){
            HashPassword = CalculateHash(Password);
            CopyPermissions(Permission, Count);
        }
        virtual ~User(){
            delete[] PermissionList;
        }
        bool Authenticate(const string& Password) const {
            return CalculateHash(Password) == HashPassword;
        }
        bool HasPermission(const string& Permission) const {
            for(int i = 0; i < NumPermission; i++) {
                if(PermissionList[i] == Permission){
                    return true;
                }
            }
            return false;
        }
        virtual void AccessLab(const string& Action) {
            if(HasPermission(Action)) {
                cout << "Access granted to " << Action << endl;
            } else {
                cout << "Access denied to " << Action << endl;
            }
        }
        virtual void Display() const{
            cout << "ID: " << UserID << endl;
            cout << "Name: " << UserName << endl;
            cout << "Email: " << Email << endl;
            cout << "Permission: " << endl;
            for (int i = 0; i < NumPermission; i++){
                cout << "\t-" << PermissionList[i] << endl;
            }
            cout << endl;
        }
        string GetName(){ return UserName;}
};

class Student: public User{
    private:
        int* ListAssignment;
        int NumAssignment;
    public:
        Student(const int& ID, const string& Name, const string& email, const string& Password) : User(ID, Name, email, Password, new string[1]{"Submit Assignment"} , 1), ListAssignment(nullptr), NumAssignment(0){
            // Paramatrized Constructor that calls Parent Class Constructor. Also passes the Permission list and Count
            
        }
        ~Student() override{
            delete[] ListAssignment;
        }
        void AddAssignment() {
            int* NewListAssignment = new int[NumAssignment + 1];
            for(int i = 0; i < NumAssignment; i++) {
                NewListAssignment[i] = ListAssignment[i];
            }
            NewListAssignment[NumAssignment] = 0;   // O means assignment is not submitted
            delete[] ListAssignment;
            ListAssignment = NewListAssignment;
            NumAssignment++;
        }
        void SubmitAssignment(int Index){
            if(Index >= 0 && Index < NumAssignment) {
                ListAssignment[Index] = 1; // 1 means assignment submitted
            }
        }
        virtual void Display() const override{
            cout << "Role: Student" << endl;
            User::Display();
            cout << "ListAssignment: ";
            for (int i = 0; i < NumAssignment; i++){
                cout << ListAssignment[i] << " ";
            }
            cout << endl << endl;
        }
};

class TA : public Student{
    private:
        string* ListStudents;
        int NumStudents;
        string* ListProjects;
        int NumProjects;
    public:
        TA(const int& ID, const string& Name, const string& email, const string& Password) : Student(ID, Name, email, Password), ListStudents(nullptr), NumStudents(0), ListProjects(nullptr), NumProjects(0){
            // Paramatrized Constructor that calls Parent Class Constructor. Also passes the Permission list and Count
            delete[] PermissionList;
            CopyPermissions(new string[2]{"View Projects", "Manage Students"}, 2);
        }
        ~TA(){
            delete[] ListStudents;
            delete[] ListProjects;
        }

        void AddStudent(const int& StudentID) {
            if(NumStudents >= 10) return;
            // Create a new array of students with + 1 space, copy old students than add in the latest students, delete old list transfer new list to old list place and delete new list
            string* NewListStudents = new string[NumStudents + 1];
            for(int i = 0; i < NumStudents; i++) {
                NewListStudents[i] = ListStudents[i];
            }
            NewListStudents[NumStudents] = to_string(StudentID);
            delete[] ListStudents;
            ListStudents = NewListStudents;
            NumStudents++;
        }
        void AddProject(const string& Project) {
            if(NumProjects >= 2) return;
            // Create a new array of projects with + 1 space, copy old projects than add in the latest project, delete old list transfer new list to old list place and delete new list
            string* NewListProjects = new string[NumProjects + 1];
            for(int i = 0; i < NumProjects; i++) {
                NewListProjects[i] = ListProjects[i];
            }
            NewListProjects[NumProjects] = Project;
            delete[] ListProjects;
            ListProjects = NewListProjects;
            NumProjects++;
        }
        void Display() const override {
            cout << endl <<"Role: TA" << endl;
            User::Display();
            cout << "Managed Students ID: " <<endl;
            for(int i = 0; i < NumStudents; i++) {
                cout << "\t-" << ListStudents[i] << endl;
            }
            cout << endl << "Projects: " << endl;
            for(int i = 0; i < NumProjects; i++) {
                cout << "\t-" << ListProjects[i] << endl;
            }
            ///cout << endl;
        }
    
};

class Professor: public User{
    public:
        Professor(const int& ID, const string& Name, const string& email, const string& Password) : User(ID, Name, email, Password, new string[2]{"Assign Projects", "Full Lab Access"}, 2){
            // Paramatrized Constructor that calls Parent Class Constructor.
        }
        void AssignProjects(TA& ta, const string& Project){
            ta.AddProject(Project);
        }
        void Display() const override{
            cout << "Role: Professor" << endl;
            User::Display();
        }
};

void AuthenticateAndPerformAction(User* user, const string& Action) {
    string Password;
    cout << "Enter password for " << user->GetName() << ": ";
    cin >> Password;
    
    if(user->Authenticate(Password)) {
        user->AccessLab(Action);
    } else {
        cout << "Authentication failed!" << endl;
    }
}
int main() {
    cout << "Sana Munir Alam 24K-0573" << endl;
    cout << endl << "~========== UNIVERSITY LAB MANAGEMENT SYSTEM ==========~" << endl << endl;

    // Section 1: Creating Users
    /*
    Password:
        Student: StuPass123
        TA:      TAPass456
        Prof     ProfPass789
    */
    cout << "~===== Creating System Users =====~" << endl;
    cout << "Creating student John Doe..." << endl;
    Student student(1001, "John Doe", "john@uni.edu", "StuPass123");
    
    cout << endl << "Creating TA Jane Smith..." << endl;
    TA ta(2001, "Jane Smith", "jane@uni.edu", "TaPass456");
    
    cout << endl << "Creating Professor Albert Einstein..." << endl;
    Professor prof(3001, "Albert Einstein", "albert@uni.edu", "ProfPass789");
    cout << "~===================================~" << endl << endl;

    // Section 2: Initializing Data
    cout << "~===== Setting Up Initial Data =====~" << endl;
    cout << "Adding assignments for John..." << endl;
    student.AddAssignment();  // Assignment 0
    student.AddAssignment();  // Assignment 1
    student.SubmitAssignment(0);

    cout << endl << "Configuring TA responsibilities..." << endl;
    ta.AddStudent(1001);     // Assign John to Jane
    ta.AddProject("Quantum Physics Lab Setup");
    cout << "~===================================~" << endl << endl;

    // Section 3: Display User Information
    cout << "~===== Displaying User Information =====~" << endl;
    cout << endl << "Student Details:" << endl;
    student.Display();
    
    cout << "--" << endl << "TA Details:";
    ta.Display();
    
    cout << "--" << endl << "Professor Details:" << endl;
    prof.Display();
    cout << "~========================================~" << endl << endl;

    // Section 4: Authentication Tests
    cout << "~===== Testing System Security =====~" << endl;
    cout << endl << "Testing Student Authentication:" << endl;
    AuthenticateAndPerformAction(&student, "Submit Assignment");
    
    cout << endl << "Testing TA Authentication:" << endl;
    AuthenticateAndPerformAction(&ta, "Manage Students");
    
    cout << endl << "Testing Professor Authentication:" << endl;
    AuthenticateAndPerformAction(&prof, "Full Lab Access");
    cout << "~===================================~" << endl << endl;

    // Section 5: Project Management Demo
    cout << "~===== Research Project Management =====~" << endl;
    cout << endl << "Professor assigning new project to TA..." << endl;
    prof.AssignProjects(ta, "Relativity Theory Experiments");
    
    cout << endl << "Updated TA Details:" << endl;
    ta.Display();
    cout << "~=======================================~" << endl << endl;

    cout << "~========== Exiting Program ==========~" << endl;
    return 0;
}
