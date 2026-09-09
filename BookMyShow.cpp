
/*--------------------BOOK MY SHOW-----------------------*/

#include<bits/stdc++.h>
#include<mutex>
using namespace std;

enum class SeatStatus{
    BOOKED,RESERVED,EMPTY,
};

class Seat{
    private:
        SeatStatus status;
        int id;
    public:
        Seat(int id) : id(id),status(SeatStatus::EMPTY){}
        SeatStatus getStatus(){return status;}
        void setStatus(SeatStatus s){status = s;}
};


class Screen{
    private:
        int id;
        vector<Seat*> seats;
    public:
        Screen(int id,int numSeats) : id(id){
            for(int i=0;i<numSeats;i++){
                seats.push_back(new Seat(i));
            }
        }
        const vector<Seat*> getSeats(){return seats;}
        int getScreenId(){return id;}
        bool check(vector<Seat*> s){
            for(auto it : s){
                if(it->getStatus() != SeatStatus::EMPTY)return false;
            }
            return true;
        }
        void changeSeatStatus(vector<Seat*> seats,SeatStatus status){
            for(auto s : seats){
                s->setStatus(status);
            }
        }
};

class Movie{
    private:
        string name;
    public:
        Movie(string n) : name(n){}
        string getName(){return name;}
};

class Show{
    private:
        Movie* movie;
        Screen* screen;
        double ticketFare;
        string timeSlot;
    public:
        Show(Movie* m,Screen* s,double cost,string t) : movie(m),screen(s),ticketFare(cost),timeSlot(t){}
        double getTicketFare(){return ticketFare;}
        Screen* getScreen(){return screen;}
        Movie* getMovie(){return movie;}
};  

class PaymentStrategy{
    public:
        virtual ~PaymentStrategy() = default;
        virtual bool pay(double amt) = 0;
};

class UPIPaymentStrategy : public PaymentStrategy{
    public:
        bool pay(double amt)override {
            cout<<"Paid "<<amt<<" using UPI Payment.\n";
            return true;
        }
};

class CreditCardPaymentStrategy : public PaymentStrategy{
    public:
        bool pay(double amt)override {
            cout<<"Paid "<<amt<<" using Cedit Card Payment.\n";
            return true;
        }
};

class NotificationStrategy{
    public:
        virtual ~NotificationStrategy() = default;
        virtual void notify() = 0;
};

class SMSNotificationStrategy : public NotificationStrategy{
    public:
        void notify()override {
            cout<<"SMS Notification.\n";
        }
};

class EmailNotificationStrategy : public NotificationStrategy{
    public:
        void notify()override {
            cout<<"Email Notification.\n";
        }
};

class Client{
    private:
        vector<Seat*> reqSeats;
        Show* show;
        PaymentStrategy* ps;
        NotificationStrategy* ns;
        string name,email;
    public:
        Client(vector<Seat*> rs,Show* s,PaymentStrategy* p,string n,string e,NotificationStrategy* noti) : name(n),email(e),ps(p),show(s),reqSeats(rs),ns(noti){}
        vector<Seat*> getReqSeats(){return reqSeats;}
        Show* getShow(){return show;}
        string getName(){return name;}
        string getEmail(){return email;}
        PaymentStrategy* getPaymentStrategy(){return ps;}
        NotificationStrategy* getNotificationStrategy(){return ns;}
};

enum class PaymentStrategyType{
    UPI,CREDIT_CARD,
};

enum class NotificationStrategyType{
    SMS,EMAIL,
};

class PaymentStrategyFactory{
    public:
        PaymentStrategy* create(PaymentStrategyType type){
            if(type == PaymentStrategyType::UPI)return new UPIPaymentStrategy();
            if(type == PaymentStrategyType::CREDIT_CARD)return new CreditCardPaymentStrategy();
            return nullptr;
        }
};

class NotificationStrategyFactory{
    public:
        NotificationStrategy* create(NotificationStrategyType type){
            if(type == NotificationStrategyType::SMS)return new SMSNotificationStrategy();
            if(type == NotificationStrategyType::EMAIL)return new EmailNotificationStrategy();
            return nullptr;
        }
};

class BookingService{
    private:
        static std::mutex mtx;
        static BookingService* instance;
        bool checkSeatStatus(Client* client){
            std::lock_guard<std::mutex> lock(mtx);
            vector<Seat*> reqSeats = client->getReqSeats();
            Show* show = client->getShow();
            if(!show->getScreen()->check(reqSeats)){
                return false;
            }
            show->getScreen()->changeSeatStatus(reqSeats,SeatStatus::RESERVED);
            return true;
        }
    public:
        static BookingService* getInstance(){
            if(instance == nullptr){
                lock_guard<mutex> lock(mtx);
                if(instance == nullptr){
                    instance = new BookingService();
                }
            }
            return instance;
        }
        void bookTicket(Client* client){
            if(!checkSeatStatus(client)){
                cout<<"Please select some other empty seats.\n";
                return;
            }
            double amount = client->getShow()->getTicketFare()*client->getReqSeats().size();
            if(client->getPaymentStrategy()->pay(amount)){
                cout<<"Successfully Booked Seats for the movie: "<<client->getShow()->getMovie()->getName()<<"\n";
                client->getShow()->getScreen()->changeSeatStatus(client->getReqSeats(),SeatStatus::BOOKED);
                client->getNotificationStrategy()->notify();
            }else{
                cout<<"Payment Failure.\n";
                client->getShow()->getScreen()->changeSeatStatus(client->getReqSeats(),SeatStatus::EMPTY);
            }
            return;
        }
};

BookingService* BookingService::instance = nullptr;
mutex BookingService::mtx;

int main(){

    PaymentStrategyFactory* pay_fac = new PaymentStrategyFactory();
    NotificationStrategyFactory* not_fac = new NotificationStrategyFactory();

    Movie* movie1 = new Movie("Dhurandhar: The Revenge");
    Movie* movie2 = new Movie("Hanuman Ansh");
    Movie* movie3 = new Movie("SpiderMan");

    Screen* screen1 = new Screen(1,5);
    Screen* screen2 = new Screen(2,5);

    vector<Seat*> reqSeats1 = {screen1->getSeats()[0],screen1->getSeats()[1]};
    vector<Seat*> reqSeats2 = {screen2->getSeats()[0],screen2->getSeats()[1]};

    Show* show1 = new Show(movie2,screen1,155.50,"2:30pm");
    Show* show2 = new Show(movie1,screen2,200.50,"12:30pm");

    Client* client1 = new Client(reqSeats1,show1,pay_fac->create(PaymentStrategyType::UPI),"Milind","mp@gmail.com",not_fac->create(NotificationStrategyType::EMAIL));

    Client* client2 = new Client(reqSeats2,show2,pay_fac->create(PaymentStrategyType::CREDIT_CARD),"Milind","mp@gmail.com",not_fac->create(NotificationStrategyType::SMS));

    BookingService* svc = BookingService::getInstance();
    svc->bookTicket(client1);
    svc->bookTicket(client2);
