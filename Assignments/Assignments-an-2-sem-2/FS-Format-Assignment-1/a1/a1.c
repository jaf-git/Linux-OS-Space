#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>

#define PATH_SIZE 1024
#define MIN_ARGS 2
#define MAX_ARGS 6
#define LIST_MIN_ARGS 3
#define LIST_MAX_ARGS 6
#define PARSE_MIN_ARGS 3
#define PARSE_MAX_ARGS 3
#define MAX_NR_SECTIONS 16
#define MAGIC_SIZE 4
#define HEADER_SIZE 2
#define VERSION_SIZE 2
#define NO_OF_SECTIONS 1
#define SECT_NAME 7
#define SECT_TYPE 4
#define SECT_OFFSET 4
#define SECT_SIZE 4

#define MAGIC_OFFSET 0X00
#define HEADER_SIZE_OFFSET 0X04
#define VERSION_OFFSET 0X06
#define NO_OF_SECTIONS_OFFSET 0x08
#define SECTIONS_HEADERS_START_OFFSET 0X09
#define SECTIONS_HEADERS_END_OFFSET 0X2E
#define SECTION_NAME_OFFSET 0X09
#define SECTION_OFFSET 0X14
#define SECTION_TYPE_OFFSET 0X10
#define SECTION_SIZE 0X18 


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


// GLOBAL VARIABLES SECTION

const char* program_name = "undefined";
const char* variant = "43813";

// GLOBAL VARIABLES SECTION ENDS --


// DATA STRUCTURES SECTION

typedef struct list_options 
{
    char* path;
    char* name_ends_with;
    int have_perm_write;
    int recursive;
}list_options;

typedef struct fs_file
{
    char* magic;
    int version;
    int nr_sections;
    struct fs_file_section** section;
}fs_file;

typedef struct fs_file_section
{
    char* name;
    unsigned int type;
    unsigned int offset;
    unsigned int size;
}section;

// DATA STRUCTUES SECTION ENDS --


// INITIALIZATION SECTION

list_options* create_list()
{
    list_options* list = malloc(sizeof(list_options));
    if(list == NULL){
        DISPLAY_ERR("Memory allocation failed!");
        exit(ENOMEM);
    }
    list->path = NULL;
    list->have_perm_write = 0;
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

void display_file_content(fs_file* file)
{   
    if(file == NULL){
        DISPLAY_ERR("Null pointer error!");
        exit(70);
    }
    fprintf(stdout, "SUCCESS\nmagic=%s\nversion=%d\nnr_sections=%d\n", file->magic, file->version, file->nr_sections);
    for(int i = 0; i < file->nr_sections; i++)
    {
        fprintf(stdout, "section%d: %s %d %zu\n", i+1, 
        file->section[i]->name, file->section[i]->type, file->section[i]->size);
    }
}

// INITIALIZATION SECTION ENDS --



// API SECTION

int get_options(int argc, char* argv[]);
int check_args_num(int args, int min, int max);
void print_usage();
void stdout_display(char* msg);
struct list_options* fetch_list_options(int argc, char* argv[]);
int execute_list_operation(int argc, char* argv[]);
int isDir(char* path);
int isOpenDir(int fd);
int is_name_ends_with(char* file_name, char* constraint);
int is_perm_write(char* path);
int list_in_dir(char* path, char* name_ends_with, int have_perm_write);
int recursive_listing(char* path, char* name_ends_with, int have_perm_write);

// API SECTION ENDS --


// HELPER METHODS SECTION

void print_usage()
{
    fprintf(stdout, "\n\nUSAGE: %s <operation> [options]\n\n\
List of operations:\n\
|-> variant     -displays the identifier of the assignment variant\n\
| SYNTAX: variant\n\
|-> list        -display the names of some elements in the specified path\n\
| SYNTAX: list [recursive] <name_ends_with=string || have_perm_write> path=<file_path>\n\
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

// HELPER METHODS SECTION ENDS --

// list [recursive] <name_ends_with=string || have_perm_write> path=<file_path>
struct list_options* fetch_list_options(int argc, char* argv[])
{
    // at least 2 args should be provided
    if(!check_args_num(argc, LIST_MIN_ARGS, LIST_MAX_ARGS))
        return NULL;
    else
    {
        struct list_options* ops = create_list();
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
                ops->path = argv[i] + 5;
            } 
            else if(!strncmp(argv[i], "name_ends_with=", 15))
            {
                ops->name_ends_with = argv[i] + 15;
            } 
            else if(!strcmp(argv[i], "have_perm_write"))
            {
                ops->have_perm_write = 1;
            }
        }
        if(ops->path == NULL)
        {
            DISPLAY_ERR("Path is not specified");
            print_usage();
            exit(1);
        }
        return ops;
    }
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


// list [recursive] <name_ends_with=string || have_perm_write> path=<file_path>
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
    if(isDir(options->path))
        if(options->recursive)
        {
            printf("SUCCESS");
            result = recursive_listing(options->path, options->name_ends_with, options->have_perm_write);
            free(options);
            return result;
        }
        else
        {
            result = list_in_dir(options->path, options->name_ends_with, options->have_perm_write);
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

int isDir(char* path)
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

int is_name_ends_with(char* file_name, char* constraint)
{
    size_t name_length = strlen(file_name);
    size_t const_len = strlen(constraint);
    if(name_length < const_len)
        return 0;
    else return strncmp(file_name + name_length - const_len, constraint, const_len) == 0;
}

int is_perm_write(char* path)
{
    struct stat info;
    int get_stat = stat(path, &info);
    if(get_stat == -1)
    {
        DISPLAY_ERR("Error fetching file statistics");
        exit(3);
    }
    if(info.st_mode & S_IWUSR)
        return 1;
    else return 0;
}

int list_in_dir(char* path, char* name_ends_with, int have_perm_write)
{   
    struct stat info;
    struct dirent* entry;
    char entry_path[PATH_SIZE];

    DIR* dir = opendir(path);
    if(dir < 0)
    {
        DISPLAY_ERR("Enable to open a directory");
        return 0;
    }
    printf("SUCCESS");
    while((entry = readdir(dir)))
    {
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
        int get_stat = stat(entry_path, &info);
        if(get_stat < 0)
        {
            DISPLAY_ERR("Error fetching file statistics");
            return 0;
        }
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if(name_ends_with != NULL && !have_perm_write)
        {

            if(is_name_ends_with(entry->d_name, name_ends_with) && is_perm_write(entry_path))
                stdout_display(entry_path);
            else continue;
        } 
        else if (name_ends_with != NULL)
        {
            if(is_name_ends_with(entry->d_name, name_ends_with))
                stdout_display(entry_path);
            else continue;
        } 
        else if(is_perm_write(entry_path))
        {
            if(is_perm_write(entry_path))
                stdout_display(entry_path);
            else continue;
        } 
        else 
        {
            stdout_display(entry_path);
        }
    }
    return 1;
}

int recursive_listing(char* path, char* name_ends_with, int have_perm_write)
{
    struct stat info = {0};
    struct dirent* entry = NULL;
    char entry_path[PATH_SIZE] = {0};
    int exit = 0;

    DIR* dir = opendir(path);
    if(dir < 0)
    {
        DISPLAY_ERR("Enable to open a directory");
        return 0;
    }
    while((entry = readdir(dir)))
    {
        snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);
        int get_stat = lstat(entry_path, &info);
        if(get_stat != 0){
            DISPLAY_ERR("Error fetching file statistics");
            exit = 1;
            return 0;
        }
        if(!S_ISDIR(info.st_mode))
        {
            if(name_ends_with != NULL && !have_perm_write)
            {
                if(is_name_ends_with(entry->d_name, name_ends_with) && is_perm_write(entry_path))
                    stdout_display(entry_path);
                else continue;
            } 
            else if (name_ends_with != NULL)
            {
                if(is_name_ends_with(entry->d_name, name_ends_with))
                    stdout_display(entry_path);
                else continue;
            } 
            else if(is_perm_write(entry_path))
            {
                if(is_perm_write(entry_path))
                    stdout_display(entry_path);
                else continue;
            } 
            else 
            {
                stdout_display(entry_path);
            }
        } 
        else if(S_ISDIR(info.st_mode))
        {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            recursive_listing(entry_path, name_ends_with, have_perm_write);
        }
    }
    if(exit == 1)
        return 0;
    return 1;
}

// "8maz"
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
    else return 0;
    
    return 0;
}

// must be 2 or between [7, 15]
int check_sections_nr(int fd)
{
    unsigned char section_buffer = '\0';
    int read_bytes = 0;
    int seek = lseek(fd, NO_OF_SECTIONS_OFFSET, SEEK_SET);
    if(seek < 0)
    {
        DISPLAY_ERR("Enable to set the position pointer");
        return 0;
    }

    if((read_bytes = read(fd, &section_buffer, NO_OF_SECTIONS)) > 0)
    {   
        if(section_buffer == 2 || (section_buffer >= 7 && section_buffer <= 15))
            return section_buffer;
        else return 0;
    }
    else 
    {
        DISPLAY_ERR("Enable to read the file number of sections");
        return 0;
    }
    return 0;
}

int check_sections_type(int fd, fs_file* file)
{
    file->section = (section**)malloc(file->nr_sections * sizeof(section*)); // allocate memory based on the number
    // of the sections specified in the fs file
    if(file->section == NULL)
    {
        DISPLAY_ERR("Memory allocations error - file section");
        free(file);
        close(fd);
        exit(ENOMEM);
    }
    char* section_name = (char*)malloc((SECT_NAME + 1) * sizeof(char));
    char* section_type = (char*)malloc((SECT_TYPE + 1) * sizeof(char));
    char* section_offset = (char*)malloc((SECT_OFFSET + 1) * sizeof(char));
    char* section_size = (char*)malloc((SECT_SIZE + 1) * sizeof(char));
    int read_bytes = 0;

    int seek = lseek(fd, 0, SEEK_SET);
    if(seek == -1)
    {
        DISPLAY_ERR("Enable to set the position pointer");
        return 0;
    }

    for(int i = 0; i < file->nr_sections; ++i)
    {
        if(     (read_bytes += read(fd, section_name, SECT_NAME)) < 0
            ||  (read_bytes += read(fd, section_type, SECT_TYPE)) < 0
            ||  (read_bytes += read(fd, section_offset, SECT_OFFSET)) < 0
            ||  (read_bytes += read(fd, section_size, SECT_SIZE)) < 0)
            {
                DISPLAY_ERR("Error reading the section file");
                return 0;
            }
        section_name[SECT_NAME - 1] = '\0';
        section_type[SECT_TYPE - 1] = '\0';
        section_offset[SECT_OFFSET - 1] = '\0';
        section_size[SECT_SIZE - 1] = '\0';

        file->section[i] = (section*)malloc(sizeof(section));

        file->section[i]->name = malloc(sizeof(section_name));
        file->section[i]->name = section_name;
        file->section[i]->type = atoi(section_type);
        file->section[i]->offset = atoi(section_offset);
        file->section[i]->size = atoi(section_size);
    }
    display_file_content(file);

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

fs_file* test_parse_constraints(char* path, int fd)
{
    fs_file* file = create_fs_file();
    if(!(file->magic = check_magic(fd)))
    {
        DISPLAY_ERR("Wrong magic");
        free(file->section);
        free(file);
        close(fd);
        exit(11);
    }
    if(!(file->version = check_version(fd)))
    {
        DISPLAY_ERR("Wrong version");
        free(file->section);
        free(file);
        close(fd);
        exit(11);
    }
    if(!(file->nr_sections = check_sections_nr(fd)))
    {
        DISPLAY_ERR("Wrong sect_nr");
        free(file->section);
        free(file);
        close(fd);
        exit(11);
    }

    if(!(check_sections_type(fd, file)))
    {
        DISPLAY_ERR("Wrong sect_types");
        free(file->section);
        free(file);
        close(fd);
        exit(11);
    }
    return file;
}

//magic = 8maz
//version = 72 - 93
//nr_sections = 2 or (7-15)
//section_type = 88 31
fs_file* is_fs(char* path, int fd)
{ 
    fs_file* file = test_parse_constraints(path, fd);
    close(fd);
    return file;
}

int is_printable(char* string)
{
    for(int i = 0 ; string[i] < sizeof(string); i++)
    {
        if(string[i] < 33 || string[i] > 126)
        {
            return i;
        }
    }
    return 1;
}

int is_number(char* string)
{   
    int j = 0;
    while(j != sizeof(string)){
            if(string[j] < 48 || string[j] > 57)
                return 0;
    }
    return 1;
}

typedef struct {
    unsigned short version;
    unsigned char no_of_sections;
} SFHeader;

// parse path=<file_name>
int execute_parse_operation(int argc, char* argv[])
{
    if(!check_args_num(argc, PARSE_MIN_ARGS, PARSE_MAX_ARGS))
        return 0;
    char* path = NULL;
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
    fs_file* file = NULL;
    file = is_fs(path, fd);
    display_file_content(file);

//     #define BUFFER_SIZE 4096
// #define MAGIC "8maz"

// int fd = open(path, O_RDONLY);
//     if (fd == -1) {
//         perror("Error opening file");
//         return 1; // Return failure
//     }


//     char byte;
//     while (read(fd, &byte, 1) > 0) { // Read byte by byte until the end of the file
//     printf("byte=%c ", byte);
//         if (byte >= 72 && byte <= 92) { // Check if the byte is between 72 and 92
//             char num_str[4]; // Assuming the maximum length of number in string form is 4
//             int n = snprintf(num_str, sizeof(num_str), "%d ", byte); // Convert byte to string
//             if (n > 0) {
//                 // write(STDOUT_FILENO, num_str, n); // Write the number to standard output
//             }
//         }
//     }


    return 0;
}

/*
The values of the file version must be one from the interval mentioned
above, i.e. between 72 and 93, including that values.
• The number of sections must be 2 or between 7 and 15, including that
values.
• The existing sections’ type must be only in the set mentioned above, i.e.
88 31 .*/

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
            // execute_list_operation(argc, argv);
            break;
        case 'p':
            execute_parse_operation(argc, argv);
            break;
        case 'e':
            printf("Operation specified: extract\n");
            break;
        case 'f':
            printf("Operation specified: findall\n");
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

