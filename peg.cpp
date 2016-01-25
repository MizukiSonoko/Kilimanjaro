#include "peg.h"

#include <regex>
#include <stack>
#include <vector>

#include <functional>
#include <map>

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
		vector<function<Sign()>> rules;
	public:
		char value;
		string type;
		Sign(initializer_list<function<Sign()>> l,string t):rules(l){
			type = t;
		}		
		Sign(function<Sign()> s,string t): rules({s}){
			type = t;
		}
		Sign(char v, string t){
			type = t;
			value = v;
		}
		Sign(string t){
			type = t;
		}
		Sign(){
			type = "";
		}
		bool execution(){
			if(type == "sequence"){
				mark();
				cout<< "--- sequence! ---\n";
				for(auto r : rules){
					if(r().type == "Terminal"){
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r().value<< endl;
						if(raw_source_[cursor] != r().value){
							back();
							return false;
						}
						cursor++;
					}else{
						if(!r().execution()){
							back();
							return false;
						}
					}
				}
				cout<<"sequence success!\n";
				return true;
			}else if(type == "orderedChoice"){
				mark();
				cout<< "--- orderedChoice! ---\n";		
				for(auto r : rules){
					if(r().type == "Terminal"){
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r().value<< endl;
						if(raw_source_[cursor] == r().value){
							cursor++;
							return true;
						}
					}else{
						if(r().execution()){
							return true;
						}
					}					
				}
				back();
				return false;
			}else if(type == "optional"){
				mark();
				cout<< "--- optional! ---\n";
				cout<< "optional "<<raw_source_[cursor]<<" "<<rules[0]().value<< endl;
				if(rules[0]().value == raw_source_[cursor]){
					cursor++;
					return true;
				}
				back();
				return true;
			}else if(type == "zeroOrMore"){
				mark();
				cout<< "--- zeroOrMore! ---\n";
				bool ok = false;
				bool first = false;
				do{
					ok = false;
					for(auto r : rules){
						if(r().type == "Terminal"){
							cout<< "Terminal "<<raw_source_[cursor]<<" "<<r().value<< endl;
							if(raw_source_[cursor] == r().value){
								ok = true;
								cursor++;
							}
						}else{
							if(r().execution()){
								ok = true;
							}
						}
					}					
					if(ok)
						first = true;
				}while(ok);

				if(!first){
					back();
				}
				return true;
			}else if(type == "oneOrMore"){
				mark();
				cout<< "--- oneOrMore! ---\n";
				bool ok = false;
				bool first = false;
				do{
					ok = false;
					for(auto r : rules){
						if(r().type == "Terminal"){
							cout<< "Terminal ["<<raw_source_[cursor]<<","<<r().value<<"]"<< endl;
							if(raw_source_[cursor] == r().value){
								ok = true;
								cursor++;
							}
						}else{
							if(r().execution()){
								ok = true;
							}
						}
					}
					if(ok)
						first = true;		
				}while(ok);

				if(!first){
					back();
					cursor--;
					return false;
				}
				return true;
			}else if(type == "Number"){
				cout<< "Number "<<raw_source_[cursor]<< endl;	
				if('0' <= raw_source_[cursor] && raw_source_[cursor] <= '9'){
					cursor++;
					return true;
				}
				return false;
			}
			return false;
		}
	};

	function<Sign()> sequence(initializer_list<function<Sign()>> l){ // A B C ...
		return [l]() -> Sign{ return Sign(l, "sequence"); };
	}

	function<Sign()> orderedChoice(initializer_list<function<Sign()>> l){ // A / B / C ...
		return [l]() -> Sign{ return Sign(l, "orderedChoice"); };
	}
	function<Sign()> optional(function<Sign()> s){ // E?
		return [s]() -> Sign{ return Sign(s, "optional"); };
	}
	function<Sign()> zeroOrMore(function<Sign()> s){ // E*
		return [s]() -> Sign{ return Sign(s, "zeroOrMore"); };
	}
	function<Sign()> oneOrMore(function<Sign()> s){  // E+
		return [s]() -> Sign{ return Sign(s, "oneOrMore"); };
	}
	Sign andPredicate();
	Sign notPredicate();
	Sign endOfString();

	function<Sign()> Terminal(char v){
		return [v]() -> Sign{ return Sign(v, "Terminal"); };
	}
	function<Sign()> Number(){		
		return []() -> Sign{ return Sign("Number"); };
	}


	map<string, function<Sign()>> rules;
	void init(){

		rules["Value"] = []()-> Sign{
			cout<<"Exec value\n";
			return orderedChoice({ oneOrMore(Number()), sequence({Terminal('('), rules["Expr"], Terminal(')')})})();
		};

		rules["Product"] = []()-> Sign{
			cout<<"Exec product\n";
			return sequence({ rules["Value"], zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), rules["Value"]}))})();
		};
		rules["Sum"] = []()-> Sign{
			cout<<"Exec sum\n";
			return sequence({ rules["Product"], zeroOrMore(sequence({orderedChoice({Terminal('+'), Terminal('-')}), rules["Product"]}))})();
		};
		rules["Expr"] = []()-> Sign{
			cout<<"Exec expr\n";
			return rules["Sum"]();
		};
	}
	/*
	map<string, function<Sign()>> rules;
	void init(){
		rules["A"] = []()-> Sign{
			cout<<"call A\n";
			auto res = sequence({ Terminal('-'), rules["A"] });
			return res();
		};

		rules["B"] = []()-> Sign{
			cout<<"call B\n";
			auto res = orderedChoice({sequence({Terminal('-'),rules["B"] }), Terminal('.')});
			return res();
		};
	}
	*/
	void set_source(string s){
		raw_source_ = s;
	}

	bool exec(string n){
		set_source(n);
		cursor = 0;
		cout << "Input:"<< raw_source_ <<"\n";
		init();
		return rules["Expr"]().execution();
		//return A()().execution();
		//Expr()().execution();
	}


	bool tex(string code, function<Sign()> c){
		cursor = 0;
		if( c().execution() ){
			cout << "\x1b[32mPassed!\x1b[39m\n";
			return true;
		}else{
			cout << "\033[1;31mFaild\033[0m\n";
			return false;
		}
	}
	void test(){
		cout<< "# Test for sequence\n";
		{
			cout<<"====== test 1 ======\n";
			tex("abcd", sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}));
			
			cout<<"====== test 2 ======\n";
			set_source("adbc");
			cursor = 0;
			if(! sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 3 ======\n";
			set_source("a");
			cursor = 0;
			if( sequence({Terminal('a')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 4 ======\n";
			set_source("");
			cursor = 0;
			if(! sequence({Terminal('a')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
		}
		cout<< "# Test for orderedChoice\n";
		{
			cout<<"====== test 5 ======\n";
			set_source("a");
			cursor = 0;
			if( orderedChoice({Terminal('a')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 6 ======\n";
			set_source("a");
			cursor = 0;
			if(! orderedChoice({Terminal('b')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 7 ======\n";
			set_source("a");
			cursor = 0;
			if( orderedChoice({Terminal('a'),Terminal('b')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 8 ======\n";
			set_source("a");
			cursor = 0;
			if( orderedChoice({Terminal('b'),Terminal('a')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 9 ======\n";
			set_source("ab");
			cursor = 0;
			if( orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('a'),Terminal('b')})})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 10 ======\n";
			set_source("ab");
			cursor = 0;
			if(! orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('b'),Terminal('b')})})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 11 ======\n";
			set_source("ba");
			cursor = 0;
			if( orderedChoice({Terminal('a'),Terminal('b')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}	
			cout<<"====== test 12 ======\n";
			set_source("ba");
			cursor = 0;
			if( orderedChoice({sequence({Terminal('a')}),sequence({Terminal('b')})})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}		
		}
		cout<< "# Test for optional\n";
		{
			cout<<"====== test 13 ======\n";
			set_source("");
			cursor = 0;
			if( optional(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}	
			cout<<"====== test 14 ======\n";
			set_source("ab");
			cursor = 0;
			if( sequence({optional(Terminal('a')), Terminal('b')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 15 ======\n";
			set_source("b");
			cursor = 0;
			if( sequence({optional(Terminal('a')), Terminal('b')})().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 16 ======\n";
			set_source("");
			cursor = 0;
			if( optional(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
		}
		cout<< "# Test for zeroOrMore\n";
		{
			cout<<"====== test 1 ======\n";
			set_source("");
			cursor = 0;
			if( zeroOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 2 ======\n";
			set_source("a");
			cursor = 0;
			if( zeroOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 3 ======\n";
			set_source("aaa");
			cursor = 0;
			if( zeroOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 4 ======\n";
			set_source("b");
			cursor = 0;
			if( zeroOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 5 ======\n";
			set_source("ab");
			cursor = 0;
			if( zeroOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 6 ======\n";
			set_source("");
			cursor = 0;
			if( zeroOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 7 ======\n";
			set_source("c");
			cursor = 0;
			if( zeroOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

		}

		cout<< "# Test for oneOrMore\n";
		{
			cout<<"====== test 1 ======\n";
			set_source("");
			cursor = 0;
			if(! oneOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 2 ======\n";
			set_source("a");
			cursor = 0;
			if( oneOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 3 ======\n";
			set_source("aaa");
			cursor = 0;
			if( oneOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 4 ======\n";
			set_source("b");
			cursor = 0;
			if(! oneOrMore(Terminal('a'))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 5 ======\n";
			set_source("ab");
			cursor = 0;
			if( oneOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 6 ======\n";
			set_source("ababab");
			cursor = 0;
			if( oneOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}			
			cout<<"====== test 7 ======\n";
			set_source("a");
			cursor = 0;
			if(! oneOrMore(sequence({Terminal('a'),Terminal('b')}))().execution() ){
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
		}
		cout<< "# Test for mix\n";
		{
			cout<<"====== test 1 ======\n";
			set_source("1+1");
			cursor = 0;
			if( sequence({ Number(), Terminal('+'), Number()})().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 2 ======\n";
			set_source("1234+1");
			cursor = 0;
			if( sequence({ oneOrMore(Number()), Terminal('+'), Number()})().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 3 ======\n";
			set_source("1234+5678");
			cursor = 0;
			if( sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())})().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}

			cout<<"====== test 4 ======\n";
			set_source("-1234+5678");
			cursor = 0;
			if( sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())})().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}			

			cout<<"====== test 5 ======\n";
			set_source("1234+");
			cursor = 0;
			if(! sequence({ oneOrMore(Number()), Terminal('+'), oneOrMore(Number())})().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}					
		}
		cout<< "# Test for calculator\n";
		{
			init();
			cout<<"====== test 1 ======\n";
			set_source("1+1");
			cursor = 0;
			if( rules["Expr"]().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 2 ======\n";
			set_source("1+");
			cursor = 0;
			if(! rules["Expr"]().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
			cout<<"====== test 3 ======\n";
			set_source("1+");
			cursor = 0;
			if(! rules["Expr"]().execution()) {
				cout << "\x1b[32mPassed!\x1b[39m\n";
			}else{
				cout << "\033[1;31mFaild\033[0m\n";
			}
					

		}

	}

};