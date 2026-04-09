#include <iostream>
#include <string>
using namespace std;

class Ball {
    public:
        int X;
        int Y;
        Ball() : X(0), Y(0) {
            // Default Constructor
        }
        int GetX() { 
            return X; 
        }
        int GetY() { 
            return Y; 
        }
        void move(int dx, int dy) {
            X += dx;
            Y += dy;
        }
        pair<int, int> GetPosition() {
            return make_pair(X, Y);
        }
};

class Goal {
    private:
        int X;
        int Y;
    public:
        Goal(int x, int y) : X(x), Y(y) {
            // Paramtrized Constructor Using Inline Initilization
        }
        bool isGoalReached(int BallX, int BallY) {
            return (BallX == X && BallY == Y);
        }
};

class Robot {
private:
    string Name;
    int Hits;
public:
    Robot(string name) : Name(name), Hits(0) {
        // Paramtrized Constructor Using Inline Initilization
    }
    
    void HitBall(int &BallX, int &BallY, const string &Direction) {
        if (Direction == "up" || Direction == "Up") {
            BallY++;
        }else if (Direction == "down" || Direction == "Down") {
            BallY--;
        }else if (Direction == "left" || Direction == "Left") {
            BallX--;
        }else if (Direction == "right" || Direction == "Right") {
            BallX++;
        }
        Hits++;
    }
    
    int GetHits() { 
        return Hits; 
    }
    string GetName() { 
        return Name; 
    }
};

class Team {
private:
    string TeamName;
    Robot* RobotPlayer;
public:
    Team(string name, Robot* robot) : TeamName(name), RobotPlayer(robot) {
        // Paramtrized Constructor Using Inline Initilization
    }
    string GetTeamName() { 
        return TeamName; 
    }
    Robot* GetRobot() { 
        return RobotPlayer; 
    }
};

class Game {
    private:
        Team* TeamOne;
        Team* TeamTwo;
        Ball GameBall;
        Goal GameGoal;
    public:
        Game(Team* t1, Team* t2) : TeamOne(t1), TeamTwo(t2), GameGoal(3, 3) {}
        
        void startGame() {
            cout << endl << "------ Starting Game ------" << endl;
            play(TeamOne);
            play(TeamTwo);
            declareWinner();
        }
        
        void play(Team* team) {
            cout <<  endl << "------ " << team->GetTeamName() << "'s Turn ------" << endl;
            GameBall.X = 0;
            GameBall.Y = 0;
            int startHits = team->GetRobot()->GetHits();
            
            while (!GameGoal.isGoalReached(GameBall.X, GameBall.Y)) {
                char Input;
                bool valid = false;
                
                // Get valid WASD input
                do {
                    cout << "Enter direction (W/A/S/D): ";
                    cin >> Input;
                    Input = tolower(Input);
                    
                    if (Input == 'w' || Input == 'a' || Input == 's' || Input == 'd') {
                        valid = true;
                    } else {
                        cout << "Invalid direction! Please use W/A/S/D." << endl;
                        cin.clear();
                    }
                } while (!valid);
                
                // Convert input to direction string
                string Direction;
                switch (Input) {
                    case 'w': Direction = "up"; break;
                    case 'a': Direction = "left"; break;
                    case 's': Direction = "down"; break;
                    case 'd': Direction = "right"; break;
                }
                
                team->GetRobot()->HitBall(GameBall.X, GameBall.Y, Direction);
                cout << "Ball moved to (" << GameBall.X << ", " << GameBall.Y << ")" << endl;
            }
            cout << "Reached goal in " << (team->GetRobot()->GetHits() - startHits) << " Hits!" << endl;
        }
        
        void declareWinner() {
            int t1Hits = TeamOne->GetRobot()->GetHits();
            int t2Hits = TeamTwo->GetRobot()->GetHits();
            
            cout << endl <<"------ Final Results ------" << endl
                << TeamOne->GetTeamName() << ": " << t1Hits << " Hits" << endl
                << TeamTwo->GetTeamName() << ": " << t2Hits << " Hits" << endl;
                
            if (t1Hits < t2Hits) {
                cout << TeamOne->GetTeamName() << " Wins!" << endl;
            }else if (t2Hits < t1Hits) {
                cout << TeamTwo->GetTeamName() << " Wins!" << endl;
            }else {
                cout << "It's a tie!" << endl;
            }
        }
};

int main() {
    cout << "------ Creating Robots ------" << endl;
    Robot* Robot1 = new Robot("StrikerBot");
    Robot* Robot2 = new Robot("GoalMaster");
    cout << "Robots created: " << Robot1->GetName() << " & " << Robot2->GetName() << endl;
    
    cout << endl << "------ Creating Teams ------" << endl;
    Team TeamA("Team SLytherin", Robot1);
    Team TeamB("Team Gryffindor", Robot2);
    cout << "Teams created: " << TeamA.GetTeamName() << " & " << TeamB.GetTeamName() << endl;
    
    cout << endl << "------ Initializing Game ------" << endl;
    Game FootballGame(&TeamA, &TeamB);
    FootballGame.startGame();
    
    delete Robot1;
    delete Robot2;
    return 0;
}
