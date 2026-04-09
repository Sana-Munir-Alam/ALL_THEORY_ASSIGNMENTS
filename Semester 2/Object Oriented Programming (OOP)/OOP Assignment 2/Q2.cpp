#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;

// Forward declarations
class Ghost;

// Visitor class
class Visitor {
    private:
        string Name;
        int BraveryLevel;
    public:
        Visitor(string n, int bravery) : Name(n), BraveryLevel(bravery) {}

        string GetName() const { return Name; }
        int GetBraveryLevel() const { return BraveryLevel; }

        void ReactToGhost(const Ghost& ghost) const;
};

// Base Ghost class
class Ghost {
    protected:
        string Name;
        int ScareLevel;
    public:
        Ghost(string n, int Scare) : Name(n), ScareLevel(Scare) {/* Paramatrized Constructor*/}
        virtual ~Ghost() {}

        string GetName() const { return Name; }
        int GetScareLevel() const { return ScareLevel; }

        virtual void Haunt() const = 0; // Virtual Function used for Haunting
        // Overloading Operators (Decleration)
        friend ostream& operator<<(ostream& os, const Ghost& ghost);
        Ghost* operator+(const Ghost& other) const;
};

ostream& operator<<(ostream& os, const Ghost& ghost) {
    os << "Ghost Name: " << ghost.Name << ", Scare Level: " << ghost.ScareLevel;
    return os;
}

// Derived Ghost classes [Poltergeist, Banshee, ShadowGhost]
class Poltergeist : virtual public Ghost {
    public:
        Poltergeist(string n, int Scare) : Ghost(n, Scare) {/* Paramatrized Constructor that also calls Parent Constructor*/}
        void Haunt() const override {
            cout << "\t" << Name << " moves objects around!" << endl;
        }
};

class Banshee : virtual public Ghost {
    public:
        Banshee(string n, int Scare) : Ghost(n, Scare) {/* Paramatrized Constructor that also calls Parent Constructor*/}
        void Haunt() const override {
            cout << "\t" << Name << " screams loudly!" << endl;
        }
};

class ShadowGhost : virtual public Ghost {
    public:
        ShadowGhost(string n, int Scare) : Ghost(n, Scare) {/* Paramatrized Constructor that also calls Parent Constructor*/}
        void Haunt() const override {
            cout << "\t" << Name << " whispers creepily!" << endl;
        }
};

// Hybrid Ghost class [Shadow Ghost + Poltergiest]
class ShadowPoltergeist : public ShadowGhost, public Poltergeist {
    public:
        ShadowPoltergeist(string n, int Scare) : Ghost(n, Scare), ShadowGhost(n, Scare), Poltergeist(n, Scare) {
            // Paramatrized Constructor that Calls Intermediate and Base Class Constrcutor
        }
        void Haunt() const override {
            ShadowGhost::Haunt();
            Poltergeist::Haunt();
        }
};

// Operator+ implementation combining two ghost to make one ghost
Ghost* Ghost::operator+(const Ghost& other) const {
    string NewName = Name + " + " + other.Name;         // Sort of COncatinating the two ghost name [hybrid naming].
    int NewScareLevel = ScareLevel + other.ScareLevel;  // Adding the scare level of both ghost
    return new ShadowPoltergeist(NewName, NewScareLevel);
}

// HauntedHouse class
class HauntedHouse {
    private:
        string Name;
        Ghost** ghosts;
        int NumGhosts;
        int Capacity;
    public:
        HauntedHouse(string n) : Name(n), ghosts(nullptr), NumGhosts(0), Capacity(0) {}

        ~HauntedHouse() {
            for (int i = 0; i < NumGhosts; ++i) {
                delete ghosts[i];
            }
            delete[] ghosts;
        }
        void AddGhost(Ghost* ghost) {
            if (NumGhosts >= Capacity) {
                int NewCapacity = (Capacity == 0) ? 2 : Capacity * 2;   // Checks if Ghost qty is 0 than gives 2 spaces for ghost to enter, if ghost qty is not equal to 0 than increase the current ghost qty * 2 so that more ghost can enter.
                Ghost** NewGhosts = new Ghost*[NewCapacity];
                
                for (int i = 0; i < NumGhosts; i++) {
                    NewGhosts[i] = ghosts[i];
                }
                delete[] ghosts;
                ghosts = NewGhosts;
                Capacity = NewCapacity;
            }
            ghosts[NumGhosts++] = ghost;
        }
        void SimulateVisit(const Visitor* visitors, int NumVisitors) const;
};

// Visitor Reaction implementation
void Visitor::ReactToGhost(const Ghost& ghost) const {
    int ScareLevel = ghost.GetScareLevel();
    if (ScareLevel < BraveryLevel - 2) {
        cout << Name << " laughs at " << ghost.GetName() << "!" << endl;
    } else if (ScareLevel > BraveryLevel + 2) {
        cout << Name << " screams and runs away from " << ghost.GetName() << "!" << endl;
    } else {
        cout << Name << " Gets a shaky voice from " << ghost.GetName() << "!" << endl;
    }
}

// Simulation implementation
void HauntedHouse::SimulateVisit(const Visitor* visitors, int NumVisitors) const {
    cout << endl << "~-------------------------------------------~";
    cout << endl << "Welcome to " << Name << "!" << endl;
    cout << "~-------------------------------------------~";
    
    for (int i = 0; i < NumVisitors; ++i) {
        cout << endl << "Visitor: " << visitors[i].GetName() << " (Bravery: " << visitors[i].GetBraveryLevel() << ")" << endl;
        for (int j = 0; j < NumGhosts; ++j) {
            // Print the ghost's name and scare level before haunting
            cout << "Ghost Encountered: " << endl << "\t- " << *ghosts[j] << endl;    // Using the operator overloading << code here.
            visitors[i].ReactToGhost(*ghosts[j]);
        }
    }
}

// Global visit function that takes visitor and house as parameters
void Visit(const Visitor* visitors, int NumVisitors, const HauntedHouse& house) {
    house.SimulateVisit(visitors, NumVisitors); // Call for the Simulation function
}

int main() {
    srand(time(0)); // For random scare level generation
    cout << "Sana Munir Alam 24K-0573" <<endl;
    // Create Haunted houses
    HauntedHouse house1("Spooky Mansion");
    house1.AddGhost(new Poltergeist("Polty", rand() % 10 + 1)); // I am using random scare level for the ghosts
    house1.AddGhost(new Banshee("Banshee", rand() % 10 + 1)); // I am using random scare level for the ghosts

    HauntedHouse house2("Creepy Cabin");
    house2.AddGhost(new ShadowGhost("ShadyGhost", rand() % 10 + 1)); // I am using random scare level for the ghosts
    house2.AddGhost(new ShadowPoltergeist("ShadowPolty", rand() % 10 + 1)); // I am using random scare level for the ghosts

    // Creating visitors
    const int NumVisitors = 3;
    Visitor visitors[NumVisitors] = { /* Character Name and their bravery level*/
        Visitor("Kevin", 3),
        Visitor("Christopher", 6),
        Visitor("Richard", 9)
    };

    // Simulate visits
    Visit(visitors, NumVisitors, house1); // Visiting Spooky Mansion
    Visit(visitors, NumVisitors, house2); // Visiting  Creepy Cabin
    return 0;
}
