#include "iniparser.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include <ctype.h>


/* for modification purposes, put dirty entries into a set (m_dirty)
 * 
 * 
 * 
 */

// constants
const char kGroupBegin   = '[';
const char kGroupEnd     = ']';
const char kCommentBegin = ';';
const char kKeyValDelim  = '=';


FileIO::FileIO(const char* path)
{
	m_path = path;
	m_file = fopen(m_path, "r");
	if (m_file == NULL) {
		fprintf(stderr, "Unable to open %s: %s\n",
		        string_id().c_str(), strerror(errno));
	}
}

FileIO::~FileIO()
{
	fclose(m_file);
}

std::string FileIO::string_id()
{
	return std::string(m_path);
}

inline int FileIO::getc()
{
	return ::getc(m_file);
}

int FileIO::ungetc(int c)
{
	return ::ungetc(c, m_file);
}

int FileIO::pos()
{
	return ftell(m_file);
}

void FileIO::skip_space()
{
	int c;
	while (c = getc(), isblank(c))
		continue;
	ungetc(c);
}

void FileIO::skip_line()
{
	int c;
	while (c = getc(), c != EOF && c != '\n')
		continue;
}

static inline int isname(char c)
{
	return isalnum(c) || c == '_';
}

std::string FileIO::read_name()
{
	int c;
	std::vector<char> buf;
	
	while (c = getc(), isname(c))
		buf.push_back(c);
	ungetc(c);
	
	return std::string(buf.begin(), buf.end());
}

static inline int isval(char c)
{
	return !(c == kGroupEnd    || c == kCommentBegin ||
	         c == kKeyValDelim || isspace(c));
}

std::string FileIO::read_val()
{
	int c;
	std::vector<char> buf;
	
	while (c = getc(), isval(c))
		buf.push_back(c);
	ungetc(c);
	
	return std::string(buf.begin(), buf.end());
}

IniParser::IniParser(FileIO* io)
{
	m_io = io;
	parse_ini();
}

IniParser::~IniParser()
{
	if (m_io != NULL) {
		delete m_io;
		m_io = NULL;
	}
}

ParsedLine& IniParser::parse_line()
{
	int c;
	static ParsedLine parsed_line;
	
	m_io->skip_space();
	c = m_io->getc();
	
	if (c == '\n') {
		parsed_line.type = EMPTY;
		return parsed_line;
	}
	
	else if (c == EOF) {
		parsed_line.type = END_OF_INI;
		return parsed_line;
	}
	
	else if (c == kCommentBegin) {
		parsed_line.type = COMMENT;
		m_io->skip_line();
		return parsed_line;
	}
	
	else if (c == kGroupBegin) {
		parsed_line.type = GROUP;
		m_io->skip_space();
		parsed_line.group.pos = m_io->pos();
		parsed_line.group.str = m_io->read_name();
		parsed_line.group.end = m_io->pos();
		m_io->skip_space();
		if (m_io->getc() != kGroupEnd) {
			parsed_line.error = std::string("expected '")+kGroupEnd+"'";
			parsed_line.type = ERROR;
			return parsed_line;
		}
		m_io->skip_line();
		return parsed_line;
	}
	
	else if (isname(c)) {
		parsed_line.type = KEY_VAL;
		m_io->ungetc(c);
		
		// read key
		parsed_line.key.pos = m_io->pos();
		parsed_line.key.str = m_io->read_name();
		parsed_line.key.end = m_io->pos();
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
		parsed_line.val.end = m_io->pos();
		m_io->skip_line();
		
		return parsed_line;
	}
	
	else {
		parsed_line.error = std::string("expected '")+kGroupBegin+"' or key";
		parsed_line.type = ERROR;
		return parsed_line;
	}
}

int IniParser::parse_ini()
{
	int n_line = 0;
	int last_pos = 0;
	GroupInfo *group_info = NULL;
	EntryInfo *entry_info = NULL;
	
	for (ParsedLine& l = parse_line(); l.type != END_OF_INI; parse_line()) {
		switch (l.type) {
		case ERROR:
			fprintf(stderr, "Error parsing %s at line %d, pos %d (%s)\n",
			        m_io->string_id().c_str(), n_line+1, m_io->pos()-last_pos,
			        l.error.c_str());
			return -1;
		
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
			entry_info = &group_info->entries[l.key.str];
			entry_info->key = l.key;
			entry_info->val = l.val;
			break;
		}
		
		last_pos = m_io->pos();
		n_line++;
	}
	
	return 0;
}
