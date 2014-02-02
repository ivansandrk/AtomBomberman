#include <stdio.h>
#include <string>

// Make a separate class for reading/writing to ease extensibility
class FileIO {
  public:
	explicit FileIO(const char* path);
	~FileIO();
	//int init();
	inline int getc();
	int ungetc(int c);
	int pos();
	
	// eats space (& tab) chars from input
	void skip_space();
	
	// eats input chars until newline (eats it too)
	void skip_line();
	
	// reads [_A-Za-z0-9]*
	std::string read_name();

  private:
  	const char* m_path;
	FILE* m_file;
};


enum LineType {
	ERROR,
	EMPTY,
	COMMENT,
	GROUP,
	KEY_VAL, // with a possible comment after VAL
	END_OF_INI
};

struct TokenInfo {
	std::string str;
	int pos;
	int end;
	
	TokenInfo() {}
  private:
  	TokenInfo(const TokenInfo&);
  	void operator=(const TokenInfo&);
};

struct ParsedLine {
	LineType type;
	TokenInfo group;
	TokenInfo key;
	TokenInfo val;
	
	ParsedLine() {}
  private:
  	ParsedLine(const ParsedLine&);
  	void operator=(const ParsedLine&);
};


class IniParser {
  public:
	explicit IniParser(FileIO* io);
	~IniParser();
	//int init();
	ParsedLine& parse_line();
	void parse_ini();
	

  private:
	FileIO* m_io;
	int m_line;
};
