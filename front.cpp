#include "peg.h"

using namespace std;

int main(int argc, char* argv[]){

	peg::test();
	return 0;

	std::string in;
	while(std::cin >> in){
		bool res = peg::exec(in);
		if(res){
			cout << "Parse successful!!\n";
		}else{
			cout << "Parse faild!!\n";
		}
	}
	return 0;

	if(argc == 3){		
		peg::load_cpp(std::string(argv[1]));
		peg::load_rule(std::string(argv[2]));
		bool res = peg::exec(" ");
		if(res){
			cout << "Parse successful!!\n";
		}else{
			cout << "Parse faild!!\n";
		}
	}else{
		cout << "\n\033[1;31m Error!! \033[0m\n";
		exit(1);
		//throw std::runtime_error("\n\033[1;31msharo: error: no input files\033[0m\n");
	}
	return 0;
}
