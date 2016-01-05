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


    class Rule{
        string s;
        vector<vector<shared_ptr<Rule>>> f;

        bool isTerm_;
        bool epsilon;
      public:

        Rule(string s): 
            s(s),isTerm_(true),epsilon(false){};
        Rule(string s,bool isTerm): 
            s(s),isTerm_(isTerm),epsilon(false){};
        Rule(vector< vector<shared_ptr<Rule>>> f):
            f(f),isTerm_(false),epsilon(false){};
        Rule():isTerm_(false),epsilon(true){};
        Rule(string name,initializer_list<shared_ptr<Rule>> lr):
            s(name),isTerm_(false){
            vector<shared_ptr<Rule>> ext(lr.begin(), lr.end());
            f.push_back(ext);
        }

        void add(vector<shared_ptr<Rule>> a){
            epsilon = false; 
            f.push_back(a);
        }

        bool isTerm() const{
            return isTerm_;
        }
        vector<vector<shared_ptr<Rule>>> rules() const{
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

        friend ostream& operator<<(ostream &out, const Rule &r){
            if(r.isTerm()){
                out << string(r);
            }else{ 
                for(auto rule : r.rules()){
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
    
    unordered_map<string, vector<shared_ptr<Rule>>> follow_table;
    // Rules
    vector<shared_ptr<Rule>> rules;
    shared_ptr<Rule> Epsilon(new Rule("Epsilon"));
    shared_ptr<Rule> FIN(new Rule("FIN"));

    shared_ptr<Rule> E(new Rule("E", false));
    shared_ptr<Rule> Eq(new Rule("Eq", false));
    shared_ptr<Rule> T(new Rule("T", false));
    shared_ptr<Rule> Tq(new Rule("Tq",false));
    shared_ptr<Rule> F(new Rule("F",false));

    vector<shared_ptr<Rule>> first(shared_ptr<Rule> Rs){
        if(Rs->isTerm()){
            return {Rs};
        }
        vector<shared_ptr<Rule>> res; 
        for(auto& r : Rs->rules()){
            if(!r.empty()){
                auto ext = first(r[0]);
                if(find(ext.begin(), ext.end(), Epsilon) != ext.end()){
                    ext.erase(remove(ext.begin(), ext.end(), Epsilon), ext.end());
                    res.insert(res.end(), ext.begin(), ext.end());
                    if(r.size() >= 2){
                        auto nxt = first(r[1]);
                        res.insert(res.end(), nxt.begin(), nxt.end());
                    }else{
                        res.push_back(Epsilon);
                    }
                }
                res.insert(res.end(), ext.begin(), ext.end());
            }
        }
        return res;
    }

    vector<shared_ptr<Rule>> first(initializer_list<shared_ptr<Rule>> l){
        if(l.size() == 0)
            return {Epsilon};

        vector<shared_ptr<Rule>> res;
        
        auto it = l.begin();
        if(*it == Epsilon) return {Epsilon};
        if((*it)->isTerm()) return {*it};

        auto ext = first(*it); 
        if(find(ext.begin(), ext.end(), Epsilon) != ext.end()){
            ext.erase(remove(ext.begin(), ext.end(), Epsilon), ext.end());
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

    vector<shared_ptr<Rule>> follow(shared_ptr<Rule> Rs){
        vector<shared_ptr<Rule>> res;
        if(Rs == E){
            res.push_back(FIN);
        }

        for(auto rule : rules){

            if(rule == Rs) continue;

            for(auto r : rule->rules()){
                for(size_t i = 1; i < r.size(); i++){
                    if(string(*r[i]) == string(*Rs)){
                        if(i + 1 < r.size()){                            
                            auto ext = first(r[i+1]);
                            if(find(ext.begin(), ext.end(), Epsilon) != ext.end()){
                                auto left = follow(rule);
                                res.insert(res.end(), left.begin(), left.end());
                            }
                            ext.erase(remove(ext.begin(), ext.end(), Epsilon), ext.end());
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

    void closure(shared_ptr<Rule> I){
        int size = I->size(); 
        int c = 0;
        while(c<1){
            c++;

            for(auto rule : I->rules()){
                if(rule.size() >= 2){
                    auto X = rule[0];
                    cout<<"===\n";
                    cout<< *X;
                    cout<<"===\n";

                    if(!X->isTerm()){
                        for(auto ext : X->rules()){
                            I->add(ext);
                        }
                    }
                }
            }
            if(size == I->size()) return;
            size = I->size();
        }
    }

    shared_ptr<Rule> mR(string v){
        return shared_ptr<Rule>(new Rule(v));
    }
    shared_ptr<Rule> mR(string name,initializer_list<shared_ptr<Rule>> l){
        return shared_ptr<Rule>(new Rule(name,l));
    }

    void setup(){
        E->add(
            {T, Eq}
        );
        Eq->add(
            {mR("+"), T, Eq}
        );
        Eq->add(
            {Epsilon}
        );
        T->add(
            {F, Tq}
        );
        Tq->add(
            {mR("*"), F, Tq}
        );
        Tq->add(
            {Epsilon}
        );
        F->add(
            {mR("("), E, mR(")")}
        );
        F->add(
            {mR("i")}
        );

        rules.push_back(E);
        rules.push_back(Eq);
        rules.push_back(T);
        rules.push_back(Tq);
        rules.push_back(F);

    } 
    
    void test(shared_ptr<Rule> R){
        cout << "==== "<<string(*R)<< " ===\n";        
        for(auto r: first(R)){
            cout << string(*r) << endl;
        }
        cout<<"===\n";
        for(auto r: follow(R)){
            cout << string(*r) << endl;
        }
    }

    void parser(){
        setup();
/*
        test(E);

        test(Eq);

        test(T);

        test(Tq);

        test(F);

        cout<<"===\n";
        for(auto r: first({F,Tq})){
            cout << *r << endl;
        }
        cout <<"~~~~~~~\n";
*/
        auto items = mR("S",{E,FIN});
        cout << *items;
        cout <<"~~~~~~~\n";
        closure(items);
        cout << *items;
    }
}

int main(){
    parser::parser();
    return 0;
}