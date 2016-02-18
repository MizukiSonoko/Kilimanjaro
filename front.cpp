#include "lexer.h"
#include "parser.h"

int main(int argc, char* argv[]){
	if(argc == 2){
		std::cout<<"Load file["<< argv[1] <<"]\n";		
		lexser::loader(std::string(argv[1]));
		lexser::lexser();
		auto tokens = lexser::tokens();
		std::cout<<"Done!\n---- tokens ----\n";
		for(auto t : tokens){
			std::cout<< t.value() <<" "<<t.type()<< std::endl;
		}
		std::cout<<"---- tokens ----\n";
		parser::parser(tokens);

	}else{
		exit(1);
		//throw std::runtime_error("\n\033[1;31msharo: error: no input files\033[0m\n");
	}
	return 0;
}
