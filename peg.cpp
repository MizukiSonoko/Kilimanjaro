#include "peg.h"

#include <regex>
#include <stack>
#include <vector>

#include <cstdlib>

// for debug
#include <random>

#include <functional>
#include <map>

namespace peg{
	using namespace std;

	string raw_rule_;
	string raw_source_;

	string route = "";
	int deep = 0;
	string git(string type){
		string res = 0;
		for(int i =0;i<deep*2; i++){
			res += " ";
		}
		return res + type; 
	}

	void log(string s);

	// for debug
	string uuid(){

		string ascii = "abcdefghijklnmopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ0123456789~!@#$^&*()_+";

		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_int_distribution<int> pos(0,ascii.size()-1);

		string res = "";
		for(int i = 0;i<16;i++)
			res += ascii[i];
		return res;
	}

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

	void clear(){
		while(!markers.empty())
			markers.pop();
	}


	class Sign{
		vector<function<Sign()>> rules;
	public:
		string id;
		char value;
		string type;
		Sign(initializer_list<function<Sign()>> l,string t):
			rules(l),type(t){
				id = uuid();
				cout<< " construct :"<< id <<" size:" << l.size()<<" type:"<< t<< endl;
		}

		Sign(function<Sign()> s,string t):
			rules({s}), type(t){
				id = uuid();
				cout<< " construct:"<< id <<" type:"<< t<< endl;				
			}

		Sign(char v, string t):value(v),type(t){
			id = uuid();
			cout<< " construct:"<< id <<" type:"<< t<< endl;
		}

		Sign(string t):type(t){}

		Sign(){
			type = "";
		}
		int execution(){
			route += type + " ";
			
			cout<<"      - execution! "<< type <<" id:"<< id << "\n";
			if(type == "sequence"){
				int start = cursor;
				int num_rule = rules.size();
				int seq_cursor = 0;
				mark();
				#ifdef DEBUG
				cout<< "--- sequence! ["<< raw_source_[cursor]<<"]---\n";
				cout<< "rules:" << rules.size() << endl;
				#endif
				for(auto r : rules){
					auto sign = r();
					#ifdef DEBUG
					cout << " loop " << sign.type <<"\n";
					#endif
					if(sign.type == "Terminal"){
						#ifdef DEBUG
						cout<< "--- seq! "<< raw_source_[cursor] <<" "<< sign.value <<"---\n";
						#endif
						if(raw_source_[cursor] == sign.value){
							#ifdef DEBUG
							cout<< "            corsor++ "<< cursor+1<< endl;
							#endif
							cursor++;
							seq_cursor++;
						}
					}else{
						#ifdef DEBUG
						cout << "execution in execution \n";
						#endif
						if(sign.execution() == 1){
							seq_cursor++;
						}
					}
				}
				if(num_rule == seq_cursor){
					#ifdef DEBUG
					cout << "seq success return 1\n";
					#endif
					return 1;
				}
				if(start == seq_cursor){
					back();
					#ifdef DEBUG
					cout<< "           sequence back "<< cursor<< endl;
					cout<< "seq faild return 0\n";
					#endif
					return 0;
				}
				back();				
				#ifdef DEBUG
				cout<< "seq faild return -1 ("<< cursor<<")\n";
				#endif
				return -1;
			}else if(type == "orderedChoice"){
				#ifdef DEBUG
				cout<< "--- orderedChoice! "<< raw_source_[cursor]<<"---\n";
				#endif
				for(auto r : rules){
					mark();
					auto sign = r();
					if(sign.type == "Terminal"){
						if(raw_source_[cursor] == sign.value){
							#ifdef DEBUG
							cout<< "            corsor++ "<< cursor+1<< endl;
							#endif
							cursor++;
							return 1;
						}
					}else{
						auto res = sign.execution();
						if(res == 1){ 
							return 1;
						}
					}	
					back();
					#ifdef DEBUG
					cout<< "           orderedChoice back "<< cursor<< endl;				
					#endif
				}
				return 0;
			}else if(type == "optional"){
				mark();
				auto sign = rules[0]();
				#ifdef DEBUG
				cout<< "--- optional! ---\n";
				cout<< "optional ["<<raw_source_[cursor]<<"] ["<<sign.type <<"]"<< endl;
				#endif
				if(sign.type == "Terminal"){
					if(raw_source_[cursor] == sign.value){
						#ifdef DEBUG
						cout<< "            corsor++ "<< cursor+1<< endl;
						#endif
						cursor++;
						#ifdef DEBUG
						cout<< "opt success return 1\n";
						#endif
						remove();
						return 1;
					}
				}else{
					auto res = sign.execution();
					if(res == -1){
						back();
						#ifdef DEBUG
						cout<< "           optional back "<< cursor<< endl;
						cout<< "opt serror return -1\n";
						#endif
						return -1;
					}
				}
				remove();
				#ifdef DEBUG
				cout<< "opt none return 1\n";
				#endif
				return 1;
			}else if(type == "zeroOrMore"){
				auto sign = rules[0]();
				#ifdef DEBUG
				cout<< "--- zeroOrMore! "<< sign.type <<" ---\n";
				#endif
				if(sign.type == "Terminal"){
					bool none = true;
					while(raw_source_[cursor] == sign.value){
						#ifdef DEBUG
						cout<< "            corsor++ "<< cursor+1<< endl;
						#endif
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
				auto sign = rules[0]();
				#ifdef DEBUG
				cout<< "--- oneOrMore! "<< sign.type <<" ---\n";
				#endif
				bool none = true;
				if(sign.type == "Terminal"){
					while(raw_source_[cursor] == sign.value){
						#ifdef DEBUG
						cout<< "            corsor++ "<< cursor+1<< endl;
						#endif
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
						#ifdef DEBUG
						cout << " res: "<< res<< endl;
						#endif
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
					#ifdef DEBUG
					cout<< "            corsor++ "<< cursor+1<< endl;
					#endif
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
				#ifdef DEBUG
				cout<<"endOfString "<< cursor <<" "<< raw_source_.size() << endl;
				#endif
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


	string test_class = "";
	int test_class_num = 0;
	vector<string> passed_tests;
	vector<string> faild_tests;
	int test_counter = 0;
	bool tex( string code, function<Sign()> c,bool correct = true){
		test_counter++;
		cout<<"\x1b[32m#>- test "<<test_counter<< " Input:["<< code <<"]\x1b[39m\n";
		cursor = 0;
		set_source(code);
		clear();
		cout<<"    - execute!\n";
		route = "";
		deep = 0;
		auto res = sequence({c, endOfString()})().execution();
		if(!((res > 0) ^ correct)){
			passed_tests.push_back("     \x1b[32m     "+ to_string(test_counter) +" ["+ code +"] :"+ route +" is Passed! ("+ to_string(res) +") \x1b[39m\n");
			return true;
		}else{
			faild_tests.push_back("     \033[1;31m"+ to_string(test_counter) +" ["+ code +"] :"+ route +" is Faild ("+ to_string(res) +") \033[0m\n");
			return false;
		}
	}

	void test_init(){
		cout<< "\e[96m# Test "<< test_class <<" \033[0m\n";
		passed_tests.push_back("     \e[35;1m "+test_class + " start! \x1b[39m\n");
		faild_tests.push_back("     \e[35;1m "+test_class + " start! \x1b[39m\n");
		test_class_num++;
	}

	
	void test(){

		test_class = "Sequence";
		test_init();
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
			tex( "a", sequence({sequence({sequence({sequence({sequence({Terminal('a')})})})})}));
		}
		// */
		test_class = "OrderedChoice";
		test_init();
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
		
		test_class = "Optional";
		test_init();
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
		
		
		test_class = "ZeroOrMore";
		test_init();
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
		
		test_class = "OneOrMore";
		test_init();
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
		
		test_class = "Mix";
		test_init();
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

		}
		// */
		test_class = "using variables";
		test_init();
		{
			test_counter = 0;
			
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
			auto H = sequence({optional(Terminal('a'))});
			tex( "a", H);

			auto O = sequence({zeroOrMore(Terminal('1'))});
			tex( "123", O);
			auto P = sequence({oneOrMore(Number())});
			tex( "124", P);
			auto Q = sequence({zeroOrMore(Terminal('1')), Terminal('5')});
			tex( "15", Q);
			auto R = sequence({optional(Terminal('a')), oneOrMore(Number())});
			tex( "a26", R);

			//auto X = sequence({optional(Terminal('a')), zeroOrMore(sequence({orderedChoice({Terminal('*'), Terminal('/')}), oneOrMore(Number())}))});
			//tex( "111", X);

			for(auto v : passed_tests){
				cout<< v;
			}
			cout <<"Passed test "<< passed_tests.size() - test_class_num << "/"<<( passed_tests.size() + faild_tests.size() - test_class_num*2 )<<" \n";

			for(auto v : faild_tests){
				cout<< v;
			}
			cout <<"Faild test "<< faild_tests.size() - test_class_num << "/"<<( passed_tests.size() + faild_tests.size() - test_class_num*2 )<<" \n";
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