#include "peg.h"

#include <regex>
#include <stack>
#include <vector>

namespace peg{
	using namespace std;

	string raw_rule_;
	string raw_source_;

	string load_file(string filename){
		std::ifstream ifs( filename, std::ios::in | std::ios::binary);
		if(ifs.fail()){
			cout << "\n\033[1;31mNo such file or directory \""<< filename <<"\"\033[0m\n";
			exit(1);
		}
		ifs.seekg( 0, std::ios::end);
		int pos = ifs.tellg();
		ifs.seekg( 0, std::ios::beg);

		vector<char> buf(pos);
		ifs.read(buf.data(), pos);
		return string(buf.begin(),buf.end());
	}

	void load_cpp(string filename){
		raw_source_ = load_file(filename);
	}

	void load_rule(string filename){
		raw_rule_ = load_file(filename);
	}

	int cursor;
	stack<int> markers;
	
	void mark(){
		cout << "push "<<cursor <<endl;
		markers.push(cursor);
	}
	void back(){
		cursor = markers.top();
		cout << "pop "<<cursor <<endl;
		markers.pop();
	}

	class Sign{
		vector<vector<Sign>> rules;
	public:
		char value;
		string type;
		Sign(initializer_list<Sign> l,string t){
			vector<Sign> v(l);
			rules.push_back(v);
			type = t;
		}
		Sign(Sign s,string t){
			vector<Sign> v{s};
			rules.push_back(v);
			type = t;
		}
		Sign(char v, string t){
			type = t;
			value = v;
		}
		Sign(string t){
			type = t;
		}

		bool execution(){
			if(type == "sequence"){
				mark();
				cout<< "--- sequence! ---\n";
				for(auto r : rules[0]){
					if(r.type == "Terminal"){
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r.value<< endl;
						if(raw_source_[cursor] != r.value){
							back();
							return false;
						}
					}else{
						if(!r.execution()){
							back();
							return false;
						}
					}
					cursor++;
				}
				return true;
			}else if(type == "orderedChoice"){
				mark();
				cout<< "--- orderedChoice! ---\n";				
				for(auto r : rules[0]){
					if(r.type == "Terminal"){
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r.value<< endl;
						if(raw_source_[cursor] == r.value){
							return true;
						}
					}else{
						if(r.execution()){
							return true;
						}
					}					
				}
				back();
				return false;
			}else if(type == "optional"){
				return false;
			}else if(type == "zeroOrMore"){

				return false;
			}
			return false;
		}
	};

	Sign sequence(initializer_list<Sign> l){ // A B C ...
		return Sign(l, "sequence");
	}
	Sign orderedChoice(initializer_list<Sign> l){ // A / B / C ...
		return Sign(l, "orderedChoice");
	}
	Sign optional(Sign s){ // E?
		return Sign(s, "optional");
	}
	Sign zeroOrMore(Sign s){ // E*
		return Sign(s, "zeroOrMore");
	}
	Sign oneOrMore(Sign s){  // E+
		return Sign(s, "oneOrMore");
	}
	Sign andPredicate();
	Sign notPredicate();
	Sign endOfString();

	Sign Terminal(char v){
		return Sign(v, "Terminal");
	}
	Sign Number(){		
		return Sign("Number");
	}

	Sign Expr();

	Sign Value(){
		return orderedChoice({ oneOrMore(Number()), sequence({Terminal('('), Expr(), Terminal(')')})});
	}
	Sign Product(){
		return sequence({ Value(), zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), Value()}))});
	}
	Sign Sum(){
		return sequence({ Product(), zeroOrMore(sequence({orderedChoice({Terminal('+'), Terminal('-')}), Value()}))});
	}
	Sign Expr(){
		return Sum();
	}

	bool exec(string n){
		raw_source_ = n;
		cursor = 0;
		cout << "Input:"<< raw_source_ <<"\n";
		return orderedChoice({sequence({Terminal('a'),Terminal('b')}), Terminal('c'), Terminal('d')}).execution();
		//Expr().execution();
	}

};