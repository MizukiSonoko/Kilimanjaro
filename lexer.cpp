#include "lexer.h"

namespace lexser{
	using namespace std;

	namespace decide{
		bool isNumber(char c){
	        return c >= '0' && c <= '9'?
		        true : false;
		}
		bool isLetter(char c){
			return (c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'z')?
					true : false;
		}
		bool isAlphanumeric(char c){
			return isLetter(c) || isNumber(c);
		}

		void test(){
			assert( isNumber('0') );
			assert( isNumber('9') );
			assert(!isNumber('a') );

			assert( isLetter('a') );
			assert( isLetter('A') );
			assert(!isLetter('0') );

			assert( isAlphanumeric('0') );
			assert( isAlphanumeric('a') );
			assert( isAlphanumeric('A') );
			assert(!isAlphanumeric('*') );
		}
	};

	bool loader(string filename){
		std::ifstream ifs( filename, std::ios::in | std::ios::binary);
		if(ifs.fail()){
			throw std::runtime_error("\n\033[1;31mNo such file or directory \""+ filename +"\"\033[0m\n");
		}
		ifs.seekg( 0, std::ios::end);
		int pos = ifs.tellg();
		ifs.seekg( 0, std::ios::beg);

		vector<char> buf(pos);
		ifs.read(buf.data(), pos);
		rowString_ = std::move(string(buf.begin(),buf.end()));
		return true;
	}
	
	void lexser(){
		int status = 0;
    	string buffer = "";
    	char next = 0;
    	char c;
        for(int i = 0;i < rowString_.size();){
        	c = rowString_[i]; 
        	switch(status){
        		case 1:
	                if(c == '"'){
	                    tokens_.push_back(Token(Token::NAME,buffer));
	                    buffer = "";
	                    status = 0;
	                }else{
		                buffer += c;
		            }
	                i++;
	                break;
	            case 2:
	            	// ============================ //
	            	//  とりあえず10e+1等の表記は除外　 //
	            	// ============================ //
	            	if(!decide::isNumber(c) && c != '.'){
	            		if(buffer.find(".", 0) != string::npos){
		                    tokens_.push_back(Token(Token::NUMBER,buffer));
		                }else{
		                    tokens_.push_back(Token(Token::REALNUMBER,buffer));
		                }
	                    buffer = "";
	                    status = 0;
	            	}else if(decide::isNumber(c)){
	            		buffer += c;
	            		i++;
	            	}
	            	break;
	            case 3:
	            	if(decide::isAlphanumeric(c)){
	                    buffer += c;
	                    i++;
	            	}else{
	                    tokens_.push_back(Token(Token::IDENTIFIER,buffer));
	                    buffer = "";
	                    status = 0;
	            	}
	            	break;
	            case 0:
		            if(decide::isNumber(c)){
                		status = 2;
                	}else if(decide::isLetter(c)){
                		status = 3;
                	}else{
			            switch(c){
			                case ' ': case '\t':
			                case '\n':case '\r':
			    		    	break;
			                case '"':
			                	status = 1;
			                	break;  
			                case '(':
			                    tokens_.push_back(Token(Token::LPARENT,"("));
			                    break;
			                case ')':
			                    tokens_.push_back(Token(Token::RPARENT,")"));
			                    break;
			                case ']':
			                    tokens_.push_back(Token(Token::RBRACKET,"]"));
			                    break;
			                case '[':
			                    tokens_.push_back(Token(Token::LBRACKET,"["));
			                    break;
			                case '>':
			                    tokens_.push_back(Token(Token::RABRACKET,">"));
			                    break;
			                case '<':
			                    tokens_.push_back(Token(Token::LABRACKET,"<"));
			                    break;
			                case ';':
			                    tokens_.push_back(Token(Token::SEMICOLON,";"));
			                    break;
			                case ':':
			                    tokens_.push_back(Token(Token::COLON,":"));
			                    break;
			                case ',':
			                    tokens_.push_back(Token(Token::COMMA,","));
			                    break;
			                case '.':
			                    tokens_.push_back(Token(Token::PERIOD,"."));
			                    break;
			                case '^':
			                    tokens_.push_back(Token(Token::CARET,"^"));                    
			                    break;
			                case '@':
			                    tokens_.push_back(Token(Token::AT_SIGN,"@"));
			                    break;                    
			                case '=':
			                    tokens_.push_back(Token(Token::EQUAL,"="));
			                    break;
			                case '+':
			                    tokens_.push_back(Token(Token::OPE_ADD,"+"));
			                    break;
			                case '-':
			                    tokens_.push_back(Token(Token::OPE_SUB,"-"));
			                    break;
			                case '*':
			                    tokens_.push_back(Token(Token::OPE_MUL,"*"));
			                    break;
			                case '/':
			                    tokens_.push_back(Token(Token::OPE_DIV,"/"));
			                    break;
			                case '!':
			                    tokens_.push_back(Token(Token::EXCLAMATION,"!"));
			                    break;
			                default:
		                		cerr << "Error! [" << c << "]\n";
			            }
		            	i++;
	            	}		        
            }
        }
        tokens_.push_back(Token(Token::FIN,"<FIN>"));        
	}

	void test(){
		for(auto v : tokens_){
			cout<<"<"<< v.type() << "," << v.value() << ">\n";
		}
	}
};

int main(int argc, char* argv[]){
	if(argc == 2){
		lexser::loader(std::string(argv[1]));
		lexser::lexser();
		lexser::test();
	}else{
		throw std::runtime_error("\n\033[1;31mclang: error: no input files\033[0m\n");
	}
	return 0;
}