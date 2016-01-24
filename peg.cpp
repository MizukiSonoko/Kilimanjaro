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
		vector<Sign> rules;
	public:
		char value;
		string type;
		Sign(initializer_list<Sign> l,string t):rules(l){
			type = t;
		}
		Sign(Sign s,string t): rules({s}){
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
				for(auto r : rules){
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
				for(auto r : rules){
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
				mark();
				cout<< "--- optional! ---\n";
				cout<< "optional "<<raw_source_[cursor]<<" "<<rules[0].value<< endl;
				if(rules[0].value == raw_source_[cursor]){
					return true;
				}
				back();
				return true;
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
		return orderedChoice({sequence({optional(Terminal('a')),Terminal('b'),Terminal('c')}), Terminal('d')}).execution();
		//Expr().execution();
	}

	void test(){
		cout<< "# Test for sequence\n";
		{
			cout<<"====== test 1 ======\n";
			raw_source_ = "abcd";
			cursor = 0;
			if( sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 2 ======\n";
			raw_source_ = "adbc";
			cursor = 0;
			if(! sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 3 ======\n";
			raw_source_ = "a";
			cursor = 0;
			if( sequence({Terminal('a')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 4 ======\n";
			raw_source_ = "";
			cursor = 0;
			if(! sequence({Terminal('a')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
		}
		cout<< "# Test for orderedChoice\n";
		{
			cout<<"====== test 5 ======\n";
			raw_source_ = "a";
			cursor = 0;
			if( orderedChoice({Terminal('a')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 6 ======\n";
			raw_source_ = "a";
			cursor = 0;
			if(! orderedChoice({Terminal('b')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 7 ======\n";
			raw_source_ = "a";
			cursor = 0;
			if( orderedChoice({Terminal('a'),Terminal('b')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 8 ======\n";
			raw_source_ = "a";
			cursor = 0;
			if( orderedChoice({Terminal('b'),Terminal('a')}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 9 ======\n";
			raw_source_ = "ab";
			cursor = 0;
			if( orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('a'),Terminal('b')})}).execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}			
		}

	}
 

};