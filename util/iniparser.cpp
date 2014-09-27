#include "iniparser.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include <ctype.h>
#include <sstream>
#include <algorithm>



// constants
const char kGroupBegin   = '[';
const char kGroupEnd     = ']';
const char kCommentBegin = ';';
const char kKeyValDelim  = '=';


FileIO::FileIO(const char* path)
{
	m_path = path;
	m_file = NULL;
	m_read = 0;
	m_pos  = 0;
	m_done = 0;
	m_file_buf = NULL;
}

FileIO::~FileIO()
{
	finish_reading();
	finish_writing();
}

int FileIO::init_reading()
{
	m_file = fopen(m_path, "r");
	if (!m_file) {
		m_error = "Unable to open " + id() + ": " + strerror(errno);
		return -1;
	}
	return 0;
}

void FileIO::finish_reading()
{
	if (m_file) {
		fclose(m_file);
		m_file = NULL;
	}
}

int FileIO::init_writing()
{
	m_file = fopen(m_path, "r");
	if (!m_file) {
		m_error = "Unable to open " + id() + ": " + strerror(errno);
		return -1;
	}
	
	fseek(m_file, 0, SEEK_END);
	m_file_len = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	
	m_file_buf = (char*)malloc(m_file_len);
	if (!m_file_buf) {
		m_error = "Unable to malloc memory for file " + id();
		return -1;
	}
	
	fread(m_file_buf, m_file_len, 1, m_file);
	fclose(m_file);
	
	m_file = fopen(m_path, "w");
	if (!m_file) {
		m_error = "Unable to open " + id() + ": " + strerror(errno);
		return -1;
	}
	
	m_pos = 0;
	
	return 0;
}

void FileIO::finish_writing()
{
	if (m_file) {
		fclose(m_file);
		m_file = NULL;
	}
	
	if (m_file_buf) {
		free(m_file_buf);
		m_file_buf = NULL;
	}
}

int FileIO::write(int pos)
{
	int to_write = std::min(pos, m_file_len) - m_pos;
	if (pos == -1) {
		to_write = m_file_len - m_pos;
	}
	
	fwrite(&m_file_buf[m_pos], to_write, 1, m_file);
	m_pos += to_write;
	
	if (m_pos == m_file_len) {
		return -1;
	}
	return 0;
}

int FileIO::skip(int n)
{
	m_pos += n;
	//m_pos = std::min(pos, m_file_len);
	m_pos = std::min(m_pos, m_file_len);
	
	if (m_pos == m_file_len) {
		return -1;
	}
	return 0;
}

void FileIO::write_str(std::string str)
{
	fwrite(str.c_str(), str.length(), 1, m_file);
}

std::string FileIO::id()
{
	return std::string(m_path);
}

std::string FileIO::get_error()
{
	return m_error;
}

int FileIO::getc()
{
	if (m_pos >= m_read) {
		m_read = fread(m_buf, 1, sizeof(m_buf), m_file);
		m_done += m_pos;
		m_pos = 0;
		if (m_read == 0)
			return -1;
	}
	return m_buf[m_pos++];
}

int FileIO::ungetc(int c)
{
	if (m_pos > 0)
		m_pos--;
	return c;
}

int FileIO::pos()
{
	return m_done + m_pos;
}

void FileIO::skip_space()
{
	int c;
	while (c = getc(), c == ' ' || c == '\t')
		continue;
	ungetc(c);
}

void FileIO::skip_line()
{
	int c;
	while (c = getc(), c != EOF && c != '\n' && c != '\r')
		continue;
	if (c == EOF) {
		return;
	}
	c = getc();
	if (c != '\r' && c != '\n') {
		ungetc(c);
	}
}

static int isname(char c)
{
	return c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
	                || (c >= '0' && c <= '9');
}

static int isval(char c)
{
	return c != kCommentBegin && c != EOF && c != '\n' && c != '\r';
}

// reads group/key [_A-Za-z0-9]*
std::string FileIO::read_name()
{
	int c, pos=0;
	char buf[128];
	
	while (c = getc(), isname(c) && pos < (int)sizeof(buf))
		buf[pos++] = c;
	ungetc(c);
	
	return std::string(buf, buf+pos);
}

// reads val [^\s;]*
std::string FileIO::read_val()
{
	int c, pos=0;
	// used for parsing things like "abc \"def\"\\ghi" -> abc "def"\ghi
	int quoted = 0, escaped = 0;
	char buf[65536];
	
	c = getc();
	if (c == '"') {
		while (pos+1 < (int)sizeof(buf)) {
			c = getc();
			if (!escaped) {
				if (c == '\\') {
					escaped = 1;
				}
				else if (c == '"') {
					break;
				}
				else {
					buf[pos++] = c;
				}
			}
			else {
				escaped = 0;
				if (c != '\\' && c != '"') {
					buf[pos++] = '\\';
				}
				buf[pos++] = c;
			}
		}
	}
	else {
		ungetc(c);
		while (c = getc(), isval(c) && pos < (int)sizeof(buf))
			buf[pos++] = c;
		ungetc(c);
		while (buf[pos-1] == ' ' || buf[pos-1] == '\t')
			pos--;
	}
	
	return std::string(buf, buf+pos);
}

IniParser::IniParser(FileIO* io)
{
	m_io = io;
}

IniParser::~IniParser()
{
	delete m_io;
}

ParsedLine& IniParser::parse_line()
{
	int c;
	static ParsedLine parsed_line;
	
	m_io->skip_space();
	c = m_io->getc();
	
	switch (c) {
	case '\r':
	case '\n':
		m_io->ungetc(c);
		m_io->skip_line();
		parsed_line.type = EMPTY;
		return parsed_line;
	
	case EOF:
		parsed_line.type = END_OF_INI;
		return parsed_line;
	
	case kCommentBegin:
		parsed_line.type = COMMENT;
		m_io->skip_line();
		return parsed_line;
	
	case kGroupBegin:
		parsed_line.type = GROUP;
		m_io->skip_space();
		parsed_line.group.pos = m_io->pos();
		parsed_line.group.str = m_io->read_name();
		parsed_line.group.orig_len = parsed_line.group.str.length();
		m_io->skip_space();
		if (m_io->getc() != kGroupEnd) {
			parsed_line.error = std::string("expected '")+kGroupEnd+"'";
			parsed_line.type = ERROR;
			return parsed_line;
		}
		m_io->skip_line();
		return parsed_line;
	
	default:
		if (!isname(c))
			break;
		
		parsed_line.type = KEY_VAL;
		m_io->ungetc(c);
		
		// read key
		parsed_line.key.pos = m_io->pos();
		parsed_line.key.str = m_io->read_name();
		parsed_line.key.orig_len = parsed_line.key.str.length();
		m_io->skip_space();
		if (m_io->getc() != kKeyValDelim) {
			parsed_line.error = std::string("expected '")+kKeyValDelim+"'";
			parsed_line.type = ERROR;
			return parsed_line;
		}
		
		// read val
		m_io->skip_space();
		parsed_line.val.pos = m_io->pos();
		parsed_line.val.str = m_io->read_val();
		parsed_line.val.orig_len = parsed_line.val.str.length();
		m_io->skip_line();
		
		return parsed_line;
	}
	
	parsed_line.error = std::string("expected '")+kGroupBegin+"' or key";
	parsed_line.type = ERROR;
	return parsed_line;
}

int IniParser::parse_ini()
{
	int n_line = 0;
	int last_pos = 0;
	GroupInfo *group_info = NULL;
	EntryInfo *entry_info = NULL;
	
	if (m_io->init_reading() == -1) {
		m_error = m_io->get_error();
		return -1;
	}
	
	for (ParsedLine& l = parse_line(); l.type != END_OF_INI; parse_line()) {
		switch (l.type) {
		case ERROR: {
			std::stringstream ss;
			ss << "Error parsing " << m_io->id() << " at line " << n_line+1
			   << ", pos " << m_io->pos()-last_pos << " (" << l.error << ")";
			m_error = ss.str();
			return -1;
		}
		
		case EMPTY:
		case COMMENT:
		case END_OF_INI: break;
		
		case GROUP:
			// constructor activated on accessing key
			group_info = &m_groups[l.group.str];
			group_info->first_line_pos = m_io->pos();
			group_info->group = l.group;
			break;
		
		case KEY_VAL:
			// key/val pair without a group isn't allowed
			if (!group_info)
				break;
			entry_info = &group_info->entries[l.key.str];
			entry_info->key = l.key;
			entry_info->val = l.val;
			break;
		}
		
		last_pos = m_io->pos();
		n_line++;
	}
	
	last_pos_in_file = m_io->pos();
	m_io->finish_reading();
	
	return 0;
}

std::string IniParser::get_error()
{
	return m_error;
}

const char* IniParser::get_string(const char* group, const char* key, const char* def)
{
	if (!group || !key)
		return def;
	
	GroupInfoMap::iterator g = m_groups.find(group);
	if (g == m_groups.end())
		return def;
	
	EntryInfoMap::iterator e = g->second.entries.find(key);
	if (e == g->second.entries.end())
		return def;
	
	return e->second.val.str.c_str();
}

int IniParser::get_int(const char* group, const char* key, int def)
{
	const char *str = get_string(group, key);
	if (!str)
		return def;
	return strtol(str, 0, 0);
}

float IniParser::get_float(const char* group, const char* key, float def)
{
	const char *str = get_string(group, key);
	if (!str)
		return def;
	return strtof(str, 0);
}

int IniParser::set_string(const char* group, const char* key, const char* val)
{
	// val == NULL -> remove key/val ?
	
	if (!group || !key) {
		m_error = "set_string called with " + std::string(!group ? "group" : "key") + "=NULL";
		return -1;
	}
	
	GroupInfoMap::iterator g = m_groups.find(group);
	if (g == m_groups.end()) {
		m_error = "Group [" + std::string(group) + "] doesn't exist";
		return -1;
	}
	
	EntryInfoMap::iterator e = g->second.entries.find(key);
	if (e == g->second.entries.end()) {
		// TODO: add key/val
		//m_error = "Key [" + std::string(key) + "] doesn't exist. TODO: Implement adding keys!";
		//return -1;
		
		// case GROUP:
		// 	// constructor activated on accessing key
		// 	group_info = &m_groups[l.group.str];
		// 	group_info->first_line_pos = m_io->pos();
		// 	group_info->group = l.group;
		// 	break;
		
		// case KEY_VAL:
		// 	// key/val pair without a group isn't allowed
		// 	if (!group_info)
		// 		break;
		// 	entry_info = &group_info->entries[l.key.str];
		// 	entry_info->key = l.key;
		// 	entry_info->val = l.val;
		// 	break;
		//GroupInfo *group_info = NULL;
		EntryInfo& entry_info = g->second.entries[std::string(key)];
		entry_info.key.str = key;
		entry_info.val.str = val;
		entry_info.key.orig_len = entry_info.val.orig_len = 0;
		entry_info.key.pos = entry_info.val.pos = g->second.first_line_pos;
		m_dirty.insert(&entry_info);
		
		return 0;
	}
	
	e->second.val.str = val;
	m_dirty.insert(&e->second);
	
	return 0;
}

// helper function that dumps all map values into vector and sorts them
template<typename M, typename V>
static void map_to_vec(const M& m, V& v)
{
	for (typename M::const_iterator it = m.begin(); it != m.end(); it++) {
		v.push_back(it->second);
	}
	std::sort(v.begin(), v.end());
}

void IniParser::print_ini()
{
	std::vector<GroupInfo> v_groups;
	map_to_vec(m_groups, v_groups);
	
	for (auto& g : v_groups) {
		TokenInfo info = g.group;
		printf("[%s] %d+%lu\n", info.str.c_str(), info.pos, info.str.length());
		
		std::vector<EntryInfo> v_entries;
		map_to_vec(g.entries, v_entries);
		
		for (auto& e : v_entries) {
			auto info = e;
			printf("---- %s=%s [%d,%d] [%d,%d]\n", info.key.str.c_str(), info.val.str.c_str(),
			       info.key.pos, info.key.orig_len,
			       info.val.pos, info.val.orig_len);
		}
	}
}

int IniParser::write_ini()
{
	int offset = 0;
	unsigned i_g = 0, i_e = 0;
	
	std::vector<TokenInfo*> v_groups;
	std::vector<EntryInfo*> v_entries;
	for (auto& g : m_groups) {
		v_groups.push_back(&g.second.group);
		for (auto& e : g.second.entries) {
			v_entries.push_back(&e.second);
		}
	}
	std::sort(v_groups.begin(), v_groups.end(), TokenInfoCmp());
	std::sort(v_entries.begin(), v_entries.end(), EntryInfoCmp());
	
	
	if (m_io->init_writing() == -1) {
		m_error = m_io->get_error();
		return -1;
	}
	
	for (auto entry : m_dirty) {
		const TokenInfo& key = entry->key;
		const TokenInfo& val = entry->val;
		
		// write unchanged bits from the original file up to current key/val
		m_io->write(val.pos);
		
		// fix offsets from last point up until current val (modified thingie)
		while (i_g < v_groups.size() && v_groups[i_g]->pos <= val.pos) {
			v_groups[i_g]->pos += offset;
			i_g++;
		}
		while (i_e < v_entries.size() && v_entries[i_e]->val.pos <= val.pos) {
			fprintf(stderr, "fix [%s,%s] by %d before [%s, %s]\n", v_entries[i_e]->key.str.c_str(), v_entries[i_e]->val.str.c_str(), offset, key.str.c_str(), val.str.c_str());
			v_entries[i_e]->key.pos += offset;
			v_entries[i_e]->val.pos += offset;
			i_e++;
			if (v_entries[i_e-1]->val.pos == val.pos) {
				break;
			}
		}
		
		// adding a new key/val pair
		if (key.orig_len == 0) {
			// write key=val\n
			m_io->write_str(key.str);
			m_io->write_str(std::string("="));
			m_io->write_str(val.str);
			m_io->write_str(std::string("\n"));
			
			// change offset
			offset += val.str.length() + key.str.length() + 2;
			
			// update val pos (add "key=" length to it)
			const_cast<TokenInfo&>(val).pos += key.str.length() + 1;
			
			// update orig_len
			const_cast<TokenInfo&>(key).orig_len = key.str.length();
			const_cast<TokenInfo&>(val).orig_len = val.str.length();
			
			continue;
		}
		
		// write val
		m_io->write_str(val.str);
		
		// skip len(val) bytes from original file
		m_io->skip(val.orig_len);
		
		// change offset
		offset += val.str.length() - val.orig_len;
		
		// update orig_len
		const_cast<TokenInfo&>(val).orig_len = val.str.length();
	}
	
	// fix all remaining offsets
	while (i_g < v_groups.size()) {
		v_groups[i_g]->pos += offset;
		i_g++;
	}
	while (i_e < v_entries.size()) {
		fprintf(stderr, "fix [%s,%s] by %d last\n", v_entries[i_e]->key.str.c_str(), v_entries[i_e]->val.str.c_str(), offset);
		v_entries[i_e]->key.pos += offset;
		v_entries[i_e]->val.pos += offset;
		i_e++;
	}
	
	// write out any remaining bits from original file
	m_io->write(-1);
	m_io->finish_writing();
	
	return 0;
}

#ifdef USE_SDL_ZZIP
RWIO::RWIO(const char* path)
  : FileIO(path)
{
	m_data = NULL;
}

RWIO::~RWIO()
{
	finish_reading();
}

int RWIO::init_reading()
{
	m_data = SDL_RWFromZZIP(m_path, "r");
	if (!m_data) {
		m_error = "Unable to open " + id() + ": " + strerror(errno);
		return -1;
	}
	return 0;
}

void RWIO::finish_reading()
{
	if (m_data) {
		SDL_RWclose(m_data);
		m_data = NULL;
	}
}

int RWIO::getc()
{
	char c;
	
	if (SDL_RWread(m_data, &c, 1, 1) == 0)
		return -1;
	
	return c;
}

int RWIO::ungetc(int c)
{
	if (c == EOF)
		return EOF;
	
	SDL_RWseek(m_data, -1, RW_SEEK_CUR);
	
	return c;
}

int RWIO::pos()
{
	return SDL_RWtell(m_data);
}
#endif

IniParser* ParseFILE(const char* path)
{
	IniParser *parser = new IniParser(new FileIO(path));
	
	if (parser->parse_ini() == -1) {
		fputs(parser->get_error().c_str(), stderr);
		delete parser;
		return NULL;
	}
	
	return parser;
}

#ifdef USE_SDL_ZZIP
IniParser* ParseZZIP(const char* path)
{
	IniParser *parser = new IniParser(new RWIO(path));
	
	if (parser->parse_ini() == -1) {
		fputs(parser->get_error().c_str(), stderr);
		delete parser;
		return NULL;
	}
	
	return parser;
}
#endif
