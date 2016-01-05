#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>


#include <iostream>

#include <memory>

namespace parser{

    using namespace std;
    bool isFirst = true;

    class Rule_;

    using Rule = shared_ptr<Rule_>;

    class Rule_{
        string s;
        vector<vector<Rule>> f;

        bool isTerm_;
        bool epsilon;
      public:

        Rule_(string s): 
            s(s),isTerm_(true),epsilon(false){};
        Rule_(string s,bool isTerm): 
            s(s),isTerm_(isTerm),epsilon(false){};
        Rule_(vector< vector<Rule>> f):
            f(f),isTerm_(false),epsilon(false){};
        Rule_():isTerm_(false),epsilon(true){};
        Rule_(string name,initializer_list<Rule> lr):
            s(name),isTerm_(false){
            vector<Rule> ext(lr.begin(), lr.end());
            f.push_back(ext);
        }
        ~Rule_(){
            //cout << s << " is released!\n";
        }
        void add(vector<Rule> a){
            epsilon = false; 
            f.push_back(a);
        }

        bool isTerm() const{
            return isTerm_;
        }
        vector<vector<Rule>> rules() const{
            return f;
        }
        size_t size() const{
            return f.size();
        }
        operator string() const{
            return s;
        }
        string name()const{
            return s;
        }

        friend ostream& operator<<(ostream &out, const Rule_ &r){
            if(r.isTerm()){
                out << string(r);
            }else{ 
                for(auto& rule : r.rules()){
                    out << string(r) << " -> ";
                    for(auto s : rule){
                        out << string(*s) <<" "; 
                    }
                    out << "\n"; 
                }
            }
            return out;
        }
    };


    unordered_map<string, Rule> rule_table;
    // Rules
    vector<Rule> rules;

    Rule R(string v){
        if(rule_table.find(v) == rule_table.end()){
            return Rule(new Rule_(v));
        }else{
            return rule_table[v];
        }
    }
    Rule R(string name,initializer_list<Rule> l){
        return Rule(new Rule_(name,l));
    }

    void cR(string name, bool isTerm = false){
        if(rule_table.find(name) == rule_table.end()){
            rule_table.insert(make_pair(name, Rule(new Rule_(name, isTerm))));
        }
    }

    vector<Rule> first(Rule Rs){
        if(Rs->isTerm()){
            return {Rs};
        }
        vector<Rule> res; 
        for(auto& r : Rs->rules()){
            if(!r.empty()){
                auto ext = first(r[0]);
                if(find(ext.begin(), ext.end(), R("Epsilon")) != ext.end()){
                    ext.erase(remove(ext.begin(), ext.end(), R("Epsilon")), ext.end());
                    res.insert(res.end(), ext.begin(), ext.end());
                    if(r.size() >= 2){
                        auto nxt = first(r[1]);
                        res.insert(res.end(), nxt.begin(), nxt.end());
                    }else{
                        res.push_back(R("Epsilon"));
                    }
                }
                res.insert(res.end(), ext.begin(), ext.end());
            }
        }
        return res;
    }

    vector<Rule> first(initializer_list<Rule>& l){
        if(l.size() == 0)
            return {R("Epsilon")};

        vector<Rule> res;
        
        auto it = l.begin();
        if(*it == R("Epsilon")) return {R("Epsilon")};
        if((*it)->isTerm()) return {*it};

        auto ext = first(*it); 
        if(find(ext.begin(), ext.end(), R("Epsilon")) != ext.end()){
            ext.erase(remove(ext.begin(), ext.end(), R("Epsilon")), ext.end());
            res.insert(res.end(), ext.begin(), ext.end());                
            if(l.size() >= 2 ){
                it++;
                auto next = first(*it);
                res.insert(res.end(), next.begin(), next.end());
            }
            return res;
        }else{
            return ext;
        }
    }

    vector<Rule> follow(Rule& Rs){
        vector<Rule> res;
        
        if(Rs == R("E")){
            res.push_back(R("FIN"));
        }

        for(auto rit = rule_table.cbegin(); rit != rule_table.cend(); ++rit){
            auto rule = rit->second; 
            if(rule == Rs) continue;

            for(auto r : rule->rules()){
                for(size_t i = 1; i < r.size(); i++){
                    if(string(*r[i]) == string(*Rs)){
                        if(i + 1 < r.size()){                            
                            auto ext = first(r[i+1]);
                            if(find(ext.begin(), ext.end(), R("Epsilon")) != ext.end()){
                                auto left = follow(rule);
                                res.insert(res.end(), left.begin(), left.end());
                            }
                            ext.erase(remove(ext.begin(), ext.end(), R("Epsilon")), ext.end());
                            res.insert(res.end(), ext.begin(), ext.end());
                        }else{
                            auto left = follow(rule);
                            res.insert(res.end(), left.begin(), left.end());
                        }
                    }
                }
            }
        }
        return res;
    }


    class Item{
        vector<Rule> f;
        string s;
      public:
        Item(string name,initializer_list<Rule> lr):
            s(name){
            f.insert(f.end(),lr.begin(),lr.end());
            //auto ext = R(name, lr);
            //f.push_back(ext);
        }
        Item(){};
        void add(Rule r){
            f.push_back(r);
        }
        size_t size() const{
            return f.size();
        }
        vector<Rule> rules() const{
            return f;
        }
        string name() const{
            return s;
        }
        friend ostream& operator<<(ostream &out, const Item &i){
            for(auto r : i.rules()){
                if(!r->isTerm()){
                    for(auto& rule : r->rules()){
                        out << string(*r) << " -> ";
                        for(auto s : rule){
                            out << string(*s) <<" "; 
                        }
                        out << "\n"; 
                    }
                }
            }
            return out;
        }
    };

    shared_ptr<Item> Closure(shared_ptr<Item> I){
        int size = I->size();
        vector<string> alreadys;
        while(1){
            for(auto& left : I->rules()){ 
                for(auto rule : left->rules()){
                    if(rule.size() >= 2 &&
                        find(alreadys.begin(), alreadys.end(), rule[0]->name()) == alreadys.end()){
                        auto X = R(rule[0]->name());
                        alreadys.push_back(rule[0]->name());
                        if(!X->isTerm()){                        
                            I->add(X);
                        }
                    }
                }
            }
            if(size == I->size()) break;
            size = I->size();
        }
        return I;
    }

    shared_ptr<Item> Goto(shared_ptr<Item> I,Rule rule){
        shared_ptr<Item> J(new Item());
        for(auto& vR: I->rules()){
            for(auto& r : vR->rules()){
                if(find(r.begin(),r.end(),rule) != r.end()){
                    J->add(vR);
                }
            }
        }
        Closure(J);
        return J;
    }

    struct Edge{
        shared_ptr<Item> I;
        shared_ptr<Item> J;
        Rule X;
    };

    void create_dfa(){
        vector<Edge> E;
        auto T = 
            vector<shared_ptr<Item>>({
                Closure(shared_ptr<Item>(new Item("S'",{R("E"),R("FIN")})))
            });
        // T = {I,I,I}.
        auto t_size = T.size();
        auto e_size = E.size();
        int c = 0;
        while(c < 10){
            c++;
            // I = Item
            for(auto& I : T){
                // I has "S -> aXb"; 
                for(auto& X : I->rules()){
                    auto J = Goto( I, X);
                    cout<<"~~~~~~\n";
                    cout << "name:" << J->name() << endl;
                    cout << *J;
                    cout<<"======\n";
                    //T->add(J);      
                    Edge edge;
                    edge.I = I;
                    edge.J = J;
                    edge.X = X;
                    E.push_back(edge);     
                }          
            }
            if(t_size == T.size()) break;
            t_size = T.size();

            if(e_size == E.size()) break;
            e_size = E.size();
        }

        cout << "======== E =====\n";
        for(auto e : E){
            cout<< e.I->name() <<" -(" << e.X->name() <<")-> "<< e.J->name() << endl;
        }


    }
    void setup(){

        string r1 = "E";
        string r2 = "Eq";
        string r3 = "T";
        string r4 = "Tq";
        string r5 = "F";

        cR(r1);
        cR(r2);
        cR(r3);
        cR(r4);
        cR(r5);

        cR("FIN",true);
        cR("Epsilon",true);

        R(r1)->add(
            {R(r3), R(r2)}
        );

        R(r2)->add(
            {R("+"), R(r3), R(r2)}
        );
        R(r2)->add(
            {R("Epsilon")}
        );
        R(r3)->add(
            {R(r5), R(r4)}
        );
        R(r4)->add(
            {R("*"), R(r5), R(r4)}
        );
        R(r4)->add(
            {R("Epsilon")}
        );
        R(r5)->add(
            {R("("), R(r1), R(")")}
        );
        R(r5)->add(
            {R("i")}
        );
    } 
    
    void test(Rule R){
        cout << "==== "<<string(*R)<< " ===\n";        
        for(auto& r: first(R)){
            cout << string(*r) << endl;
        }
        cout<<"===\n";
        for(auto& r: follow(R)){
            cout << string(*r) << endl;
        }
    }

    void parser(){
        setup();        /*
        test(R("E"));

        test(R("Eq"));

        test(R("T"));

        test(R("Tq"));

        test(R("F"));

        cout<<"===\n";
        auto items = new Item("S'",{R("F"),R("FIN")});
        Closure(items);
        cout << *items;

        delete items;
        */
        create_dfa();
        for(auto rit = rule_table.begin(); rit != rule_table.end(); ++rit){
            if(rit->second)
                rit->second.reset();
        }
    }
}


int main(){
    parser::parser();
    return 0;
}