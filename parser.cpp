#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>


#include <iostream>

#include <memory>

namespace parser{

 	class Sign{
        std::string name_;
        bool isTerm_;
      public:
        Sign(std::string n, bool t){
            name_ = n;
            isTerm_ = t;
        }
		Sign(std::string n){
			name_ = n;
		    isTerm_ = true;	
		}
		Sign(){
			name_ ="";
			isTerm_ = true;
		}
        bool isTerm(){
            return isTerm_;
        }
        std::string name(){
            return name_;
        }
		operator std::string() const{
    		return name_;
		}
	};

    class Item{
	  public:
        Sign left;
        std::vector<Sign> rights;
        int pos;
        Item(Sign l, std::initializer_list<Sign> const & r){
            left = l;
            rights.insert(rights.end(),r.begin(), r.end());
        }
        Sign nextSign(){
            if(rights.size() > pos)
                return rights[pos];
            return Sign();
        }
        void next(){
            pos++;
        }
    };


	std::vector<Item> rules;

	std::vector<Item> getItems(Sign s){
		std::vector<Item> res;
		for(auto& i : rules){
			if(i.left.name() == s.name()){
				res.push_back(i);
			}
		}
		return res;
	}

    std::vector<Sign> first(Sign sign){
        if(sign.isTerm()){
            return {sign};
        }
        std::vector<Sign> res; 
        for(auto& i : getItems(sign)){
        	auto ext = first(i.rights[0]);
            if(find(ext.begin(), ext.end(), Sign("Epsilon",true)) != ext.end()){
            	ext.erase(remove(ext.begin(), ext.end(), Sign("Epsilon",true)), ext.end());
               	res.insert(res.end(), ext.begin(), ext.end());
                    if(i.rights.size() >= 2){
                        auto nxt = first(i.rights[1]);
                        res.insert(res.end(), nxt.begin(), nxt.end());
                    }else{
                        res.push_back(Sign("Epsilon",true));
                    }
            }
            res.insert(res.end(), ext.begin(), ext.end());
        }
        return res;
    }
/*
    std::vector<Sing> first(initializer_list<Sign>& l){
        if(l.size() == 0)
            return {Sign("Epsilon")};

        std::vector<Rule> res;
        
        auto it = l.begin();
        if(*it == Sign("Epsilon")) return {Sign("Epsilon")};
        if((*it)->isTerm()) return {*it};

        auto ext = first(*it); 
        if(find(ext.begin(), ext.end(), Sign("Epsilon")) != ext.end()){
            ext.erase(remove(ext.begin(), ext.end(), Sign("Epsilon")), ext.end());
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

    std::vector<Sing> follow(Sign& Rs){
        std::vector<Rule> res;
        
        if(Rs == Sign("E")){
            res.push_back(Sign("FIN"));
        }

        for(auto rit = rule_table.cbegin(); rit != rule_table.cend(); ++rit){
            auto rule = rit->second; 
            if(rule == Rs) continue;

            for(auto r : rule->rules()){
                for(size_t i = 1; i < r.size(); i++){
                    if(std::string(*r[i]) == std::string(*Rs)){
                        if(i + 1 < r.size()){                            
                            auto ext = first(r[i+1]);
                            if(find(ext.begin(), ext.end(), Sign("Epsilon")) != ext.end()){
                                auto left = follow(rule);
                                res.insert(res.end(), left.begin(), left.end());
                            }
                            ext.erase(remove(ext.begin(), ext.end(), Sign("Epsilon")), ext.end());
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
*/

    void setup(){

        std::string r1 = "E";
        std::string r2 = "Eq";
        std::string r3 = "T";
        std::string r4 = "Tq";
        std::string r5 = "F";

        rules.push_back(Item(Sign(r1,false),
            {Sign(r3), Sign(r2)}
        ));

        rules.push_back(Item(Sign(r2,false),
            {Sign("+"), Sign(r3,false), Sign(r2,false)}
        ));
        rules.push_back(Item(Sign(r2,false),
            {Sign("Epsilon")}
        ));
        rules.push_back(Item(Sign(r3,false),
            {Sign(r5,false), Sign(r4,false)}
        ));
        rules.push_back(Item(Sign(r4,false),
            {Sign("*"), Sign(r5,false), Sign(r4,false)}
        ));
        rules.push_back(Item(Sign(r4,false),
            {Sign("Epsilon")}
        ));
        rules.push_back(Item(Sign(r5,false),
            {Sign("("), Sign(r1,false), Sign(")")}
        ));
        rules.push_back(Item(Sign(r5,false),
            {Sign("i")}
		));
    } 
    
    void test(Sign S){
        std::cout << "==== "<<std::string(S)<< " ===\n";        
        for(auto& s: first(S)){
            std::cout << std::string(s) << std::endl;
        }
		/*
        std::cout<<"===\n";
        for(auto& r: follow(R)){
            std::cout << std::string(*r) << std::endl;
        }
		*/
    }

    void parser(){
        setup();        
        test(Sign("E"));

        test(Sign("Eq"));

        test(Sign("T"));

        test(Sign("Tq"));

        test(Sign("F"));

        std::cout<<"===\n";
        //auto items = new Item("S'",{Sign("F"),Sign("FIN")});
        //Closure(items);
        //std::cout << *items;

        //delete items;
        
        //create_dfa();
        //for(auto rit = rule_table.begin(); rit != rule_table.end(); ++rit){
        //    if(rit->second)
        //        rit->second.reset();
        //}
    }
}


int main(){
    parser::parser();
    return 0;
}
/*
 * ==== T ===
 * (
 * i
 * ===
 * FIN
 * )
 * +
 * ==== Tq ===
 * *
 * Epsilon
 * ===
 * FIN
 * )
 * +
 * ==== F ===
 * (
 * i
 * ===
 * FIN
 * )
 * +
 * *
 * ===
 */

