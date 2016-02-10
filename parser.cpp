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

 	struct Sign{

        std::string name;
        bool isTerm;

        Sign(std::string n, bool t):
	        name(move(n)),
	        isTerm(move(t))
        {}

		Sign(std::string n):
			name(move(n)),
		    isTerm(true)
	    {}

		Sign():
			name(""),
			isTerm(true)
		{}

		operator std::string() const{
    		return name;
		}
 
		bool operator==(const Sign& rhs) const{
			return name == rhs.name;
		};
	
		inline bool operator!=(const Sign& rhs) const{
			return !(*this == rhs);
		}
	};

    class HashSign {
		public:
	    size_t operator()(const Sign& s) const {
	        const int C =  9873967;
	        size_t t = 0;
	        for(int i = 0; i != s.name.size(); ++i) {
	            t = t * C + (char)s.name[i];
	        }
	        return t;
	    }
	};

	struct Item{
        Sign left;
        std::vector<Sign> rights;
        int pos;

        Item(Sign l, vector<Sign> r):
        pos(0),
        left(move(l)),
        rights(move(r))
        {}

        Sign nextSign(){
        	if(isLast())
        		return Sign();
        	return rights.at(pos);
        }

        void next(){
        	pos++;
        }

		bool isLast(){
			return pos == rights.size();
		}

		friend std::ostream& operator<<(std::ostream &out, const Item &i){
			out << std::string(i.left) <<" => ";

			if(i.pos == 0){
				out<< " . ";
			}
			for(int j=0;j<i.rights.size();j++){
				out << std::string(i.rights.at(j));
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
	
	struct State{
		int id;
		vector<Item> items;
		unordered_map<string, int> transitions;
		State():
			id(-1)
		{}

		State(int id):
			id(id)
		{}

		void append(vector<Item> newItems){
			items.insert(items.end(), move(newItems).begin(), move(newItems).end());
		}

		friend std::ostream& operator<<(std::ostream &out, const State &s){
			out <<"- Q"<< std::to_string(s.id) <<" -\n";
			for(auto item : s.items){
				cout <<" "<< item;
			}
			out << "\n";
			return out;
		}
	};

	Sign mS(std::string name){
		return Sign(name,false);
	}
	Sign mtS(std::string name){
		return Sign(name);
	}

    auto  E = mS("E");
    auto Eq = mS("Eq");
    auto  T = mS("T");
    auto Tq = mS("Tq");
    auto  F = mS("F");

	auto Eps = mtS("Epsilon");
	auto Fin = mtS("Fin");

	std::vector<Item> grammar;

	std::vector<Item> getItems(Sign s){
		std::vector<Item> res;
		for(auto& i : grammar){
			if(i.left.name == s.name){
				res.push_back(i);
			}
		}
		return res;
	}

    std::vector<Sign> first(Sign sign){
        if(sign.isTerm){
            return {sign};
        }
        std::vector<Sign> res; 
		auto items = getItems( sign );
		if(items.size() == 0)
			return res;

        for(auto& i : items){
        	auto ext = first(i.rights[0]);
            if(find(ext.begin(), ext.end(), Eps) != ext.end()){
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

    std::vector<Sign> first(vector<Sign>& l){
        if(l.size() == 0)
            return {Eps};

        std::vector<Sign> res;
        
        auto it = l.begin();
        if(*it == Eps) return {Eps};
        if((*it).isTerm) return {*it};

        auto ext = first(*it); 
        if(find(ext.begin(), ext.end(), Eps) != ext.end()){
            ext.erase(remove(ext.begin(), ext.end(), Eps), ext.end());
            res.insert(res.end(), ext.begin(), ext.end());                
            if(l.size() >= 2 ){
                it++;
                auto next = first(*it);
                res.insert(res.end(), next.begin(), next.end());
            }else{
				res.push_back(Eps);
			}
        }
        return ext;
    }

    std::vector<Sign> follow(Sign s){
        std::vector<Sign> res;
        
        if(s == E){
            res.push_back(Fin);
        }

        for(auto rit = grammar.cbegin(); rit != grammar.cend(); ++rit){
            auto ls = (*rit).left; 
            if(ls == s) continue;

			auto rs = (*rit).rights;
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

    unordered_map<Sign, vector<Sign>, HashSign> follows;
	vector<shared_ptr<State>> DFAutomaton;
	
	void generateDFAutomaton(int st){
		cout<< "generateDFAutomaton("<<st<<") "<<DFAutomaton.size()<<"\n";

		vector<int> newStateNumbers;
		auto state = DFAutomaton.at(st);

		for(auto item : state->items){
			if(!item.isLast()){
				auto first = item.nextSign();

				if(!first.isTerm){
					state->append(getItems(first));
				}

				if(state->transitions.find(first) == state->transitions.end()){
					DFAutomaton.push_back(make_shared<State>(st+1));
					state->transitions[first] = DFAutomaton.size() - 1;
					newStateNumbers.push_back(DFAutomaton.size() - 1);
				}
				item.next();
				DFAutomaton.at(DFAutomaton.size() - 1)->append({item});
			}
		}

		cout << DFAutomaton.at(DFAutomaton.size() - 1)->items.size() << " " <<DFAutomaton.size() - 1 << endl;
		for(auto s : newStateNumbers){
       		generateDFAutomaton(s);
       	}
	}
	
	void setup(){

        grammar.push_back(Item( E,
            { T, Eq }
        ));

        grammar.push_back(Item( Eq,
            {mtS("+"), T,  Eq }
        ));
        grammar.push_back(Item( Eq,
            { Eps }
        ));
        
		grammar.push_back(Item( T,
            { F, Tq}
        ));
        
		grammar.push_back(Item( Tq,
            { mtS("*"), F, Tq }
        ));
        grammar.push_back(Item( Tq,
            { Eps }
        ));

        grammar.push_back(Item( F,
            { mtS("("), E, mtS(")")}
        ));
        grammar.push_back(Item( F,
            { mtS("i")}
		));

		for(auto I : grammar){
			follows.emplace( I.left, follow(I.left));
		}

		auto Q0 = make_shared<State>(0);
		Q0->append(getItems(E));
		DFAutomaton.push_back(Q0);

		generateDFAutomaton(0);

		for(int i=0;i<DFAutomaton.size();i++){
			cout << *DFAutomaton[i] << endl;
		}
    } 

	using namespace std;
    
    void test(Sign S){
        std::cout << "==== First is ==="<<std::string(S)<< " ===\n";        
        for(auto& s: first(S)){
            std::cout << std::string(s) << std::endl;
        }
        std::cout<<"===== Follow is ===\n";
        for(auto& r: follow(S)){
            std::cout << std::string(r) << std::endl;
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
	
        //delete items;
        
        //create_dfa();
        //for(auto rit = rule_table.begin(); rit != rule_table.end(); ++rit){
        //    if(rit.second)
        //        rit.second.reset();
        //}
*/        
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

