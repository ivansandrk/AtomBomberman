#include <stdio.h>
#include <string>
#include <map>
#include <set>

/* ************ IMPORTANT NOTES, USAGE, RESTRICTIONS, ETC. ************
 *
 * - max length of group or key is 128 chars
 * - max length of val is 65536 chars -- if you need more, change the value
 * - key/val pair without a group isn't allowed; it is silently ignored when reading
 * - set_string on an existing key/val changes the val
 * - **** if key/val doesn't exist, it adds it as first in the group
 * - **** if group doesn't exist, it fails
 * - save_ini reads the whole file into memory; memory may be an issue
 * - max ini file size is 2^31-1 (int) ~~ 2GB
 * - skip_line handles only linux newlines (\n) - fix this? TODO
 * - renaming keys isn't allowed; only adding or deleting
 *
 * PROBLEM: when something new gets saved, all following indexes are invalid
 */

// TODO: this define needs to be removed
#define USE_SDL_ZZIP


// a separate class for reading/writing to ease extensibility
// if you want to use some other source to read from (eg. SDL_RWops)
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
	// write up to pos
	int write(int pos);
	// skip n bytes
	int skip(int n);
	void write_str(std::string str);
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
	int orig_len;
	
	TokenInfo() {}
	
	bool operator<(const TokenInfo& o) const {
		if (pos != o.pos)
			return pos < o.pos;
		return str < o.str;
	}
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
	
	bool operator <(const EntryInfo& o) const {
		if (key.pos != o.key.pos)
			return key.pos < o.key.pos;
		return key.str < o.key.str;
	}
};
typedef std::map<std::string, EntryInfo> EntryInfoMap;


struct DirtySetCmp {
	bool operator() (const EntryInfo* A, const EntryInfo* B) const
	{
		if (A->key.pos != B->key.pos)
			return A->key.pos < B->key.pos;
		return A->key.str < B->key.str;
	}
};
typedef std::set<EntryInfo*, DirtySetCmp> DirtySet;


struct GroupInfo {
	TokenInfo group;
	int first_line_pos;
	EntryInfoMap entries;
	
	bool operator <(const GroupInfo& o) const {
		if (group.pos != o.group.pos)
			return group.pos < o.group.pos;
		return group.str < o.group.str;
	}
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
	
	// used for debugging purposes
	void print_ini();
	
  private:
	ParsedLine& parse_line();
  	
	FileIO* m_io;
	GroupInfoMap m_groups;
	DirtySet m_dirty;
	int last_pos_in_file;
	
	std::string m_error;
};


// helper functions
#ifdef USE_SDL_ZZIP
IniParser* ParseZZIP(const char* path);
#endif
IniParser* ParseFILE(const char* path);
