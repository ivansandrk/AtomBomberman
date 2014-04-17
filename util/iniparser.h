#include <stdio.h>
#include <string>
#include <map>
#include <set>


// a separate class for reading/writing to ease extensibility
// if you want to use some other source to read from (ie. SDL_RWops)
// just inherit this class and write virtual methods

class FileIO {
  public:
  	// **** methods to override
	explicit FileIO(const char* path);
	virtual ~FileIO();
	virtual int init();
	
	virtual std::string id();
	
	virtual int getc();
	virtual int ungetc(int c);
	virtual int pos();
	// **** end of methods to override
	
	std::string get_error();
	
	void skip_space();
	void skip_line();
	
	std::string read_name();
	std::string read_val();
	
  protected:
  	const char* m_path;
	FILE* m_file;
	
	std::string m_error;
	
  	char m_buf[4096];
  	int m_read;
  	int m_pos;
  	int m_done;
};

#ifdef USE_SDL_ZZIP
#include "SDL_rwops_zzip.h"
class RWIO : public FileIO {
  public:
	explicit RWIO(const char* path);
	virtual ~RWIO();
	virtual int init();
	
	virtual int getc();
	virtual int ungetc(int c);
	virtual int pos();
	
  protected:
	SDL_RWops* m_data;
};
#endif


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
typedef std::set<EntryInfo*> DirtySet;

struct GroupInfo {
	TokenInfo group;
	int first_line_pos;
	EntryInfoMap entries;
};
typedef std::map<std::string, GroupInfo> GroupInfoMap;

class IniParser {
  public:
	explicit IniParser(FileIO* io);
	~IniParser();
	
	std::string get_error();
	FileIO* get_io();
	
	int parse_ini();
	const char* get_string(const char* group, const char* key, const char* def = NULL);
	int get_int(const char* group, const char* key, int def);
	float get_float(const char* group, const char* key, float def);
	
  private:
	ParsedLine& parse_line();
  	
	FileIO* m_io;
	GroupInfoMap m_groups;
	DirtySet m_dirty;
	std::string m_error;
};


// helper functions
#ifdef USE_SDL_ZZIP
FileIO* OpenRW(const char* path);
#endif
FileIO* OpenFile(const char* path);
IniParser* ParseFile(FileIO* io);
