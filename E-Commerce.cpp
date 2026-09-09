#include<bits/stdc++.h>
#include<mutex>
using namespace std;

class Item{
    private:
        string name;
        double price;
    public:
        Item(string n,double p) : name(n),price(p){}
        string getName(){return name;}
        double getPrice(){return price;}
};


class Inventory{
    private:
        static Inventory* instance;
        map<Item*,int> ItemQuantity; 
        static std::mutex mtx;
        Inventory(){ItemQuantity.clear();}
    public:
        static Inventory* getInstance(){
            if(instance == nullptr){
                lock_guard<mutex> lock(mtx);
                if(instance == nullptr){
                    instance = new Inventory();
                }
            }
            return instance;
        }
        void addItemToInventory(Item* item,int quantity){
            std::lock_guard<std::mutex> lock(mtx);
            ItemQuantity[item] += quantity;
        }
        bool removeItemFromInventory(Item* item,int quantity){
            std::lock_guard<std::mutex> lock(mtx);
            if(ItemQuantity[item] < quantity){
                return false;
            }
            ItemQuantity[item] -= quantity;
            return true;
        }
};

Inventory* Inventory::instance = nullptr;
mutex Inventory::mtx;

class Cart{
    private:
        map<Item*,int> selectedQuantity;
        Inventory* inventory;
        double total;
        std::mutex mtx;
    public:
        Cart(Inventory* i){
            selectedQuantity.clear();
            inventory = i;
            total = 0;
        }
        bool addToCart(Item* item,int quantity){
            if(!inventory->removeItemFromInventory(item,quantity)){
                cout << "Item " << item->getName() << " Out of Stock.\n";
                return false;
            }
            lock_guard<mutex> lock(mtx);
            selectedQuantity[item] += quantity;
            total += quantity*(item->getPrice());
            cout << "Item " << item->getName() << " Added to cart successfully.\n";
            return true;
        }
        bool removeFromCart(Item* item,int quantity){
            lock_guard<mutex> lock(mtx);
            if(selectedQuantity.find(item) == selectedQuantity.end() || selectedQuantity[item] < quantity){
                cout << "Cannot remove " << item->getName() << " — quantity exceeds what's in cart.\n";
                return false;
            }
            selectedQuantity[item] -= quantity;
            inventory->addItemToInventory(item,quantity);
            total -= quantity*(item->getPrice());
            cout << "Item " << item->getName() << " removed from cart successfully.\n";
            return true;
        }
        double getTotalCartPrice(){return total;}
};

class Client{
    public:
        string name,email;
        Client(string n,string e) : name(n),email(e){}
        void update(){
            cout<<"Hey "<<name<<"!!, you have a Notification.\n";
            return;
        }
};

class PaymentStrategy{
    public:
        virtual void pay(double amt) = 0;
};

class UPIPaymentStrategy : public PaymentStrategy{
    public:
        void pay(double amt)override{
            cout<<"Paid :"<<amt<<" using UPI Payment Strategy.\n";
        }
};

class CreditCardPaymentStrategy : public PaymentStrategy{
    public:
        void pay(double amt)override {
            cout<<"Paid :"<<amt<<" using Credit Card Payment Strategy.\n";
        }
};

class NotificationStrategy{
    public:
        virtual void notify(Client* c) = 0;
};

enum class NotificationStrategyType{
    SMS,EMAIL,
};

enum class PaymentStrategyType{
    UPI,CREDIT_CARD,
};

class SMSNotificationStrategy : public NotificationStrategy{
    public:
        void notify(Client* c)override {
            cout<<"SMS Notification.\n";
            c->update();
        }
};

class EmailNotificationStartegy : public NotificationStrategy{
    public:
        void notify(Client* c)override {
            cout<<"Email Notification.\n";
            c->update();
        }
};



class NotificationFactory{
    public:
        NotificationStrategy* create(NotificationStrategyType type){
            if(type == NotificationStrategyType::SMS)return new SMSNotificationStrategy();
            if(type == NotificationStrategyType::EMAIL)return new EmailNotificationStartegy();
            return nullptr;
        }
};

class PaymentFactory{
    public:
        PaymentStrategy* create(PaymentStrategyType type){
            if(type == PaymentStrategyType::UPI)return new UPIPaymentStrategy();
            if(type == PaymentStrategyType::CREDIT_CARD)return new CreditCardPaymentStrategy();
            return nullptr;
        }
};

class ECommerceService{
    private:
        Cart* cart;
        Client* client;
    public:
        ECommerceService(Cart* c,Client* cl) : cart(c),client(cl){}
        void processPayment(PaymentStrategy* ps,NotificationStrategy* ns){
            ps->pay(cart->getTotalCartPrice());
            cout<<"\n======================================\n";
            ns->notify(client);
            cout<<"\n======================================\n";
        }
};

int main(){

    NotificationFactory* noti_fac = new NotificationFactory();
    PaymentFactory* pay_fac = new PaymentFactory();

    Inventory* inventory = Inventory::getInstance();

    Item* watch = new Item("Watch",499.0);
    Item* headphones = new Item("Headphones",1299.0);
    Item* iphone = new Item("IPhone 17",72999.0);
    Item* kettle = new Item("Electric Kettle",999.0);

    inventory->addItemToInventory(watch,10);
    inventory->addItemToInventory(headphones,15);
    inventory->addItemToInventory(iphone,5);
    inventory->addItemToInventory(kettle,10);

    Client* client = new Client("Milind","milind@gmail.com");

    Cart* cart = new Cart(inventory);
    ECommerceService* service = new ECommerceService(cart,client);

    cart->addToCart(iphone,1);
    cart->addToCart(watch,5);
    cart->addToCart(headphones,10);
    cart->addToCart(iphone,5);

    service->processPayment(pay_fac->create(PaymentStrategyType::UPI),noti_fac->create(NotificationStrategyType::EMAIL));
}
