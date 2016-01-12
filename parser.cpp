#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>


#include <iostream>

#include <memory>

namespace parser{

 	class Sign_{
        std::string name_;
        bool isTerm_;
      public:
        Sign_(std::string n, bool t){
            name_ = n;
            isTerm_ = t;
        }
		Sign_(std::string n){
			name_ = n;
		    isTerm_ = true;	
		}
		Sign_(){
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

	
	using Sign = std::shared_ptr<Sign_>;
    
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
	
	Sign mS(std::string name){
		return Sign(new Sign_(name,false));
	}
	Sign mtS(std::string name){
		return Sign(new Sign_(name));
	}

    auto  E = mS("E");
    auto Eq = mS("Eq");
    auto  T = mS("T");
    auto Tq = mS("Tq");
    auto  F = mS("F");

	auto Eps = mtS("Epsilon");
	auto Fin = mtS("Fin");

	std::vector<Item> rules;

	std::vector<Item> getItems(Sign s){
		//std::cout << std::string(*s) << std::endl;
		std::vector<Item> res;
		for(auto& i : rules){
			if(i.left->name() == s->name()){
				res.push_back(i);
			}
		}
		return res;
	}

    std::vector<Sign> first(Sign sign){
        if(sign->isTerm()){
            return {sign};
        }
        std::vector<Sign> res; 
		auto items = getItems( sign );
		if(items.size() == 0)
			return res;

        for(auto& i : items){
			//std::cout << std::string( *i.left ) << " " << i.rights.size()  <<std::endl;
        	auto ext = first(i.rights[0]);
            if(find(ext.begin(), ext.end(), Eps) != ext.end()){
				//std::cout <<"Eps!\n";
            	ext.erase(remove(ext.begin(), ext.end(), Eps), ext.end());
               	res.insert(res.end(), ext.begin(), ext.end());
                    if(i.rights.size() >= 2){
                        auto nxt = first(i.rights[1]);
                        res.insert(res.end(), nxt.begin(), nxt.end());
                    }else{
                        res.push_back( Eps);
                    }
            }else{
            	res.insert(res.end(), ext.begin(), ext.end());
			}
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
*/
    std::vector<Sign> follow(Sign& s){
        std::vector<Sign> res;
        
        if(s == E){
            res.push_back(Fin);
        }

        for(auto rit = rules.cbegin(); rit != rules.cend(); ++rit){
            auto ls = rit->left; 
            if(ls == s) continue;

			auto rs = rit->rights;
            for(size_t i = 1; i < rs.size(); i++){
            	if(rs[i] == s){
                	if(i + 1 < rs.size()){                            
                    	auto ext = first(rs[i+1]);
                        if(find(ext.begin(), ext.end(), Eps) != ext.end()){
               	        	auto left = follow(ls);
                            res.insert(res.end(), left.begin(), left.end());
                        }
                        ext.erase(remove(ext.begin(), ext.end(), Eps), ext.end());
                       	res.insert(res.end(), ext.begin(), ext.end());
                    }else{
                        auto left = follow(ls);
                        res.insert(res.end(), left.begin(), left.end());
                    }
                }
            }
        }
        return res;
    }

	void closure(std::vector<Item>& I){
		int size = I->size();
		do{
			for(auto i : I){
				auto X = getItem(i.nextSign() );
				for(auto x : X){
					I.push_back(x);
				}
			}
		}while( size != I.size() );
	}

	void setup(){

        rules.push_back(Item( E,
            { T, Eq }
        ));

        rules.push_back(Item( Eq,
            {mtS("+"), T,  Eq }
        ));
        rules.push_back(Item( Eq,
            { Eps }
        ));
        
		rules.push_back(Item( T,
            { F, Tq}
        ));
        
		rules.push_back(Item( Tq,
            { mtS("*"), F, Tq }
        ));
        rules.push_back(Item( Tq,
            { Eps }
        ));

        rules.push_back(Item( F,
            { mtS("("), E, mtS(")")}
        ));
        rules.push_back(Item( F,
            { mtS("i")}
		));
    } 

	using namespace std;
    
    void test(Sign S){
        std::cout << "==== "<<std::string(*S)<< " ===\n";        
        for(auto& s: first(S)){
            std::cout << std::string(*s) << std::endl;
        }
        std::cout<<"===\n";
        for(auto& r: follow(S)){
            std::cout << std::string(*r) << std::endl;
        }
		
	}


    void parser(){
        setup();        
        test(E);

        test(Eq);

        test(T);

        test(Tq);

        test(F);

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

