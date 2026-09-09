#include<bits/stdc++.h>
#include<mutex>
using namespace std;


enum class OrderStatus{
    OUT_FOR_DELIVERY,
    DELIVERED,
    IN_MAKING,
    WAIT_FOR_DELIVERY_AGENT_TO_BE_ASSIGNED,
};

class Item{
    private:
        string name;
        double price;
        bool isAvailable;
    public:
        Item(string n,double p,bool isa) : name(n),price(p),isAvailable(isa){}
        string getName(){return name;}
        double getPrice(){return price;}
        bool getAvailability(){return isAvailable;}
        void setAvailability(bool f){isAvailable = f;}
};

class Cart{
    private:
        map<Item*,int> cart;
        double totalPrice;
        mutex mtx;
    public:
        Cart() : totalPrice(0){}
        //make it thread safe
        void addToCart(Item* item,int quantity){
            lock_guard<mutex> lock(mtx);
            cout<<"Added "<<item->getName()<<" to the cart\t "<<"Quantity: "<<quantity<<"\n";
            cart[item] += quantity;
            totalPrice += quantity*(item->getPrice());
        }
        void removeFromCart(Item* item,int quantity){
            lock_guard<mutex> lock(mtx);
            cart[item] -= quantity;
            totalPrice -= quantity*(item->getPrice());
        }
        double getPrice(){return totalPrice;}
        const map<Item*,int> getCart(){return cart;}
};

class Client{
    private:
        string name,loc;
        Cart* cart;
    public:
        Client(string name,string loc,Cart* c) : name(name),loc(loc),cart(c) {}
        string getName(){return name;}
        string getLoc(){return loc;}
        Cart* getCart(){return cart;}
};

class DeliveryAgent{
    private:
        string name,loc;
        bool isFree;
    public:
        DeliveryAgent(string name) : name(name),isFree(true){}
        bool isDeliveryAgentFree(){return isFree;}
        void acceptOrder(){isFree = false;}
        void deliverOrder(){isFree = true;}
        string getName(){return name;}
};


class Order{
    private:
        OrderStatus status;
        Client* user;
        DeliveryAgent* deliveryAgent;
    public:
        Order(Client* user) : user(user),status(OrderStatus::IN_MAKING){}
        OrderStatus getOrderStatus(){return status;}
        void setOrderStatus(OrderStatus s){status = s;}
        void setDeliveryAgent(DeliveryAgent* da){deliveryAgent = da;}
};

class Restaurant{
    private:
        vector<Item*> menu;
        vector<Order*> orders;
        string name,loc;
    public:
        Restaurant(string name,string loc) : name(name),loc(loc){}
        void addItemToMenu(Item* item){
            menu.push_back(item);
        }
        void acceptOrder(Order* o){
            cout<<"\n========================================\n";
            cout<<"Order Accepted. Shortly preparing your Order.\n";
            cout<<"\n========================================\n";
            orders.push_back(o);
        }
        string getName(){return name;}
        string getLocation(){return loc;}
        void processOrders(){
            for(auto o : orders){
                if(o->getOrderStatus() != OrderStatus::IN_MAKING) continue;   
                o->setOrderStatus(OrderStatus::WAIT_FOR_DELIVERY_AGENT_TO_BE_ASSIGNED);
                cout<<"\n========================================\n";
                cout << "Your Order has been prepared.\n";
                cout << "Waiting for delivery agent to get assigned.\n";
                cout<<"\n========================================\n";
            }
        }
};



class FoodDeliveryService{
    private:
        map<int,Restaurant*> rest;
        map<int,DeliveryAgent*> deliveryAgent;
        // map<int,Client*> users;
        int rid=0,did=0;
        static FoodDeliveryService* instance;
        static mutex mtx;
    public:
        static FoodDeliveryService* getInstance(){
            if(instance == nullptr){
                std::lock_guard<std::mutex> lock(mtx);
                if(instance == nullptr){
                    instance = new FoodDeliveryService();
                }
            }
            return instance;
        }
        void addRestaurant(Restaurant* r){
            rest[++rid] = r;
        }
        void addDeliveryAgent(DeliveryAgent* d){
            deliveryAgent[++did] = d;
        }
        Restaurant* findRestaurantByLoc(string loc){
            for(auto it : rest){
                if(it.second->getLocation() == loc)return it.second;
            }
            return nullptr;
        }
        DeliveryAgent* findFreeDeliveryAgent(){
            for(auto it : deliveryAgent){
                if(it.second->isDeliveryAgentFree())return it.second;
            }
            return nullptr;
        }
        bool checkOutService(Client* user,Restaurant* rest){
            map<Item*,int> c = user->getCart()->getCart();
            for(auto it : c){
                if(!it.first->getAvailability()){
                    cout<<"Please remove "<<it.first->getName()<< " from the cart.\n";
                    return false;
                }
            }
            DeliveryAgent* da = findFreeDeliveryAgent();
            if(da == nullptr){
                cout << "No delivery agents available right now. Please try again later.\n";
                return false;
            }

            Order* order = new Order(user);

            //restaurant accepting order
            rest->acceptOrder(order);
            rest->processOrders();

            //agent accepting order
            da->acceptOrder();
            order->setDeliveryAgent(da);
            order->setOrderStatus(OrderStatus::OUT_FOR_DELIVERY);

            cout<<"\n========================================\n";
            cout<<"Assigning Delivery Agent.\n";
            cout<<"Delivery Agent: "<<da->getName()<<" is out for delivery.\n";
            cout<<"\n=============================================\n";
            return true;
        }
};

class PaymentStrategy{
    public:
        virtual ~PaymentStrategy() = default;
        virtual void pay(double amount) = 0;
};

class UPIPaymentStrategy : public PaymentStrategy{
    public:
        void pay(double amount)override {
            cout<<"Paid "<<amount<<" amount using UPI Payment Strategy.\n";
        }
};

FoodDeliveryService* FoodDeliveryService::instance = nullptr;
mutex FoodDeliveryService::mtx;

int main(){
    FoodDeliveryService* svc = FoodDeliveryService::getInstance();

    DeliveryAgent* da = new DeliveryAgent("Mukesh");
    svc->addDeliveryAgent(da);

    //restaurant creation
    Restaurant* rest1 = new Restaurant("Cloudy","IIIT Allahabad");
    svc->addRestaurant(rest1);
    Restaurant* rest2 = new Restaurant("Kings","Jhalwa");
    svc->addRestaurant(rest2);
    Restaurant* rest3 = new Restaurant("Aryan's","Civil Lines");
    svc->addRestaurant(rest3);

    //adding items to restaurant
    Item* item1 = new Item("Pizza",150.0,true);
    Item* item2 = new Item("Chaap",199.0,true);
    Item* item3 = new Item("Veg Roll",99.0,false);
    Item* item4 = new Item("Pasta",299.0,false);
    Item* item5 = new Item("Paneer Tikka",220.0,true);

    rest1->addItemToMenu(item1);
    rest1->addItemToMenu(item2);
    rest1->addItemToMenu(item3);
    rest1->addItemToMenu(item4);
    rest1->addItemToMenu(item5);

    //client1 happy flow simulation

    //client creation
    Client* client1 = new Client("Milind","IIIT Allahabad",new Cart());
    Client* client2 = new Client("Vaibhav","Civil Lines",new Cart());
    Client* client3 = new Client("Abhishek","Jhalwa",new Cart());


    Restaurant* rest = svc->findRestaurantByLoc(client1->getLoc());
    if(rest == nullptr){
        cout<<"No nearby servicable restaurant.\n";
        return 0;
    }

    client1->getCart()->addToCart(item5,4);
    client1->getCart()->addToCart(item1,2);

    if(svc->checkOutService(client1,rest)){
        PaymentStrategy* ps = new UPIPaymentStrategy();
        ps->pay(client1->getCart()->getPrice());
    }

}

