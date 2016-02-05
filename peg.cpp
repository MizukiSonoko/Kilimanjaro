#include "peg.h"

#include <regex>
#include <stack>
#include <vector>

#include <cstdlib>
#include <memory>

// for debug
#include <random>

#include <functional>
#include <map>

namespace peg{

	using namespace std;

	struct Context{

		Context()
			: raw_rule_("")
			, raw_source_("")
			, cursor(0)
			, max_cursor(0)
			, route("")
			, deep(0)
		{}

		string raw_rule_;
		string raw_source_;

		int cursor;
		int max_cursor;
		stack<int> markers;

		string route;
		int deep;

		char get_current_char(){
			return raw_source_[cursor];
		}

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

		void reset(){
			cursor = 0;
			max_cursor = 0;
			route = "";
			deep = 0;
			this->clear();
		}

		void init(string t){
			this->reset();
			raw_source_ = t;			
		}

	};

	shared_ptr<Context> context;

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
		context->raw_source_ = load_file(filename);
	}

	void load_rule(string filename){
		context->raw_rule_ = load_file(filename);
	}
	
	class Sign{
		vector<function<Sign()>> rules;
	public:
		char value;
		string type;
		Sign(vector<function<Sign()>> l,string t)
			: rules(move(l))
			, type(t)
		{}

		Sign(function<Sign()> s,string t)
			: type(t)
		{
			rules.push_back(s);
		}

		Sign(char v, string t)
			: value(v)
			, type(t)
		{
			if(t != "Terminal")
				throw "Error! Invalid constructer";
		}

		Sign(string t)
			: type(t)
		{}

		Sign()
			: type("")
		{}

		int execution(){
			context->route += type + " ";
			if(type == "sequence"){
				int start = context->cursor;
				int num_rule = rules.size();
				int seq_cursor = 0;
				context->mark();
				#ifdef DEBUG
				cout<< "--- sequence! ["<< context->get_current_char()<<"]---\n";
				cout<< "rules:" << rules.size() << endl;
				#endif
				for(auto r : rules){
					auto sign = r();
					if(sign.type == "Terminal"){
						#ifdef DEBUG
						cout<< "--- seq! "<< context->get_current_char() <<" "<< sign.value <<"---\n";
						#endif
						if(context->get_current_char() == sign.value){
							context->cursor++;
							seq_cursor++;
						}
					}else{
						#ifdef DEBUG
						cout << "execution in execution \n";
						#endif
						auto exe_res =sign.execution();
						if(exe_res == 1){
							seq_cursor++;
						}else if(exe_res == -1){
							break;
						}
					}
				}

				cout <<" num:"<< num_rule <<" seq:"<< seq_cursor <<"\n";
				if(num_rule == seq_cursor){
					#ifdef DEBUG
					cout << "seq success return 1\n";
					#endif
					return 1;
				}
				
				if(start == seq_cursor){
					context->back();
					#ifdef DEBUG
					cout<< "           sequence back "<< context->cursor<< endl;
					cout<< "seq faild return 0\n";
					#endif
					return 0;
				}
				context->back();	
				#ifdef DEBUG
				cout<< "seq faild return -1 ("<< context->cursor<<")\n";
				#endif
				return -1;
			}else if(type == "orderedChoice"){
				#ifdef DEBUG
				cout<< "--- orderedChoice! "<< context->get_current_char()<<"---\n";
				#endif
				for(auto r : rules){
					context->mark();
					auto sign = r();
					if(sign.type == "Terminal"){
						if(context->get_current_char() == sign.value){
							context->cursor++;
							return 1;
						}
					}else{
						auto res = sign.execution();
						if(res == 1){ 
							return 1;
						}
					}	
					context->back();
					#ifdef DEBUG
					cout<< "           orderedChoice back "<< context->cursor<< endl;				
					#endif
				}
				return 0;
			}else if(type == "optional"){
				context->mark();
				auto sign = rules.at(0)();
				#ifdef DEBUG
				cout<< "--- optional! ---\n";
				cout<< "optional ["<< context->get_current_char() <<"] ["<<sign.type <<"]"<< endl;
				#endif
				if(sign.type == "Terminal"){
					if(context->get_current_char() == sign.value){
						context->cursor++;
						#ifdef DEBUG
						cout<< "opt success return 1\n";
						#endif
						context->remove();
						return 1;
					}
				}else{
					auto res = sign.execution();
					if(res == -1){
						context->back();
						return -1;
					}
				}
				context->remove();
				#ifdef DEBUG
				cout<< "opt none return 1\n";
				#endif
				return 1;
			}else if(type == "zeroOrMore"){
				auto sign = rules.at(0)();
				#ifdef DEBUG
				cout<< "--- zeroOrMore! "<< sign.type <<" ---\n";
				#endif
				if(sign.type == "Terminal"){
					bool none = true;
					while(context->get_current_char() == sign.value){
						#ifdef DEBUG
						cout<< "            corsor++ "<< context->cursor+1<< endl;
						#endif
						context->cursor++;
						none = false;
					}
				}else{
					context->mark();
					int res;
					do{
						res = sign.execution();
					}while(res == 1);

					if(res == -1){
						context->back();
						return -1;
					}
				}
				return 1;				
			}else if(type == "oneOrMore"){
				context->mark();	
				auto sign = rules[0]();
				#ifdef DEBUG
				cout<< "--- oneOrMore! "<< sign.type <<" ---\n";
				#endif
				bool none = true;
				if(sign.type == "Terminal"){
					while(context->get_current_char() == sign.value){
						#ifdef DEBUG
						cout<< "            corsor++ "<< context->cursor+1<< endl;
						#endif
						context->cursor++;
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
				cout<< "Number "<<context->get_current_char()<< endl;	
				#endif
				if('0' <= context->get_current_char() && context->get_current_char() <= '9'){
					context->cursor++;
					return 1;
				}
				return 0;
			}else if(type == "andPredicate"){
				auto sign = rules[0]();
				if(sign.type == "Terminal"){
					if(sign.value == context->get_current_char()){
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
				if(context->cursor == context->raw_source_.size()){
					return true;
				}
				return false;
			}
			return false;
		}
	};

	function<Sign()> sequence(vector<function<Sign()>> l){
		return [l=move(l)]() -> Sign{
			return Sign(move(l), "sequence");
		};
	}

	function<Sign()> orderedChoice(vector<function<Sign()>> l){
			return [l=move(l)]() -> Sign{
				return Sign(move(l), "orderedChoice");
			};
	}
	function<Sign()> optional(function<Sign()> s){
		return [s=move(s)]() -> Sign{
			return Sign(move(s), "optional");
		};
	}
	function<Sign()> zeroOrMore(function<Sign()> s){
		return [s=move(s)]() -> Sign{ 
			return Sign(move(s), "zeroOrMore");
		};
	}
	function<Sign()> oneOrMore(function<Sign()> s){
		return [s=move(s)]() -> Sign{ return Sign(move(s), "oneOrMore"); };
	}

	function<Sign()> andPredicate(function<Sign()> s){
		return [s=move(s)]() -> Sign{ return Sign(move(s), "andPredicate"); };	
	}

	Sign notPredicate();

	function<Sign()> endOfString(){
		return []() -> Sign{ return Sign("endOfString"); };
	}

	function<Sign()> Terminal(char v){
		return [v=move(v)]() -> Sign{ return Sign(move(v), "Terminal"); };
	}
	function<Sign()> Number(){		
		return []() -> Sign{ return Sign("Number"); };
	}

	int count = 0;
	map<string, function<Sign()>> rules;
	void set_rule(){
		rules["Value"] = []()-> Sign{
			count ++;
			if(count > 10)
				return oneOrMore(Number())();
			return  orderedChoice({
						oneOrMore(Number()),
						sequence({
							Terminal('('), rules["Expr"], Terminal(')')
						})
					})();
		};
		rules["Product"] =
			sequence({
				rules["Value"],
				zeroOrMore(
					sequence({
						orderedChoice({
							Terminal('*'), Terminal('/')
						}),
						rules["Value"]
					})
				)
			});

		rules["Sum"] =
			sequence({
				rules["Product"],
				zeroOrMore(
					sequence({
						orderedChoice({
							Terminal('+'), Terminal('-')
						}),
						rules["Product"]
					})
				)
			});

		rules["Expr"] =
			rules["Sum"];
		
		rules["S"] = []()-> Sign{
			count = 0;
			return sequence({rules["Expr"], endOfString()})();
		};
	}

	void init(){
		context = make_shared<Context>(Context());
		set_rule();
	}

	bool exec(string n){
		context->init(n);
		cout<<"Start \n";
		return rules["S"]().execution();
	}

	string test_class = "";
	int test_class_num = 0;
	vector<string> passed_tests;
	vector<string> faild_tests;
	int test_counter = 0;

	bool tex( string code, function<Sign()> c,bool correct = true){
		context->init(code);
		test_counter++;
		cout<<"\x1b[32m#>- test "<<test_counter<< " Input:["<< code <<"]\x1b[39m\n";
		cout<<"    - execute!\n";
		auto res = sequence({c, endOfString()})().execution();
		if(!((res > 0) ^ correct)){
			passed_tests.push_back("     \x1b[32m     "+ to_string(test_counter) +" ["+ code +"] :"+ context->route +" is Passed! ("+ to_string(res) +") \x1b[39m\n");
			return true;
		}else{
			faild_tests.push_back("     \033[1;31m"+ to_string(test_counter) +" ["+ code +"] :"+ context->route +" is Faild ("+ to_string(res) +") \033[0m\n");
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
			tex( "a", sequence({optional(Terminal('a'))}));
			tex( "", sequence({optional(Terminal('a'))}));
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
			tex( "1*", sequence({ Terminal('1'), oneOrMore(sequence({Terminal('*'),Terminal('1')}))}), false);
			tex( "+1234+5678-", sequence({ zeroOrMore(Terminal('-')), oneOrMore(Number()), Terminal('+'), oneOrMore(Number())}), false);
			tex( "123",  sequence({ zeroOrMore(sequence({Terminal('1'), Terminal('2')})), Terminal('3')}));
			tex( "12",  sequence({ zeroOrMore(sequence({Terminal('1'), Terminal('2')})), Terminal('3')}), false);
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
			function<Sign()> H1 = sequence({Terminal('w')});
			tex( "w", H1);

			function<Sign()> H = sequence({optional(Terminal('a'))});
			tex( "a", move(H));

			function<Sign()> O = sequence({zeroOrMore(Terminal('1'))});
			tex( "111", O);
			function<Sign()> P = sequence({oneOrMore(Number())});
			tex( "124", P);


			auto Q = sequence({zeroOrMore(Terminal('1')), Terminal('5')});
			tex( "15", Q);
			auto R = sequence({ optional(Terminal('-')), oneOrMore(Number()) });
			tex( "-1234", R);
			auto S = sequence({ optional(Terminal('-')), R});
			tex( "-1234", S);
			auto T = sequence({ optional(Terminal('-')), R});
			tex( "--1234", T);
//*/

			auto X = 
			sequence({
				optional(Terminal('a')),
				zeroOrMore(
					sequence({
						orderedChoice({Terminal('*'), Terminal('/')}),
						oneOrMore(Number())
					})
				)
			});

			tex( "a*", X, false);
		}

		cout<< "\e[96m# Test for calculator\033[0m\n";
		{
			test_counter = 0;

			tex( "1+1", rules["S"]);
			tex( "123+456", rules["S"]);
			tex( "+", rules["S"],false);
			tex( "a", rules["S"],false);

			tex( "1*", rules["S"],false);


			for(auto v : passed_tests){
				cout<< v;
			}

			cout <<"Passed test "<< passed_tests.size() - test_class_num << "/"<<( passed_tests.size() + faild_tests.size() - test_class_num*2 )<<" \n";

			for(auto v : faild_tests){
				cout<< v;
			}
			cout <<"Faild test "<< faild_tests.size() - test_class_num << "/"<<( passed_tests.size() + faild_tests.size() - test_class_num*2 )<<" \n";

		}
		// */
	}

};
