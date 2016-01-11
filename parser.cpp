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

    };

    class Sign{

    };


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
        setup();        
        test(R("E"));

        test(R("Eq"));

        test(R("T"));

        test(R("Tq"));

        test(R("F"));

        cout<<"===\n";
        auto items = new Item("S'",{R("F"),R("FIN")});
        //Closure(items);
        //cout << *items;

        delete items;
        
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
