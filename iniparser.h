#include <stdio.h>
#include <string>
#include <map>
#include <set>

// a separate class for reading/writing to ease extensibility
class FileIO {
  public:
	explicit FileIO(const char* path);
	~FileIO();
	
	std::string string_id();
	
	inline int getc();
	int ungetc(int c);
	int pos();
	
	// eats space (& tab) chars from input
	void skip_space();
	
	// eats input chars until newline (eats it too)
	void skip_line();
	
	// reads group/key [_A-Za-z0-9]*
	std::string read_name();
	
	// reads val [^\s;\[\]]*
	std::string read_val();
	
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
  	//TokenInfo(const TokenInfo&);
  	//void operator=(const TokenInfo&);
};

struct ParsedLine {
	LineType type;
	TokenInfo group;
	TokenInfo key;
	TokenInfo val;
	std::string error;
	
	ParsedLine() {}
  private:
  	ParsedLine(const ParsedLine&);
  	void operator=(const ParsedLine&);
};

struct EntryInfo {
	TokenInfo key;
	TokenInfo val;
};
typedef std::map<std::string, EntryInfo> EntryInfoMap;

struct GroupInfo {
	TokenInfo group;
	int first_line_pos;
	EntryInfoMap entries;
};
typedef std::map<std::string, GroupInfo>  GroupInfoMap;
typedef std::set<EntryInfo*> DirtySet;

class IniParser {
  public:
	explicit IniParser(FileIO* io);
	~IniParser();
	ParsedLine& parse_line();
	int parse_ini();
	
  private:
	FileIO* m_io;
	GroupInfoMap m_groups;
	DirtySet m_dirty;
};
