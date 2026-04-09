#include <iostream>
#include <string>
using namespace std;

class Skill {
private:
    int SkillID;
    string SkillName;
    string Description;
public:
    Skill(int id, string name, string desc) : SkillID(id), SkillName(name), Description(desc) {
        // Paramatrized Cosntructor that uses Inline Initilisation
    }

    void ShowSkillDetails() {
        cout << "Skill ID: " << SkillID << endl;
        cout << "Name: " << SkillName << endl;
        cout << "Description: " << Description << endl << endl;
    }

    void UpdateSkillDescription(string NewDesc) {
        Description = NewDesc;
    }
};

class Sport {
private:
    int SportID;
    string Name;
    string Description;
    Skill** RequiredSkills;
    int RequiredSkillsCount;
public:
    Sport(int id, string name, string desc) : SportID(id), Name(name), Description(desc), RequiredSkills(nullptr), RequiredSkillsCount(0) {
        // Paramatrized Cosntructor that uses Inline Initilisation
    }

    void AddSkill(Skill* skill) {
        Skill** NewSkills = new Skill*[RequiredSkillsCount + 1];    // Creating New Space for the new skill
        for (int i = 0; i < RequiredSkillsCount; ++i) {
            NewSkills[i] = RequiredSkills[i];       // Storing Previous Recorded Skills
        }
        NewSkills[RequiredSkillsCount] = skill;     // Storing New Skill
        delete[] RequiredSkills;                    // Freeing Old Array so That New Array data can be stored in it
        RequiredSkills = NewSkills;
        RequiredSkillsCount++;                      // Increasing The Count of Total Skill stored 
    }

    void RemoveSkill(Skill* skill) {
        int index = -1;
        for (int i = 0; i < RequiredSkillsCount; ++i) {
            if (RequiredSkills[i] == skill) {   // If the Skill that needs to be removed matches the stored Skill
                index = i;                      // Record the Index Location
                break;
            }
        }
        if (index != -1) {
            Skill** NewSkills = new Skill*[RequiredSkillsCount - 1];    // Creating A New Space As the Skill Size Is Getting Reduced.
            for (int i = 0, j = 0; i < RequiredSkillsCount; ++i) {
                if (i != index) {                                       // If The i Doesn't Match The Remove Skill Index Copy Other Skills To New Array.
                    NewSkills[j++] = RequiredSkills[i];                 // Incrementing j.
                }
            }
            delete[] RequiredSkills;                                    // Freeing Old Array so That New Array data can be stored in it
            RequiredSkills = NewSkills;
            RequiredSkillsCount--;                                      // Decreasing The Count of Total Skill stored
        }
    }
};

class Mentor; // Forward Declaration

class Student {
private:
    int StudentID;
    string Name;
    int Age;
    Sport** SportsInterests;
    int SportsInterestsCount;
    Mentor* MentorAssigned;
public:
    Student(int id, string name, int age) : StudentID(id), Name(name), Age(age), SportsInterests(nullptr), SportsInterestsCount(0), MentorAssigned(nullptr) {
        // Paramatrized Cosntructor that uses Inline Initilisation
    }

    // These Functions Code is Written Outside the class becuase Student and Mentor classes are dependent on each other for different aspect.
    void RegisterForMentorship(Mentor* m);
    void UnassignMentor() { MentorAssigned = nullptr; }
    void ViewMentorDetails();

    void UpdateSportsInterest(Sport* sport) {
        Sport** NewInterests = new Sport*[SportsInterestsCount + 1];        // Creating New Space for the New Sport Interest
        for (int i = 0; i < SportsInterestsCount; ++i) {
            NewInterests[i] = SportsInterests[i];                           // Storing Previous Sport Interest
        }
        NewInterests[SportsInterestsCount] = sport;                         // Storing the New Sport Interest
        delete[] SportsInterests;                                           // Freeing Old Array so That New Array data can be stored in it
        SportsInterests = NewInterests;
        SportsInterestsCount++;                                             // Increasing Sport Interest Counter.
    }

    // Getters
    string GetName() { 
        return Name; 
    }
    int GetStudentID() { 
        return StudentID; 
    }
    Sport** GetSportsInterests() { 
        return SportsInterests; 
    }
    int GetSportsInterestsCount() { 
        return SportsInterestsCount; 
    }

    ~Student() {
        delete[] SportsInterests;
    }
};

class Mentor {
private:
    int MentorID;
    string Name;
    Sport** SportsExpertise;
    int SportsExpertiseCount;
    int MaxLearners;
    Student** AssignedLearners;
    int CurrentLearnersCount;
public:
    Mentor(int id, string name, int max) : MentorID(id), Name(name), MaxLearners(max), SportsExpertise(nullptr), SportsExpertiseCount(0), AssignedLearners(nullptr), CurrentLearnersCount(0) {
        // Paramatrized Cosntructor that uses Inline Initilisation
    }

    bool AssignLearner(Student* s) {
        if (CurrentLearnersCount >= MaxLearners) {  // If the mentor Capacity is Full it cant take more students in until space is cleared
            cout << "Mentor " << Name << " is at full capacity. Cannot assign " << s->GetName() << ".\n";
            return false;
        }

        bool CommonSport = false;                               // This variable will be used whether student can be assigned to mentor if both have common sport.
        Sport** studentSports = s->GetSportsInterests();        // Getting Student Sport Interest through getter
        int studentSportsCount = s->GetSportsInterestsCount();  // As well as the sport Interest count.

        // If The Sport of Student and Mentor Matches than the variable CommonSport will be True.
        for (int i = 0; i < studentSportsCount; ++i) {
            for (int j = 0; j < SportsExpertiseCount; ++j) {
                if (studentSports[i] == SportsExpertise[j]) {
                    CommonSport = true;
                    break;
                }
            }
            if (CommonSport) break;
        }
        // If The Sport of Student and Mentor DOESN'T Matches than the variable CommonSport will be False and a message will be displayed.
        if (!CommonSport) {
            cout << "Student " << s->GetName() << " has no common sports with mentor " << Name << "'s expertise.\n";
            return false;
        }

        Student** NewLearners = new Student*[CurrentLearnersCount + 1];     // Creating New Space for the New Student Assigning
        for (int i = 0; i < CurrentLearnersCount; ++i) {
            NewLearners[i] = AssignedLearners[i];                           // Storing Previous Assigned Students
        }
        NewLearners[CurrentLearnersCount] = s;                              // Assigning New Students
        delete[] AssignedLearners;                                          // Freeing Old Array so That New Array data can be stored in it
        AssignedLearners = NewLearners;
        CurrentLearnersCount++;                                             // Increasing Student Assigned Counter.
        cout << "Student " << s->GetName() << " assigned to mentor " << Name << ".\n";
        return true;
    }

    void RemoveLearner(Student* s) {
        int index = -1;
        for (int i = 0; i < CurrentLearnersCount; ++i) {
            if (AssignedLearners[i] == s) {     // If the Student That Needs To Be Removed Matches The Stored Student
                index = i;                      // Store That Index Value.
                break;
            }
        }
        if (index != -1) {
            Student** NewLearners = new Student*[CurrentLearnersCount - 1];     // Creating A New Space As the Assigned Student Size Is Getting Reduced.
            for (int i = 0, j = 0; i < CurrentLearnersCount; ++i) {
                if (i != index) {                                               // If The i Doesn't Match The Remove Skill Index Copy Other Skills To New Array.
                    NewLearners[j++] = AssignedLearners[i];                     // Incrementing j
                }
            }
            delete[] AssignedLearners;                                          // Freeing Old Array so That New Array data can be stored in it
            AssignedLearners = NewLearners;
            CurrentLearnersCount--;                                             // Decreasing The Count of Total Skill stored
            s->UnassignMentor();                                                // Calling the UnassignMentor function from student class as he is no more part of mentor.
            cout << "Student " << s->GetName() << " removed from mentor " << Name << ".\n";
        }
    }

    void ViewLearners() {
        cout << "\nLearners assigned to " << Name << ":\n";
        for (int i = 0; i < CurrentLearnersCount; ++i) {
            cout << "ID: " << AssignedLearners[i]->GetStudentID() 
                 << ", Name: " << AssignedLearners[i]->GetName() << endl;
        }
    }

    void ProvideGuidance() {
        cout << "\nMentor " << Name << " is providing guidance to learners.\n";
    }

    void AddSportsExpertise(Sport* sport) {
        Sport** newExpertise = new Sport*[SportsExpertiseCount + 1];
        for (int i = 0; i < SportsExpertiseCount; ++i) {
            newExpertise[i] = SportsExpertise[i];
        }
        newExpertise[SportsExpertiseCount] = sport;
        delete[] SportsExpertise;
        SportsExpertise = newExpertise;
        SportsExpertiseCount++;
    }

    // Getters
    int GetMentorID() { 
        return MentorID; 
    }
    string GetName() { 
        return Name; 
    }

    ~Mentor() {
        delete[] SportsExpertise;
        delete[] AssignedLearners;
    }
};

void Student::RegisterForMentorship(Mentor* m) {
    if (MentorAssigned != nullptr) {
        cout << "Student " << Name << " is already assigned to a mentor.\n";
        return;
    }
    if (m->AssignLearner(this)) {
        MentorAssigned = m;
    }
}

void Student::ViewMentorDetails() {
    if (MentorAssigned) {
        cout << endl << "Mentor Details:" << endl;
        cout << "ID: " << MentorAssigned->GetMentorID() << endl;
        cout << "Name: " << MentorAssigned->GetName() << endl;
    } else {
        cout << endl << "No Mentor Assigned To " << GetName() << endl;
    }
}

int main() {
    cout << "------ Creating Skills ------" << endl;
    Skill forehand(1, "Forehand", "Basic stroke");
    Skill backhand(2, "Backhand", "Backhand stroke");
    cout << "Skills created:" << endl << endl;
    forehand.ShowSkillDetails();
    backhand.ShowSkillDetails();

    cout << "------ Creating Sports ------" << endl;
    Sport tennis(1, "Tennis", "Racket sport");
    tennis.AddSkill(&forehand);
    tennis.AddSkill(&backhand);
    cout << "Sport created: Tennis with skills Forehand and Backhand.\n" << endl;

    cout << "------ Creating Mentor ------" << endl;
    Mentor Ali(101, "Ali", 2); // Capacity of 2 learners
    Ali.AddSportsExpertise(&tennis);
    cout << "Mentor created: Ali with expertise in Tennis.\n" << endl;

    cout << "------ Creating Students ------" << endl;
    Student Asad(201, "Asad", 20);
    Asad.UpdateSportsInterest(&tennis);
    cout << "Student created: Asad with interest in Tennis." << endl;

    Student Asma(202, "Asma", 21);
    Asma.UpdateSportsInterest(&tennis);
    cout << "Student created: Asma with interest in Tennis." << endl;

    Student Bilal(203, "Bilal", 22);
    Bilal.UpdateSportsInterest(&tennis);
    cout << "Student created: Bilal with interest in Tennis.\n" << endl;

    cout << "------ Assigning Students to Mentor ------" << endl;
    cout << "Attempting to assign Asad to Ali..." << endl;
    Asad.RegisterForMentorship(&Ali);   // Should succeed
    Asad.RegisterForMentorship(&Ali);   // Should Display already assigned messgae
    
    cout << "\nAttempting to assign Asma to Ali..." << endl;
    Asma.RegisterForMentorship(&Ali); // Should succeed

    cout << "\nAttempting to assign Bilal to Ali..." << endl;
    Bilal.RegisterForMentorship(&Ali); // Should fail (capacity full)

    cout << "\n------ Viewing Mentor's Learners ------" << endl;
    Ali.ViewLearners();

    cout << "\n------ Removing a Student and Assigning a New One ------" << endl;
    cout << "Removing Asad from Ali's Learners..." << endl;
    Ali.RemoveLearner(&Asad);

    cout << "\nAttempting To Assign Bilal to Ali..." << endl;
    Bilal.RegisterForMentorship(&Ali); // Should succeed now

    cout << "\n------ Viewing Mentor's Learners Again ------" << endl;
    Ali.ViewLearners();

    cout << "\n------ Viewing Mentor Details for Students ------" << endl;
    cout << "Viewing Mentor Details for Bilal..." << endl;
    Bilal.ViewMentorDetails();

    cout << "\nViewing Mentor Details for Asad..." << endl;
    Asad.ViewMentorDetails();

    cout << "\n------ Providing Guidance ------" << endl;
    Ali.ProvideGuidance();

    cout << "\n------ Program End ------" << endl;
    return 0;
}
