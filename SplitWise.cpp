#include<bits/stdc++.h>
#include<mutex>
using namespace std;

class User{
    private:
        string name;
    public:
        User(string n) : name(n){}
        string getName(){return name;}
};

class Group{
    private:
        vector<User*> users;
        string name;
    public:
        Group(string n) : name(n){}
        string getName(){return name;}
        void addUserToGroup(User* u){
            users.push_back(u);
        }
        const vector<User*> getGroupMembers(){return users;}
};

class Split{
    private:
        User* user;
        double amt;
    public:
        Split(User* u,double a) : user(u),amt(a){}
        User* getUser(){return user;}
        double getAmount(){return amt;}
};

class Expense{
    private:
        double amount;
        Group* grp;
        vector<Split*> splits;
        User* paidBy;
        int id;
    public:
        Expense(int _id,double a,Group* g,User* u,vector<Split*> s) : paidBy(u),grp(g),amount(a),splits(s),id(_id){}
        int getId(){return id;}
        double getAmount(){return amount;}
        User* getUserPaidBy(){return paidBy;}
        const vector<Split*> getSplits(){return splits;}
        Group* getGrp(){return grp;}
};

enum class SplitType{
    EQUAL,
    PERCENTAGE,
    EXACT,
};

class SplitStrategy{
    public:
        virtual ~SplitStrategy() = default;
        virtual vector<Split*> splitAmount(vector<User*> users,double amount) = 0;
};

class EqualSplitStrategy : public SplitStrategy{
    public:
        vector<Split*> splitAmount(vector<User*> users,double amount)override {
            int n = users.size();
            double splitAmt = floor((amount / n) * 100) / 100.0;
            vector<Split*> splits;
            for(auto &it : users){
                splits.push_back(new Split(it,splitAmt));
            }
            return splits;
        }
};


class SplitStrategyFactory{
    public:
        SplitStrategy* create(SplitType type){
            if(type == SplitType::EQUAL)return new EqualSplitStrategy();
            // if(type == SplitType::PERCENTAGE)return new PercentageSplitStrategy();
            // if(type == SplitType::EXACT)return new ExactSplitStrategy();
            return nullptr;
        }
};

class SplitwiseService{
    private:
        static SplitwiseService* instance;
        map<int, User*> users;
        map<int, Group*> groups;
        map<int, Expense*> expenses;
        map<pair<User*, User*>, double> balances;

        int nextUserId = 1;
        int nextGroupId = 1;
        int nextExpenseId = 1;
        static mutex mtx;
    public:
        static SplitwiseService* getInstance(){
            if(instance == nullptr){
                lock_guard<mutex> lock(mtx);
                if(instance == nullptr){
                    instance = new SplitwiseService();
                }
            }
            return instance;
        }
        User* createUser(string name){
            User* user = new User(name);
            users[nextUserId++] = user;
            return user;
        }
        Group* createGroup(string name){
            Group* group = new Group(name);
            groups[nextGroupId++] = group;
            return group;
        }
        void addUserToGroup(Group* grp,User* user){
            grp->addUserToGroup(user);
        }

        void addBalance(User* debtor, User* creditor, double amt){
            if(debtor == creditor) return;
            if(balances.count({creditor, debtor})){
                balances[{creditor, debtor}] -= amt;
                if(balances[{creditor, debtor}] == 0){
                    balances.erase({creditor, debtor});
                } else if(balances[{creditor, debtor}] < 0){
                    balances[{debtor, creditor}] = -balances[{creditor, debtor}];
                    balances.erase({creditor, debtor});
                }
            } else {
                balances[{debtor, creditor}] += amt;
            }
        }

        void addExpense(double amount,User* paidBy,Group* grp,SplitStrategy* ss){
            vector<Split*> splits = ss->splitAmount(grp->getGroupMembers(),amount);
            Expense* exp = new Expense(nextExpenseId++,amount,grp,paidBy,splits);
            expenses[exp->getId()] = exp;
            for(auto &it : splits){
                addBalance(it->getUser(),paidBy,it->getAmount());
            }
        }

        void settle(User* payer, User* payee, double amt){
            addBalance(payer, payee, -amt); 
            cout << payer->getName() << " settled " << amt << " to " << payee->getName() << "\n";
        }

        void print(){
            for(auto it : balances){
                if(it.second > 0){
                    cout<<it.first.first->getName()<<" owes "<<it.second<<" amount to "<<it.first.second->getName()<<"\n";
                }
            }
        }
};

SplitwiseService* SplitwiseService::instance = nullptr;
mutex SplitwiseService::mtx;

int main(){
    SplitwiseService* svc = SplitwiseService::getInstance();

    User* user1 = svc->createUser("Milind");
    User* user2 = svc->createUser("Vaibhav");
    User* user3 = svc->createUser("Abhishek");
    Group* grp = svc->createGroup("AbVaMi");
    
    svc->addUserToGroup(grp,user1);
    svc->addUserToGroup(grp,user2);
    svc->addUserToGroup(grp,user3);

    SplitStrategyFactory* fact = new SplitStrategyFactory();

    svc->addExpense(300.0,user1,grp,fact->create(SplitType::EQUAL));
    svc->print();
    svc->settle(user2,user1,100.0);
    svc->print();
}
