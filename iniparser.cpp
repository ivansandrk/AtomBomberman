#include "iniparser.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include <ctype.h>


FileIO::FileIO(const char* path)
{
	m_path = path;
	m_file = fopen(m_path, "r");
	if (m_file == NULL)
	{
		fprintf(stderr, "Unable to open %s: %s\n", m_path, strerror(errno));
	}
}

FileIO::~FileIO()
{
	fclose(m_file);
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
	//return isalnum(c) || c == '_';
	return !(isspace(c) || c == ']' || c == ';' || c == '=');
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

IniParser::IniParser(FileIO* io)
{
	m_io   = io;
	m_line = 0;
}

IniParser::~IniParser()
{
	if (m_io != NULL)
	{
		delete m_io;
		m_io = NULL;
	}
}

ParsedLine& IniParser::parse_line()
{
	int c;
	std::vector<char> buf;
	static ParsedLine parsed_line;
	
	m_io->skip_space();
	c = m_io->getc();
	
	if (c == '\n')
	{
		parsed_line.type = EMPTY;
		return parsed_line;
	}
	
	if (c == EOF)
	{
		parsed_line.type = END_OF_INI;
		return parsed_line;
	}
	
	if (c == ';')
	{
		parsed_line.type = COMMENT;
		m_io->skip_line();
		return parsed_line;
	}
	
	if (c != '[' && !isname(c))
		goto error;
	
	if (c == '[')
	{
		parsed_line.type = GROUP;
		m_io->skip_space();
		parsed_line.group.pos = m_io->pos();
		parsed_line.group.str = m_io->read_name();
		parsed_line.group.end = m_io->pos();
		m_io->skip_space();
		if (m_io->getc() != ']')
			goto error;
		m_io->skip_line();
		return parsed_line;
	}
	
	parsed_line.type = KEY_VAL;
	m_io->ungetc(c);
	parsed_line.key.pos = m_io->pos();
	parsed_line.key.str = m_io->read_name();
	parsed_line.key.end = m_io->pos();
	m_io->skip_space();
	if (m_io->getc() != '=')
		goto error;
	m_io->skip_space();
	parsed_line.val.pos = m_io->pos();
	parsed_line.val.str = m_io->read_name();
	parsed_line.val.end = m_io->pos();
	m_io->skip_line();
	
	return parsed_line;
	
  error:
	parsed_line.type = ERROR;
	return parsed_line;
}

void IniParser::parse_ini()
{
	std::string last_group = "";
	//ParsedLine& line = parse_line();
	
	for (ParsedLine& line = parse_line(); line.type != END_OF_INI; parse_line())
	{
	}
	
	
	
}
