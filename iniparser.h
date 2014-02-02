#include <stdio.h>
#include <string>

// Make a separate class for reading/writing to ease extensibility
class FileIO {
  public:
	explicit FileIO(const char* path);
	~FileIO();
	int init();
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
};


struct ParsedLine {
	LineType type;
	std::string group;
	std::string key;
	std::string val;
	int pos;
	int end;
};


class IniParser {
  public:
	explicit IniParser(const char* path);
	~IniParser();
	int init();
	ParsedLine parse_line();

  private:
	FileIO* m_io;
	const char* m_path;
	int m_line;
};
