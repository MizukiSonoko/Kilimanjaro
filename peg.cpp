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
	int max_cursor;
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
				max_cursor = cursor;
				mark();				
				#ifdef DEBUG
				cout<< "--- sequence! ---\n";
				#endif
				for(auto r : rules){
					if(r().type == "Terminal"){
						#ifdef DEBUG
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r().value<< endl;
						#endif
						if(raw_source_[cursor] != r().value){
							max_cursor = cursor;
							back();
							return false;
						}
						cursor++;
					}else{
						if(!r().execution()){
							max_cursor = cursor;
							back();
							return false;
						}
					}
				}
				return true;
			}else if(type == "orderedChoice"){
				mark();
				#ifdef DEBUG
				cout<< "--- orderedChoice! ---\n";		
				#endif
				for(auto r : rules){
					if(r().type == "Terminal"){
						#ifdef DEBUG
						cout<< "Terminal "<<raw_source_[cursor]<<" "<<r().value<< endl;
						#endif
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
				#ifdef DEBUG
				cout<< "--- optional! ---\n";
				cout<< "optional "<<raw_source_[cursor]<<" "<<rules[0]().value<< endl;
				#endif
				if(rules[0]().value == raw_source_[cursor]){
					cursor++;
					return true;
				}
				back();
				return true;
			}else if(type == "zeroOrMore"){
				mark();	
				#ifdef DEBUG
				cout<< "--- zeroOrMore! "<< rules[0]().type <<" ---\n";
				#endif
				bool res = false;
				bool first = false;
				// sequence(), terminal
				auto sign = rules[0]();
				do{
					if(sign.type == "Terminal"){
						if(sign.value == raw_source_[cursor]){
							cursor++;
						}
					}else{
						res = sign.execution();
						if(!res &&  sign.type == "sequence"){
							if(cursor != max_cursor){
								return false;
							}
						}
//						cursor++;
					}
					if(res)
						first = true;
				}while(res);

				if(!first)
					back();
				return true;
			}else if(type == "oneOrMore"){
				mark();
				#ifdef DEBUG
				cout<< "--- oneOrMore! ---\n";
				#endif
				bool ok = false;
				bool first = false;
				do{
					ok = false;
					for(auto r : rules){
						if(r().type == "Terminal"){
							#ifdef DEBUG
							cout<< "Terminal ["<<raw_source_[cursor]<<","<<r().value<<"]"<< endl;
							#endif
							if(raw_source_[cursor] == r().value){
								ok = true;
							}
							cursor++;
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
				#ifdef DEBUG
				cout<< "Number "<<raw_source_[cursor]<< endl;	
				#endif
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
			#ifdef DEBUG
			cout<<"Exec value\n";
			#endif
			return orderedChoice({ oneOrMore(Number()), sequence({Terminal('('), rules["Expr"], Terminal(')')})})();
		};
		rules["Product"] = []()-> Sign{
			#ifdef DEBUG
			cout<<"Exec product\n";
			#endif
			return sequence({ rules["Value"], zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), rules["Value"]}))})();
		};
		rules["Sum"] = []()-> Sign{
			#ifdef DEBUG
			cout<<"Exec sum\n";
			#endif
			return sequence({ rules["Product"], zeroOrMore(sequence({orderedChoice({Terminal('+'), Terminal('-')}), rules["Product"]}))})();
		};
		rules["Expr"] = []()-> Sign{
			#ifdef DEBUG
			cout<<"Exec expr\n";
			#endif
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


	int test_counter = 0;
	bool tex( string code, function<Sign()> c,bool correct = true){
		test_counter++;
		cout<<"    - test "<<test_counter<< "\n";
		cursor = 0;
		set_source(code);
		if(!(c().execution() ^ correct)){
			cout << "     \x1b[32m"<< test_counter <<" ["<<code<<"] is Passed!\x1b[39m\n";
			return true;
		}else{
			cout << "     \033[1;31m"<< test_counter <<" ["<<code<<"] is Faild\033[0m\n";
			return false;
		}
	}
	void test(){
		cout<< "\e[96m# Test for sequence\033[0m\n";
		{
			test_counter = 0;
			tex( "abcd", sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}));
			tex( "adbc",sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}), false);
			tex( "a", sequence({Terminal('a')}));
			tex( "", sequence({Terminal('a')}), false);
		}
		// */
		cout<< "\e[96m# Test for orderedChoice\033[0m\n";
		{
			test_counter = 0;
			tex( "a", orderedChoice({Terminal('a')}));
			tex( "a", orderedChoice({Terminal('b')}), false);
			tex( "a", orderedChoice({Terminal('a'),Terminal('b')}));
			tex( "a", orderedChoice({Terminal('b'),Terminal('a')}));
			tex( "ab", orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('a'),Terminal('b')})}));
			tex( "ab", orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('b'),Terminal('b')})}), false);
			tex( "ba", orderedChoice({Terminal('a'),Terminal('b')}));
			tex( "ba", orderedChoice({sequence({Terminal('a')}),sequence({Terminal('b')})}));
		}
		// */
		cout<< "\e[96m# Test for optional\033[0m\n";
		{
			test_counter = 0;
			tex( "", optional(Terminal('a')));
			tex( "ab", sequence({optional(Terminal('a')), Terminal('b')}));
			tex( "b", sequence({optional(Terminal('a')), Terminal('b')}));
		}
		// */
		
		cout<< "\e[96m# Test for zeroOrMore\033[0m\n";
		{
			test_counter = 0;
			tex( "", zeroOrMore(Terminal('a')));
			tex( "a", zeroOrMore(Terminal('a')));
			tex( "aaaa", zeroOrMore(Terminal('a')));
			tex( "b", zeroOrMore(Terminal('a')));
			tex( "ab", zeroOrMore(Terminal('a')));
			tex( "b", sequence({ zeroOrMore(Terminal('a')),Terminal('b') }));
			tex( "c", zeroOrMore(sequence({Terminal('a'),Terminal('b')})) );
			tex( "c", sequence({ zeroOrMore(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}));
		}
		// */
		
		cout<< "\e[96m# Test for oneOrMore\033[0m\n";
		{
			test_counter = 0;
			tex( "", oneOrMore(Terminal('a')), false);
			tex( "a", oneOrMore(Terminal('a')));
			tex( "aaaa", oneOrMore(Terminal('a')));
			tex( "b", oneOrMore(Terminal('a')), false);
			tex( "ab", oneOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( "ababab", oneOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( "a", oneOrMore(sequence({Terminal('a'),Terminal('b')})), false);
			tex( "a+", oneOrMore(sequence({Terminal('a'),Terminal('b')})), false);
		}
		// */
		
		cout<< "\e[96m# Test for mix\033[0m\n";
		{
			test_counter = 0;
			tex( "1+1", sequence({ Number(), Terminal('+'), Number()}));
			tex( "1234+1", sequence({ oneOrMore(Number()), Terminal('+'), Number()}));
			tex( "1234+5678", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}));
			tex( "1234+", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
			tex( "-1234+5678", sequence({ optional(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}));
			tex( "-1234", sequence({ optional(Terminal('-')), oneOrMore(Number()) }));
			tex( "1234", sequence({ optional(Terminal('-')), oneOrMore(Number()) }));
			tex( "+1234+5678-", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
		}
		// */
		
		cout<< "\e[96m# Test for calculator\033[0m\n";
		{
			test_counter = 0;
			init();
			tex( "1+1", rules["Expr"]);
			tex( "1+", rules["Expr"], false);
			tex( "+", rules["Expr"], false);
			tex( "1", rules["Expr"]);

			tex( "1*1", rules["Product"] );
			tex( "1*", rules["Product"], false);
		}
		// */		

	}

};