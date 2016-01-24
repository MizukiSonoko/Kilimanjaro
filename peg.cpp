#include "peg.h"

#include <regex>
#include <stack>

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


	bool match(string s){
		cout << "s:"<< s <<endl;
		for(auto c : s){
			cout << "r[" << cursor <<"]=" << raw_source_[cursor] << " " << c << endl;
			if(raw_source_[cursor] != c)
				return false;
			cursor++;
		}
		return true;
	}
	// V <- "a" "b" V / "c" 
	bool parse_V(){
		mark();
		if(match("a")){
			if(match("b")){
				if(parse_V()){
					return true;
				}
				back();
			}else{
				return false;
			}
		}
		return match("c");
	}
	bool exec(){
		raw_source_ = "abc";
		return parse_V();
	}

};