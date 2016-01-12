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
			pos = 0;
            left = l;
            rights.insert(rights.end(),r.begin(), r.end());
        }
        Sign nextSign(){
        	return rights[pos];
        }
        void next(){
			if(rights.size() > pos+1)
            	pos++;
        }
		bool isLast(){
			return pos == rights.size()-1;
		}
		friend std::ostream& operator<<(std::ostream &out, const Item &i){
			out << std::string(*i.left) <<" => ";
			if(i.pos == 0){
				out<< " . ";
			}
			for(int j=0;j<i.rights.size();j++){
				out << std::string(*i.rights[j]);
				if(i.pos == j+1){
					out<< " . ";
				}else{
					out<<"   ";
				}
			}
			out << "\n";
			return out;
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
		std::cout<< std::string(*s) << std::endl;
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
		int size = I.size();
		std::vector<Sign> alreadys;
		do{
			size = I.size();
			std::cout<<"LOOP\n";
			for(auto i : I){
				//std::cout<< i << std::endl;
				auto X = getItems( i.nextSign() );

				std::cout<< "SIZE:"<<X.size()<<std::endl;
				//std::cout<<"item \n";
				for(auto x : X){
					std::cout<< "#######\n" << x <<"\n";
					//if(find(alreadys.begin(), alreadys.end(), x.left) != alreadys.end()){
						//std::cout<<":CON\n";
					//		continue;
					//}
					//std::cout<<"loop\n";
					//x.next();
					//std::cout<<"push X\n";
					I.push_back(x);
					//alreadys.push_back(x.left);
				}
			}
			std::cout<< size <<" "<<I.size() << "\n";
		}while( size != I.size() );
	}

	std::vector<Item> Goto(std::vector<Item> I,Sign X){
		std::vector<Item> J;
		/*
		std::cout << "Goto argv b--------\n";
		for(auto i : I) std::cout<< i;
		std::cout<< std::string(*X)<<std::endl;;
		std::cout << "Goto argv e--------\n";
		// */
		for(auto i : I){
			if(i.nextSign() == X && !i.isLast()){
				//std::cout<<"1^^^^\n";
				i.next();
				//std::cout<< i;
            	J.push_back(i);
				//std::cout<<"2^^^^\n";
			}
		}
		closure(J);
		return J;
	}

	void DFA(){
		std::cout<<"Start\n";
		std::vector<std::vector<Item>> T;
		std::vector<std::vector<Item>> Tt;
		std::vector<Item> f({ Item( mS("S"),{ E, Fin}) });
		closure(f);
		T.push_back(f);
		int size = T.size();
		int count = 0;
		std::cout<< f[0];
		std::cout<<"++++++++++++++++\n";
		Tt = T;
		std::vector<Sign> alreadys;
		while( count < 5){
			count++;
			for(auto t : T){
				for(auto i : t){
					std::cout<< "i loop start\n"<< i;
					if(find(alreadys.begin(), alreadys.end(), i.nextSign()) != alreadys.end())
						continue;
					alreadys.push_back(i.nextSign());
					auto J = Goto( t, i.nextSign());
					std::cout << "***************************\n";
					std::cout << "I:"<< std::string(*i.nextSign()) << std::endl;
					for(auto j : J)
						std::cout << j;
					std::cout << "***************************\n";
					T.push_back(J);
					std::cout<<"i loop end\n";
				}
				std::cout<<"t loop end\n";
			}
			std::cout<< size << " " << T.size() << std::endl; 
			if( size != T.size()){
				size = T.size();
			}else{
				break;
			}
		}
		std::cout<<"####################\n";
		for(auto t : T){
			std::cout<<"~~~~~~~~~~~~~~~\n";
			for(auto i : t){
				std::cout<< i;
			}
		}
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
        /*
		test(E);

        test(Eq);

        test(T);

        test(Tq);

        test(F);
		
        std::cout<<"===\n";
		std::vector<Item> items = { Item( mS("S"), { E, Fin}) };
        closure(items);
		std::cout<<"~~~~~~~~~~~~~~~\n";
        for(auto i : items)
			std::cout << i;
		*/
		DFA();
	
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

