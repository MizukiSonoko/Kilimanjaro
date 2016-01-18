#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[]){
	if(argc == 2){
		lexser::loader(std::string(argv[1]));
		lexser::lexser();
	}else if(argc == 3){
		
		std::cout<<"Load file["<< argv[1] <<"]\n";		
		lexser::loader(std::string(argv[1]));
		lexser::lexser();
		auto tokens = lexser::tokens();
		std::cout<<"Done!\n";

		std::cout<<"Load file["<< argv[2] <<"]\n";
		lexser::loader(std::string(argv[2]));
		lexser::lexser();
		auto ruleTokens = lexser::tokens();
		std::cout<<"Done!\n";

		parser::loadRule(ruleTokens);
		parser::parser(tokens);

	}else{
		throw std::runtime_error("\n\033[1;31msharo: error: no input files\033[0m\n");
	}
	return 0;
}
