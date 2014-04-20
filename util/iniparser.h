#include <stdio.h>
#include <string>
#include <map>
#include <set>

/* ************ IMPORTANT NOTES, USAGE, RESTRICTIONS, ETC. ************
 *
 * - max length of group or key is 2048 chars
 * - max length of val is 2048 chars -- is this enough?
 * - key/val pair without a group isn't allowed; it is silently ignored when reading
 * - set_string on an existing key/val changes the val
 * - **** if key/val doesn't exist, it adds it as first in the group
 * - **** if group doesn't exist, it fails
 * - save_ini reads the whole file into memory; memory may be an issue
 */

// TODO: this define needs to be removed
#define USE_SDL_ZZIP


// a separate class for reading/writing to ease extensibility
// if you want to use some other source to read from (ie. SDL_RWops)
// just inherit this class and write virtual methods

class FileIO {
  public:
  	// **** methods to override ****
	explicit FileIO(const char* path);
	virtual ~FileIO();
	virtual int init_reading();
	virtual void finish_reading();
	
	virtual std::string id();
	
	virtual int getc();
	virtual int ungetc(int c);
	virtual int pos();
	// **** end of methods to override ****
	
	// only available for FILE; no ZZIP support
	int init_writing();
	void finish_writing();
	// write/skip up to 'pos'
	int write(int pos);
	int skip(int pos);
	// ----------------------------------------
	
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
  	
  	char* m_file_buf;
  	int m_file_len;
};

#ifdef USE_SDL_ZZIP
#include "SDL_rwops_zzip.h"
class RWIO : public FileIO {
  public:
	explicit RWIO(const char* path);
	virtual ~RWIO();
	virtual int init_reading();
	virtual void finish_reading();
	
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

// sort the DirtySet by ascending pos
struct DirtySetCmp {
	bool operator() (const EntryInfo* A, const EntryInfo* B) const
	{
//#define CMP(x, y) do {if ((x) != (y)) return (x) < (y);} while(0)
		if (A->key.pos != B->key.pos)
			return A->key.pos < B->key.pos;
		return A->key.end < B->key.end;
	}
};
typedef std::set<EntryInfo*, DirtySetCmp> DirtySet;

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
	
	int parse_ini();
	int write_ini();
	
	const char* get_string(const char* group, const char* key, const char* def = NULL);
	int get_int(const char* group, const char* key, int def);
	float get_float(const char* group, const char* key, float def);
	
	int set_string(const char* group, const char* key, const char* val);
	
  private:
	ParsedLine& parse_line();
  	
	FileIO* m_io;
	GroupInfoMap m_groups;
	DirtySet m_dirty;
	
	std::string m_error;
};


// helper functions
#ifdef USE_SDL_ZZIP
IniParser* ParseZZIP(const char* path);
#endif
IniParser* ParseFILE(const char* path);
