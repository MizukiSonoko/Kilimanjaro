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
	void remove(){
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
		int execution(){
			cout<<"      - execution! "<< type << "\n";
			if(type == "sequence"){
				int start = cursor;
				int num_rule = rules.size();
				int seq_cursor = 0;
				mark();				
				#ifdef DEBUG
				cout<< "--- sequence! ["<< raw_source_[cursor]<<"]---\n";
				#endif
				for(auto r : rules){
//					cout << " loop " << r().type <<"\n";
					if(r().type == "Terminal"){
//						cout<< "--- seq! "<< raw_source_[cursor] <<" "<< r().value <<"---\n";
						if(raw_source_[cursor] == r().value){
//							cout<< "            corsor++ "<< cursor+1<< endl;
							cursor++;
							seq_cursor++;
						}
					}else{
//						cout << "aa\n";
//						cout << "execution in execution \n";
						if(r().execution() == 1){
							seq_cursor++;
						}
					}
				}
				if(num_rule == seq_cursor){
					cout << "seq success return 1\n";
					return 1;
				}
				if(start == seq_cursor){
					back();
					cout<< "           sequence back "<< cursor<< endl;
					cout<< "seq faild return 0\n";
					return 0;
				}
				back();				
				cout<< "seq faild return -1 ("<< cursor<<")\n";
				return -1;
			}else if(type == "orderedChoice"){
				#ifdef DEBUG
				cout<< "--- orderedChoice! "<< raw_source_[cursor]<<"---\n";
				#endif
				for(auto r : rules){
					mark();
					if(r().type == "Terminal"){
						if(raw_source_[cursor] == r().value){
							cout<< "            corsor++ "<< cursor+1<< endl;
							cursor++;
							return 1;
						}
					}else{
						auto res = r().execution();
						if(res == 1){ 
							return 1;
						}
					}	
					back();	
					cout<< "           orderedChoice back "<< cursor<< endl;				
				}
				return 0;
			}else if(type == "optional"){
				mark();
				#ifdef DEBUG
				cout<< "--- optional! ---\n";
				cout<< "optional ["<<raw_source_[cursor]<<"] ["<<rules[0]().value <<"]"<< endl;
				#endif
				auto sign = rules[0]();
				if(sign.type == "Terminal"){
					if(raw_source_[cursor] == sign.value){
						cout<< "            corsor++ "<< cursor+1<< endl;
						cursor++;
						cout<< "opt success return 1\n";
						remove();
						return 1;
					}
				}else{
					auto res = sign.execution();
					if(res == -1){
						back();
						cout<< "           optional back "<< cursor<< endl;
						cout<< "opt serror return -1\n";
						return -1;
					}
				}
				remove();
				cout<< "opt none return 1\n";
				return 1;
			}else if(type == "zeroOrMore"){
				#ifdef DEBUG
				cout<< "--- zeroOrMore! "<< rules[0]().type <<" ---\n";
				#endif
				auto sign = rules[0]();
				if(sign.type == "Terminal"){
					bool none = true;
					while(raw_source_[cursor] == sign.value){
						cout<< "            corsor++ "<< cursor+1<< endl;
						cursor++;
						none = false;
					}
					return 1;
				}else{
					int res;
					do{
						res = sign.execution();
					}while(res == 1);
/*					if(res == -1){
						return -1;
					}
*/					
					return 1;
				}
				return 1;				
			}else if(type == "oneOrMore"){
				mark();	
				#ifdef DEBUG
				cout<< "--- oneOrMore! "<< rules[0]().type <<" ---\n";
				#endif
				auto sign = rules[0]();
				bool none = true;
				if(sign.type == "Terminal"){
					while(raw_source_[cursor] == sign.value){
						cout<< "            corsor++ "<< cursor+1<< endl;
						cursor++;
						none = false;
					}
					if(none)
						return 0;
					return 1;
				}else{
					int res = sign.execution();
					while(res == 1){
						none = false;
						res = sign.execution();
						cout << " res: "<< res<< endl;
					}
					if(none)
						return 0;
					return 1;
				}

			}else if(type == "Number"){
				#ifdef DEBUG
				cout<< "Number "<<raw_source_[cursor]<< endl;	
				#endif
				if('0' <= raw_source_[cursor] && raw_source_[cursor] <= '9'){
					cout<< "            corsor++ "<< cursor+1<< endl;
					cursor++;
					return true;
				}
				return false;
			}else if(type == "andPredicate"){
				auto sign = rules[0]();
				if(sign.type == "Terminal"){
					if(sign.value == raw_source_[cursor]){
						return true;
					}
					return false;
				}else{
					if(sign.execution()){
						return true;
					}
					return false;
				}
			}else if(type == "endOfString"){
				cout<<"endOfString "<< cursor <<" "<< raw_source_.size() << endl;
				if(cursor == raw_source_.size()){
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
	function<Sign()> andPredicate(function<Sign()> s){
		return [s]() -> Sign{ return Sign(s, "andPredicate"); };	
	}
	Sign notPredicate();
	function<Sign()> endOfString(){
		return []() -> Sign{ return Sign("endOfString"); };
	}

	function<Sign()> Terminal(char v){
		return [v]() -> Sign{ return Sign(v, "Terminal"); };
	}
	function<Sign()> Number(){		
		return []() -> Sign{ return Sign("Number"); };
	}


	int count = 0;
	map<string, function<Sign()>> rules;
	void init(){

		rules["Product"] = []()-> Sign{
			cout<<"Exec product\n";
			return sequence({ oneOrMore(Number()), zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), oneOrMore(Number())}))})();
		};
		rules["Sum"] = []()-> Sign{
			cout<<"Exec sum\n";
			return sequence({ rules["Product"], zeroOrMore(sequence({orderedChoice({Terminal('+'), Terminal('-')}), rules["Product"]}))})();
		};
		rules["Expr"] = []()-> Sign{
			cout<<"Exec expr\n";
			return rules["Sum"]();
		};

		rules["A"] = []()-> Sign{			
			cout<<"Exec A\n";
			count++;
			return sequence({optional(sequence({Terminal('a'),Terminal('b')})), sequence({ Terminal('b'), rules["Expr"] }) })();
		};

		rules["S"] = []()-> Sign{
			if(count > 5) exit(1);
			return sequence({rules["Expr"], endOfString()})();
		};
/*
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
*/
	}

	void set_source(string s){
		raw_source_ = s;
	}

	bool exec(string n){
		set_source(n);
		cursor = 0;
		cout << "Input:"<< raw_source_ <<"\n";
		init();
		return rules["S"]().execution();
		//return A()().execution();
		//Expr()().execution();
	}


	vector<string> passed_tests;
	vector<string> faild_tests;
	int test_counter = 0;
	bool tex( string code, function<Sign()> c,bool correct = true){
		test_counter++;
		cout<<"\x1b[32m#>- test "<<test_counter<< "\x1b[39m\n";
		cursor = 0;
		set_source(code);
		cout<<"    - execute!\n";
		auto res = sequence({c, endOfString()})().execution();
		if(!((res > 0) ^ correct)){
			passed_tests.push_back("     \x1b[32m"+ to_string(test_counter) +" ["+ code +"] is Passed! ("+ to_string(res) +") \x1b[39m\n");
			return true;
		}else{
			faild_tests.push_back("     \033[1;31m"+ to_string(test_counter) +" ["+ code +"] is Faild ("+ to_string(res) +") \033[0m\n");
			return false;
		}
	}

	void test(){
		/*
		cout<< "\e[96m# Test for sequence\033[0m\n";
		{
			test_counter = 0;

			tex( "a", sequence({Terminal('a')}));
			tex( "abcd", sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}));
			tex( "ab", sequence({Terminal('a'),Terminal('b')}));
			tex( "adbc",sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}), false);
			tex( "abd",sequence({Terminal('a'),Terminal('b'),Terminal('c'),Terminal('d')}), false);
			tex( "a", sequence({Terminal('a')}));
			tex( "b", sequence({Terminal('a')}), false);
			tex( "", sequence({Terminal('a')}), false);
		}
		// */
		/*
			
		cout<< "\e[96m# Test for orderedChoice\033[0m\n";
		{
			test_counter = 0;
			tex( "a", orderedChoice({Terminal('a')}));
			tex( "a", orderedChoice({Terminal('b')}), false);
			tex( "a", orderedChoice({Terminal('a'),Terminal('b')}));
			tex( "a", orderedChoice({Terminal('b'),Terminal('a')}));
			tex( "ab", orderedChoice({sequence({Terminal('a'),Terminal('b')}),sequence({Terminal('a'),Terminal('a')})}));// x
			tex( "ab", orderedChoice({sequence({Terminal('a'),Terminal('a')}),sequence({Terminal('b'),Terminal('b')})}), false);
			tex( "ba", orderedChoice({Terminal('a'),Terminal('b')}), false);
			tex( "ba", orderedChoice({sequence({Terminal('a')}),sequence({Terminal('b')})}), false);
			tex( "_", orderedChoice({sequence({Terminal('a')}),sequence({Terminal('b')})}), false);
			tex( "abe", orderedChoice({sequence({Terminal('a'),Terminal('b'),Terminal('c')}),sequence({Terminal('a'),Terminal('b'),Terminal('d')})}), false);
		}
		// */
		
		/*
		cout<< "\e[96m# Test for optional\033[0m\n";
		{
			test_counter = 0;
			tex( "", optional(Terminal('a')));
			tex( "ab", sequence({optional(Terminal('a')), Terminal('b')}));
			tex( "b", sequence({optional(Terminal('a')), Terminal('b')}));
			tex( "a", sequence({optional(Terminal('a')), Terminal('b')}), false);
			tex( "abc", sequence({optional(Terminal('a')), Terminal('b')}), false);
			tex( "c", sequence({optional(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}));
			tex( "abc", sequence({optional(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}));
			tex( "ac", sequence({optional(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}), false);
			tex( "ab", sequence({optional(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}), false);
		}
		// */
		
		/*
		cout<< "\e[96m# Test for zeroOrMore\033[0m\n";
		{
			test_counter = 0;
			tex( "", zeroOrMore(Terminal('a')));
			tex( "a", zeroOrMore(Terminal('a')));
			tex( "aaaa", zeroOrMore(Terminal('a')));
			tex( "b", zeroOrMore(Terminal('a')), false);
			tex( "b", sequence({ zeroOrMore(Terminal('a')), Terminal('b')}));
			tex( "ab", zeroOrMore(Terminal('a')), false);
			tex( "b", sequence({ zeroOrMore(Terminal('a')),Terminal('b') }));
			tex( "c", zeroOrMore(sequence({Terminal('a'),Terminal('b')})) ,false);
			tex( "ababab", zeroOrMore(sequence({Terminal('a'),Terminal('b')})));
			tex( "aba", zeroOrMore(sequence({Terminal('a'),Terminal('b')})), false);
			tex( "c", sequence({ zeroOrMore(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}));
			tex( "abc", sequence({ zeroOrMore(sequence({Terminal('a'),Terminal('b')})), Terminal('c')}));
		}
		// */
		
		/*
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
			tex( "1+", sequence({ Terminal('1'), oneOrMore(sequence({Terminal('*'),Terminal('1')}))}), false);
			tex( "+1234+5678-", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
			tex( "123",  sequence({ zeroOrMore(sequence({Terminal('1'), Terminal('2')})), Terminal('3')}));
			tex( "1212",  sequence({ zeroOrMore(sequence({Terminal('1'), Terminal('2')}))}));
			tex( "33333",  sequence({ zeroOrMore(sequence({Terminal('1'), Terminal('2')})), Terminal('3')}), false);

			tex( "25252", sequence({oneOrMore(Number())}));
			tex( "aa", sequence({oneOrMore(Terminal('a'))}));

			auto P = sequence({ oneOrMore(Number()), zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), oneOrMore(Number())}))});
		}
		// */
		cout<< "\e[96m# Test using variable\033[0m\n";
		{

			for(auto v : faild_tests){
				cout<< v;
			}

			auto A = sequence({Terminal('a'),Terminal('a')});
			tex( "aa", A);
			auto B = orderedChoice({Terminal('a'),Terminal('b')});
			tex( "a", B);
			auto C = optional(Terminal('a'));
			tex( "a", C);
			auto D = zeroOrMore(Terminal('a'));
			tex( "aa", D);
			auto E = oneOrMore(Terminal('a'));
			tex( "aa", E);
			auto F = oneOrMore(Number());
			tex( "1234", F);
			auto G = oneOrMore(sequence({Terminal('a'),Terminal('a')}));
			tex( "aa", G);
			auto H = oneOrMore(B);
			tex( "a", H);

			auto X = sequence({oneOrMore(Terminal('a'))});
			tex( "aa", X);
		}

		/*
		cout<< "\e[96m# Test for calculator\033[0m\n";
		{
			test_counter = 0;
			init();
			tex( "1+1", rules["S"]);
			tex( "123+456", rules["S"], false);
			tex( "1*3", rules["S"], false);

		}
		// */
	}

};