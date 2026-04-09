#include <iostream>
using namespace std;

class BankAccount{
    private:
        string AccountNumber;
        double Balance;
    public:
        // Constructor to initialise Account Number
        BankAccount(string AccNumber) : AccountNumber(AccNumber), Balance(0.0) {}

        // Method (Deposit)
        void Deposit(double Amount){
            Balance = Balance + Amount;
        }
        // Method (Withdraw)
        void Withdraw(double Amount){
            if (Amount <= Balance){
                Balance = Balance - Amount;
                cout << "Withdrew: $" << Amount << endl;
            }else {
                cout << "Insufficient Balance!!" << endl;
            }
        }
        // Method (Check Balance)
        void CheckBalance() const{
            cout << "Current Balnce: $" << Balance << endl;
        }
};
int main(){
    BankAccount Account("123456");

    Account.Deposit(575.0);
    Account.Withdraw(135.0);
    Account.CheckBalance();
    
    return 0;
}