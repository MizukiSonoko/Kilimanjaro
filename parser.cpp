#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
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

        operator string() const{
            return s;
        }
    };
    
    unorderd_map<string, vector<shared_ptr<Rule>> follow_table;
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
                    }
                }
                res.insert(res.end(), ext.begin(), ext.end());
            }
        }
        return res;
    }

    vector<shared_ptr<Rule>> follow(shared_ptr<Rule> Rs){
        cout<<"follow\n";
        vector<shared_ptr<Rule>> res;
        if(Rs == E){
            res.push_back(FIN);
            return res;
        }
        for(auto rule : rules){
            for(auto r : rule->rules()){
                for(size_t i = 1; i < r.size(); i++){
                    cout<<string(*rule)<<endl;
                    if(string(*r[i]) == string(*Rs)){
                        cout<<"~~~~~\n";
                        if(i + 1 < r.size()){
                            auto ext = first(r[i+1]);
                            if(find(ext.begin(), ext.end(), Epsilon) != ext.end()){
                                ext = follow(rule);
                                res.insert(res.end(), ext.begin(), ext.end());
                            }else{
                                ext.erase(remove(ext.begin(), ext.end(), Epsilon), ext.end());
                                res.insert(res.end(), ext.begin(), ext.end());
                            }
                            for(auto e : ext){
                                cout<<string(*e) <<endl;
                            }
                            cout<<"=====\n";
                        }else{
                            auto ext = follow(rule);
                            res.insert(res.end(), ext.begin(), ext.end());
                        }
                    }
                }
            }
        }
        return res;
    }


    shared_ptr<Rule> mR(string v){
        return shared_ptr<Rule>(new Rule(v));
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

        test(E);

        test(Eq);

        test(T);

        test(Tq);

        test(F);
    }
}

int main(){
    parser::parser();
    return 0;
}