#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ---------------------------------

char *copystr(const char *str)
{
	char *new_str = (char*)malloc(strlen(str) + 1);
	strcpy(new_str, str);
	return new_str;
}

FILE *fout = stdout;

#define MAX_FILENAME_LEN 500

// ---------------------------------

class SubModule
{
public:
	char *path;
	char *url;
	SubModule *next;
	SubModule(const char *p, const char *u, SubModule *n) : next(n)
	{
		path = copystr(p);
		url = copystr(u);
	}
};

SubModule* lbs_subModules = 0;
SubModule* dist_subModules = 0;

void addSubModule(const char *path, const char *url) { lbs_subModules = new SubModule(path, url, lbs_subModules); }
void addDistSubModule(const char *path, const char *url) { dist_subModules = new SubModule(path, url, dist_subModules); }

const char *source_dir = "";
size_t len_source_dir;

void read_commit_hash(const char *path, char *commit)
{
	FILE *f = fopen(path, "r");
	if (f == 0)
		commit[0] = '\0';
	else
	{
		if (fgets(commit, 41, f))
			commit[40] = '\0';
		else
			commit[0] = '\0';
		fclose(f);
	}
	fprintf(fout, "Head file: '%s': %s\n", path, commit);
}

void read_sub_modules(const char *fn, char *modules_dir)
{
	char complete_path[MAX_FILENAME_LEN+1];
	strcpy(complete_path, fn);
	char *s_path = complete_path + strlen(complete_path);
	char *m_path = modules_dir + strlen(modules_dir);
	strcpy(s_path, ".gitmodules");
	FILE *f = fopen(complete_path, "r");
	if (f == 0)
		return;
	//fprintf(fout, "Parsing %s %s\n", complete_path, modules_dir);
	
	char path[MAX_FILENAME_LEN+1];
	char url[MAX_FILENAME_LEN+1];
	
	while (fgets(path, MAX_FILENAME_LEN, f))
	{
		if (!strncmp(path, "\tpath = ", 8) == 0)
			continue;
		if (fgets(url, MAX_FILENAME_LEN, f) && strncmp(url, "\turl = ", 7) == 0)
		{
			//fprintf(fout, "1: %s2: %s", path, url);
			while (path[strlen(path)-1] < ' ') path[strlen(path)-1] = '\0';
			while (url[strlen(url)-1] < ' ') url[strlen(url)-1] = '\0';
			char *s = strstr(url, ".git");
			if (s != 0) *s = '\0';
			sprintf(s_path, "%s/", path + 8);
			char *source = complete_path + len_source_dir;

			sprintf(m_path, "%s/HEAD", path + 8);
			char commit[41];
			read_commit_hash(modules_dir, commit);

			//fprintf(fout, "Head file: '%s': %s\n", modules_dir, commit);
			sprintf(m_path, "%s/modules/", path + 8);

			char complete_url[MAX_FILENAME_LEN+1];
			if (strncmp(url + 7, "https://github.com/", 19) == 0)
			{
				if (commit[0] == '\0')
					sprintf(complete_url, "%s/blob/main/%%s", url + 7);
				else
				   	sprintf(complete_url, "%s/blob/%s/%%s", url + 7, commit);
			}
			else if (strncmp(url + 7, "https://git.savannah.nongnu.org/git/", 36) == 0)
			{
				if (commit[0] == '\0')
					sprintf(complete_url, "https://git.savannah.nongnu.org/gitweb/?p=%s.git;a=blob;f=%%s", url + 43);
				else
					sprintf(complete_url, "https://git.savannah.nongnu.org/gitweb/?p=%s.git;a=blob;f=%%s;id=%s", url + 43, commit);
			}
			else
				sprintf(complete_url, "%s%%s", url + 7);
			//printf("Submodule %s %s|\n", source, complete_url);
			addSubModule(source, complete_url);
			read_sub_modules(complete_path, modules_dir);
		}
	}
}

char live_bootstrap_commit[41];

void init_subModules()
{
	// Root directory:
	char path[MAX_FILENAME_LEN+1];
	snprintf(path, MAX_FILENAME_LEN, "%s.git/refs/heads/main", source_dir);
	read_commit_hash(path, live_bootstrap_commit);
	snprintf(path, MAX_FILENAME_LEN, "https://github.com/FransFaase/MES-replacement/blob/%s/%%s",
		live_bootstrap_commit[0] == '\0' ? "main" : live_bootstrap_commit);
	addSubModule("", path);

	// Sub modules
	char modules_dir[MAX_FILENAME_LEN+1];
	snprintf(modules_dir, MAX_FILENAME_LEN, "%s.git/modules/", source_dir);
	read_sub_modules(source_dir, modules_dir);

	// Exclude some directories
	addSubModule("distfiles", "--");
	
	// Extra for files that have been unpacked from distribution
	addDistSubModule("/steps/tcc-0.9.26/build/mes-0.26/", "https://git.savannah.gnu.org/cgit/mes.git/tree/%s?h=v0.26");
	addDistSubModule("/steps/tcc-0.9.26/build/tcc-0.9.26-1147-gee75a10c/", "https://github.com/TinyCC/tinycc/tree/d5e22108a0dc48899e44a158f91d5b3215eb7fe6/%s");
	addDistSubModule("/steps/mes-0.26/build/mes-0.26/", "https://git.savannah.gnu.org/cgit/mes.git/tree/%s?h=v0.26");
	addDistSubModule("/steps/mes-0.26/build/nyacc-1.00.2/", "https://git.savannah.gnu.org/cgit/nyacc.git/tree/%s?h=V1.00.2");

}

char *get_url(const char *path, SubModule *subModules)
{
	for (SubModule *subModule = subModules; subModule != 0; subModule = subModule->next)
	{
		size_t path_len = strlen(subModule->path);
		if (strncmp(path, subModule->path, path_len) == 0)
		{
			char url[MAX_FILENAME_LEN+1];
			snprintf(url, MAX_FILENAME_LEN, subModule->url, path + path_len);
			return copystr(url);
		}
	}
	return 0;
}

// ---------------------------------

class File;
class Action;
class Process;



class LineInFile
{
public:
	const char *text;
	File *file;
	long line;
	LineInFile *next;
	LineInFile(const char *t, File *f, long l) : text(copystr(t)), file(f), line(l), next(0) {}
	LineInFile(LineInFile *lif, int offset = 0) : text(lif->text + offset), file(lif->file), line(lif->line), next(0) {}
};

class MergeChild;

int nr_files = 0;
class File
{
public:
	char *name;
	int nr;
	bool is_source;
	char *source_name;
	char *url;
	File *copy_from;
	Action *actions;
	File *next;
	
	File(const char *fn) : is_source(false), source_name(0), url(0), copy_from(0), actions(0), next(0)
	{
		name = copystr(fn);
		nr = nr_files++;
	}

	void init_source();

	bool exec_before_created();
	
	bool used_as_input();
	
	bool produced_and_not_removed();
};

File *files = 0;

int nr_processes = 0;
class Process
{
public:
	int nr;
	unsigned long pid;
	Process *parent;
	Action *actions;
	Process *next;
	
	Process(unsigned long _pid) : pid(_pid), parent(0), actions(0), next(0)
	{
		nr = ++nr_processes;
	}
	bool hasInputUseOf(File *f);
	Action *lastOpenAction(int handle);
};

Process *all_processes = 0;
Process **ref_next = &all_processes;
Process *next_process(unsigned long pid)
{
	Process *process = new Process(pid);
	*ref_next = process;
	ref_next = &process->next;
	return process;
}

Process *find_process(unsigned long pid)
{
	Process *cached_process = 0;
	if (cached_process != 0 && cached_process->pid == pid)
		return cached_process;
	for (Process *process = all_processes; process != 0; process = process->next)
		if (process->pid == pid)
		{
			cached_process = process;
			return process;
		}
	return 0;
}

class MergeChild
{
public:
	File *child;
	MergeChild *next;
	
	MergeChild(File *file) : child(file), next(0) {}
};

File *get_file(const char *full_fn /*, bool use_alias = true*/)
{
	File **ref = &files;
	for (; *ref != 0; ref = &(*ref)->next)
		if (strcmp((*ref)->name, full_fn) == 0)
			return /*(*ref)->alias != 0 && use_alias ? (*ref)->alias :*/ (*ref);
	*ref = new File(full_fn);
	return *ref;
}



	

unsigned long read_unsigned_long(char *&s)
{
	unsigned long result = 0;
	if (*s == '0')
	{
		for (s++; '0' <= *s && *s <= '7'; s++)
			result = 8 * result + *s - '0';
		return result;
	}
	for (; '0' <= *s && *s <= '9'; s++)
		result = 10 * result + *s - '0';
	return result;
}

long read_long(char *&s)
{
	long sign = 1;
	long result = 0;
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	for (; '0' <= *s && *s <= '9'; s++)
		result = 10 * result + *s - '0';
	return sign * result;
}

int indent_depth = 0;
void indent(FILE *fout) { fprintf(fout, "%*.*s", indent_depth, indent_depth, ""); }

FILE *fout_usage = 0;

class Action
{
	public:
	char kind; // one of 'e' - execute, 'o' - open, 'r' - removed, 'c' - change mode, 'E' - execute child
	bool o_rdonly;
	bool o_wronly;
	bool o_rdwr;
	bool o_creat;
	bool o_trunc;
	bool o_excl;
	int file_handle;
	bool is_closed;
	int mode;
	bool from_archive;
	Process *child_process;
	
	File *file;
	Process *process;
	
	char json_kind;
	Process *file_created_by;

	Action *next_in_process;
	Action *next_on_file;
	
	Action (File *_file, Process *_process, char _kind)
	: kind(_kind),
	  o_rdonly(false), o_wronly(false), o_rdwr(false), o_creat(false), o_trunc(false), o_excl(false), file_handle(-1), is_closed(false),
	  mode(0),
	  from_archive(false),
	  child_process(0), 
	  file(_file), process(_process),
	  json_kind(_kind),
	  file_created_by(0),
	  next_in_process(0), next_on_file(0)
	{
		Action **ref_action_in_process = &process->actions;
		while (*ref_action_in_process != 0) ref_action_in_process = &(*ref_action_in_process)->next_in_process;
		*ref_action_in_process = this;
		if (file != 0)
		{
			Action **ref_action_on_file = &file->actions;
			while (*ref_action_on_file != 0) ref_action_on_file = &(*ref_action_on_file)->next_on_file;
			*ref_action_on_file = this;
		}
	}
	
	const char *oper_name()
	{
		return kind == 'e' ? "Executes" :
			   kind == 'r' ? "Delete" :
			   is_produced() ? "Produces" :
			   kind != 'o' ? 0 :
			   o_rdonly ? "Uses as input" :
			   o_wronly ? "Writes" :
			   o_rdwr ? "Modifies" :
			   "Uses";
	}
	
	bool is_input() { return kind == 'o' && !is_produced() && o_rdonly; }	
	
	bool is_produced() { return kind == 'o' && (o_creat || ((o_wronly || o_rdwr) && o_trunc)); }
};

bool Process::hasInputUseOf(File *f)
{
	for (Action *action = actions; action != 0; action = action->next_in_process)
		if (action->file == f && action->o_rdonly)
			return true;
	return false;
}

Action *Process::lastOpenAction(int handle)
{
	Action *last_open_action = 0;
	for (Action *action = actions; action != 0; action = action->next_in_process)
		if (action->kind == 'o' && action->file_handle == handle)
			last_open_action = action;
	return last_open_action;
}

#define NR_MAPPIGS 21
struct 
{
	const char *from;
	const char *to;
	bool has_url;
} source_mapping[NR_MAPPIGS] = {
	{"bootstrap-seeds/POSIX/x86/kaem-optional-seed", "src/kaem-minimal_s", false},
	{"bootstrap-seeds/POSIX/x86/hex0-seed", "src/hex0_s", false},
	{"kaem.x86", "task3/kaem.x86", true},
	{"x86/tools-seed-kaem.kaem", "task3/tools-seed-kaem.kaem", true},
	{"x86/tools-mini-kaem.kaem", "task3/tools-mini-kaem.kaem", true},
	{"x86/check-tools.kaem", "task3/check-tools.kaem", true},
	{"x86/tools-kaem.kaem", "task3/tools-kaem.kaem", true},
	{"x86/after.kaem", "task3/after.kaem", true},
	{"x86/hex0_s.hex0", "src/hex0_s.hex0", false},
	{"x86/kaem-minimal_s.hex0", "src/kaem-minimal_s.hex0", false},
	{"x86/hex2_s.hex0", "src/hex2_s.hex0", false},
	{"x86/blood-elf_s.macro", "src/blood-elf_s.macro", false},
	{"x86/blood-elf_s.blood_elf", "src/blood-elf_s.blood_elf", false},
	{"x86/M1_s.macro", "src/M1_s.macro", false},
	{"x86/M1_s.blood_elf", "src/M1_s.blood_elf", false},
	{"x86/stack_c_s.M1", "src/stack_c_s.M1", false},
	{"x86/stack_c_intro.M1", "src/stack_c_intro.M1", true},
	{"x86/ELF-x86-debug.hex2", "M2libc/x86/ELF-x86-debug.hex2", true},
	{"seed.kaem", "task3/seed.kaem", true},
	{"configurator.x86.checksums", "task3/configurator.x86.checksums", true},
	{"script-generator.x86.checksums", "task3/script-generator.x86.checksums", true},
};    

void File::init_source()
{
	if (actions == 0 || actions->next_on_file != 0)
		return;
	
	if (actions->kind == 'o' && actions->is_produced())
	{
		if (actions->process->actions->kind == 'e' && strcmp(actions->process->actions->file->name, "/usr/bin/untar") == 0)
		{
			actions->from_archive = true;
			url = get_url(name, dist_subModules);
		}
	}
	else if (actions->kind == 'e' || actions->kind == 'o')
	{
		is_source = true;
		bool has_url = true;
		char *n = name;
		if (n[0] == '/')
			n++;
		for (int i = 0; i < NR_MAPPIGS; i++)
			if (strcmp(n, source_mapping[i].from) == 0)
			{
				source_name = copystr(source_mapping[i].to);
				has_url = source_mapping[i].has_url;
				break;
			}
		if (source_name == NULL)
		{
			if (strncmp(n, "src/", 4) == 0)
				source_name = copystr(n);
			else if (strncmp(n, "external/distfiles/", 19) == 0)
			{
				char buffer[200];
				sprintf(buffer, "task3/%s", n + 8);
				source_name = copystr(buffer);
			}
			else if (strncmp(n, "steps/", 6) == 0)
			{
				char buffer[200];
				sprintf(buffer, "task3/%s", n);
				source_name = copystr(buffer);
			}
		}
		
			/*
		if (source_name == NULL)
		{
			if (strncmp(n, "external/distfiles/", 19) == 0)
				n += 9;
		
			static const char *paths[] = { "replacement/", "*seed/", "*seed/stage0-posix/", "*"};
			for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); i++)
			{
				char poss_source_name[MAX_FILENAME_LEN];
				if (paths[i][0] == '*')
				{
					strcpy(poss_source_name, source_dir);
					strcat(poss_source_name, paths[i] + 1);
				}
				else
					strcpy(poss_source_name, paths[i]);
				strcat(poss_source_name, n);
				if (access(poss_source_name, R_OK) == 0)
				{
					source_name = copystr(poss_source_name);
					break;
				}
			}
		}
			*/
	
		if (source_name != 0 && has_url)
			url = get_url(source_name + len_source_dir, lbs_subModules);
	}
}

bool File::exec_before_created()
{
	bool is_created = false;
	for (Action *action = actions; action != 0; action = action->next_on_file)
		if (action->kind == 'o' && action->o_creat && !action->from_archive)
			is_created = true;
		else if (action->kind == 'r')
			is_created = false;
		else if (action->kind == 'e' && !is_created)
			return true;
	return false;
}

bool File::used_as_input()
{
	return actions != 0 && (actions->o_rdonly || actions->o_rdwr);
}

bool File::produced_and_not_removed()
{
	if (used_as_input())
		return false;
	bool is_created = false;
	for (Action *action = actions; action != 0; action = action->next_on_file)
		if (action->kind == 'o' && (action->o_creat || action->o_wronly || (action->o_rdwr && action->o_trunc)) && !action->from_archive)
			is_created = true;
		else if (action->kind == 'r')
			is_created = false;
		else if (action->kind == 'e' && !is_created)
			return false;
	return is_created;
}

// ----------------------------------



bool accept_string(const char *str, char *&s)
{
	char *t = s;
	while (*str != '\0' && *t != '\0')
	{
		if (*str != *t)
			return false;
		t++;
		if (*str == ' ')
		{
			while (*t == ' ')
				t++;
		}
		str++;
	}
	s = t;
	return true;
}

bool parse_filename(char *filename, char *&s)
{
	if (*s != '"')
		return false;
	s++;
	for (int i = 0; i < MAX_FILENAME_LEN; i++)
	{
		if (*s == '"')
		{
			filename[i] = '\0';
			s++;
			return true;
		}
		while (s[0] == '/' && s[1] == '/')
			s++;
		filename[i] = *s++;
	}
	
	fprintf(fout, "file name too long\n");
	exit(-1);
	return false;
}

char cd_path[MAX_FILENAME_LEN] = "/";

void add_cd_path(char *filename)
{
	char buf[2*MAX_FILENAME_LEN+1];

	//fprintf(log_file, "add_cd_path %s %s => ", cd_path, filename);
	if (filename[0] == '/')
	{
		char *s = filename;
		while (s[1] == '/')
			s++;
		strcpy(buf, s);
		//fprintf(fout, "add_cd_path %s %s => %s\n", cd_path, filename, buf);
		strcpy(filename, buf);
		return;
	}
	strcpy(buf, cd_path);
	int i = strlen(buf);
	while (i > 0 && buf[i-1] == '/')
		i--;
	char *f = filename;
	while (f[0] != '\0')
	{
		if (f[0] == '.' && (f[1] == '\0' || f[1] == '/'))
		{
			f++;
			while (f[0] == '/')
				f++;
		}
		else if (f[0] == '.' && f[1] == '.' && (f[2] == '\0' || f[2] == '/'))
		{
			f += 2;
			while (f[0] == '/')
				f++;
			while (i > 0 && buf[i-1] != '/')
				i--;
			while (i > 0 && buf[i-1] == '/')
				i--;
		}
		else
		{
			buf[i++] = '/';
			while (f[0] != '\0' && f[0] != '/')
				buf[i++] = *f++;
			while (f[0] == '/')
				f++;
		}
	}
	buf[i] = '\0';
	if (i > MAX_FILENAME_LEN)
	{
		fprintf(fout, "add_cd_path reached lengt %d\n", i);
		exit(1);
	}
	//fprintf(fout, "add_cd_path %s %s => %s\n", cd_path, filename, buf);
	strcpy(filename, buf);
	//fprintf(log_file, "%s\n", filename);
}

void read_filename(char *filename, char *&s)
{
	if (!parse_filename(filename, s))
	{
		fprintf(fout, "Failed to parse filename from '%s'\n", s);
		exit(0);
	}
	add_cd_path(filename);
}

bool accept_openat(char *&s)
{
	char *s2 = s;
	if (strncmp(s2, "openat(", 7) != 0)
		return false;
	s2 += 7;
	if (strncmp(s2, "AT_FDCWD", 8) == 0)
		s2 += 8;
	else
		while ('0' <= *s2 && *s2 <= '9')
			s2++;
	if (strncmp(s2, ", ", 2) != 0)
		return false;
	s = s2 + 2;
	return true;
}

#define NR_PARR_COMMANDS 4
		
bool process_trace_file(const char *trace_fn)
{
	FILE *f = fopen(trace_fn, "r");
	
	char buffer[10000];
	
	struct Command
	{
		bool active;
		unsigned long pid;
		char cmd[10000];
	};
	Command cmd[NR_PARR_COMMANDS];
	for (int i = 0; i < NR_PARR_COMMANDS; i++)
		cmd[i].active = false;

	long line_nr = 0;
	bool skip_first = true;
	
	while (fgets(buffer, 9999, f))
	{
		line_nr++;
		
		int len = strlen(buffer);
		if (len > 0 && buffer[len-1] != '\n')
		{
			printf("Line '%s' does not end with newline\n", buffer);
			return false;
		}

		if (skip_first && strstr(buffer, "kaem-optional-seed") == 0)
			continue;
		skip_first = false;

		//fprintf(fout, "Line: %s", buffer);
		
		char filename[MAX_FILENAME_LEN+1];
		
		char *s = buffer;
		unsigned long pid = read_unsigned_long(s);
		Process *process = find_process(pid);
		if (process == NULL)
		{
			printf("Line %ld: Did not process for %lu\n", line_nr, pid);
			process = next_process(pid);
		}
		
		while (*s == ' ' || *s == '\t')
			s++;
		char *command = s;
		//fprintf(fout, "%lu: %s", pid, s);
		if (strncmp(s, "<... ", 5) == 0)
		{
			printf("DEBUG: resumd\n");
			char *ns = strstr(s, "resumed>");
			if (ns == 0)
			{
				printf("Line '%s' expect 'resumed>'\n", buffer);
				return false;
			}
			s = ns + 8;
			bool found = false;
			for (int i = 0; i < NR_PARR_COMMANDS; i++)
				if (cmd[i].active && cmd[i].pid == pid)
				{
					printf("DEBUG: more resumed: '%s'\n", s);
					strcat(cmd[i].cmd, s);
					printf("DEBUG: makes: '%s'\n", cmd[i].cmd);
					char *s_unf = strstr(cmd[i].cmd, " <unfinished ...>");
					if (s_unf != 0)
					{
						*s_unf = '\0';
						s = 0;
					}
					else
					{
						cmd[i].active = false;
						s = cmd[i].cmd;
					}
					found = true;
					break;
				}
			if (!found)
			{
				printf("Line: '%s' is not correct continuation\n", buffer);
				return false;
			}
		}
		else if (strstr(s, " <unfinished ...>") != 0)
		{
			bool found = false;
			for (int i = 0; i < NR_PARR_COMMANDS; i++)
				if (!cmd[i].active)
				{
					cmd[i].active = true;
					cmd[i].pid = pid;
					strcpy(cmd[i].cmd, s);
					char *s_unf = strstr(cmd[i].cmd, " <unfinished ...>");
					if (s_unf != 0)
					{
						*s_unf = '\0';
						s = 0;
					}
					printf("DEBUG: Unfinshed %lu: '%s'\n", pid, cmd[i].cmd);
					found = true;
					break;
				}
			if (!found)
			{
				printf("Line: '%s' too many parralel commands. Increase NR_PARR_COMMANDS\n", buffer);
				return false;
			}
		}
		
		if (s == 0)
		{
			// nothing to process
		}
		else if (accept_string("execve(", s))
		{
			read_filename(filename, s);
			File *exec_file = get_file(filename);
			new Action(exec_file, process, 'e');
			if (strcmp(filename, "/usr/bin/tcc-boot0") == 0)
			{
				fprintf(fout, "Stop at %lu: %s\n", pid, s);
				break;
			}
			exec_file->init_source();
		}
		else if (accept_string("open(", s) || accept_openat(s))
		{
			read_filename(filename, s);
			//fprintf(fout, "open %s", s);
			bool o_rdonly = false;
			bool o_wronly = false;
			bool o_rdwr = false;
			bool o_creat = false;
			bool o_trunc = false;
			bool o_excl = false;
			if (!accept_string(", ", s))
				fprintf(fout, "Expecting ', at '%s'\n", s);
			for (;*s != '\0'; s++)
			{
				if (accept_string("O_RDONLY", s))
					o_rdonly = true;
				else if (accept_string("O_WRONLY", s))
					o_wronly = true;
				else if (accept_string("O_RDWR", s))
					o_rdwr = true;
				else if (accept_string("O_CREAT", s))
					o_creat = true;
				else if (accept_string("O_TRUNC", s))
					o_trunc = true;
				else if (accept_string("O_EXCL", s))
					o_excl = true;
				else if (   accept_string("O_NONBLOCK", s) || accept_string("O_CLOEXEC", s)
					     || accept_string("O_DIRECTORY", s) || accept_string("O_NOCTTY", s)
					     || accept_string("O_PATH", s) || accept_string("O_NOFOLLOW", s))
					;
				else
				{
					fprintf(fout, "Unknown %s", s);
					break;
				}
				if (*s == ',')
					break;
				if (*s != '|')
					break;
			}
			unsigned long mode = 0;
			if (accept_string(", ", s))
			{
				mode = read_unsigned_long(s);
			}
			if (!accept_string(") = ", s))
			{
				fprintf(fout, "Expecting ') = ' at '%s'\n", s);
				return false;
			}
			long handle = read_long(s);
			//if (*s != '\n')
			//	fprintf(fout, "open end with '%s'\n", s);
			if ((o_rdonly ? 1 : 0) + (o_wronly ? 1 : 0) + (o_rdwr ? 1 : 0) != 1)
				fprintf(fout, "Warning: Open '%s' as undefined read/write mode\n", filename);

			if (handle > -1)
			{
				File *file = get_file(filename);
				Action *action = new Action(file, process, 'o');
				action->file_handle = handle;
				action->o_rdonly = o_rdonly;
				action->o_wronly = o_wronly;
				action->o_rdwr = o_rdwr;
				action->o_creat = o_creat;
				action->o_trunc = o_trunc;
				action->o_excl = o_excl;
				if (o_creat)
					action->mode = mode;
				file->init_source();
			}
		}
		else if (accept_string("close(", s))
		{
			unsigned long handle = read_unsigned_long(s);
			if (*s != ')')
				fprintf(fout, "Expecting ')' at '%s'\n", s);
			Action *last_open_action = process->lastOpenAction(handle);
			if (last_open_action == 0)
				fprintf(fout, "Error: Handle %ld not opened by process %d\n", handle, process->nr);
			else if (last_open_action->is_closed)
				fprintf(fout, "Warning: File %s already closed for process %d\n", last_open_action->file->name, process->nr);
			else
				last_open_action->is_closed = true; 
		}
		else if (accept_string("chmod(", s))
		{
			read_filename(filename, s);
			unsigned long mode = 0;
			if (accept_string(", ", s))
			{
				mode = read_unsigned_long(s);
			}
			if (*s != ')')
			{
				fprintf(fout, "Expecting ')' at '%s'\n", s);
				return false;
			}
			File *file = get_file(filename);
			Action *action = new Action(file, process, 'c');
			action->mode = mode;
		}
		else if (accept_string("chdir(", s))
		{
			read_filename(filename, s);
			if (!accept_string(") = ", s))
			{
				fprintf(fout, "Expecting ') = ' at '%s'\n", s);
				return false;
			}
			
			long result = read_long(s);
			//if (!accept_string("-1 ENOENT (No such file or directory)", s))
			//	result = read_unsigned_long(s);
			if (result == 0)
				strcpy(cd_path, filename);
		}
		else if (accept_string("unlink(", s))
		{
			read_filename(filename, s);
			if (*s != ')')
				fprintf(fout, "Expecting ')' at '%s'\n", s);
			File *file = get_file(filename);
			new Action(file, process, 'r');
		}
		else if (accept_string("fork(", s))
		{
			if (!accept_string(") = ", s))
			{
				fprintf(fout, "Expecting ') = ' at '%s'\n", s);
				return false;
			}
			long new_pid = read_unsigned_long(s);
			if (*s != '\n')
				fprintf(fout, "fork end with '%s'\n", s);
			Process *new_process = next_process(new_pid);
			//fprintf(fout, "fork created %lu %lu\n", new_pid, new_process->pid);
			new_process->parent = process;
			Action *action = new Action(0, process, 'E');
			action->child_process = new_process;
		}
		else if (accept_string("+++ exited with ", s))
		{
		}
		else if (accept_string("--- SIGCHLD ", s))
		{
		}
		else if (accept_string("--- SIGWINCH ", s))
		{
		}
		else if (accept_string("fcntl(", s))
		{
			// just ignore
		}
		else
		{
			fprintf(fout, "Unknown: '%s'\n", command);
			break;
		}
	}
	fclose(f);
	
	return true;
}
	
// ----------------------------------------------------

class Source
{
public:
	const char *url;
	Source *next;
	
	Source(const char *u, Source *n) : url(u), next(n) {}
};


void collect_sources(Process *process, Source **ref_sources)
{
	//indent(fout); fprintf(fout, "Process %d\n", process->nr);
	//indent_depth += 4;
	for (Action *action = process->actions; action != 0; action = action->next_in_process)
	{
		if (action->kind == 'o' && (action->o_rdonly || (action->o_rdwr && !action->o_trunc)))
		{
			File *file = action->file;		
			if (file->url != 0)
			{
				const char *url = file->url;
				//indent(fout); fprintf(fout, "Found %s\n", url);
				Source **ref_source = ref_sources;
				while (*ref_source != 0 && strcmp((*ref_source)->url, url) < 0)
					ref_source = &(*ref_source)->next;
				if (*ref_source == 0 || strcmp((*ref_source)->url, url) > 0)
					*ref_source = new Source(url, *ref_source);
			}
			else
			{
				Process *produced_by = 0;
				for (Action *file_action = file->actions; file_action != 0; file_action = file_action->next_on_file)
					if (file_action->process == process)
						break;
					else if (file_action->kind == 'r')
						produced_by = 0;
					else if (file_action->is_produced())
						produced_by = file_action->process;
				if (produced_by != 0)
					collect_sources(produced_by, ref_sources);
			}
		}
	}
	//indent_depth -= 4;
}



bool include_source = false;

void output_file(FILE *f, FILE *f_source, bool binary)
{
	if (f_source == 0) return;
	
	fprintf(f, "<PRE>");
	if (binary)
	{
		int i = 0;
		unsigned char ch = fgetc(f_source);
		while (!feof(f_source))
		{
			fprintf(f, " %02X", ch);
			if (++i % 10 == 0)
				fprintf(f, "\n");
			ch = fgetc(f_source);
		}
	}
	else
	{
		char ch = fgetc(f_source);
		if (ch != -1)
		{
			int col = 0;
			while (!feof(f_source))
			{
				col++;
				if (ch == '<')
					fprintf(f, "&lt;");
				else if (ch == '>')
					fprintf(f, "&gt;");
				else if (ch == '&')
					fprintf(f, "&amp;");
				else if ((unsigned char)ch == 160)
					fprintf(f, "&nbsp;");
				else if ((unsigned char)ch == 169)
					fprintf(f, "&copy;");
				else if ((unsigned char)ch == 194)
					fprintf(f, "&Acirc;");
				else if ((unsigned char)ch == 195)
					fprintf(f, "&Atilde;");
				else if ((unsigned char)ch == 197)
					fprintf(f, "&Aring;");
				else if ((unsigned char)ch == 216)
					fprintf(f, "&Oslash;");
				else if ((unsigned char)ch == 231)
					fprintf(f, "&ccedil;");
				else if ((unsigned char)ch == 246)
					fprintf(f, "&ouml;");
				else if (ch < 0)
					fprintf(f, "&#%d;", (unsigned char)ch);
				else if (ch == '\n' || ch == 12)
				{
					fprintf(f, "\n");
					col = 0;
				}
				else if (ch == '\t')
				{
					fprintf(f, " ");
					while (col % 4 > 0)
					{
						fprintf(f, " ");
						col++;
					}
				}
				else if (ch < ' ')
					; // skip control characters
				else
					fprintf(f, "%c", ch);
				ch = fgetc(f_source);
			}
		}
	}
	
	fprintf(f, "</PRE>");
	fclose(f_source);
}



void write_html_file(FILE *f, File *file, bool binary)
{
	fprintf(f, "<H3><A NAME=\"F%d\">File %s</A></H3>\n\n<UL>\n", file->nr, file->name);
	
	for (Action *action = file->actions; action != 0; action = action->next_on_file)
	{
		if (action->kind == 'r')
			break;
		if (action->kind == 'e')
			fprintf(f, "<LI>Executed in <A HREF=\"#S%d\">Process %d</A>\n", action->process->nr, action->process->nr);
		if (action->kind == 'o')
		{
	 		if (action->o_wronly || action->o_rdwr)
				break;
			if (action->o_rdonly)
				fprintf(f, "<LI>Input for <A HREF=\"#S%d\">Process %d</A>\n", action->process->nr, action->process->nr);
		}
	}
	fprintf(f, "</UL>\n\n");
	
	FILE *f_source = fopen(file->source_name, "r");

	if (f_source == 0)
	{
		fprintf(f, "(Source not found at '%s')\n", file->source_name);
		return;
	}

	//if (strncmp(file->source_name, source_dir, len_source_dir) == 0)
	//	fprintf(f, "Live-bootstrap source file is '%s'.<BR>\n", file->source_name + len_source_dir);
	//else
		fprintf(f, "Source file is '%s'.<BR>\n", file->source_name);
	if (file->url != 0)
	{
		fprintf(f, "URL: <A HREF=\"%s\">%s</A>\n", file->url, file->url);
		//fprintf(fout, "Source: %s, URL: %s\n", file->source_name, file->url);
	}
	else
		fprintf(f, "<B>No URL</B>\n");
	
	size_t len = strlen(file->source_name);
	
	if (   (len > 7 && strcmp(file->source_name + len - 7, ".tar.gz") == 0)
		|| (len > 8 && strcmp(file->source_name + len - 8, ".tar.bz2") == 0))
	{
		fprintf(f, "(Not shown)\n");
		fclose(f_source);
		return;
	}
	
	output_file(f, f_source, binary);
}


void write_html(FILE *f)
{
	fprintf(f, 
		"<HTML><HEAD>\n<TITLE>MES-replacement listing</TITLE>\n"
		"</HEAD><BODY>\n\n<H1>MES-replacement listing</H1>"
		"<!--ONEWAY-->\n"
		"This page gives all the steps that are taken to arrive at <tt>tcc-boot0</tt>\n"
		"executable that is similar to the one in the <A HREF=\"https://github.com/fosslinux/live-bootstrap\"\n"
		">fosslinux/live-bootstrap</A> project for this project, which does not rely on\n"
		"the GNU Mes compiler. (The header files and the source for a standard library\n"
		"that are part of the GNU Mes compiler, are used.)\n"
		"<p>"
		"This page is generated the <A HREF=\"https://github.com/FransFaase/MES-replacement/tree/%s/scan_trace.cpp\"\n"
		"><TT>scan_trace.cpp</TT></A>, which parses the <TT>trace.txt</TT> file that is produced by\n"
		"running the <A HREF=\"https://github.com/FransFaase/MES-replacement/blob/main/task3.sh\"\n"
		"><TT>task3.sh</TT></A> Bash script (where the line with the <tt>chroot</tt> command is uncommented)\n"
		"for the commit <A HREF=\"https://github.com/FransFaase/MES-replacement/tree/%s\">\n"
		"<tt>%.8s</TT></A>.\n"
		"<P>\n"
		"<UL>\n"
		"<LI><A HREF=\"#Seeds\">Binary seeds files</A>\n"
		"<LI><A HREF=\"#Processes\">Processes</A>\n"
		"<LI><A HREF=\"#Input\">Input source files</A>\n"
		"<LI><A HREF=\"#Output\">Output files</A>\n"
		"<LI><A HREF=\"#Parser\">Parse program</A>\n"
		"</UL>\n", live_bootstrap_commit, live_bootstrap_commit, live_bootstrap_commit);

	fprintf(f, "\n\n<H2><A NAME=\"Seeds\">Binary seeds files</A></H2>\n\n");
	
	for (File *file = files; file != 0; file = file->next)
		if (file->exec_before_created())
			write_html_file(f, file, true);
	 
	fprintf(f, "\n<H2><A NAME=\"Processes\">Processes</A></H2>\n\n");
	for (Process *process = all_processes; process != 0; process = process->next)
	{
		fprintf(f, "<H3><A NAME=\"S%d\">Process %d</A></H3>\n\n", process->nr, process->nr);
		if (process->parent != 0)
			fprintf(f, "(Executed by <A HREF=\"#S%d\">Process %d</A>)\n", process->parent->nr, process->parent->nr);
		fprintf(f, "<UL>\n");
		for (Action *action = process->actions; action != 0; action = action->next_in_process)
		{
			if (action->kind == 'E' && action->child_process != 0)
			{
				fprintf(f, "<LI>Executes <A HREF=\"#S%d\">Process %d</A>\n", action->child_process->nr, action->child_process->nr);
			}
			else
			{
				const char *oper = action->oper_name();
				if (oper != 0)
				{
					bool repeated = false;
					for (Action *prev_action = process->actions; prev_action != action; prev_action = prev_action->next_in_process)
						if (prev_action->kind == 'o' && prev_action->file == action->file && prev_action->oper_name() == oper)
						{
							repeated = true;
							break;
						}
					
					if (!repeated)
					{
						File *file = action->file;
						fprintf(f, "<LI>%s ", oper);
						
						if (action->is_produced())
						{
							fprintf(f, "%s\n<UL>\n", file->name);
							for (Action *file_action = action->next_on_file; file_action != 0; file_action = file_action->next_on_file)
								if (file_action->kind == 'r')
								{
									fprintf(f, "<LI>Deleted by <A HREF=\"#S%d\">process %d</A>\n", file_action->process->nr, file_action->process->nr);
									break;
								}
								else if (file_action->kind == 'o' && (file_action->o_creat || ((file_action->o_wronly || file_action->o_rdwr) && file_action->o_trunc)))
									break;
								else if (file_action->kind == 'e' || file_action->o_rdonly)
									fprintf(f, "<LI>%s <A HREF=\"#S%d\">process %d</A>\n",
										file_action->kind == 'e' ? "Used as executable" : 
										file_action->kind == 'o' ? (file_action->o_rdonly ? "Used as input" : file_action->o_wronly ? "Produced by" : file_action->o_rdwr ? "Modified by" : "Modified by") :
										"Used in",
										file_action->process->nr, file_action->process->nr);
							fprintf(f, "</UL>\n\n");
						}
						else
						{
							Process *produced_by = 0;
							for (Action *file_action = file->actions; file_action != 0; file_action = file_action->next_on_file)
								if (file_action->process == process)
									break;
								else if (file_action->kind == 'r')
									produced_by = 0;
								else if (file_action->is_produced())
									produced_by = file_action->process;
							if (file->is_source)
								fprintf(f, "<A HREF=\"#F%d\">%s</A>", file->nr, file->name);
							else
								fprintf(f, "%s", action->file->name);
							File *file_copy_from = file;
							while (file_copy_from->copy_from != 0)
								file_copy_from = file_copy_from->copy_from;
							if (file_copy_from->url != 0)
							{
								fprintf(f, " from <A HREF=\"%s\">source</A>", file_copy_from->url);
								if (file != file_copy_from)
									fprintf(f, " (through copy)");
								if (produced_by)
									fprintf(f, " (produced by <A HREF=\"#S%d\">process %d</A>)", produced_by->nr, produced_by->nr);
							}
							else if (produced_by != 0)
								fprintf(f, " produced by <A HREF=\"#S%d\">process %d</A>", produced_by->nr, produced_by->nr);
							fprintf(f, "\n");
						}
					}
				}
			}
		}
		fprintf(f, "</UL>\n\n");
		
		if (process->nr == 731)
		{
			Source *sources = 0;
			//fprintf(fout, "Process %d\n", process->nr);
			collect_sources(process, &sources);
			
			fprintf(f, "<P>Sources used:\n<UL>\n");
			for (Source *source = sources; source != 0; source = source->next)
				fprintf(f, "<LI> %s\n", source->url);
			fprintf(f, "</UL>\n");
		}
	}
		
	fprintf(f, "<H2><A NAME=\"Input\">Input source files</A></H2>\n\n");
	
	for (File *file = files; file != 0; file = file->next)
		if (file->used_as_input()) //(file->is_source && !file->exec_before_created())
			write_html_file(f, file, false); 

	fprintf(f, "\n<H2><A NAME=\"Output\">Output files</A></H2>\n\n\n");
 
 	for (int t = 0; t < 3; t++)
 	{
 		switch (t)
 		{
 			case 0: fprintf(f, "Executables files:\n<UL>\n"); break;
 			case 1: fprintf(f, "Intermediary files (not from sources and used):\n<UL>\n"); break;
 			case 2: fprintf(f, "Produced (not from source and also not used):\n<UL>\n"); break;
 		}
		for (File *file = files; file != 0; file = file->next)
			if (!file->used_as_input() && !file->exec_before_created())
			{
				bool used = false;
				bool executed = false;
				unsigned long mode = 0;
				int process_nr = -1;
				for (Action *action = file->actions; action != 0; action = action->next_on_file)
				{
					if (action->kind == 'e')
						executed = true;
					else if (action->kind == 'o')
					{
						if (action->o_creat || action->o_wronly)
						{
							mode = action->mode;
							process_nr = action->process->nr;
						}
						else if (action->o_rdonly || (action->o_rdwr && !action->o_trunc))
							used = true;
					}
					else if (action->kind == 'r')
					{
						used = false;
						executed = false;
						process_nr = -1;
					}
					else if (action->kind == 'c')
					{
						mode = action->mode;
					}
				}
				if (process_nr != -1)
				{
					bool is_executable = (mode & 0700) == 0700;
					if (   (t == 0 && is_executable)
						|| (t == 1 && file->url == 0 && !is_executable && used)
						|| (t == 2 && file->url == 0 && !is_executable && !used))
					{
						fprintf(f, "<LI> %s", file->name);
						if (process_nr > 0)
							fprintf(f, " produced by <A HREF=\"#S%d\">Process %d</A>", process_nr, process_nr);
						if (mode != 0 && mode != 0600 && mode != 0700)
							fprintf(f, " (mode is %lo)", mode);
						if (executed)
							fprintf(f, " (also executed)");
						fprintf(f, "\n");
					}
				}
			}
		fprintf(f, "</UL>\n\n");
	}

	fprintf(f, "\n<H2><A NAME=\"Parser\">Parse program</A></H2>\n\n");
	fprintf(f, "Below the Bash script <TT>task3.sh</TT> to produce the <TT>trace.txt</TT> file.\n"
			"(The line with <tt>chroot</TT> should be uncommented.)<P>\n");
	output_file(f, fopen("task3.sh", "r"), false);
	fprintf(f, "Below the version of the <TT>scan_trace.cpp</TT> program is given that is used to produce this page.\n<P>\n");
	output_file(f, fopen("scan_trace.cpp", "r"), false);

	fprintf(f,
		"\n\n"
		"<P><HR>\n"
		"<ADDRESS>\n"
		"<A HREF=\"index.html\">Home</A>\n"
		"</ADDRESS>\n"
		"</BODY></HTML>\n");
}

bool only_graph = false;

void write_json(FILE *f)
{
	// calculate json_kind and file_created_by
	for (File *file = files; file != 0; file = file->next)
	{
		bool file_exists = false;
		Process *file_created_by = 0;
		for (Action *action = file->actions; action != 0; action = action->next_on_file)
		{
			if (action->kind == 'r')
				file_exists = false;
			else if (action->kind == 'o')
			{
				if (action->o_rdonly)
					action->json_kind = 'R';
				else if (!file_exists || action->o_wronly)
				{
					action->json_kind = 'W';
					file_exists = true;
					file_created_by = action->process;
				}
				else
					action->json_kind = 'M';
			}
			action->file_created_by = file_created_by;
		}
	}

	fprintf(f, "var data = {\n  processes:[\n");
	for (Process *process = all_processes; process != 0; process = process->next)
	{
		fprintf(f, "\t{ nr:%d, x:null, y:0, w:0, h:0, ie:\"\", iw:0, oe:\"\", ow:0, elf:null, ins:[], outs:[]", process->nr);
		if (process->parent != 0)
			fprintf(f, ", parent:%d", process->parent->nr);
		fprintf(f, ", actions:[");
		bool first = true;
		bool is_M2_Mesoplanet = false;
		for (Action *action = process->actions; action != 0; action = action->next_in_process)
		{
			// Some clean-up for M2_Mesoplanet driver program
			if (action->kind == 'e' && (strcmp(action->file->name, "/x86/bin/M2-Mesoplanet") == 0 || strcmp(action->file->name, "/usr/bin/M2-Mesoplanet") == 0))
				is_M2_Mesoplanet = true;
			if (is_M2_Mesoplanet)
			{
				if (action->json_kind == 'W' && strstr(action->file->name, "/M2-Mesoplanet-000000") == 0)
					continue;
				if (action->json_kind == 'R' && action->next_in_process != 0 && action->next_in_process->kind == 'E')
					continue;
			}
			if (   action->json_kind == 'R' && action->next_in_process != 0 && action->next_in_process->kind == 'E'
				&& action->next_on_file != 0 && action->next_on_file->kind == 'e' 
				&& action->next_in_process->child_process == action->next_on_file->process)
				continue;
			
			if (action->json_kind == 'R')
			{
				bool already_include = false;
				for (Action *prev_action = process->actions; prev_action != action; prev_action = prev_action->next_in_process)
					if (prev_action->json_kind == 'R' && prev_action->file == action->file)
					{
						already_include = true;
						break;
					}
				if (already_include)
					continue;
			}

			fprintf(f, "%s%s{ kind:\"%c\"", first ? "" : ",", only_graph ? "" : "\n\t\t", action->json_kind);
			if (action->file != 0)
			{
				fprintf(f, ", file:%d", action->file->nr);
				if (action->file_created_by != 0)
					fprintf(f, ", by:%d", action->file_created_by->nr);
			}
			if (action->child_process != 0)
				fprintf(f, ", child:%d", action->child_process->nr);
			fprintf(f, " }");
			first = false;
		}
		fprintf(f, "%s}%s\n", only_graph ? "]" : first ? "] \n" : "\n\t  ]\n\t", process->next != 0 ? "," : "");
	}
	fprintf(f, "  ],\n  files:[\n");

	for (File *file = files; file != 0; file = file->next)
	{
		fprintf(f, "\t{ nr:%d, name:\"%s\", type:\"%s\", x:null, y:0, label:\"\", w:0", file->nr, file->name,
				file->exec_before_created() ? "seed" : "");
		if (!only_graph)
		{
			if (file->is_source && file->source_name != 0)
				fprintf(f, ", src:\"%s\"",
						file->source_name /*+ (strncmp(file->source_name, source_dir, len_source_dir) == 0 ? len_source_dir : 0)*/);
			if (file->url != 0)
				fprintf(f, ", url:\"%s\"", file->url);
			if (file->copy_from != 0)
				fprintf(f, ", copy_from:%d", file->copy_from->nr);
		}
		fprintf(f, ", actions:[");
		bool first = true;
		Action *prev_action = 0;
		for (Action *action = file->actions; action != 0; action = action->next_on_file)
		{
			if (prev_action == 0 || action->json_kind != prev_action->json_kind || action->process->nr != prev_action->process->nr)
			{
				fprintf(f, "%s%s{ kind:\"%c\", proc:%d }", first ? "" : ",", only_graph ? "" : "\n\t\t", action->json_kind, action->process->nr);
				first = false;
			}
			prev_action = action;
		}
		fprintf(f, "%s]", only_graph || first ? "" : "\n\t  ");
		if (file->is_source && file->source_name != 0 && !only_graph)
		{
			size_t len = strlen(file->source_name);
			
			if (   (len <= 7 || strcmp(file->source_name + len - 7, ".tar.gz") != 0)
				&& (len <= 8 || strcmp(file->source_name + len - 8, ".tar.bz2") != 0))
			{
				FILE *f_source = fopen(file->source_name, "r");
				if (f_source != 0)
				{
					fprintf(f, ",\n\t  lines:[\n");
					if (file->exec_before_created())
					{
						fprintf(f, "\t\t\"");
						int i = 0;
						unsigned char ch = fgetc(f_source);
						while (!feof(f_source))
						{
							if (i == 10)
							{
								fprintf(f, "\",\n\t\t\"");
								i = 0;
							}
							fprintf(f, "%s%02X", i == 0 ? "" : " ", ch);
							i++;
							ch = fgetc(f_source);
						}
						fprintf(f, "\"\n");
					}
					else
					{
						char ch = fgetc(f_source);
						bool first = true;
						if (ch != -1)
						{
							int col = 0;
							bool in_line = false;
							while (!feof(f_source))
							{
								if (ch == '\n' && in_line)
								{
									fprintf(f, "\"");
									in_line = false;
									ch = fgetc(f_source);
									col = 0;
									continue;
								}
								if (!in_line)
								{
									fprintf(f, "%s\t\t\"", first ? "" : ",\n");
									first = false;
									if (ch == '\n')
									{
										fprintf(f, "\"");
										ch = fgetc(f_source);
										continue;
									}
									in_line = true;
								}
								if (ch < ' ' && ch != '\t')
								{
									ch = fgetc(f_source);
									continue;
								}
								col++;
								if (ch == '"')
									fprintf(f, "\\" "\"");
								else if (ch == '\\')
									fprintf(f, "\\\\");
								else if (ch == '<')
									fprintf(f, "&lt;");
								else if (ch == '>')
									fprintf(f, "&gt;");
								else if (ch == '&')
									fprintf(f, "&amp;");
								else if ((unsigned char)ch == 160)
									fprintf(f, "&nbsp;");
								else if ((unsigned char)ch == 169)
									fprintf(f, "&copy;");
								else if ((unsigned char)ch == 194)
									fprintf(f, "&Acirc;");
								else if ((unsigned char)ch == 195)
									fprintf(f, "&Atilde;");
								else if ((unsigned char)ch == 197)
									fprintf(f, "&Aring;");
								else if ((unsigned char)ch == 216)
									fprintf(f, "&Oslash;");
								else if ((unsigned char)ch == 231)
									fprintf(f, "&ccedil;");
								else if ((unsigned char)ch == 246)
									fprintf(f, "&ouml;");
								else if (ch < 0)
									fprintf(f, "&#%d;", (unsigned char)ch);
								else if (ch == '\t')
								{
									fprintf(f, " ");
									while (col % 4 != 0)
									{
										fprintf(f, " ");
										col++;
									}
								}
								else
									fprintf(f, "%c", ch);
								ch = fgetc(f_source);
							}
							if (col > 0)
								fprintf(f, "\"\n");
						}
					}
					fprintf(f, "\t  ]\n");
				}
			}
		}
		fprintf(f, "%s}", first && !file->is_source ? " " : "\t");
		fprintf(f, "%s\n", file->next != 0 ? "," : "");
	}
	fprintf(f, "  ]\n");
	fprintf(f, "}\n");
	return;
		 
	for (Process *process = all_processes; process != 0; process = process->next)
	{
		fprintf(f, "<H3><A NAME=\"S%d\">Process %d</A></H3>\n\n", process->nr, process->nr);
		if (process->parent != 0)
			fprintf(f, "(Executed by <A HREF=\"#S%d\">Process %d</A>)\n", process->parent->nr, process->parent->nr);
		fprintf(f, "<UL>\n");
		
		if (process->nr == 731)
		{
			Source *sources = 0;
			//fprintf(fout, "Process %d\n", process->nr);
			collect_sources(process, &sources);
			
			fprintf(f, "<P>Sources used:\n<UL>\n");
			for (Source *source = sources; source != 0; source = source->next)
				fprintf(f, "<LI> %s\n", source->url);
			fprintf(f, "</UL>\n");
		}
	}
		
	fprintf(f, "<H2><A NAME=\"Input\">Input source files</A></H2>\n\n");
	
	for (File *file = files; file != 0; file = file->next)
		if (file->used_as_input()) //(file->is_source && !file->exec_before_created())
			write_html_file(f, file, false); 

	fprintf(f, "\n<H2><A NAME=\"Output\">Output files</A></H2>\n\n\n");
 
 	for (int t = 0; t < 3; t++)
 	{
 		switch (t)
 		{
 			case 0: fprintf(f, "Executables files:\n<UL>\n"); break;
 			case 1: fprintf(f, "Intermediary files (not from sources and used):\n<UL>\n"); break;
 			case 2: fprintf(f, "Produced (not from source and also not used):\n<UL>\n"); break;
 		}
		for (File *file = files; file != 0; file = file->next)
			if (!file->used_as_input() && !file->exec_before_created())
			{
				bool used = false;
				bool executed = false;
				unsigned long mode = 0;
				int process_nr = -1;
				for (Action *action = file->actions; action != 0; action = action->next_on_file)
				{
					if (action->kind == 'e')
						executed = true;
					else if (action->kind == 'o')
					{
						if (action->o_creat || action->o_wronly)
						{
							mode = action->mode;
							process_nr = action->process->nr;
						}
						else if (action->o_rdonly || (action->o_rdwr && !action->o_trunc))
							used = true;
					}
					else if (action->kind == 'r')
					{
						used = false;
						executed = false;
						process_nr = -1;
					}
					else if (action->kind == 'c')
					{
						mode = action->mode;
					}
				}
				if (process_nr != -1)
				{
					bool is_executable = (mode & 0700) == 0700;
					if (   (t == 0 && is_executable)
						|| (t == 1 && file->url == 0 && !is_executable && used)
						|| (t == 2 && file->url == 0 && !is_executable && !used))
					{
						fprintf(f, "<LI> %s", file->name);
						if (process_nr > 0)
							fprintf(f, " produced by <A HREF=\"#S%d\">Process %d</A>", process_nr, process_nr);
						if (mode != 0 && mode != 0600 && mode != 0700)
							fprintf(f, " (mode is %lo)", mode);
						if (executed)
							fprintf(f, " (also executed)");
						fprintf(f, "\n");
					}
				}
			}
		fprintf(f, "</UL>\n\n");
	}

	fprintf(f, "\n<H2><A NAME=\"Parser\">Parse program</A></H2>\n\n");
	fprintf(f, "Below the Bash script <TT>task3.sh</TT> to produce the <TT>trace.txt</TT> file.\n"
			"(The line with <tt>chroot</TT> should be uncommented.)<P>\n");
	output_file(f, fopen("task3.sh", "r"), false);
	fprintf(f, "Below the version of the <TT>scan_trace.cpp</TT> program is given that is used to produce this page.\n<P>\n");
	output_file(f, fopen("scan_trace.cpp", "r"), false);
	
	fprintf(f,
		"\n\n"
		"<P><HR>\n"
		"<ADDRESS>\n"
		"<A HREF=\"index.html\">Home</A>\n"
		"</ADDRESS>\n"
		"</BODY></HTML>\n");
}

int main(int argc, char *argv[])
{
	const char *data_js_filename = "docs/data.js";
	
	if (argc == 3 && strcmp(argv[1], "-d") == 0)
	{
		only_graph = true;
		data_js_filename = argv[2];
	}
	
	len_source_dir = strlen(source_dir);

	init_subModules();
	
	if (!process_trace_file("trace.txt"))
		return 0;
	
	for (Process *process = all_processes; process != 0; process = process->next)
	{
		Action *action = process->actions;
		if (   action != 0 && action->kind == 'e'
			&& action->file != 0 && strcmp(action->file->name, "/usr/bin/cp") == 0)
		{
			action = action->next_in_process;
			if (action != 0 && action->kind == 'o' && action->o_rdonly)
			{
				File *source = action->file;
				action = action->next_in_process;
				if (action != 0 && action->kind == 'o' && action->o_wronly)
				{
					fprintf(fout, "Copy %s -> %s\n", source->name, action->file->name);
					action->file->copy_from = source;
				}
			}
		}
	}
		

	
	if (!only_graph)
	{
		FILE *f_html = fopen("docs/listing.html", "w");
		if (f_html != 0)
		{
			write_html(f_html);
			fclose(f_html);
		}
	}
		
	FILE *f_json = fopen(data_js_filename, "w");
	if (f_json != 0)
	{
		write_json(f_json);
		fclose(f_json);
	}
	
	return 0;
}
