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

	void log(string s);

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
		markers.push(cursor);
	}
	void back(){
		cursor = markers.top();
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


	bool tex(int num, string code, function<Sign()> c,bool correct = true){
		cout<<"====== test "<<num<< " ======\n";
		cursor = 0;
		set_source(code);
		if(!(c().execution() ^ correct)){
			cout << "\x1b[32m"<< num <<" is Passed!\x1b[39m\n";
			return true;
		}else{
			cout << "\033[1;31m"<< num <<" is Faild\033[0m\n";
			return false;
		}
	}
	void test(){
		cout<< "# Test for sequence\n";
		{
			tex( 1, "abcd", sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}));
			tex( 2, "adbc",sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}), false);
			tex( 3, "a", sequence({Terminal('a')}));
			tex( 4, "", sequence({Terminal('a')}), false);
		}
		cout<< "# Test for orderedChoice\n";
		{
			tex( 1, "a", orderedChoice({Terminal('a')}));
			tex( 2, "a", orderedChoice({Terminal('b')}), false);
			tex( 3, "a", orderedChoice({Terminal('a'),Terminal('b')}));
			tex( 4, "a", orderedChoice({Terminal('b'),Terminal('a')}));
			tex( 5, "ab", orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('a'),Terminal('b')})}));
			tex( 6, "ab", orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('b'),Terminal('b')})}), false);
			tex( 7, "ba", orderedChoice({Terminal('a'),Terminal('b')}));
			tex( 8, "ba", orderedChoice({sequence({Terminal('a')}),sequence({Terminal('b')})}));
		}
		cout<< "# Test for optional\n";
		{
			tex( 1, "", optional(Terminal('a')));
			tex( 2, "ab", sequence({optional(Terminal('a')), Terminal('b')}));
			tex( 3, "b", sequence({optional(Terminal('a')), Terminal('b')}));
		}
		cout<< "# Test for zeroOrMore\n";
		{
			tex( 1, "", zeroOrMore(Terminal('a')));
			tex( 2, "a", zeroOrMore(Terminal('a')));
			tex( 3, "aaaa", zeroOrMore(Terminal('a')));
			tex( 4, "b", zeroOrMore(Terminal('a')));
			tex( 5, "ab", zeroOrMore(Terminal('a')));
			tex( 6, "", zeroOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( 7, "c", zeroOrMore(sequence({Terminal('a'),Terminal('b')})));
		}

		cout<< "# Test for oneOrMore\n";
		{
			tex( 1, "", oneOrMore(Terminal('a')), false);
			tex( 2, "a", oneOrMore(Terminal('a')));
			tex( 3, "aaaa", oneOrMore(Terminal('a')));
			tex( 4, "b", oneOrMore(Terminal('a')), false);
			tex( 5, "ab", oneOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( 6, "ababab", oneOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( 7, "a", oneOrMore(sequence({Terminal('a'),Terminal('b')})), false);
		}
		cout<< "# Test for mix\n";
		{
			tex( 1,"1+1", sequence({ Number(), Terminal('+'), Number()}));
			tex( 2,"1234+1", sequence({ oneOrMore(Number()), Terminal('+'), Number()}));
			tex( 3,"1234+5678", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}));
			tex( 4,"1234+", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
			tex( 5,"-1234+5678", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}));
			tex( 6,"+1234+5678-", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
		}
		cout<< "# Test for calculator\n";
		{
			init();
			tex( 1, "1+1", rules["Expr"]);
			tex( 2, "1+", rules["Expr"], false);
			tex( 3, "+", rules["Expr"], false);
			tex( 4, "1", rules["Expr"]);
		}

	}

};