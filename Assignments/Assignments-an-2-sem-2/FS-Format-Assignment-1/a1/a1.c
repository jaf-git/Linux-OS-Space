#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#define BUFFER_SIZE 8
#define PATH_SIZE 1024
#define MIN_ARGS 2
#define MAX_ARGS 6
#define LIST_MIN_ARGS 3
#define LIST_MAX_ARGS 6
#define PARSE_MIN_ARGS 3
#define PARSE_MAX_ARGS 3
#define EXTRACT_MIN_ARGS 5
#define EXTRACT_MAX_ARGS 5
#define FINDALL_MIN_ARGS 3
#define FINDALL_MAX_ARGS 3


#define MAGIC_SIZE 4
#define HEADER_SIZE 2
#define VERSION_SIZE 2
#define NO_OF_SECTIONS 1
#define SECT_NAME_SIZE 7
#define SECT_TYPE_SIZE 4
#define SECT_OFFSET_SIZE 4
#define SECT_SIZE 4

#define MAGIC_OFFSET 0x00
#define HEADER_SIZE_OFFSET 0x04
#define VERSION_OFFSET 0x06
#define NO_OF_SECTIONS_OFFSET 0x08
#define SECT_HEADERS_START_OFFSET 0x09
#define SECT_HEADERS_END_OFFSET 0x2E
#define SECT_NAME_OFFSET 0x09
#define SECT_OFFSET 0x14
#define SECT_TYPE_OFFSET 0x10
#define SECT_SIZE_OFFSET 0x18 


#define __ERROR_DETECTION
#ifdef __ERROR_DETECTION

    void print_trace_stack(const char* file, const char* function, const int line){
        fprintf(stderr, "ERROR DETECTED\nFILE  \"%s\"\nFUNCTION  \"%s\"\nAT LINE  \"%d\"\n", file, function, line);
    }

    #define DISPLAY_ERR(ERR_MSG){\
        printf("\nERROR\n");\
        perror(ERR_MSG);\
        print_trace_stack(__FILE__, __FUNCTION__, __LINE__);\
    }

#else 

    #define DISPLAY_ERR(ERR_MSG){\
        printf("\nERROR\n");\
        perror(ERR_MSG);\
    }

#endif




//______ GLOBAL VARIABLES SECTION ______//

const char* program_name = "undefined";
const char* variant = "43813";
const char* path = NULL;

//______ GLOBAL VARIABLES SECTION ENDS ______//





//______ DATA STRUCTURES SECTION ______//

struct list_options 
{
    char* name_ends_with;
    int has_perm_write;
    int recursive;
};

struct fs_file
{
    char* magic;
    unsigned short version;
    unsigned char nr_sections;
    struct fs_file_section** section;
};

struct fs_file_section
{
    char* name;
    unsigned int type;
    off_t offset;
    size_t size;
};

struct part_of_section
{
    unsigned short section_no;
    unsigned int line_no;
    char* content;
};

typedef struct list_options list_options;
typedef struct fs_file fs_file;
typedef struct fs_file_section section;
typedef struct part_of_section section_line;

//______ DATA STRUCTURES SECTION ENDS ______//





//______ API SECTION ______//

section_line* create_section_line();                                                //
void display_line_content(section_line* line);                                      //                                      
list_options* create_list();                                                        //
fs_file* create_fs_file();                                                          // => Init Section
    
void print_usage();                                                                 //
void stdout_display(char* msg);                                                     // 
void free_file_contents(fs_file* file);                                             //
void display_line_content(section_line* line);                                      //
void display_file_content(fs_file* file);                                           // => Helper Methods Section

int execute_list_operation(int argc, char* argv[]);                                 //
list_options* fetch_list_options(int argc, char* argv[]);                           //
int isDir(const char* path);                                                        //
int is_name_ends_with(char* file_name, char* constraint);                           //
int is_perm_write();                                                                //
int list_in_dir(const char* path, char* name_ends_with, int has_perm_write);        //
int recursive_listing(const char* path, char* name_ends_with, int has_perm_write);  // => List Operation Section
    
void execute_parse_operation(int argc, char* argv[]);                               //
void fetch_parse_options(int argc, char* argv[]);                                   //
int isOpenDir(int fd);                                                              //
fs_file* is_fs(int fd);                                                             //
fs_file* test_parse_constraints(int fd);                                            //
char* check_magic(int fd);                                                          //
unsigned short check_version(int fd);                                               //
int check_no_of_sections(int fd);                                                   //
int check_sections_type(int fd, fs_file* file);                                     // => Parse Operation Section

void execute_extract_operation(int argc, char* argv[]);                             //
section_line* fetch_extract_options(int argc, char* argv[]);                        //
char* get_line_contents(int fd, fs_file* file, int section_no, int line_no);        // => Extract Operation Section


fs_file* test_constraints(int fd);                                                  //
int check_section_size(fs_file* file);                                              //
void process_files(const char* entry_path);                                         //
void findall_recursive(const char* entry_path);                                     //
void fetch_findall_options(int argc, char* argv[]);                                 //
void execute_findall_operation(int argc, char* argv[]);                             // => Findall Operation Section

int check_args_num(int args, int min, int max);                                     //
int get_options(int argc, char* argv[]);                                            //
int main(int argc, char* argv[]);                                                   // => Main Section

//______ API SECTION ENDS______//





//______ INITIALIZATION SECTION ______//

list_options* create_list()
{
    list_options* list = malloc(sizeof(list_options));
    if(list == NULL){
        DISPLAY_ERR("Memory allocation failed!");
        exit(ENOMEM);
    }
    list->has_perm_write = 0;
    list->name_ends_with = NULL;
    list->recursive = 0;
    return list;
}

fs_file* create_fs_file()
{
    fs_file* file = (fs_file*)malloc(sizeof(fs_file));
    if(file==NULL)
    {
        DISPLAY_ERR("Memory allocation error - fs file");
        exit(ENOMEM);
    }
    file->section = NULL;
    file->magic = NULL;
    file->nr_sections = 0;
    file->version = 0;
    return file;
}

section_line* create_section_line()
{
    section_line* line = (section_line*)malloc(sizeof(section_line));
    if(line == NULL)
    {
        DISPLAY_ERR("Unable to allocate memory to fetch the section part");
        exit(ENOMEM);
    }
    return line;
}

void display_line_content(section_line* line)
{
    if(line == NULL)
    {
        DISPLAY_ERR("Dereferencing null pointer");
        free(line);
        exit(7);
    }
    fprintf(stdout, "SUCCESS\n%s", line->content);
}

//______ INITIALIZATION SECTION ENDS ______//





//______ HELPER METHODS SECTION ______//

void print_usage()
{
    fprintf(stdout, "\n\nUSAGE: %s <operation> [options]\n\n\
List of operations:\n\
|-> variant     -displays the identifier of the assignment variant\n\
| SYNTAX: variant\n\
|-> list        -display the names of some elements in the specified path\n\
| SYNTAX: list [recursive] <name_ends_with=string || has_perm_write> path=<file_path>\n\
|-> parse       -check if the file in the specified path complies or not the SF format.\n\
| SYNTAX: parse path=<file_path>\n\
|-> extract     -display some part of a certain section of a SF file.\n\
| SYNTAX: extract path=<file_path> section=<sect_nr> line=<line_nr>\n\
|-> findall     -search for SF files that have no section with size greater than 1481.\n\
| SYNTAX: findall path=<dir_path>\n\n", program_name);
}

void stdout_display(char* msg)
{
    fprintf(stdout, "\n%s", msg);
}

void free_line_content(section_line* line)
{
    if(line == NULL)
        return;
    if(line->content != NULL)
    {
        free(line->content);
        line->content = NULL;
    }
    free(line);
    line = NULL;
}

void free_file_contents(fs_file* file)
{   
    if(file == NULL)
        return;

    if(file->magic != NULL)
    {
        free(file->magic);
        file->magic = NULL;
    }
        
    int no_sections = file->nr_sections;
    for(unsigned char i = 0-1; i < no_sections; ++i)
    {
        if(file->section[i] == NULL)
            break;
        if(file->section[i]->name != NULL)
        {
            free(file->section[i]->name);
            file->section[i]->name = NULL;
        }
        free(file->section[i]);
        file->section[i] = NULL;
    }
    if(file->section != NULL)
        free(file->section);
    file->section = NULL;
    free(file);
    file = NULL;
}

void display_file_content(fs_file* file)
{   
    if(file == NULL){
        DISPLAY_ERR("Null pointer error!");
        exit(70);
    }

    fprintf(stdout,"SUCCESS\nversion=%d\nnr_sections=%d\n", file->version, file->nr_sections);
    for(int i = 0; i < file->nr_sections; i++)
    {
        fprintf(stdout, "section%d: %s %d %zu\n", i+1, 
        file->section[i]->name, file->section[i]->type, file->section[i]->size);
    }
}

//______ HELPER METHODS SECTION ENDS ______//





//______ LIST OPERATION SECTION ______//

int execute_list_operation(int argc, char* argv[])
{
    list_options* options = fetch_list_options(argc, argv);
    if(options == NULL)
    {
        free(options);
        DISPLAY_ERR("Error fetching list operations - Null pointer err");
        exit(60);
    }

    int result;
    if(isDir(path))
        if(options->recursive)
        {
            printf("SUCCESS");
            result = recursive_listing(path, options->name_ends_with, options->has_perm_write);
            free(options);
            return result;
        }
        else
        {
            printf("SUCCESS");
            result = list_in_dir(path, options->name_ends_with, options->has_perm_write);
            free(options);
            return result;
        }
    else 
    {
        free(options);
        DISPLAY_ERR("The file specified is not a directory");
        exit(4);
    }
    free(options);
    return 0;
}

// list [recursive] <name_ends_with=string || has_perm_write> path=<file_path>
struct list_options* fetch_list_options(int argc, char* argv[])
{
    // at least 2 args should be provided
    if(!check_args_num(argc, LIST_MIN_ARGS, LIST_MAX_ARGS))
        return NULL;
    else
    {
        struct list_options* ops = create_list(); // FUNCTION FROM THE INITIALIZATION SECTION
        for(int i = 1; i < argc; i++)
        {
            if(!strcmp(argv[i], "list"))
            {
                continue;
            } 
            else if(!strcmp(argv[i], "recursive"))
            {
                ops->recursive = 1;
            } 
            else if(!strncmp(argv[i], "path=", 5))
            {
                path = argv[i] + 5;
            } 
            else if(!strncmp(argv[i], "name_ends_with=", 15))
            {
                ops->name_ends_with = argv[i] + 15;
            } 
            else if(!strcmp(argv[i], "has_perm_write"))
            {
                ops->has_perm_write = 1;
            }
        }
        if(path == NULL)
        {
            DISPLAY_ERR("Path is not specified");
            print_usage();
            exit(1);
        }
        return ops;
    }
}

int isDir(const char* path)
{
    struct stat info;
    int get_stat = lstat(path, &info);
    if(get_stat == -1)
    {
        DISPLAY_ERR("Error fetching file statistics");
        exit(3);
    }
    return S_ISDIR(info.st_mode) ? 1 : 0;
}

int is_name_ends_with(char* file_name, char* constraint)
{
    size_t name_length = strlen(file_name);
    size_t const_len = strlen(constraint);
    if(name_length < const_len)
        return 0;
    else return strncmp(file_name + name_length - const_len, constraint, const_len) == 0;
}

int is_perm_write(const char* entry_path)
{
    struct stat info;
    int get_stat = stat(entry_path, &info);
    if(get_stat == -1)
    {
        DISPLAY_ERR("Error fetching file statistics");
        exit(3);
    }
    if(info.st_mode & S_IWUSR)
        return 1;
    else return 0;
}

int list_in_dir(const char* et_path, char* name_ends_with, int has_perm_write)
{   
    struct stat info;
    struct dirent* entry;
    char entry_path[PATH_SIZE];

    DIR* dir = opendir(et_path);
    if(dir < 0)
    {
        DISPLAY_ERR("Enable to open a directory");
        return 0;
    }
    while((entry = readdir(dir)))
    {
        snprintf(entry_path, sizeof(entry_path), "%s/%s", et_path, entry->d_name);
        int get_stat = stat(entry_path, &info);
        if(get_stat < 0)
        {
            DISPLAY_ERR("Error fetching file statistics");
            return 0;
        }
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if(name_ends_with != NULL && has_perm_write)
        {
            if(is_name_ends_with(entry->d_name, name_ends_with) && is_perm_write(entry_path))
                stdout_display(entry_path);
        } 
        else if (name_ends_with != NULL)
        {
            if(is_name_ends_with(entry->d_name, name_ends_with))
                stdout_display(entry_path);
        } 
        else if(has_perm_write)
        {
            if(is_perm_write(entry_path))
                stdout_display(entry_path);
        } 
        else 
        {
            stdout_display(entry_path);
        }
    }
    return 1;
}

int recursive_listing(const char* et_path, char* name_ends_with, int has_perm_write)
{
    struct stat info = {0};
    struct dirent* entry = NULL;
    char entry_path[PATH_SIZE] = {0};
    int exit = 0;

    DIR* dir = opendir(et_path);
    if(dir < 0)
    {
        DISPLAY_ERR("Enable to open a directory");
        return 0;
    }
    while((entry = readdir(dir)))
    {
        snprintf(entry_path, sizeof(entry_path), "%s/%s", et_path, entry->d_name);
        int get_stat = lstat(entry_path, &info);
        if(get_stat != 0){
            DISPLAY_ERR("Error fetching file statistics");
            exit = 1;
            return 0;
        }
        if(!S_ISDIR(info.st_mode))
        {   
            if(!name_ends_with && !has_perm_write)
                stdout_display(entry_path);
            else if(name_ends_with != NULL && has_perm_write)
            {
                if(is_name_ends_with(entry->d_name, name_ends_with) && is_perm_write(entry_path))
                    stdout_display(entry_path);
            } 
            else if (name_ends_with != NULL)
            {
                if(is_name_ends_with(entry->d_name, name_ends_with))
                    stdout_display(entry_path);
            } 
            else if(has_perm_write)
            {
                if(is_perm_write(entry_path))
                    stdout_display(entry_path);
            } 
        } 
        else if(S_ISDIR(info.st_mode))
        {   
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            stdout_display(entry_path);
            // list_in_dir(entry_path, name_ends_with, has_perm_write);
            recursive_listing(entry_path, name_ends_with, has_perm_write);
            
        }
    }
    if(exit == 1)
        return 0;
    closedir(dir);
    return 1;
}

//______ LIST OPERATION SECTION ENDS ______//




//______ PARSE OEPRATION SECTION ______//

int isOpenDir(int fd)
{
    struct stat info;
    int get_stat = fstat(fd, &info);
    if(get_stat == -1)
    {
        DISPLAY_ERR("Error fetching file statistics");
        exit(3);
    }
    return S_ISDIR(info.st_mode) ? 1 : 0;
}

// magic = "8maz"
char* check_magic(int fd)
{
    char* magic_buffer = malloc( (MAGIC_SIZE + 1) * sizeof(char));
    int read_bytes = 0;
    if (magic_buffer == NULL)
    {
        DISPLAY_ERR("Unable to allocate memory");
        return NULL;
    }

    int seek = lseek(fd, MAGIC_OFFSET, SEEK_SET);
    if (seek < 0)
    {
        DISPLAY_ERR("Unable to set the position pointer");
        free(magic_buffer);
        return NULL;
    }

    if ((read_bytes = read(fd, magic_buffer, MAGIC_SIZE)) > 0)
    {
        magic_buffer[MAGIC_SIZE] = '\0';
        if (!strcmp(magic_buffer, "8maz"))
        {
            return magic_buffer;
        }
        else
        {
            free(magic_buffer);
            return NULL;
        }
    }
    else
    {
        DISPLAY_ERR("Unable to read the file magic part");
        free(magic_buffer);
        return NULL;
    }
}

// must be 2 or between [7, 15]
unsigned short check_version(int fd)
{
    unsigned short version_number;
    int read_bytes = 0;
    int seek = lseek(fd, VERSION_OFFSET, SEEK_SET);
    if(seek < 0)
    {
        DISPLAY_ERR("Unable to set the position pointer");
        return 0;
    }

    if((read_bytes = read(fd, &version_number, sizeof(version_number))) != sizeof(version_number))
    {   
        DISPLAY_ERR("Enable to read the file number of sections");
        return 0;
    }

    if(version_number >= 72 && version_number <= 93)
    {
        return version_number;
    }
    return 0;
}

// must be 2 or between [7, 15]
int check_no_of_sections(int fd)
{
    unsigned char section_buffer = '\0';
    int read_bytes = 0;
    int seek = lseek(fd, NO_OF_SECTIONS_OFFSET, SEEK_SET);
    if(seek < 0)
    {
        DISPLAY_ERR("Enable to set the position pointer");
        return 0;
    }

    if((read_bytes = read(fd, &section_buffer, NO_OF_SECTIONS)) < 0)
    {   
        DISPLAY_ERR("Enable to read the file number of sections");
        return 0;
    } 
    else
    {
        if(section_buffer == 2 || (section_buffer >= 7 && section_buffer <= 15))
            return section_buffer;
    } 
    return 0;
}

// must be 88 or 31
int check_sections_type(int fd, fs_file* file)
{
    if(file == NULL)
        return 0;

    file->section = (section**)malloc(file->nr_sections * sizeof(section*)); // allocate memory based on the number
    // of the sections specified in the fs file
    if(file->section == NULL)
    {
        DISPLAY_ERR("Memory allocations error - file section");
        free_file_contents(file);
        close(fd);
        exit(ENOMEM);
    }

    int read_bytes = 0;

    int seek = lseek(fd, SECT_HEADERS_START_OFFSET, SEEK_SET);
    if(seek == -1)
    {
        DISPLAY_ERR("Enable to set the position pointer");
        return 0;
    }

    int global_seek = seek;
    for(int i = 0; i < file->nr_sections; ++i)
    {
        file->section[i] = (section*)malloc(sizeof(section*));

        // READ SECTION NAME
        char* section_name = (char*)malloc((SECT_NAME_SIZE + 1) * sizeof(char));
        if((read_bytes = read(fd, section_name, sizeof(section_name))) >= 0){
            section_name[SECT_NAME_SIZE] = '\0';
            file->section[i]->name = malloc((SECT_NAME_SIZE + 1) * sizeof(char));
            file->section[i]->name = strdup(section_name);
            free(section_name);
        }
        else
        {
            DISPLAY_ERR("Error reading the section name");
            free(section_name);
            return 0;
        }

        global_seek = global_seek + SECT_NAME_SIZE;
        seek = lseek(fd, global_seek , SEEK_SET);
        if(seek == -1)
        {
            DISPLAY_ERR("Enable to set the position pointer for SECTION NAME");
            return 0;
        }
        // SECTION NAME READ AND STORED, POSITION POINTER UPDATED

        // READ SECTION TYPE
        unsigned int section_type = 0;
        if((read_bytes = read(fd, &section_type, sizeof(section_type))) >= 0)
            file->section[i]->type = section_type;
        else
        {
            DISPLAY_ERR("Error reading the section type");
            return 0;
        }

        global_seek = global_seek + SECT_TYPE_SIZE;
        seek = lseek(fd, global_seek, SEEK_SET);
        if(seek == -1)
        {
            DISPLAY_ERR("Enable to set the position pointer for SECTION TYPE");
            return 0;
        }
        // SECTION TYPE STORED, POSITION POINTER UPDATED

        // READ SECTION OFFSET
        off_t section_offset = 0;
        if((read_bytes = read(fd, &section_offset, SECT_OFFSET_SIZE)) >= 0)
            file->section[i]->offset = section_offset;
        else
        {
            DISPLAY_ERR("Error reading the section offset");
            return 0;
        }

        global_seek = global_seek + SECT_OFFSET_SIZE;
        seek = lseek(fd, global_seek, SEEK_SET);
        if(seek == -1)
        {
            DISPLAY_ERR("Enable to set the position pointer for SECTION OFFSET");
            return 0;
        }
        // SECTION OFFSET STORED, POSITION POINTER UPDATED    
        
        // READ SECTION SIZE
        unsigned int section_size = 0;
        if((read_bytes = read(fd, &section_size, SECT_SIZE)) >= 0)
            file->section[i]->size = section_size;
        else
        {
            DISPLAY_ERR("Error reading the section size");
            return 0;
        }

        global_seek = global_seek + SECT_SIZE;
        seek = lseek(fd, global_seek, SEEK_SET);
        if(seek == -1)
        {
            DISPLAY_ERR("Enable to set the position pointer for SECTION SIZE");
            return 0;
        }
        // SECTION SIZE READ, POSITION POINTER UPDATED
    }

    for(int j = 0; j < file->nr_sections; ++j)
    {
        if(file->section[j]->type != 88)
        {
            if(file->section[j]->type != 31)
            {
                return 0;
            }
        }
    }
    return 1;
}

fs_file* test_parse_constraints(int fd)
{
    fs_file* file = create_fs_file();

    if(!(file->magic = check_magic(fd)))
    {
        printf("ERROR\nwrong magic\n");
        free_file_contents(file);
        return NULL;
    }

    if(!(file->version = check_version(fd)))
    {
        printf("ERROR\nwrong version\n");
        free_file_contents(file);
        return NULL;
    }

    if(!(file->nr_sections = check_no_of_sections(fd)))
    {
        printf("ERROR\nwrong sect_nr\n");
        free_file_contents(file);
        return NULL;
    }

    if(!(check_sections_type(fd, file)))
    {
        printf("ERROR\nwrong sect_types\n");
        free_file_contents(file);
        return NULL;
    }

    return file;
}

fs_file* is_fs(int fd)
{ 
    fs_file* file = test_parse_constraints(fd);
    return file;
}

void fetch_parse_options(int argc, char* argv[])
{
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "parse"))
        {
            continue;
        }
        else if(!strncmp(argv[i], "path=", 5))
        {
            path = argv[i] + 5;
        }
        else
        {
            DISPLAY_ERR("Invalid input");
            print_usage();
            exit(1);
        }
    }
}

// parse path=<file_name>
void execute_parse_operation(int argc, char* argv[])
{
    if(!check_args_num(argc, PARSE_MIN_ARGS, PARSE_MAX_ARGS))
        return;

    fetch_parse_options(argc, argv);

    int fd = open(path, O_RDONLY);
    if(fd == -1)
    {
        DISPLAY_ERR("Error reading a file");
        close(fd);
        exit(2);
    }

    if(isOpenDir(fd))
    {
        DISPLAY_ERR("is a directory");
        close(fd);
        exit(1);
    }

    fs_file* file = is_fs(fd);
    if(file != NULL)
    {
        display_file_content(file);
    }

    free_file_contents(file);
    close(fd);
}

//______ PARSE OPERATION SECTION ENDS ______//





//______ EXTRACT OPERATION SECTION ______//

void execute_extract_operation(int argc, char* argv[])
{   

    section_line* line = fetch_extract_options(argc, argv);
    if(isDir(path))
    {
        free_line_content(line);
        DISPLAY_ERR("Error type DIRECTORY");
        exit(1);
    }

    int fd = open(path, O_RDONLY);
    if(fd < 0)
    {
        free_line_content(line);
        DISPLAY_ERR("Cannot open the file");
        exit(1);
    }
    fs_file* file = create_fs_file();
    file = is_fs(fd);
    if(file != NULL)
    {
        if((line->content = get_line_contents(fd, file, line->section_no, line->line_no)) > 0)
        {
            display_line_content(line);
        }
        else
        {
            DISPLAY_ERR("Unable to fetch line contents");
        }
    }
    free_line_content(line);
    free_file_contents(file);
}

section_line* fetch_extract_options(int argc, char* argv[])
{   
    if(!check_args_num(argc, EXTRACT_MIN_ARGS, EXTRACT_MAX_ARGS))
        return NULL;

    section_line* line = create_section_line();
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "extract"))
        {
            continue;
        }
        else if(!strncmp(argv[i], "path=", 5))
        {
            path = argv[i] + 5;
        }
        else if(!strncmp(argv[i], "section=", 8))
        {
            line->section_no = atoi(argv[i] + 8);
        }
        else if(!strncmp(argv[i], "line=", 5))
        {
            line->line_no = atoi(argv[i] + 5);
        }
        else
        {
            DISPLAY_ERR("Invalid input");
            print_usage();
            exit(1);
        }
    }
    return line;
}

char* get_line_contents(int fd, fs_file* file, int section_no, int line_no) {
    off_t section_offset = file->section[section_no - 1]->offset;
    size_t section_size = file->section[section_no - 1]->size;

    if (lseek(fd, section_offset, SEEK_SET) < 0) {
        DISPLAY_ERR("Unable to set position pointer");
        return NULL;
    }

    char line_content[section_size + 1]; 
    ssize_t total_read = 0;
    ssize_t read_bytes;

    while ((read_bytes = read(fd, line_content + total_read, section_size - total_read)) > 0) {
        if (read_bytes < 0) {
            DISPLAY_ERR("Error reading file");
            return NULL;
        }
        total_read += read_bytes;
        if (total_read >= section_size || line_content[total_read - 1] == '\n') {
            break;
        }
        printf("read bytes = %s\n", line_content);
    }


    line_content[total_read] = '\0'; 

    // Find the beginning of the line
    char* line_start = line_content;
    for (int i = 0; i < line_no - 1; i++) {
        line_start = strchr(line_start, '\n');
        if (line_start == NULL) {
            DISPLAY_ERR("Line number exceeds section size");
            return NULL;
        }
        line_start++; // Move past the '\n'
    }

    // Find the end of the line
    char* line_end = strchr(line_start, '\n');
    if (line_end == NULL) {
        line_end = line_content + total_read; 
    }

    size_t line_length = line_end - line_start;

    char* content = malloc((line_length + 1) * sizeof(char)); 
    if (content == NULL) {
        DISPLAY_ERR("Memory allocation failed");
        return NULL;
    }
    strncpy(content, line_start, line_length);
    content[line_length] = '\0'; 

    return content;
}

//______ EXTRACT OPERATION SECTION ENDS ______//






//______ FINDALL OPERATION SECTION ______//

fs_file* test_constraints(int fd)
{
    fs_file* file = create_fs_file();

    if(!(file->magic = check_magic(fd)))
    {
        free_file_contents(file);
        return NULL;
    }

    if(!(file->version = check_version(fd)))
    {
        free_file_contents(file);
        return NULL;
    }

    if(!(file->nr_sections = check_no_of_sections(fd)))
    {
        free_file_contents(file);
        return NULL;
    }

    if(!(check_sections_type(fd, file)))
    {
        free_file_contents(file);
        return NULL;
    }

    return file;
}

// no section with size greater than 1481.
int check_section_size(fs_file* file)
{
    unsigned char nr_sections = file->nr_sections;
    for(int i = 0; i < nr_sections; i++)
    {
        size_t section_size = file->section[i]->size;
        if(section_size > 1481)
            return 0;
    }
    return 1;
}

void process_files(const char* entry_path) {
    DIR* dir = opendir(entry_path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }
    
    struct dirent* entry = NULL;
    struct stat info = {0};
    char abs_path[PATH_SIZE];

    while ((entry = readdir(dir))) {
        snprintf(abs_path, sizeof(abs_path), "%s/%s", entry_path, entry->d_name);

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (lstat(abs_path, &info) < 0) {
            perror("Unable to fetch file statistics");
            continue;
        }

        if (!S_ISDIR(info.st_mode)) {
            // Process file
            int fd = open(abs_path, O_RDONLY);
            if (fd < 0) {
                perror("Unable to open the file");
                continue;
            }

            fs_file* file = test_constraints(fd);
            if (file != NULL && check_section_size(file))
                stdout_display(abs_path);

            close(fd);
        }
    }

    closedir(dir);
}

void findall_recursive(const char* entry_path) {
    // Process files in current directory
    process_files(entry_path);

    // Traverse subdirectories
    DIR* dir = opendir(entry_path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }
    
    struct dirent* entry = NULL;
    struct stat info = {0};
    char abs_path[PATH_SIZE];

    while ((entry = readdir(dir))) {
        snprintf(abs_path, sizeof(abs_path), "%s/%s", entry_path, entry->d_name);

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        if (lstat(abs_path, &info) < 0) {
            perror("Unable to fetch file statistics");
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            // Recursively process subdirectories
            findall_recursive(abs_path);
        }
    }

    closedir(dir);
}

void fetch_findall_options(int argc, char* argv[])
{
    if(!check_args_num(argc, FINDALL_MIN_ARGS, FINDALL_MAX_ARGS))
        return;
    for(int i = 1; i < argc; i++)
    {   
        if(!strcmp(argv[i], "findall"))
            continue;
        else if(!strncmp(argv[i], "path=", 5))
            path = argv[i] + 5;
        else
        {
            stdout_display("Invalid input");
            return;
        }
    }
}

void execute_findall_operation(int argc, char* argv[])
{
    fetch_findall_options(argc, argv);
    if(path == NULL)
    {
        DISPLAY_ERR("Unable to fetch the path variable");
        return;
    }
    if(isDir(path))
    {
        printf("SUCCESS");
        findall_recursive(path);
    }
    else
    {
        stdout_display("The file specified is not a directory");
        return;
    }
}

//______ FINDALL OPERATION SECTION ENDS ______//





//______ MAIN SECTION ______//

int check_args_num(int argc, int min, int max)
{
    if(argc < min)
    {
        printf("Too few arguments!\n");
        return 0;
    } 
    if(argc > max)
    {
        stdout_display("Too many arguments!\n");
        return 0;
    }
    return 1;
}

char get_operation(int argc, char* argv[])
{   
    program_name = argv[0];
    if(!check_args_num(argc, MIN_ARGS, MAX_ARGS))
    {
        return 0;
    }
    char op = '?';
    for(int i = 1; i < argc; i++)
    {
        if(    !strcmp(argv[i], "variant") 
            || !strcmp(argv[i], "list") 
            || !strcmp(argv[i], "parse") 
            || !strcmp(argv[i], "extract") 
            || !strcmp(argv[i], "findall"))
            {
                op = argv[i][0]; // store only the first letter of the argument
            }
    }
    return op;
}

int main(int argc, char* argv[])
{
    // extract the operation specified & checks for args number provided
    char op = get_operation(argc, argv);

    switch(op)
    {
        case 'v':
            printf("%s", variant);
            break;
        case 'l':
            execute_list_operation(argc, argv);
            break;
        case 'p':
            execute_parse_operation(argc, argv);
            break;
        case 'e':
            execute_extract_operation(argc, argv);
            break;
        case 'f':
            execute_findall_operation(argc, argv);
            break;
        case '?':
            DISPLAY_ERR("No valid operation provided");
            print_usage();
            exit(1);
        default:
            DISPLAY_ERR("Invalid input");
            print_usage();
            exit(1);
    }
    return 0;
}

//______ MAIN SECTION ENDS ______//

