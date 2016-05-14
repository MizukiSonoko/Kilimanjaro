#include <functional>
#include <initializer_list>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include <iomanip>

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

        Sign head;
        int pos;

        Item(Sign l, vector<Sign> r,Sign head):
        pos(0),
        left(move(l)),
        rights(move(r)),
        head(move(head))
        {}

        Sign nextSign() noexcept{
        	if(isLast())
        		return Sign();
        	return rights.at(pos);
        }

  void next() noexcept{
    pos++;
  }

		bool isLast() noexcept{
			return pos == rights.size();
		}

		int posOf(Sign s) noexcept{
			int i = 0;
			for(auto v : rights){
				if(v == s)
					return i;
				i++;
			}
			return -1;
		}

		friend ostream& operator<<(ostream &out, const Item &i){
			out <<"("<< std::string(i.left) <<" => ";

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
			out <<", "<< std::string(i.head)<< ") \n";
			return out;
		}
	};
	
	struct State{
		int id;
		vector<Item> items;
		vector<Item> expands;

		State():
			id(-1)
		{}

		State(int id):
			id(id)
		{}

		void append(vector<Item>&& newItems){
			this->items.insert(items.end(), move(newItems).begin(), move(newItems).end());
		}

		void expand(vector<Item>&& newItems){
			this->expands.insert(expands.end(), move(newItems).begin(), move(newItems).end());
		}

		void merge(){
			this->items.insert(items.end(), expands.begin(), expands.end());
		}

		friend std::ostream& operator<<(std::ostream &out, const State &s){
			out <<"- Q"<< std::to_string(s.id) <<" -\n";
			for(auto item : s.items){
				cout <<" "<< item;
			}
			for(auto item : s.expands){
				cout <<"  "<< item;
			}
			out << "\n";
			return out;
		}
	};

struct Action{
		int id;
		enum class A{
			SHIFT,
			GOTO,
			REDUCE,
			ACCEPT
		} action;

		Action(int id,A action):
			id(move(id)),
			action(move(action))
		{}
};

Sign mS(std::string&& name){
  return Sign(move(name),false);
}

Sign mtS(std::string&& name){
  return Sign(move(name));
}

auto  E = mS("E");
auto Eq = mS("Eq");
auto  T = mS("T");
auto Tq = mS("Tq");
auto  F = mS("F");

auto  S = mS("S");

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

std::vector<Item> closure(std::vector<Item> I){
        vector<Item> res; 
        int size;
        do{
            size = res.size();
            for(auto i : I){
                if(!i.isLast()){
                    auto X = i.nextSign();
                    auto z = i.head;
                    i.next();
                    if(!i.isLast()){
                        auto beta = i.nextSign();
                        for(auto x : getItems(X)){
                            auto w = first(beta);
                            for(auto s : w){
                                x.head = s; 
                                res.push_back(x);
                            }
                        }
                    }
                }
            }
        }while(size != res.size());
        return res;
    }
/*

    std::vector<Sign> follow(Sign s){
    	if(s == E)
    		return { Eps };

    	cout << string(s) << endl;
        std::vector<Sign> res;
	    for(auto item : grammar){
        	if(item.posOf(s) == -1)
        		continue;

            if(item.posOf(s) == item.rights.size()-1){
            	if(s != item.left){
	            	auto newFollow = follow(item.left);
	                res.insert(res.end(), newFollow.begin(), newFollow.end());
	            }
            }else{
                res.push_back(item.rights.at(item.posOf(s)+1));
            }
	    }
	    return res;
	}
*/
  unordered_map<Sign, vector<Sign>, HashSign> follows;
	vector<shared_ptr<State>> DFAutomaton;
	unordered_multimap<Sign, pair<int,int>, HashSign> transitions;

	int cnt = 0;
	void generateDFAutomaton(int st){
		
		cout<< "generateDFAutomaton("<<st<<") \n";
		for(auto i : (*DFAutomaton[st]).items)
			cout << i;
		cout << "============\n";
		cnt++;
		if(cnt > 100) return;
		
		vector<int> newStateNumbers;
		auto state = DFAutomaton.at(st);
		cout <<"state:"<< st << endl;
		for(auto item : state->items){
			//cout <<" size is "<< state->items.size() << endl;
			Sign first = item.nextSign();
			if(first.name=="")
				continue;
			
			//cout << string(first) << endl;
			if(!first.isTerm){
				state->expand(getItems(first));
			}

			if(!item.isLast()){
				if(transitions.find(first.name) == transitions.end()){
					DFAutomaton.push_back(make_shared<State>(DFAutomaton.size()));
					transitions.emplace(first, make_pair( DFAutomaton.size()-1, st));
					newStateNumbers.push_back(DFAutomaton.size() - 1);

				//cout<<"** \n"<< item <<" added "<< DFAutomaton.size() - 1 << endl;
				item.next();
				DFAutomaton.at(DFAutomaton.size() - 1)->append({item});

				}
			}
		}
		//cout << "extends\n";
		for(auto item : state->expands){
			Sign first = item.nextSign();
			if(first.name=="")
				continue;
			//cout << string(first) << endl;
			if(!item.isLast()){
				if(transitions.find(first.name) == transitions.end()){
					DFAutomaton.push_back(make_shared<State>(DFAutomaton.size()));
					transitions.emplace(first, make_pair( DFAutomaton.size()-1, st));
					newStateNumbers.push_back(DFAutomaton.size() - 1);
				}
				//cout<<"** \n"<< item <<" added "<< DFAutomaton.size() - 1 << endl;
				item.next();
				DFAutomaton.at(DFAutomaton.size() - 1)->append({item});
				
			}
		}

		for(auto s : newStateNumbers){
			cout<< st <<"'s sub generateDFAutomaton("<<s<<") "<<(*DFAutomaton[s]).items.size()<<"\n";
       		generateDFAutomaton(s);
       	}
	}
	
	void setup(){

	
        grammar.push_back(Item( E,
            { T, Eq }
        , Fin));
        grammar.push_back(Item( Eq,
            {mtS("+"), T,  Eq }
        , E));
        grammar.push_back(Item( Eq,
            { Eps }
        , Fin));        
		grammar.push_back(Item( T,
            { F, Tq}
        , Fin));
		grammar.push_back(Item( Tq,
            { mtS("*"), F, Tq }
        , E));
        grammar.push_back(Item( Tq,
            { Eps }
        , Fin));
        grammar.push_back(Item( F,
            { mtS("("), E, mtS(")")}
        , Fin));
        grammar.push_back(Item( F,
            { mtS("i")}
		, Fin));
        grammar.push_back(Item( S,
            { E, Fin}
		, Fin));

		for(auto I : grammar){
			follows.emplace( I.left, follow(I.left));
		}

		auto Q0 = make_shared<State>(0);
		Q0->append(getItems(S));
		DFAutomaton.push_back(Q0);

		generateDFAutomaton(0);
		cout << "=======\n";
		for(int i=0;i<DFAutomaton.size();i++){
			cout << *DFAutomaton[i] << endl;
		}

		for(auto itr = transitions.begin(); itr != transitions.end(); ++itr) {
        	std::cout << "key = " << itr->first.name
	        << ": from:"<< itr->second.second<< " to:" << itr->second.first << "\n";
    	}

    	vector<unordered_map<Sign, shared_ptr<Action>, HashSign>> parserTable(DFAutomaton.size());

		for(auto it = transitions.begin(); it != transitions.end(); ++it){
			if(it->first.isTerm){
				parserTable.at(it->second.second).emplace( it->first, make_shared<Action>(it->second.first, Action::A::SHIFT));
				cout <<"shift("<< it->second.second <<","<< it->second.first <<")\n";
			}else{
				parserTable.at(it->second.second).emplace( it->first, make_shared<Action>(it->second.first, Action::A::GOTO));
				cout <<"goto("<< it->second.second <<","<< it->second.first <<")\n";
			}
		}


		vector<Sign> signs{mtS("i"), mtS("*"), mtS("+"), mtS("("), mtS(")"), E, Eq, T, Tq, F};		
		cout<<"  |";
		for(auto s : signs){
			cout <<setw(3)<< string(s) <<"|  ";
		}
		cout << endl; 
		for(int i=0;i< parserTable.size();i++){
			cout <<setw(2)<< i << "|";
			for(auto s : signs){
				if(parserTable.at(i).find(s) != parserTable.at(i).end()){
					auto ac = parserTable.at(i).find(s)->second;
					if(ac->action == Action::A::SHIFT){
						cout << "s"<<setw(2)<< ac->id <<"|";
					}else{
						cout << "g"<<setw(2)<< ac->id <<"|";
					}
				}else{
					cout << "   |";
				}
			}
			cout << endl;
		}
//    	parserTableparserTable.
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

