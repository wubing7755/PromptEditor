#include <prompteditor/cli.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <windows.h>
#define mkdir_one(path) _mkdir(path)
/* Windows POSIX compat */
#define popen _popen
#define pclose _pclose
#ifndef __MINGW32__
/* _mktemp_s is MSVC-only; MinGW use tmpnam-style fallback */
#define HAS_MKTEMP_S
#endif
#include <process.h>
#define getpid _getpid
#define getcwd _getcwd
#else
#include <unistd.h>
#define mkdir_one(path) mkdir((path), 0755)
#endif
#define PP_CLI_VERSION "0.1.0"
#define PP_PATH_MAX 4096
#define PP_FIELD_MAX 512
#define PP_BODY_MAX 65536

struct prompt_options {
    const char *root;
    const char *title;
    const char *body;
    const char *folder;
    const char *category;
    const char *description;
    const char *id_or_title;
    const char *filter_folder;
    const char *filter_category;
    const char *filter_tag;
    const char *note;
    const char *compare_version;
    char tags[PP_FIELD_MAX];
    int raw;
    int json;
    int no_pager;
    int tags_set;
    int promote;
    int editor;
    int history;
};

struct prompt_index_row {
    char id[PP_FIELD_MAX];
    char title[PP_FIELD_MAX];
    char folder[PP_FIELD_MAX];
    char category[PP_FIELD_MAX];
    char tags[PP_FIELD_MAX];
    char updated_at[PP_FIELD_MAX];
};

static int index_file_path(char *out, size_t out_size, const char *root);

static void print_help(void) {
    puts("Usage: pp <command> [options]");
    puts("");
    puts("Commands:");
    puts("  init       Initialize a prompt library folder");
    puts("  add        Save a prompt");
    puts("  folder     Manage prompt folders");
    puts("  category   Manage prompt categories");
    puts("  list       List saved prompts");
    puts("  show       Print a prompt by id or title");
    puts("  edit       Update prompt metadata or body");
    puts("  delete     Archive or remove a prompt");
    puts("  search     Search prompt titles, bodies, tags, and descriptions");
    puts("  browse     Interactive prompt browser with fuzzy search");
    puts("  optimize   Create an improved version of a prompt");
    puts("  export     Export prompts or a library folder");
    puts("  import     Import prompts from another library or export");
    puts("  backup     Create a full library backup");
    puts("");
    puts("Global options:");
    puts("  -h, --help       Show this help");
    puts("  -v, --version    Show version");
    puts("");
    puts("Run 'pp <command> --help' for command-specific usage.");
}

static void print_init_help(void) {
    puts("Usage: pp init [--root <path>]");
    puts("");
    puts("Initializes a prompt library folder.");
    puts("");
    puts("Options:");
    puts("  --root <path>    Library root to create or validate");
    puts("  -h, --help       Show this help");
}

static void print_add_help(void) {
    puts("Usage: pp add --title <text> --body <text> [options]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --title <text>         Prompt title");
    puts("  --body <text>          Prompt body");
    puts("  --folder <path>        Folder inside the library, default: inbox");
    puts("  --category <name>      Category, default: general");
    puts("  --tag <name>           Repeatable tag");
    puts("  --description <text>   Optional description");
    puts("  --editor               Open $EDITOR to enter prompt body");
}

static void print_list_help(void) {
    puts("Usage: pp list [--root <path>] [filters] [--json] [--no-pager]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --folder <path>        Filter by folder");
    puts("  --category <name>      Filter by category");
    puts("  --tag <name>           Filter by tag");
    puts("  --json                 Output in JSON format");
    puts("  --no-pager             Disable automatic paging");
}

static void print_show_help(void) {
    puts("Usage: pp show <id-or-title> [--root <path>] [--raw] [--json]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --raw                  Print only the prompt body");
    puts("  --json                 Output in JSON format");
}

static void print_search_help(void) {
    puts("Usage: pp search <query> [--root <path>] [filters] [--raw] [--json]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --folder <path>        Filter by folder");
    puts("  --category <name>      Filter by category");
    puts("  --tag <name>           Filter by tag");
    puts("  --raw                  Print matching prompt bodies only");
}

static void print_edit_help(void) {
    puts("Usage: pp edit <id-or-title> [--root <path>] [options]");
    puts("");
    puts("If no --body, --category, --tag, --description, --title, or --folder is given,");
    puts("the editor opens for editing the prompt body.");
    puts("");
    puts("Options:");
    puts("  --body <text>          Replace prompt body");
    puts("  --category <name>      Replace prompt category");
    puts("  --tag <name>           Replace tags; repeat for multiple tags");
    puts("  --description <text>   Replace prompt description");
    puts("  --title <text>         Replace prompt title");
    puts("  --folder <path>        Move prompt to a different folder");
}

static void print_browse_help(void) {
    puts("Usage: pp browse [--root <path>] [filters]");
    puts("");
    puts("Opens an interactive prompt browser.");
    puts("If fzf is installed, it is used for fuzzy filtering with a preview window.");
    puts("Otherwise, a numbered menu is shown.");
    puts("");
    puts("Keys in numbered menu:");
    puts("  <number>     Select and view a prompt");
    puts("  q            Quit");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --folder <path>        Filter by folder");
    puts("  --category <name>      Filter by category");
    puts("  --tag <name>           Filter by tag");
}

static void print_delete_help(void) {
    puts("Usage: pp delete <id-or-title> [--root <path>] [--yes]");
    puts("");
    puts("Deletes are archived by default. Pass --yes to confirm.");
}

static void print_optimize_help(void) {
    puts("Usage: pp optimize <id-or-title> [--root <path>] [options]");
    puts("");
    puts("Options:");
    puts("  --body <text>          Optimized prompt body");
    puts("  --note <text>          Optimization rationale");
    puts("  --promote              Make the optimized version current");
    puts("  --history              Show version history");
    puts("  --compare <version>    Compare current body with a version, e.g. 0002");
}

static void print_folder_help(void) {
    puts("Usage: pp folder <list|create|remove|rename> [name] [options]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --to <name>            New name for rename");
    puts("  --yes                  Confirm remove");
}

static void print_category_help(void) {
    puts("Usage: pp category <list|create|remove|rename> [name] [options]");
    puts("");
    puts("Options:");
    puts("  --root <path>          Library root");
    puts("  --to <name>            New name for rename");
    puts("  --yes                  Confirm remove");
}

static void print_export_help(void) {
    puts("Usage: pp export --out <path> [--root <path>] [--folder <path>]");
}

static void print_import_help(void) {
    puts("Usage: pp import <path> [--root <path>] [--on-conflict skip|replace]");
}

static void print_backup_help(void) {
    puts("Usage: pp backup --out <path> [--root <path>]");
}

static void print_version(void) {
    puts("pp " PP_CLI_VERSION);
}

static int is_help_flag(const char *arg) {
    return strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0;
}

static int is_version_flag(const char *arg) {
    return strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0;
}

static int path_exists_as_dir(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        return 0;
    }

    return (info.st_mode & S_IFDIR) != 0;
}

static int path_exists_as_file(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        return 0;
    }

    return (info.st_mode & S_IFREG) != 0;
}

static int join_path(char *out, size_t out_size, const char *left, const char *right) {
    size_t left_len;
    const char *separator;

    if (left == NULL || right == NULL || out == NULL || out_size == 0) {
        return 0;
    }

    left_len = strlen(left);
#ifdef _WIN32
    separator =
        (left_len > 0 && (left[left_len - 1] == '/' || left[left_len - 1] == '\\')) ? "" : "\\";
#else
    separator = (left_len > 0 && left[left_len - 1] == '/') ? "" : "/";
#endif

    return snprintf(out, out_size, "%s%s%s", left, separator, right) > 0 && strlen(out) < out_size;
}

static int make_dir_if_missing(const char *path) {
    if (path_exists_as_dir(path)) {
        return 1;
    }

    if (mkdir_one(path) == 0) {
        return 1;
    }

    if (errno == EEXIST && path_exists_as_dir(path)) {
        return 1;
    }

    fprintf(stderr, "Could not create directory '%s': %s\n", path, strerror(errno));
    return 0;
}

static int make_dir_recursive(const char *path) {
    char partial[PP_PATH_MAX];
    size_t index;
    size_t length;

    if (path == NULL || strlen(path) >= sizeof(partial)) {
        return 0;
    }

    strcpy(partial, path);
    length = strlen(partial);

    for (index = 1; index < length; ++index) {
#ifdef _WIN32
        if (index == 2 && partial[1] == ':') {
            continue;
        }
#endif
        if (partial[index] == '/' || partial[index] == '\\') {
            char saved = partial[index];
            partial[index] = '\0';
            if (strlen(partial) > 0 && !make_dir_if_missing(partial)) {
                partial[index] = saved;
                return 0;
            }
            partial[index] = saved;
        }
    }

    return make_dir_if_missing(partial);
}

static int write_text_file_if_missing(const char *path, const char *content) {
    FILE *file;

    if (path_exists_as_file(path)) {
        return 1;
    }

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Could not write '%s': %s\n", path, strerror(errno));
        return 0;
    }

    if (fputs(content, file) == EOF) {
        fprintf(stderr, "Could not write '%s': %s\n", path, strerror(errno));
        fclose(file);
        return 0;
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close '%s': %s\n", path, strerror(errno));
        return 0;
    }

    return 1;
}

static int write_text_file(const char *path, const char *content) {
    char temp_path[PP_PATH_MAX];
    FILE *file;

    if (snprintf(temp_path, sizeof(temp_path), "%s.tmp", path) <= 0 ||
        strlen(temp_path) >= sizeof(temp_path)) {
        fprintf(stderr, "Temporary path is too long for '%s'.\n", path);
        return 0;
    }

    file = fopen(temp_path, "wb");

    if (file == NULL) {
        fprintf(stderr, "Could not write '%s': %s\n", temp_path, strerror(errno));
        return 0;
    }

    if (fputs(content, file) == EOF) {
        fprintf(stderr, "Could not write '%s': %s\n", temp_path, strerror(errno));
        fclose(file);
        remove(temp_path);
        return 0;
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "Could not close '%s': %s\n", temp_path, strerror(errno));
        remove(temp_path);
        return 0;
    }

    if (remove(path) != 0 && errno != ENOENT) {
        fprintf(stderr, "Could not replace '%s': %s\n", path, strerror(errno));
        remove(temp_path);
        return 0;
    }

    if (rename(temp_path, path) != 0) {
        fprintf(stderr, "Could not replace '%s': %s\n", path, strerror(errno));
        remove(temp_path);
        return 0;
    }

    return 1;
}

static int replace_file_with_temp(const char *temp_path, const char *final_path) {
    if (remove(final_path) != 0 && errno != ENOENT) {
        fprintf(stderr, "Could not replace '%s': %s\n", final_path, strerror(errno));
        remove(temp_path);
        return 0;
    }

    if (rename(temp_path, final_path) != 0) {
        fprintf(stderr, "Could not replace '%s': %s\n", final_path, strerror(errno));
        remove(temp_path);
        return 0;
    }

    return 1;
}

static int append_text_file(const char *path, const char *content) {
    FILE *file = fopen(path, "ab");

    if (file == NULL) {
        fprintf(stderr, "Could not append '%s': %s\n", path, strerror(errno));
        return 0;
    }

    if (fputs(content, file) == EOF) {
        fprintf(stderr, "Could not append '%s': %s\n", path, strerror(errno));
        fclose(file);
        return 0;
    }

    return fclose(file) == 0;
}

static int read_text_file(const char *path, char *out, size_t out_size) {
    FILE *file;
    size_t read_count;

    if (out == NULL || out_size == 0) {
        return 0;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read '%s': %s\n", path, strerror(errno));
        return 0;
    }

    read_count = fread(out, 1, out_size - 1, file);
    out[read_count] = '\0';

    if (ferror(file)) {
        fprintf(stderr, "Could not read '%s': %s\n", path, strerror(errno));
        fclose(file);
        return 0;
    }

    return fclose(file) == 0;
}

static int copy_text_file(const char *source, const char *destination) {
    char content[PP_BODY_MAX];

    return read_text_file(source, content, sizeof(content)) &&
           write_text_file(destination, content);
}

static int copy_file_if_exists(const char *source, const char *destination) {
    if (!path_exists_as_file(source)) {
        return 1;
    }

    return copy_text_file(source, destination);
}

static int config_file_path(char *out, size_t out_size) {
    const char *home;
#ifdef _WIN32
    home = getenv("USERPROFILE");
#else
    home = getenv("HOME");
#endif
    if (home == NULL || home[0] == '\0') {
        return 0;
    }
    return join_path(out, out_size, home, ".promptconfig");
}

static int default_root(char *out, size_t out_size) {
    const char *root_env;
    const char *home;
    char cwd[PP_PATH_MAX];
    char test[PP_PATH_MAX];

    /*
     * 1. Walk up from the current directory looking for .promptlib/.
     *    This mirrors how git discovers a repository and makes the
     *    common case friction-free: cd into a library and run pp.
     */
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char *scan = cwd;

        for (;;) {
            struct stat info;
            char *sep;
#ifdef _WIN32
            char *sep_bs;
#endif

            {
                int len = snprintf(test, sizeof(test), "%s/.promptlib", scan);
                if (len <= 0 || (size_t)len >= sizeof(test)) {
                    break;
                }
            }
            if (stat(test, &info) == 0 && (info.st_mode & S_IFDIR)) {
                return snprintf(out, out_size, "%s", scan) > 0 && strlen(out) < out_size;
            }

            sep = strrchr(scan, '/');
#ifdef _WIN32
            sep_bs = strrchr(scan, '\\');
            if (sep_bs > sep) {
                sep = sep_bs;
            }
#endif
            if (sep == NULL) {
                break;
            }
            *sep = '\0';
            if (strlen(scan) == 0) {
                break;
            }
#ifdef _WIN32
            /* Stop at drive root, e.g. "C:" */
            if (strlen(scan) == 2 && scan[1] == ':') {
                break;
            }
#endif
        }
    }

    /*
     * 2. Check the saved default root (~/.promptconfig).
     *    pp init writes this automatically so commands run from
     *    unrelated directories still find the library.
     */
    {
        char config_file[PP_PATH_MAX];
        char saved_root[PP_PATH_MAX];
        if (config_file_path(config_file, sizeof(config_file)) &&
            read_text_file(config_file, saved_root, sizeof(saved_root))) {
            saved_root[strcspn(saved_root, "\r\n")] = '\0';
            if (saved_root[0] != '\0' && path_exists_as_dir(saved_root)) {
                return snprintf(out, out_size, "%s", saved_root) > 0 && strlen(out) < out_size;
            }
        }
    }

    /*
     * 3. Honour PROMPTLIB_ROOT environment variable.
     */
    root_env = getenv("PROMPTLIB_ROOT");
    if (root_env != NULL && root_env[0] != '\0') {
        return snprintf(out, out_size, "%s", root_env) > 0 && strlen(out) < out_size;
    }

    /*
     * 4. Fall back to the user's home directory.  init_library() (and
     *    every other path) creates .promptlib/ *inside* the resolved
     *    root, so the root must be the parent directory — not the
     *    .promptlib directory itself.
     */
#ifdef _WIN32
    home = getenv("USERPROFILE");
#else
    home = getenv("HOME");
#endif

    if (home == NULL || home[0] == '\0') {
        fprintf(stderr, "No library root provided and user home could not be detected.\n");
        fprintf(stderr, "Run 'pp init --root <path>' or set PROMPTLIB_ROOT.\n");
        return 0;
    }

    return snprintf(out, out_size, "%s", home) > 0 && strlen(out) < out_size;
}

static int resolve_root_arg(int start_index, int argc, char **argv, char *out, size_t out_size) {
    int index;

    for (index = start_index; index < argc; ++index) {
        if (strcmp(argv[index], "--root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --root.\n");
                return 0;
            }

            return snprintf(out, out_size, "%s", argv[index + 1]) > 0 && strlen(out) < out_size;
        }
    }

    return default_root(out, out_size);
}

static int init_library(const char *root) {
    char meta_dir[PP_PATH_MAX];
    char prompts_dir[PP_PATH_MAX];
    char archive_dir[PP_PATH_MAX];
    char version_file[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char folders_file[PP_PATH_MAX];
    char categories_file[PP_PATH_MAX];

    if (!make_dir_recursive(root) || !join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") ||
        !join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") ||
        !join_path(archive_dir, sizeof(archive_dir), root, "archive") ||
        !make_dir_if_missing(meta_dir) || !make_dir_if_missing(prompts_dir) ||
        !make_dir_if_missing(archive_dir) ||
        !join_path(version_file, sizeof(version_file), meta_dir, "version") ||
        !join_path(index_file, sizeof(index_file), meta_dir, "index.tsv") ||
        !join_path(folders_file, sizeof(folders_file), meta_dir, "folders.tsv") ||
        !join_path(categories_file, sizeof(categories_file), meta_dir, "categories.tsv") ||
        !write_text_file_if_missing(version_file, "1\n") ||
        !write_text_file_if_missing(index_file,
                                    "id\ttitle\tfolder\tcategory\ttags\tupdated_at\n") ||
        !write_text_file_if_missing(folders_file, "name\ninbox\n") ||
        !write_text_file_if_missing(categories_file, "name\ngeneral\n")) {
        return 0;
    }

    /* Remember this root as the default for commands run from other directories. */
    {
        char config_file[PP_PATH_MAX];
        if (config_file_path(config_file, sizeof(config_file))) {
            write_text_file(config_file, root);
        }
    }

    printf("Prompt library ready: %s\n", root);
    return 1;
}

static int validate_library_root(const char *root) {
    char meta_dir[PP_PATH_MAX];
    char prompts_dir[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];

    if (!join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") ||
        !join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") ||
        !join_path(index_file, sizeof(index_file), meta_dir, "index.tsv")) {
        fprintf(stderr, "Library root path is too long.\n");
        return 0;
    }

    if (!path_exists_as_dir(root) || !path_exists_as_dir(meta_dir) ||
        !path_exists_as_dir(prompts_dir) || !path_exists_as_file(index_file)) {
        fprintf(stderr, "Invalid prompt library root: %s\n", root);
        fprintf(stderr, "Run 'pp init --root %s' first.\n", root);
        return 0;
    }

    return 1;
}

static void current_timestamp(char *out, size_t out_size) {
    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    if (local == NULL || strftime(out, out_size, "%Y-%m-%dT%H:%M:%S", local) == 0) {
        snprintf(out, out_size, "unknown");
    }
}

static unsigned long hash_text(const char *text) {
    unsigned long hash = 5381;

    while (*text != '\0') {
        hash = ((hash << 5) + hash) + (unsigned char)*text;
        ++text;
    }

    return hash;
}

static void slugify_title(const char *title, char *out, size_t out_size) {
    size_t written = 0;
    int last_dash = 0;

    while (*title != '\0' && written + 1 < out_size) {
        unsigned char ch = (unsigned char)*title;

        if (isalnum(ch)) {
            out[written++] = (char)tolower(ch);
            last_dash = 0;
        } else if (!last_dash && written > 0) {
            out[written++] = '-';
            last_dash = 1;
        }

        ++title;
    }

    while (written > 0 && out[written - 1] == '-') {
        --written;
    }

    if (written == 0 && out_size > 1) {
        out[written++] = 'p';
    }

    out[written] = '\0';
}

static void prompt_id_from_title(const char *title, char *out, size_t out_size) {
    char slug[PP_FIELD_MAX];

    slugify_title(title, slug, sizeof(slug));
    snprintf(out, out_size, "%s-%04lx", slug, hash_text(title) & 0xffffUL);
}

static int path_has_unsafe_segment(const char *value) {
    return strstr(value, "..") != NULL || strchr(value, ':') != NULL || value[0] == '/' ||
           value[0] == '\\';
}

static int field_has_unsupported_chars(const char *value) {
    return value != NULL && (strchr(value, '\t') != NULL || strchr(value, '\n') != NULL ||
                             strchr(value, '\r') != NULL);
}

static int field_is_too_long(const char *value) {
    return value != NULL && strlen(value) >= PP_FIELD_MAX;
}

static int valid_version_name(const char *value) {
    size_t index;

    if (value == NULL || strlen(value) != 4) {
        return 0;
    }

    for (index = 0; index < 4; ++index) {
        if (!isdigit((unsigned char)value[index])) {
            return 0;
        }
    }

    return 1;
}

static int add_tag_value(struct prompt_options *options, const char *tag) {
    size_t current_len;
    size_t tag_len;

    if (tag == NULL || tag[0] == '\0' || field_is_too_long(tag) ||
        field_has_unsupported_chars(tag) || strchr(tag, ',') != NULL) {
        fprintf(stderr, "Tag contains unsupported characters.\n");
        return 0;
    }

    current_len = strlen(options->tags);
    tag_len = strlen(tag);
    if (current_len + tag_len + 2 >= sizeof(options->tags)) {
        fprintf(stderr, "Tag list is too long.\n");
        return 0;
    }

    if (current_len > 0) {
        strcat(options->tags, ",");
    }
    strcat(options->tags, tag);
    options->tags_set = 1;
    return 1;
}

static int tag_list_contains(const char *tags, const char *tag) {
    char copy[PP_FIELD_MAX];
    char *current;

    if (tag == NULL || tag[0] == '\0') {
        return 1;
    }

    snprintf(copy, sizeof(copy), "%s", tags == NULL ? "" : tags);
    current = strtok(copy, ",");
    while (current != NULL) {
        if (strcmp(current, tag) == 0) {
            return 1;
        }
        current = strtok(NULL, ",");
    }

    return 0;
}

static int contains_text(const char *haystack, const char *needle) {
    size_t haystack_len;
    size_t needle_len;
    size_t index;
    size_t inner;

    if (needle == NULL || needle[0] == '\0') {
        return 1;
    }
    if (haystack == NULL) {
        return 0;
    }

    haystack_len = strlen(haystack);
    needle_len = strlen(needle);
    if (needle_len > haystack_len) {
        return 0;
    }

    for (index = 0; index <= haystack_len - needle_len; ++index) {
        int matches = 1;
        for (inner = 0; inner < needle_len; ++inner) {
            unsigned char left = (unsigned char)haystack[index + inner];
            unsigned char right = (unsigned char)needle[inner];
            if (tolower(left) != tolower(right)) {
                matches = 0;
                break;
            }
        }
        if (matches) {
            return 1;
        }
    }

    return 0;
}

static int row_matches_filters(const struct prompt_index_row *row,
                               const struct prompt_options *filters) {
    if (filters->filter_folder != NULL && strcmp(row->folder, filters->filter_folder) != 0) {
        return 0;
    }

    if (filters->filter_category != NULL && strcmp(row->category, filters->filter_category) != 0) {
        return 0;
    }

    if (filters->filter_tag != NULL && !tag_list_contains(row->tags, filters->filter_tag)) {
        return 0;
    }

    return 1;
}

static int row_matches_query(const struct prompt_index_row *row, const char *body,
                             const char *query) {
    return contains_text(row->id, query) || contains_text(row->title, query) ||
           contains_text(row->folder, query) || contains_text(row->category, query) ||
           contains_text(row->tags, query) || contains_text(body, query);
}

static int parse_index_row(char *line, struct prompt_index_row *row) {
    char *field;

    field = strtok(line, "\t");
    if (field == NULL) {
        return 0;
    }
    snprintf(row->id, sizeof(row->id), "%s", field);

    field = strtok(NULL, "\t");
    if (field == NULL) {
        return 0;
    }
    snprintf(row->title, sizeof(row->title), "%s", field);

    field = strtok(NULL, "\t");
    if (field == NULL) {
        return 0;
    }
    snprintf(row->folder, sizeof(row->folder), "%s", field);

    field = strtok(NULL, "\t");
    if (field == NULL) {
        return 0;
    }
    snprintf(row->category, sizeof(row->category), "%s", field);

    field = strtok(NULL, "\t\r\n");
    if (field == NULL) {
        row->tags[0] = '\0';
        row->updated_at[0] = '\0';
        return strcmp(row->id, "id") != 0;
    }
    snprintf(row->tags, sizeof(row->tags), "%s", field);

    field = strtok(NULL, "\t\r\n");
    if (field == NULL) {
        row->updated_at[0] = '\0';
    } else {
        snprintf(row->updated_at, sizeof(row->updated_at), "%s", field);
    }

    return strcmp(row->id, "id") != 0;
}

static int find_prompt(const char *root, const char *id_or_title, struct prompt_index_row *found) {
    char meta_dir[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *file;

    if (!join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") ||
        !join_path(index_file, sizeof(index_file), meta_dir, "index.tsv")) {
        return 0;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read index '%s': %s\n", index_file, strerror(errno));
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        struct prompt_index_row row;

        if (parse_index_row(line, &row) &&
            (strcmp(row.id, id_or_title) == 0 || strcmp(row.title, id_or_title) == 0)) {
            *found = row;
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static int prompt_body_path(char *out, size_t out_size, const char *root,
                            const struct prompt_index_row *row) {
    char prompts_dir[PP_PATH_MAX];
    char folder_dir[PP_PATH_MAX];
    char prompt_dir[PP_PATH_MAX];

    return join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") &&
           join_path(folder_dir, sizeof(folder_dir), prompts_dir, row->folder) &&
           join_path(prompt_dir, sizeof(prompt_dir), folder_dir, row->id) &&
           join_path(out, out_size, prompt_dir, "current.md");
}

static int prompt_dir_path(char *out, size_t out_size, const char *root,
                           const struct prompt_index_row *row) {
    char prompts_dir[PP_PATH_MAX];
    char folder_dir[PP_PATH_MAX];

    return join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") &&
           join_path(folder_dir, sizeof(folder_dir), prompts_dir, row->folder) &&
           join_path(out, out_size, folder_dir, row->id);
}

static int prompt_metadata_path(char *out, size_t out_size, const char *root,
                                const struct prompt_index_row *row) {
    char prompt_dir[PP_PATH_MAX];

    return prompt_dir_path(prompt_dir, sizeof(prompt_dir), root, row) &&
           join_path(out, out_size, prompt_dir, "metadata.tsv");
}

static int versions_dir_path(char *out, size_t out_size, const char *root,
                             const struct prompt_index_row *row) {
    char prompt_dir[PP_PATH_MAX];

    return prompt_dir_path(prompt_dir, sizeof(prompt_dir), root, row) &&
           join_path(out, out_size, prompt_dir, "versions");
}

static int version_body_path(char *out, size_t out_size, const char *root,
                             const struct prompt_index_row *row, const char *version) {
    char versions_dir[PP_PATH_MAX];
    char filename[PP_FIELD_MAX];

    return snprintf(filename, sizeof(filename), "%s.md", version) > 0 &&
           strlen(filename) < sizeof(filename) &&
           versions_dir_path(versions_dir, sizeof(versions_dir), root, row) &&
           join_path(out, out_size, versions_dir, filename);
}

static int version_metadata_path(char *out, size_t out_size, const char *root,
                                 const struct prompt_index_row *row, const char *version) {
    char versions_dir[PP_PATH_MAX];
    char filename[PP_FIELD_MAX];

    return snprintf(filename, sizeof(filename), "%s.tsv", version) > 0 &&
           strlen(filename) < sizeof(filename) &&
           versions_dir_path(versions_dir, sizeof(versions_dir), root, row) &&
           join_path(out, out_size, versions_dir, filename);
}

static int versions_index_path(char *out, size_t out_size, const char *root,
                               const struct prompt_index_row *row) {
    char versions_dir[PP_PATH_MAX];

    return versions_dir_path(versions_dir, sizeof(versions_dir), root, row) &&
           join_path(out, out_size, versions_dir, "index.tsv");
}

static int append_prompt_index_row(const char *root, const struct prompt_index_row *row) {
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];

    if (!index_file_path(index_file, sizeof(index_file), root)) {
        return 0;
    }

    snprintf(line, sizeof(line), "%s\t%s\t%s\t%s\t%s\t%s\n", row->id, row->title, row->folder,
             row->category, row->tags, row->updated_at);

    return append_text_file(index_file, line);
}

static int copy_prompt_versions(const char *source_root, const char *destination_root,
                                const struct prompt_index_row *row) {
    char source_versions_dir[PP_PATH_MAX];
    char destination_versions_dir[PP_PATH_MAX];
    char source_index[PP_PATH_MAX];
    char destination_index[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *file;

    if (!versions_dir_path(source_versions_dir, sizeof(source_versions_dir), source_root, row) ||
        !versions_dir_path(destination_versions_dir, sizeof(destination_versions_dir),
                           destination_root, row) ||
        !versions_index_path(source_index, sizeof(source_index), source_root, row) ||
        !versions_index_path(destination_index, sizeof(destination_index), destination_root, row)) {
        return 0;
    }

    if (!path_exists_as_file(source_index)) {
        return 1;
    }

    if (!make_dir_if_missing(destination_versions_dir) ||
        !copy_text_file(source_index, destination_index)) {
        return 0;
    }

    file = fopen(source_index, "rb");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char row_line[PP_BODY_MAX];
        char *version;
        char source_body[PP_PATH_MAX];
        char destination_body[PP_PATH_MAX];
        char source_meta[PP_PATH_MAX];
        char destination_meta[PP_PATH_MAX];

        snprintf(row_line, sizeof(row_line), "%s", line);
        version = strtok(row_line, "\t\r\n");
        if (version == NULL || strcmp(version, "version") == 0 || !valid_version_name(version)) {
            continue;
        }

        if (!version_body_path(source_body, sizeof(source_body), source_root, row, version) ||
            !version_body_path(destination_body, sizeof(destination_body), destination_root, row,
                               version) ||
            !version_metadata_path(source_meta, sizeof(source_meta), source_root, row, version) ||
            !version_metadata_path(destination_meta, sizeof(destination_meta), destination_root,
                                   row, version) ||
            !copy_file_if_exists(source_body, destination_body) ||
            !copy_file_if_exists(source_meta, destination_meta)) {
            fclose(file);
            return 0;
        }
    }

    return fclose(file) == 0;
}

static int copy_prompt_payload(const char *source_root, const char *destination_root,
                               const struct prompt_index_row *row) {
    char destination_prompt_dir[PP_PATH_MAX];
    char source_body[PP_PATH_MAX];
    char destination_body[PP_PATH_MAX];
    char source_metadata[PP_PATH_MAX];
    char destination_metadata[PP_PATH_MAX];

    return prompt_dir_path(destination_prompt_dir, sizeof(destination_prompt_dir), destination_root,
                           row) &&
           prompt_body_path(source_body, sizeof(source_body), source_root, row) &&
           prompt_body_path(destination_body, sizeof(destination_body), destination_root, row) &&
           prompt_metadata_path(source_metadata, sizeof(source_metadata), source_root, row) &&
           prompt_metadata_path(destination_metadata, sizeof(destination_metadata),
                                destination_root, row) &&
           make_dir_recursive(destination_prompt_dir) &&
           copy_text_file(source_body, destination_body) &&
           copy_text_file(source_metadata, destination_metadata) &&
           copy_prompt_versions(source_root, destination_root, row);
}

static int index_file_path(char *out, size_t out_size, const char *root) {
    char meta_dir[PP_PATH_MAX];

    return join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") &&
           join_path(out, out_size, meta_dir, "index.tsv");
}

static int registry_file_path(char *out, size_t out_size, const char *root, const char *kind) {
    char meta_dir[PP_PATH_MAX];
    const char *filename = strcmp(kind, "folder") == 0 ? "folders.tsv" : "categories.tsv";

    return join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") &&
           join_path(out, out_size, meta_dir, filename);
}

static int registry_contains(const char *root, const char *kind, const char *name) {
    char registry_file[PP_PATH_MAX];
    char line[PP_FIELD_MAX];
    FILE *file;

    if (!registry_file_path(registry_file, sizeof(registry_file), root, kind)) {
        return 0;
    }

    file = fopen(registry_file, "rb");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, name) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static int registry_add(const char *root, const char *kind, const char *name) {
    char registry_file[PP_PATH_MAX];
    char line[PP_FIELD_MAX];

    if (field_is_too_long(name) || field_has_unsupported_chars(name) ||
        (strcmp(kind, "folder") == 0 && path_has_unsafe_segment(name))) {
        fprintf(stderr, "%s name contains unsupported characters.\n", kind);
        return 0;
    }

    if (registry_contains(root, kind, name)) {
        return 1;
    }

    if (!registry_file_path(registry_file, sizeof(registry_file), root, kind)) {
        return 0;
    }

    snprintf(line, sizeof(line), "%s\n", name);
    return append_text_file(registry_file, line);
}

static int registry_rewrite(const char *root, const char *kind, const char *from, const char *to,
                            int remove_target) {
    char registry_file[PP_PATH_MAX];
    char temp_file[PP_PATH_MAX];
    char line[PP_FIELD_MAX];
    FILE *input;
    FILE *output;

    if (!registry_file_path(registry_file, sizeof(registry_file), root, kind) ||
        snprintf(temp_file, sizeof(temp_file), "%s.tmp", registry_file) <= 0 ||
        strlen(temp_file) >= sizeof(temp_file)) {
        return 0;
    }

    input = fopen(registry_file, "rb");
    if (input == NULL) {
        fprintf(stderr, "Could not read %s registry.\n", kind);
        return 0;
    }

    output = fopen(temp_file, "wb");
    if (output == NULL) {
        fclose(input);
        fprintf(stderr, "Could not write %s registry.\n", kind);
        return 0;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        char clean[PP_FIELD_MAX];
        snprintf(clean, sizeof(clean), "%s", line);
        clean[strcspn(clean, "\r\n")] = '\0';
        if (strcmp(clean, from) == 0) {
            if (!remove_target) {
                fprintf(output, "%s\n", to);
            }
        } else {
            fputs(line, output);
        }
    }

    if (fclose(input) != 0 || fclose(output) != 0) {
        return 0;
    }

    if (!replace_file_with_temp(temp_file, registry_file)) {
        fprintf(stderr, "Could not update %s registry.\n", kind);
        return 0;
    }

    return 1;
}

static int registry_list(const char *root, const char *kind) {
    char registry_file[PP_PATH_MAX];
    char line[PP_FIELD_MAX];
    FILE *file;

    if (!registry_file_path(registry_file, sizeof(registry_file), root, kind)) {
        return 0;
    }

    file = fopen(registry_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read %s registry.\n", kind);
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        fputs(line, stdout);
    }

    return fclose(file) == 0;
}

static int read_metadata_value(const char *metadata_file, const char *key, char *out,
                               size_t out_size) {
    FILE *file;
    char line[PP_BODY_MAX];
    size_t key_len = strlen(key);

    file = fopen(metadata_file, "rb");
    if (file == NULL) {
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '\t') {
            char *value = line + key_len + 1;
            value[strcspn(value, "\r\n")] = '\0';
            snprintf(out, out_size, "%s", value);
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static int write_prompt_metadata(const char *root, const struct prompt_index_row *row,
                                 const char *description, const char *current_version) {
    char metadata_file[PP_PATH_MAX];
    char metadata[PP_BODY_MAX];

    if (!prompt_metadata_path(metadata_file, sizeof(metadata_file), root, row)) {
        return 0;
    }

    snprintf(metadata, sizeof(metadata),
             "id\t%s\n"
             "title\t%s\n"
             "folder\t%s\n"
             "category\t%s\n"
             "updated_at\t%s\n"
             "current_version\t%s\n"
             "tag\t%s\n"
             "description\t%s\n",
             row->id, row->title, row->folder, row->category, row->updated_at, current_version,
             row->tags, description == NULL ? "" : description);

    return write_text_file(metadata_file, metadata);
}

static void format_version_number(int version, char *out, size_t out_size) {
    snprintf(out, out_size, "%04d", version);
}

static int next_version_number(const char *root, const struct prompt_index_row *row) {
    char index_file[PP_PATH_MAX];
    FILE *file;
    char line[PP_BODY_MAX];
    int max_version = 1;

    if (!versions_index_path(index_file, sizeof(index_file), root, row) ||
        !path_exists_as_file(index_file)) {
        return 2;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        return 2;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        int version = atoi(line);
        if (version > max_version) {
            max_version = version;
        }
    }

    fclose(file);
    return max_version + 1;
}

static int rewrite_index_row(const char *root, const struct prompt_index_row *target,
                             int remove_target) {
    char index_file[PP_PATH_MAX];
    char temp_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *input;
    FILE *output;

    if (!index_file_path(index_file, sizeof(index_file), root) ||
        snprintf(temp_file, sizeof(temp_file), "%s.tmp", index_file) <= 0 ||
        strlen(temp_file) >= sizeof(temp_file)) {
        fprintf(stderr, "Index path is too long.\n");
        return 0;
    }

    input = fopen(index_file, "rb");
    if (input == NULL) {
        fprintf(stderr, "Could not read index '%s': %s\n", index_file, strerror(errno));
        return 0;
    }

    output = fopen(temp_file, "wb");
    if (output == NULL) {
        fprintf(stderr, "Could not write temporary index '%s': %s\n", temp_file, strerror(errno));
        fclose(input);
        return 0;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        char row_line[PP_BODY_MAX];
        struct prompt_index_row row;

        snprintf(row_line, sizeof(row_line), "%s", line);
        if (parse_index_row(row_line, &row) && strcmp(row.id, target->id) == 0) {
            if (!remove_target) {
                fprintf(output, "%s\t%s\t%s\t%s\t%s\t%s\n", target->id, target->title,
                        target->folder, target->category, target->tags, target->updated_at);
            }
        } else {
            fputs(line, output);
        }
    }

    if (fclose(input) != 0 || fclose(output) != 0) {
        fprintf(stderr, "Could not close index files.\n");
        return 0;
    }

    if (!replace_file_with_temp(temp_file, index_file)) {
        return 0;
    }

    return 1;
}

static int run_init(int argc, char **argv) {
    char root[PP_PATH_MAX];
    int index;

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_init_help();
            return 0;
        }
    }

    if (!resolve_root_arg(2, argc, argv, root, sizeof(root))) {
        return 1;
    }

    return init_library(root) ? 0 : 1;
}

static int parse_add_options(int argc, char **argv, struct prompt_options *options) {
    int index;

    options->folder = "inbox";
    options->category = "general";

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_add_help();
            return 2;
        }

        if (strcmp(argv[index], "--editor") == 0) {
            options->editor = 1;
        } else if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--title") == 0 ||
                   strcmp(argv[index], "--body") == 0 || strcmp(argv[index], "--folder") == 0 ||
                   strcmp(argv[index], "--category") == 0 || strcmp(argv[index], "--tag") == 0 ||
                   strcmp(argv[index], "--description") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 0;
            }
            if (strcmp(argv[index], "--root") == 0) {
                options->root = argv[++index];
            } else if (strcmp(argv[index], "--title") == 0) {
                options->title = argv[++index];
            } else if (strcmp(argv[index], "--body") == 0) {
                options->body = argv[++index];
            } else if (strcmp(argv[index], "--folder") == 0) {
                options->folder = argv[++index];
            } else if (strcmp(argv[index], "--category") == 0) {
                options->category = argv[++index];
            } else if (strcmp(argv[index], "--tag") == 0) {
                if (!add_tag_value(options, argv[++index])) {
                    return 0;
                }
            } else {
                options->description = argv[++index];
            }
        } else {
            fprintf(stderr, "Unknown add option: %s\n", argv[index]);
            return 0;
        }
    }

    if (options->title == NULL || options->title[0] == '\0') {
        fprintf(stderr, "Missing required option: --title.\n");
        return 0;
    }

    if (options->body == NULL || options->body[0] == '\0') {
        if (!options->editor) {
            fprintf(stderr, "Missing required option: --body (or use --editor).\n");
            return 0;
        }
    }

    if (field_is_too_long(options->title) || field_is_too_long(options->folder) ||
        field_is_too_long(options->category) || field_is_too_long(options->description) ||
        field_has_unsupported_chars(options->title) ||
        field_has_unsupported_chars(options->folder) ||
        field_has_unsupported_chars(options->category) ||
        field_has_unsupported_chars(options->description) ||
        path_has_unsafe_segment(options->folder)) {
        fprintf(stderr, "Prompt title, folder, or category contains unsupported characters.\n");
        return 0;
    }

    return 1;
}

/*
 * spawn_editor - Opens $EDITOR (or fallback) with pre-filled content.
 * Writes `current_content` to a temporary file, spawns the editor, then
 * reads the modified content back into `out` (up to `out_size`).
 * Returns 1 on success, 0 if the editor returned non-zero or the
 * temporary file could not be read.
 */
static int spawn_editor(const char *current_content, char *out, size_t out_size) {
    const char *editor;
    char temp_path[PP_PATH_MAX];
    char cmd[PP_PATH_MAX + 128];
    FILE *file;

#ifdef _WIN32
    char tmp_dir[MAX_PATH + 1];
    UINT tmp_len;

    editor = getenv("EDITOR");
    if (editor == NULL || editor[0] == '\0') {
        /* Prefer a Markdown-capable editor for .md files.
         * Try VS Code first, fall back to notepad. */
        if (system("where code >NUL 2>&1") == 0) {
            editor = "code --wait";
        } else {
            editor = "notepad";
        }
    }

    tmp_len = GetTempPathA(sizeof(tmp_dir), tmp_dir);
    if (tmp_len == 0 || tmp_len >= sizeof(tmp_dir)) {
        fprintf(stderr, "Could not get temporary directory.\n");
        return 0;
    }

    {
#ifdef HAS_MKTEMP_S
        if (snprintf(temp_path, sizeof(temp_path), "%spromptlib_edit_XXXXXX.md", tmp_dir) <= 0 ||
            strlen(temp_path) >= sizeof(temp_path)) {
            fprintf(stderr, "Temporary path is too long.\n");
            return 0;
        }
        if (_mktemp_s(temp_path, strlen(temp_path) + 1) != 0) {
            fprintf(stderr, "Could not create temporary file name.\n");
            return 0;
        }
#else
        /* Fallback: use pid + time */
        if (snprintf(temp_path, sizeof(temp_path), "%spromptlib_edit_%d_%d.md", tmp_dir,
                     (int)getpid(), (int)time(NULL)) <= 0 ||
            strlen(temp_path) >= sizeof(temp_path)) {
            fprintf(stderr, "Temporary path is too long.\n");
            return 0;
        }
#endif
    }
#else
    const char *tmpdir;
    int fd;

    editor = getenv("EDITOR");
    if (editor == NULL || editor[0] == '\0') {
        editor = "vi";
    }

    tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL || tmpdir[0] == '\0') {
        tmpdir = "/tmp";
    }
    if (snprintf(temp_path, sizeof(temp_path), "%s/promptlib_edit_XXXXXX", tmpdir) <= 0 ||
        strlen(temp_path) >= sizeof(temp_path)) {
        fprintf(stderr, "Temporary path is too long.\n");
        return 0;
    }
    fd = mkstemp(temp_path);
    if (fd < 0) {
        fprintf(stderr, "Could not create temporary file.\n");
        return 0;
    }
    close(fd);
#endif

    file = fopen(temp_path, "wb");
    if (file == NULL) {
        fprintf(stderr, "Could not write temporary file '%s': %s\n", temp_path, strerror(errno));
        return 0;
    }
    if (current_content != NULL && fputs(current_content, file) == EOF) {
        fprintf(stderr, "Could not write temporary file.\n");
        fclose(file);
        remove(temp_path);
        return 0;
    }
    fclose(file);

    snprintf(cmd, sizeof(cmd), "%s \"%s\"", editor, temp_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "Editor returned non-zero. Content not updated.\n");
        remove(temp_path);
        return 0;
    }

    out[0] = '\0';
    file = fopen(temp_path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read temporary file after editing.\n");
        remove(temp_path);
        return 0;
    }
    {
        size_t read_count = fread(out, 1, out_size - 1, file);
        out[read_count] = '\0';
        if (ferror(file)) {
            fprintf(stderr, "Could not read edited content.\n");
            fclose(file);
            remove(temp_path);
            return 0;
        }
    }
    fclose(file);
    remove(temp_path);

    return 1;
}

static int run_add(int argc, char **argv) {
    struct prompt_options options;
    char root[PP_PATH_MAX];
    char id[PP_FIELD_MAX];
    char timestamp[PP_FIELD_MAX];
    char prompts_dir[PP_PATH_MAX];
    char folder_dir[PP_PATH_MAX];
    char prompt_dir[PP_PATH_MAX];
    char body_file[PP_PATH_MAX];
    char metadata_file[PP_PATH_MAX];
    char meta_dir[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char metadata[PP_BODY_MAX];
    char index_row[PP_BODY_MAX];
    int parsed;

    memset(&options, 0, sizeof(options));
    parsed = parse_add_options(argc, argv, &options);
    if (parsed == 2) {
        return 0;
    }
    if (!parsed) {
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root)) {
        return 1;
    }

    prompt_id_from_title(options.title, id, sizeof(id));
    current_timestamp(timestamp, sizeof(timestamp));

    if (!join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") ||
        !join_path(folder_dir, sizeof(folder_dir), prompts_dir, options.folder) ||
        !join_path(prompt_dir, sizeof(prompt_dir), folder_dir, id) ||
        !join_path(body_file, sizeof(body_file), prompt_dir, "current.md") ||
        !join_path(metadata_file, sizeof(metadata_file), prompt_dir, "metadata.tsv") ||
        !join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") ||
        !join_path(index_file, sizeof(index_file), meta_dir, "index.tsv")) {
        fprintf(stderr, "Prompt path is too long.\n");
        return 1;
    }

    if (path_exists_as_dir(prompt_dir)) {
        fprintf(stderr, "Prompt already exists: %s\n", id);
        return 1;
    }

    snprintf(metadata, sizeof(metadata),
             "id\t%s\n"
             "title\t%s\n"
             "folder\t%s\n"
             "category\t%s\n"
             "created_at\t%s\n"
             "updated_at\t%s\n"
             "current_version\t1\n"
             "tag\t%s\n"
             "description\t%s\n",
             id, options.title, options.folder, options.category, timestamp, timestamp,
             options.tags, options.description == NULL ? "" : options.description);

    snprintf(index_row, sizeof(index_row), "%s\t%s\t%s\t%s\t%s\t%s\n", id, options.title,
             options.folder, options.category, options.tags, timestamp);

    if (!make_dir_recursive(prompt_dir)) {
        return 1;
    }

    if (options.editor) {
        char edited_body[PP_BODY_MAX];
        if (!spawn_editor(options.body == NULL ? "" : options.body, edited_body,
                          sizeof(edited_body)) ||
            !write_text_file(body_file, edited_body)) {
            return 1;
        }
    } else if (!write_text_file(body_file, options.body)) {
        return 1;
    }

    if (!write_text_file(metadata_file, metadata) || !append_text_file(index_file, index_row)) {
        return 1;
    }

    if (!registry_add(root, "folder", options.folder) ||
        !registry_add(root, "category", options.category)) {
        return 1;
    }

    printf("Saved prompt: %s\n", id);
    return 0;
}

/*
 * escape_json - Escapes a string for JSON output.
 * Writes to `out` up to `out_size` bytes. Returns the number of bytes written.
 */
static int escape_json(const char *input, char *out, size_t out_size) {
    size_t in_idx, out_idx;

    if (out == NULL || out_size == 0) {
        return 0;
    }

    out[0] = '\0';
    if (input == NULL) {
        return 0;
    }

    in_idx = 0;
    out_idx = 0;
    while (input[in_idx] != '\0' && out_idx + 6 < out_size) {
        switch (input[in_idx]) {
        case '"':
            out[out_idx++] = '\\';
            out[out_idx++] = '"';
            break;
        case '\\':
            out[out_idx++] = '\\';
            out[out_idx++] = '\\';
            break;
        case '\n':
            out[out_idx++] = '\\';
            out[out_idx++] = 'n';
            break;
        case '\t':
            out[out_idx++] = '\\';
            out[out_idx++] = 't';
            break;
        case '\r':
            out[out_idx++] = '\\';
            out[out_idx++] = 'r';
            break;
        default:
            if ((unsigned char)input[in_idx] < 0x20) {
                /* skip control chars */
                ++in_idx;
                continue;
            }
            out[out_idx++] = input[in_idx];
            break;
        }
        ++in_idx;
    }
    out[out_idx] = '\0';
    return (int)out_idx;
}

/*
 * print_json_prompt_row - Prints a single prompt index row as JSON to stdout.
 */
static void print_json_prompt_row(const struct prompt_index_row *row) {
    char buf[PP_BODY_MAX];

    printf("  {");
    escape_json(row->id, buf, sizeof(buf));
    printf("\"id\": \"%s\"", buf);
    escape_json(row->title, buf, sizeof(buf));
    printf(", \"title\": \"%s\"", buf);
    escape_json(row->folder, buf, sizeof(buf));
    printf(", \"folder\": \"%s\"", buf);
    escape_json(row->category, buf, sizeof(buf));
    printf(", \"category\": \"%s\"", buf);
    escape_json(row->tags, buf, sizeof(buf));
    printf(", \"tags\": \"%s\"", buf);
    escape_json(row->updated_at, buf, sizeof(buf));
    printf(", \"updated_at\": \"%s\"", buf);
    printf(" }");
}

/*
 * print_json_prompt_body - Prints a prompt body as a JSON string to stdout.
 */
static void print_json_body(const char *body) {
    char buf[PP_BODY_MAX];

    escape_json(body, buf, sizeof(buf));
    printf("\"%s\"", buf);
}

/* Color and pager support */

/* ANSI color codes */
#define COLOR_CLEAR "\x1b[0m"
#define COLOR_BOLD "\x1b[1m"
#define COLOR_DIM "\x1b[2m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_MAGENTA "\x1b[35m"

/*
 * use_color - Checks whether ANSI color output should be used.
 * Returns 1 if stdout is a terminal and NO_COLOR is not set.
 */
static int use_color(void) {
    const char *no_color;

    if (!isatty(1))
        return 0;

    no_color = getenv("NO_COLOR");
    if (no_color != NULL && no_color[0] != '\0')
        return 0;

    return 1;
}

/*
 * color - Returns a color escape sequence if color is enabled, empty string otherwise.
 */
static const char *color(const char *code, int enabled) {
    return enabled ? code : "";
}

/*
 * auto_pager - Pipes output through $PAGER (or less/more fallback).
 * Returns the FILE* to write to, or stdout if paging is disabled.
 * Caller should call pager_close() at the end.
 */
static FILE *pager_open(int use_paging) {
    const char *pager;

    if (!use_paging || !isatty(1))
        return stdout;

    pager = getenv("PAGER");
    if (pager == NULL || pager[0] == '\0') {
#ifdef _WIN32
        pager = "more";
#else
        pager = "less -R";
#endif
    }

    {
        FILE *p = popen(pager, "w");
        if (p != NULL)
            return p;
    }
    return stdout;
}

static void pager_close(FILE *p) {
    if (p != NULL && p != stdout) {
        pclose(p);
    }
}

static int run_list(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char meta_dir[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    struct prompt_options filters;
    FILE *file;
    FILE *out;
    int index;
    int count;
    int use_paging;
    int c;

    memset(&filters, 0, sizeof(filters));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_list_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --root.\n");
                return 1;
            }
            filters.root = argv[++index];
        } else if (strcmp(argv[index], "--folder") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --folder.\n");
                return 1;
            }
            filters.filter_folder = argv[++index];
        } else if (strcmp(argv[index], "--category") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --category.\n");
                return 1;
            }
            filters.filter_category = argv[++index];
        } else if (strcmp(argv[index], "--tag") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --tag.\n");
                return 1;
            }
            filters.filter_tag = argv[++index];
        } else if (strcmp(argv[index], "--json") == 0) {
            filters.json = 1;
        } else if (strcmp(argv[index], "--no-pager") == 0) {
            filters.no_pager = 1;
        } else {
            fprintf(stderr, "Unknown list option: %s\n", argv[index]);
            return 1;
        }
    }

    if (filters.root != NULL) {
        snprintf(root, sizeof(root), "%s", filters.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (field_is_too_long(filters.filter_folder) || field_is_too_long(filters.filter_category) ||
        field_is_too_long(filters.filter_tag) ||
        field_has_unsupported_chars(filters.filter_folder) ||
        field_has_unsupported_chars(filters.filter_category) ||
        field_has_unsupported_chars(filters.filter_tag)) {
        fprintf(stderr, "List filter contains unsupported characters.\n");
        return 1;
    }

    if (!validate_library_root(root) ||
        !join_path(meta_dir, sizeof(meta_dir), root, ".promptlib") ||
        !join_path(index_file, sizeof(index_file), meta_dir, "index.tsv")) {
        return 1;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read index '%s': %s\n", index_file, strerror(errno));
        return 1;
    }

    /* For JSON output, skip colors and pager */
    if (filters.json) {
        puts("[");
        count = 0;
        while (fgets(line, sizeof(line), file) != NULL) {
            struct prompt_index_row row;
            if (parse_index_row(line, &row) && row_matches_filters(&row, &filters)) {
                if (count > 0)
                    puts(",");
                print_json_prompt_row(&row);
                ++count;
            }
        }
        puts("\n]");
        fclose(file);
        return 0;
    }

    c = use_color();
    use_paging = !filters.no_pager;
    out = pager_open(use_paging);

    /* Table header with colors */
    fprintf(out, "%s%-24s  %-28s  %-16s  %-16s  %s%s\n", color(COLOR_CYAN, c), "ID", "TITLE",
            "FOLDER", "CATEGORY", "TAGS", color(COLOR_CLEAR, c));
    fprintf(out, "%s%-24s  %-28s  %-16s  %-16s  %s%s\n", color(COLOR_DIM, c), "---", "---", "---",
            "---", "---", color(COLOR_CLEAR, c));

    count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        struct prompt_index_row row;
        if (parse_index_row(line, &row) && row_matches_filters(&row, &filters)) {
            fprintf(out, "%s%-24s%s  %-28s  %-16s  %-16s  %s\n", color(COLOR_YELLOW, c), row.id,
                    color(COLOR_CLEAR, c), row.title, row.folder, row.category, row.tags);
            ++count;
        }
    }
    fclose(file);

    if (count == 0) {
        fprintf(out, "No prompts found.\n");
    } else {
        fprintf(out, "\n%s%d prompt(s)%s\n", color(COLOR_DIM, c), count, color(COLOR_CLEAR, c));
    }

    pager_close(out);
    return 0;
}

static int run_show(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char body_file[PP_PATH_MAX];
    char body[PP_BODY_MAX];
    struct prompt_options options;
    struct prompt_index_row row;
    int index;

    memset(&options, 0, sizeof(options));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_show_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --root.\n");
                return 1;
            }
            options.root = argv[++index];
        } else if (strcmp(argv[index], "--raw") == 0) {
            options.raw = 1;
        } else if (strcmp(argv[index], "--json") == 0) {
            options.json = 1;
        } else if (options.id_or_title == NULL) {
            options.id_or_title = argv[index];
        } else {
            fprintf(stderr, "Unexpected show argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (options.id_or_title == NULL) {
        fprintf(stderr, "Missing prompt id or title.\n");
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !find_prompt(root, options.id_or_title, &row) ||
        !prompt_body_path(body_file, sizeof(body_file), root, &row) ||
        !read_text_file(body_file, body, sizeof(body))) {
        if (!find_prompt(root, options.id_or_title, &row)) {
            fprintf(stderr, "Prompt not found: %s\n", options.id_or_title);
        }
        return 1;
    }

    if (options.json) {
        char buf[PP_BODY_MAX];

        printf("{\n");
        escape_json(row.id, buf, sizeof(buf));
        printf("  \"id\": \"%s\",\n", buf);
        escape_json(row.title, buf, sizeof(buf));
        printf("  \"title\": \"%s\",\n", buf);
        escape_json(row.folder, buf, sizeof(buf));
        printf("  \"folder\": \"%s\",\n", buf);
        escape_json(row.category, buf, sizeof(buf));
        printf("  \"category\": \"%s\",\n", buf);
        escape_json(row.tags, buf, sizeof(buf));
        printf("  \"tags\": \"%s\",\n", buf);
        printf("  \"body\": ");
        print_json_body(body);
        printf("\n}\n");
    } else if (!options.raw) {
        int c = use_color();
        /* Separator line */
        printf("%s%s%s\n", color(COLOR_CYAN, c), "----------", color(COLOR_CLEAR, c));
        printf("%sID:%s      %s%s\n", color(COLOR_CYAN, c), color(COLOR_CLEAR, c),
               color(COLOR_YELLOW, c), row.id);
        printf("%sTitle:%s   %s%s\n", color(COLOR_CYAN, c), color(COLOR_CLEAR, c),
               color(COLOR_BOLD, c), row.title);
        printf("%sFolder:%s  %s\n", color(COLOR_CYAN, c), color(COLOR_CLEAR, c), row.folder);
        printf("%sCategory:%s %s\n", color(COLOR_CYAN, c), color(COLOR_CLEAR, c), row.category);
        printf("%sTags:%s    %s\n", color(COLOR_CYAN, c), color(COLOR_CLEAR, c), row.tags);
        printf("%s%s%s\n", color(COLOR_CYAN, c), "----------", color(COLOR_CLEAR, c));
        printf("\n");
        printf("%s", body);
        if (body[0] != '\0' && body[strlen(body) - 1] != '\n') {
            puts("");
        }
        printf("%s%s%s\n", color(COLOR_DIM, c), "----------", color(COLOR_CLEAR, c));
    } else {
        printf("%s", body);
        if (body[0] != '\0' && body[strlen(body) - 1] != '\n') {
            puts("");
        }
    }

    return 0;
}

static int run_search(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    struct prompt_options options;
    FILE *file;
    int index;
    int matches = 0;

    memset(&options, 0, sizeof(options));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_search_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--folder") == 0 ||
            strcmp(argv[index], "--category") == 0 || strcmp(argv[index], "--tag") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                options.root = argv[++index];
            } else if (strcmp(argv[index], "--folder") == 0) {
                options.filter_folder = argv[++index];
            } else if (strcmp(argv[index], "--category") == 0) {
                options.filter_category = argv[++index];
            } else {
                options.filter_tag = argv[++index];
            }
        } else if (strcmp(argv[index], "--raw") == 0) {
            options.raw = 1;
        } else if (strcmp(argv[index], "--json") == 0) {
            options.json = 1;
            options.raw = 1;
        } else if (options.id_or_title == NULL) {
            options.id_or_title = argv[index];
        } else {
            fprintf(stderr, "Unexpected search argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (options.id_or_title == NULL || options.id_or_title[0] == '\0') {
        fprintf(stderr, "Missing search query.\n");
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (field_is_too_long(options.filter_folder) || field_is_too_long(options.filter_category) ||
        field_is_too_long(options.filter_tag) ||
        field_has_unsupported_chars(options.filter_folder) ||
        field_has_unsupported_chars(options.filter_category) ||
        field_has_unsupported_chars(options.filter_tag)) {
        fprintf(stderr, "Search filter contains unsupported characters.\n");
        return 1;
    }

    if (!validate_library_root(root) || !index_file_path(index_file, sizeof(index_file), root)) {
        return 1;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read index '%s': %s\n", index_file, strerror(errno));
        return 1;
    }

    if (!options.raw) {
        printf("%-24s  %-28s  %-16s  %-16s  %s\n", "ID", "TITLE", "FOLDER", "CATEGORY", "TAGS");
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char row_line[PP_BODY_MAX];
        char body_file[PP_PATH_MAX];
        char body[PP_BODY_MAX];
        struct prompt_index_row row;

        snprintf(row_line, sizeof(row_line), "%s", line);
        if (!parse_index_row(row_line, &row) || !row_matches_filters(&row, &options)) {
            continue;
        }

        if (!prompt_body_path(body_file, sizeof(body_file), root, &row) ||
            !read_text_file(body_file, body, sizeof(body))) {
            fclose(file);
            return 1;
        }

        if (!row_matches_query(&row, body, options.id_or_title)) {
            continue;
        }

        ++matches;
        if (options.raw) {
            printf("%s", body);
            if (body[0] != '\0' && body[strlen(body) - 1] != '\n') {
                puts("");
            }
        } else {
            printf("%-24s  %-28s  %-16s  %-16s  %s\n", row.id, row.title, row.folder, row.category,
                   row.tags);
        }
    }

    fclose(file);
    if (matches == 0 && !options.raw) {
        puts("No prompts found.");
    }

    return 0;
}

static int run_edit(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char body_file[PP_PATH_MAX];
    char metadata_file[PP_PATH_MAX];
    char timestamp[PP_FIELD_MAX];
    char metadata[PP_BODY_MAX];
    char edited_body[PP_BODY_MAX];
    struct prompt_options options;
    struct prompt_index_row row;
    int index;
    int edit_mode;

    memset(&options, 0, sizeof(options));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_edit_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--body") == 0 ||
            strcmp(argv[index], "--category") == 0 || strcmp(argv[index], "--tag") == 0 ||
            strcmp(argv[index], "--description") == 0 || strcmp(argv[index], "--title") == 0 ||
            strcmp(argv[index], "--folder") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                options.root = argv[++index];
            } else if (strcmp(argv[index], "--body") == 0) {
                options.body = argv[++index];
            } else if (strcmp(argv[index], "--category") == 0) {
                options.category = argv[++index];
            } else if (strcmp(argv[index], "--tag") == 0) {
                if (!add_tag_value(&options, argv[++index])) {
                    return 1;
                }
            } else if (strcmp(argv[index], "--title") == 0) {
                options.title = argv[++index];
            } else if (strcmp(argv[index], "--folder") == 0) {
                options.folder = argv[++index];
            } else {
                options.description = argv[++index];
            }
        } else if (options.id_or_title == NULL) {
            options.id_or_title = argv[index];
        } else {
            fprintf(stderr, "Unexpected edit argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (options.id_or_title == NULL) {
        fprintf(stderr, "Missing prompt id or title.\n");
        return 1;
    }

    edit_mode = (options.body == NULL && options.category == NULL && options.description == NULL &&
                 !options.tags_set && options.title == NULL && options.folder == NULL);

    if (!edit_mode && options.body == NULL && options.category == NULL &&
        options.description == NULL && !options.tags_set && options.title == NULL &&
        options.folder == NULL) {
        fprintf(stderr, "Nothing to edit. Pass --body, --category, --tag, --description,\n");
        fprintf(stderr, "  --title, or --folder.\n");
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !find_prompt(root, options.id_or_title, &row)) {
        fprintf(stderr, "Prompt not found: %s\n", options.id_or_title);
        return 1;
    }

    current_timestamp(timestamp, sizeof(timestamp));
    snprintf(row.updated_at, sizeof(row.updated_at), "%s", timestamp);
    if (options.category != NULL) {
        if (field_is_too_long(options.category) || field_has_unsupported_chars(options.category)) {
            fprintf(stderr, "Category contains unsupported characters.\n");
            return 1;
        }
        snprintf(row.category, sizeof(row.category), "%s", options.category);
    }
    if (options.tags_set) {
        snprintf(row.tags, sizeof(row.tags), "%s", options.tags);
    }
    if (options.title != NULL) {
        if (field_is_too_long(options.title) || field_has_unsupported_chars(options.title)) {
            fprintf(stderr, "Title contains unsupported characters.\n");
            return 1;
        }
        snprintf(row.title, sizeof(row.title), "%s", options.title);
    }
    if (options.folder != NULL) {
        if (field_is_too_long(options.folder) || field_has_unsupported_chars(options.folder) ||
            path_has_unsafe_segment(options.folder)) {
            fprintf(stderr, "Folder contains unsupported characters.\n");
            return 1;
        }
        snprintf(row.folder, sizeof(row.folder), "%s", options.folder);
    }

    if (!prompt_body_path(body_file, sizeof(body_file), root, &row)) {
        return 1;
    }

    if (edit_mode) {
        char current_body[PP_BODY_MAX];
        if (!read_text_file(body_file, current_body, sizeof(current_body)) ||
            !spawn_editor(current_body, edited_body, sizeof(edited_body)) ||
            !write_text_file(body_file, edited_body)) {
            return 1;
        }
    } else if (options.body != NULL) {
        if (!write_text_file(body_file, options.body)) {
            return 1;
        }
    }

    if (!prompt_metadata_path(metadata_file, sizeof(metadata_file), root, &row)) {
        return 1;
    }

    snprintf(metadata, sizeof(metadata),
             "id\t%s\n"
             "title\t%s\n"
             "folder\t%s\n"
             "category\t%s\n"
             "updated_at\t%s\n"
             "current_version\t1\n"
             "tag\t%s\n"
             "description\t%s\n",
             row.id, row.title, row.folder, row.category, row.updated_at, row.tags,
             options.description == NULL ? "" : options.description);

    if (!write_text_file(metadata_file, metadata) || !rewrite_index_row(root, &row, 0)) {
        return 1;
    }

    if (!registry_add(root, "category", row.category) ||
        !registry_add(root, "folder", row.folder)) {
        return 1;
    }

    printf("Updated prompt: %s\n", row.id);
    return 0;
}

static int append_version_index(const char *root, const struct prompt_index_row *row,
                                const char *version, const char *timestamp, const char *promoted,
                                const char *note) {
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];

    if (!versions_index_path(index_file, sizeof(index_file), root, row) ||
        !write_text_file_if_missing(index_file, "version\tpromoted\tcreated_at\tnote\n")) {
        return 0;
    }

    snprintf(line, sizeof(line), "%s\t%s\t%s\t%s\n", version, promoted, timestamp,
             note == NULL ? "" : note);

    return append_text_file(index_file, line);
}

static int show_version_history(const char *root, const struct prompt_index_row *row) {
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *file;

    if (!versions_index_path(index_file, sizeof(index_file), root, row) ||
        !path_exists_as_file(index_file)) {
        puts("No optimized versions found.");
        return 1;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read version history '%s': %s\n", index_file, strerror(errno));
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        fputs(line, stdout);
    }

    return fclose(file) == 0;
}

static int compare_version(const char *root, const struct prompt_index_row *row,
                           const char *version) {
    char current_file[PP_PATH_MAX];
    char version_file[PP_PATH_MAX];
    char current_body[PP_BODY_MAX];
    char version_body[PP_BODY_MAX];

    if (!valid_version_name(version)) {
        fprintf(stderr, "Invalid version.\n");
        return 0;
    }

    if (!prompt_body_path(current_file, sizeof(current_file), root, row) ||
        !version_body_path(version_file, sizeof(version_file), root, row, version) ||
        !path_exists_as_file(version_file)) {
        fprintf(stderr, "Version not found: %s\n", version);
        return 0;
    }

    if (!read_text_file(current_file, current_body, sizeof(current_body)) ||
        !read_text_file(version_file, version_body, sizeof(version_body))) {
        return 0;
    }

    printf("Current:\n%s", current_body);
    if (current_body[0] != '\0' && current_body[strlen(current_body) - 1] != '\n') {
        puts("");
    }
    printf("\nVersion %s:\n%s", version, version_body);
    if (version_body[0] != '\0' && version_body[strlen(version_body) - 1] != '\n') {
        puts("");
    }

    return 1;
}

static int run_optimize(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char versions_dir[PP_PATH_MAX];
    char version[PP_FIELD_MAX];
    char timestamp[PP_FIELD_MAX];
    char version_body_file[PP_PATH_MAX];
    char version_metadata_file[PP_PATH_MAX];
    char current_body_file[PP_PATH_MAX];
    char metadata_file[PP_PATH_MAX];
    char current_version[PP_FIELD_MAX];
    char version_metadata[PP_BODY_MAX];
    struct prompt_options options;
    struct prompt_index_row row;
    int index;
    int next_version;

    memset(&options, 0, sizeof(options));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_optimize_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--body") == 0 ||
            strcmp(argv[index], "--note") == 0 || strcmp(argv[index], "--compare") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                options.root = argv[++index];
            } else if (strcmp(argv[index], "--body") == 0) {
                options.body = argv[++index];
            } else if (strcmp(argv[index], "--note") == 0) {
                options.note = argv[++index];
            } else {
                options.compare_version = argv[++index];
            }
        } else if (strcmp(argv[index], "--promote") == 0) {
            options.promote = 1;
        } else if (strcmp(argv[index], "--history") == 0) {
            options.history = 1;
        } else if (options.id_or_title == NULL) {
            options.id_or_title = argv[index];
        } else {
            fprintf(stderr, "Unexpected optimize argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (options.id_or_title == NULL) {
        fprintf(stderr, "Missing prompt id or title.\n");
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !find_prompt(root, options.id_or_title, &row)) {
        fprintf(stderr, "Prompt not found: %s\n", options.id_or_title);
        return 1;
    }

    if (options.history) {
        return show_version_history(root, &row) ? 0 : 1;
    }

    if (options.compare_version != NULL) {
        return compare_version(root, &row, options.compare_version) ? 0 : 1;
    }

    if (options.body == NULL || options.body[0] == '\0') {
        fprintf(stderr, "Missing required option: --body.\n");
        return 1;
    }

    if (field_is_too_long(options.note) || field_has_unsupported_chars(options.note)) {
        fprintf(stderr, "Optimization note contains unsupported characters.\n");
        return 1;
    }

    next_version = next_version_number(root, &row);
    format_version_number(next_version, version, sizeof(version));
    current_timestamp(timestamp, sizeof(timestamp));

    if (!versions_dir_path(versions_dir, sizeof(versions_dir), root, &row) ||
        !version_body_path(version_body_file, sizeof(version_body_file), root, &row, version) ||
        !version_metadata_path(version_metadata_file, sizeof(version_metadata_file), root, &row,
                               version) ||
        !prompt_body_path(current_body_file, sizeof(current_body_file), root, &row) ||
        !prompt_metadata_path(metadata_file, sizeof(metadata_file), root, &row) ||
        !make_dir_if_missing(versions_dir)) {
        return 1;
    }

    snprintf(version_metadata, sizeof(version_metadata),
             "version\t%s\n"
             "created_at\t%s\n"
             "promoted\t%s\n"
             "note\t%s\n",
             version, timestamp, options.promote ? "yes" : "no",
             options.note == NULL ? "" : options.note);

    if (!write_text_file(version_body_file, options.body) ||
        !write_text_file(version_metadata_file, version_metadata) ||
        !append_version_index(root, &row, version, timestamp, options.promote ? "yes" : "no",
                              options.note)) {
        return 1;
    }

    if (options.promote) {
        snprintf(row.updated_at, sizeof(row.updated_at), "%s", timestamp);
        snprintf(current_version, sizeof(current_version), "%s", version);
        if (!copy_text_file(version_body_file, current_body_file) ||
            !write_prompt_metadata(root, &row, "", current_version) ||
            !rewrite_index_row(root, &row, 0)) {
            return 1;
        }
        printf("Created and promoted optimized version: %s\n", version);
    } else {
        printf("Created optimized version: %s\n", version);
    }

    return 0;
}

static int run_delete(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char prompt_dir[PP_PATH_MAX];
    char archive_dir[PP_PATH_MAX];
    char archived_prompt_dir[PP_PATH_MAX];
    struct prompt_options options;
    struct prompt_index_row row;
    int confirmed = 0;
    int index;

    memset(&options, 0, sizeof(options));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_delete_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for --root.\n");
                return 1;
            }
            options.root = argv[++index];
        } else if (strcmp(argv[index], "--yes") == 0) {
            confirmed = 1;
        } else if (options.id_or_title == NULL) {
            options.id_or_title = argv[index];
        } else {
            fprintf(stderr, "Unexpected delete argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (options.id_or_title == NULL) {
        fprintf(stderr, "Missing prompt id or title.\n");
        return 1;
    }

    if (!confirmed) {
        fprintf(stderr, "Delete archives prompts. Re-run with --yes to confirm.\n");
        return 1;
    }

    if (options.root != NULL) {
        snprintf(root, sizeof(root), "%s", options.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !find_prompt(root, options.id_or_title, &row)) {
        fprintf(stderr, "Prompt not found: %s\n", options.id_or_title);
        return 1;
    }

    if (!prompt_dir_path(prompt_dir, sizeof(prompt_dir), root, &row) ||
        !join_path(archive_dir, sizeof(archive_dir), root, "archive") ||
        !join_path(archived_prompt_dir, sizeof(archived_prompt_dir), archive_dir, row.id)) {
        return 1;
    }

    if (path_exists_as_dir(archived_prompt_dir)) {
        fprintf(stderr, "Archived prompt already exists: %s\n", row.id);
        return 1;
    }

    if (rename(prompt_dir, archived_prompt_dir) != 0) {
        fprintf(stderr, "Could not archive prompt '%s': %s\n", row.id, strerror(errno));
        return 1;
    }

    if (!rewrite_index_row(root, &row, 1)) {
        return 1;
    }

    printf("Archived prompt: %s\n", row.id);
    return 0;
}

static int registry_name_in_use(const char *root, const char *kind, const char *name) {
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *file;

    if (!index_file_path(index_file, sizeof(index_file), root)) {
        return 1;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        return 1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        struct prompt_index_row row;
        if (parse_index_row(line, &row)) {
            if ((strcmp(kind, "folder") == 0 && strcmp(row.folder, name) == 0) ||
                (strcmp(kind, "category") == 0 && strcmp(row.category, name) == 0)) {
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}

static int update_registry_references(const char *root, const char *kind, const char *from,
                                      const char *to) {
    char index_file[PP_PATH_MAX];
    char temp_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *input;
    FILE *output;

    if (!index_file_path(index_file, sizeof(index_file), root) ||
        snprintf(temp_file, sizeof(temp_file), "%s.tmp", index_file) <= 0 ||
        strlen(temp_file) >= sizeof(temp_file)) {
        return 0;
    }

    input = fopen(index_file, "rb");
    if (input == NULL) {
        return 0;
    }

    output = fopen(temp_file, "wb");
    if (output == NULL) {
        fclose(input);
        return 0;
    }

    while (fgets(line, sizeof(line), input) != NULL) {
        char row_line[PP_BODY_MAX];
        char metadata_file[PP_PATH_MAX];
        char description[PP_FIELD_MAX] = "";
        char current_version[PP_FIELD_MAX] = "1";
        struct prompt_index_row row;
        int changed = 0;

        snprintf(row_line, sizeof(row_line), "%s", line);
        if (!parse_index_row(row_line, &row)) {
            fputs(line, output);
            continue;
        }

        if (strcmp(kind, "folder") == 0 && strcmp(row.folder, from) == 0) {
            snprintf(row.folder, sizeof(row.folder), "%s", to);
            changed = 1;
        } else if (strcmp(kind, "category") == 0 && strcmp(row.category, from) == 0) {
            snprintf(row.category, sizeof(row.category), "%s", to);
            changed = 1;
        } else {
            fputs(line, output);
            continue;
        }

        fprintf(output, "%s\t%s\t%s\t%s\t%s\t%s\n", row.id, row.title, row.folder, row.category,
                row.tags, row.updated_at);

        if (prompt_metadata_path(metadata_file, sizeof(metadata_file), root, &row)) {
            (void)read_metadata_value(metadata_file, "description", description,
                                      sizeof(description));
            (void)read_metadata_value(metadata_file, "current_version", current_version,
                                      sizeof(current_version));
        }

        if (changed && !write_prompt_metadata(root, &row, description, current_version)) {
            fclose(input);
            fclose(output);
            return 0;
        }
    }

    if (fclose(input) != 0 || fclose(output) != 0) {
        return 0;
    }

    if (!replace_file_with_temp(temp_file, index_file)) {
        fprintf(stderr, "Could not update prompt index.\n");
        return 0;
    }

    return 1;
}

static int run_registry_command(int argc, char **argv, const char *kind) {
    char root[PP_PATH_MAX];
    const char *action = NULL;
    const char *name = NULL;
    const char *to = NULL;
    const char *root_arg = NULL;
    int confirmed = 0;
    int index;

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            if (strcmp(kind, "folder") == 0) {
                print_folder_help();
            } else {
                print_category_help();
            }
            return 0;
        }

        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--to") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                root_arg = argv[++index];
            } else {
                to = argv[++index];
            }
        } else if (strcmp(argv[index], "--yes") == 0) {
            confirmed = 1;
        } else if (action == NULL) {
            action = argv[index];
        } else if (name == NULL) {
            name = argv[index];
        } else {
            fprintf(stderr, "Unexpected %s argument: %s\n", kind, argv[index]);
            return 1;
        }
    }

    if (action == NULL) {
        fprintf(stderr, "Missing %s action.\n", kind);
        return 1;
    }

    if (root_arg != NULL) {
        snprintf(root, sizeof(root), "%s", root_arg);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root)) {
        return 1;
    }

    if (strcmp(action, "list") == 0) {
        return registry_list(root, kind) ? 0 : 1;
    }

    if (name == NULL || name[0] == '\0') {
        fprintf(stderr, "Missing %s name.\n", kind);
        return 1;
    }

    if (field_is_too_long(name) || field_has_unsupported_chars(name) ||
        (strcmp(kind, "folder") == 0 && path_has_unsafe_segment(name))) {
        fprintf(stderr, "%s name contains unsupported characters.\n", kind);
        return 1;
    }

    if (strcmp(action, "create") == 0) {
        if (!registry_add(root, kind, name)) {
            return 1;
        }
        printf("Created %s: %s\n", kind, name);
        return 0;
    }

    if (strcmp(action, "remove") == 0) {
        if (!confirmed) {
            fprintf(stderr, "Re-run with --yes to remove %s '%s'.\n", kind, name);
            return 1;
        }
        if (registry_name_in_use(root, kind, name)) {
            fprintf(stderr, "Cannot remove %s '%s' while prompts still use it.\n", kind, name);
            return 1;
        }
        if (!registry_rewrite(root, kind, name, "", 1)) {
            return 1;
        }
        printf("Removed %s: %s\n", kind, name);
        return 0;
    }

    if (strcmp(action, "rename") == 0) {
        char prompts_dir[PP_PATH_MAX];
        char from_dir[PP_PATH_MAX];
        char to_dir[PP_PATH_MAX];

        if (to == NULL || to[0] == '\0') {
            fprintf(stderr, "Missing --to <name>.\n");
            return 1;
        }
        if (field_is_too_long(to) || field_has_unsupported_chars(to) ||
            (strcmp(kind, "folder") == 0 && path_has_unsafe_segment(to))) {
            fprintf(stderr, "%s name contains unsupported characters.\n", kind);
            return 1;
        }
        if (strcmp(kind, "folder") == 0) {
            if (!join_path(prompts_dir, sizeof(prompts_dir), root, "prompts") ||
                !join_path(from_dir, sizeof(from_dir), prompts_dir, name) ||
                !join_path(to_dir, sizeof(to_dir), prompts_dir, to)) {
                return 1;
            }
            if (path_exists_as_dir(from_dir) && rename(from_dir, to_dir) != 0) {
                fprintf(stderr, "Could not rename folder directory '%s': %s\n", name,
                        strerror(errno));
                return 1;
            }
        }
        if (!registry_rewrite(root, kind, name, to, 0) || !registry_add(root, kind, to) ||
            !update_registry_references(root, kind, name, to)) {
            return 1;
        }
        printf("Renamed %s: %s -> %s\n", kind, name, to);
        return 0;
    }

    fprintf(stderr, "Unknown %s action: %s\n", kind, action);
    return 1;
}

static int export_library(const char *source_root, const char *destination_root,
                          const char *folder_filter) {
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    FILE *file;
    int exported = 0;

    if (path_exists_as_dir(destination_root)) {
        fprintf(stderr, "Output directory already exists: %s\n", destination_root);
        return 0;
    }

    if (!init_library(destination_root) ||
        !index_file_path(index_file, sizeof(index_file), source_root)) {
        return 0;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read source index.\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        struct prompt_index_row row;

        if (!parse_index_row(line, &row)) {
            continue;
        }

        if (folder_filter != NULL && strcmp(row.folder, folder_filter) != 0) {
            continue;
        }

        if (!copy_prompt_payload(source_root, destination_root, &row) ||
            !append_prompt_index_row(destination_root, &row) ||
            !registry_add(destination_root, "folder", row.folder) ||
            !registry_add(destination_root, "category", row.category)) {
            fclose(file);
            return 0;
        }

        ++exported;
    }

    fclose(file);
    printf("Exported prompts: %d\n", exported);
    return 1;
}

static int run_export(int argc, char **argv) {
    char root[PP_PATH_MAX];
    const char *root_arg = NULL;
    const char *out = NULL;
    const char *folder = NULL;
    int index;

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_export_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--out") == 0 ||
            strcmp(argv[index], "--folder") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                root_arg = argv[++index];
            } else if (strcmp(argv[index], "--out") == 0) {
                out = argv[++index];
            } else {
                folder = argv[++index];
            }
        } else {
            fprintf(stderr, "Unknown export option: %s\n", argv[index]);
            return 1;
        }
    }

    if (out == NULL || out[0] == '\0') {
        fprintf(stderr, "Missing required option: --out.\n");
        return 1;
    }

    if (field_has_unsupported_chars(out) || field_has_unsupported_chars(folder)) {
        fprintf(stderr, "Export path or filter contains unsupported characters.\n");
        return 1;
    }

    if (root_arg != NULL) {
        snprintf(root, sizeof(root), "%s", root_arg);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root)) {
        return 1;
    }

    return export_library(root, out, folder) ? 0 : 1;
}

static int run_backup(int argc, char **argv) {
    char root[PP_PATH_MAX];
    const char *root_arg = NULL;
    const char *out = NULL;
    int index;

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_backup_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--out") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                root_arg = argv[++index];
            } else {
                out = argv[++index];
            }
        } else {
            fprintf(stderr, "Unknown backup option: %s\n", argv[index]);
            return 1;
        }
    }

    if (out == NULL || out[0] == '\0') {
        fprintf(stderr, "Missing required option: --out.\n");
        return 1;
    }

    if (root_arg != NULL) {
        snprintf(root, sizeof(root), "%s", root_arg);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root)) {
        return 1;
    }

    return export_library(root, out, NULL) ? 0 : 1;
}

static int run_import(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char source_index[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    const char *source = NULL;
    const char *root_arg = NULL;
    const char *conflict = "skip";
    FILE *file;
    int index;
    int imported = 0;
    int skipped = 0;

    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_import_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--on-conflict") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                root_arg = argv[++index];
            } else {
                conflict = argv[++index];
            }
        } else if (source == NULL) {
            source = argv[index];
        } else {
            fprintf(stderr, "Unexpected import argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (source == NULL || source[0] == '\0') {
        fprintf(stderr, "Missing import source.\n");
        return 1;
    }

    if (strcmp(conflict, "skip") != 0 && strcmp(conflict, "replace") != 0) {
        fprintf(stderr, "Unsupported conflict strategy: %s\n", conflict);
        return 1;
    }

    if (root_arg != NULL) {
        snprintf(root, sizeof(root), "%s", root_arg);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !validate_library_root(source) ||
        !index_file_path(source_index, sizeof(source_index), source)) {
        return 1;
    }

    file = fopen(source_index, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read import source index.\n");
        return 1;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        struct prompt_index_row row;
        struct prompt_index_row existing;
        int exists;

        if (!parse_index_row(line, &row)) {
            continue;
        }

        exists = find_prompt(root, row.id, &existing);
        if (exists && strcmp(conflict, "skip") == 0) {
            ++skipped;
            continue;
        }

        if (exists && !rewrite_index_row(root, &existing, 1)) {
            fclose(file);
            return 1;
        }

        if (!copy_prompt_payload(source, root, &row) || !append_prompt_index_row(root, &row) ||
            !registry_add(root, "folder", row.folder) ||
            !registry_add(root, "category", row.category)) {
            fclose(file);
            return 1;
        }
        ++imported;
    }

    fclose(file);
    printf("Imported prompts: %d\n", imported);
    printf("Skipped prompts: %d\n", skipped);
    return 0;
}

static int is_planned_command(const char *arg) {
    static const char *commands[] = {
        "",
    };
    size_t index;

    for (index = 0; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        if (strcmp(arg, commands[index]) == 0) {
            return 1;
        }
    }

    return 0;
}

static int run_browse(int argc, char **argv) {
    char root[PP_PATH_MAX];
    char index_file[PP_PATH_MAX];
    char line[PP_BODY_MAX];
    /* body vars used in macros */
    char *items[4096];
    int item_count;
    int index;
    int fzf_available;
    FILE *file;
    struct prompt_options filters;

    memset(&filters, 0, sizeof(filters));
    for (index = 2; index < argc; ++index) {
        if (is_help_flag(argv[index])) {
            print_browse_help();
            return 0;
        }
        if (strcmp(argv[index], "--root") == 0 || strcmp(argv[index], "--folder") == 0 ||
            strcmp(argv[index], "--category") == 0 || strcmp(argv[index], "--tag") == 0) {
            if (index + 1 >= argc) {
                fprintf(stderr, "Missing value for %s.\n", argv[index]);
                return 1;
            }
            if (strcmp(argv[index], "--root") == 0) {
                filters.root = argv[++index];
            } else if (strcmp(argv[index], "--folder") == 0) {
                filters.filter_folder = argv[++index];
            } else if (strcmp(argv[index], "--category") == 0) {
                filters.filter_category = argv[++index];
            } else {
                filters.filter_tag = argv[++index];
            }
        } else {
            fprintf(stderr, "Unexpected browse argument: %s\n", argv[index]);
            return 1;
        }
    }

    if (filters.root != NULL) {
        snprintf(root, sizeof(root), "%s", filters.root);
    } else if (!default_root(root, sizeof(root))) {
        return 1;
    }

    if (!validate_library_root(root) || !index_file_path(index_file, sizeof(index_file), root)) {
        return 1;
    }

    file = fopen(index_file, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not read index.\n");
        return 1;
    }

    item_count = 0;
    while (fgets(line, sizeof(line), file) != NULL && item_count < 4096) {
        char row_line[PP_BODY_MAX];
        struct prompt_index_row row;

        snprintf(row_line, sizeof(row_line), "%s", line);
        if (!parse_index_row(row_line, &row) || !row_matches_filters(&row, &filters)) {
            continue;
        }

        items[item_count] = malloc(strlen(row.id) + 1 + strlen(row.title) + 1);
        if (items[item_count] != NULL) {
            snprintf(items[item_count], strlen(row.id) + 1 + strlen(row.title) + 1, "%s\t%s",
                     row.id, row.title);
            ++item_count;
        }
    }
    fclose(file);

    if (item_count == 0) {
        puts("No prompts found.");
        return 0;
    }

#ifdef _WIN32
    {
        /* cmd not needed */
        char input_buf[131072];
        size_t input_len;
        int i;

        /* build input: one line per prompt */
        input_buf[0] = '\0';
        input_len = 0;
        for (i = 0; i < item_count; ++i) {
            char *tab;
            size_t needed;

            tab = strchr(items[i], '\t');
            if (tab == NULL)
                continue;

            needed = strlen(items[i]) + 1;
            if (input_len + needed >= sizeof(input_buf) - 1)
                break;

            memcpy(input_buf + input_len, items[i], needed - 1);
            input_len += needed - 1;
            input_buf[input_len] = '\n';
            ++input_len;
        }
        input_buf[input_len] = '\0';

        if (input_len == 0) {
            puts("No prompts found.");
            for (i = 0; i < item_count; ++i)
                free(items[i]);
            return 0;
        }

        /* Check if fzf is on PATH */
        fzf_available = (system("where fzf >nul 2>nul") == 0);
        if (fzf_available) {
            /* Use fzf with preview */
            char fzf_cmd[132000];
            snprintf(fzf_cmd, sizeof(fzf_cmd),
                     "fzf --preview \"echo {} | awk '{print \\$1}' | xargs -I{} %s show --root "
                     "\\\"%s\\\" {} --raw 2>nul\" --with-nth=2..",
                     argv[0], root);

            /* Temporarily write input to temp file for fzf */
            char temp_input[PP_PATH_MAX];
            if (GetTempPathA(sizeof(temp_input), temp_input) == 0) {
                fprintf(stderr, "Could not get temp path.\n");
                for (i = 0; i < item_count; ++i)
                    free(items[i]);
                return 1;
            }
            if (strlen(temp_input) + 20 >= sizeof(temp_input)) {
                for (i = 0; i < item_count; ++i)
                    free(items[i]);
                return 1;
            }
            strcat(temp_input, "promptlib_browse_XXXXXX.md");
#ifdef HAS_MKTEMP_S
            if (_mktemp_s(temp_input, strlen(temp_input) + 1) != 0) {
                for (i = 0; i < item_count; ++i)
                    free(items[i]);
                return 1;
            }
#else
            /* Fallback: use pid + time */
            {
                char pid_buf[32];
                snprintf(pid_buf, sizeof(pid_buf), "_%d_%d", (int)getpid(), (int)time(NULL));
                size_t base_len = strlen(temp_input) - 3; /* before .md */
                size_t pid_len = strlen(pid_buf);
                if (base_len + pid_len + 4 < sizeof(temp_input)) {
                    memcpy(temp_input + base_len, pid_buf, pid_len + 1);
                    strcat(temp_input, ".md");
                }
            }
#endif

            FILE *tf = fopen(temp_input, "wb");
            if (tf != NULL) {
                fwrite(input_buf, 1, input_len, tf);
                fclose(tf);
                snprintf(fzf_cmd, sizeof(fzf_cmd),
                         "fzf --preview \"%s show --root \\\"%s\\\" {1} --raw 2>nul\" < \"%s\"",
                         argv[0], root, temp_input);
                system(fzf_cmd);
                remove(temp_input);
            }
        } else {
            /* Numbered menu fallback */
            puts("Prompt Library Browser");
            puts("---------------------");
            for (i = 0; i < item_count; ++i) {
                char *tab_ptr = strchr(items[i], '\t');
                printf("%3d. %s\n", i + 1, tab_ptr != NULL ? tab_ptr + 1 : items[i]);
            }
            puts("");
            puts("Enter number to view, q to quit:");

            {
                char choice[16];

                while (1) {
                    printf("> ");
                    if (fgets(choice, sizeof(choice), stdin) == NULL)
                        break;

                    if (choice[0] == 'q' || choice[0] == 'Q')
                        break;

                    int sel = atoi(choice);
                    if (sel < 1 || sel > item_count) {
                        puts("Invalid selection.");
                        continue;
                    }

                    /* Show the selected prompt */
                    {
                        char *t = strchr(items[sel - 1], '\t');
                        if (t != NULL) {
                            *t = '\0';
                        }
                        printf("\n--- Prompt: %s ---\n", items[sel - 1]);
                        fflush(stdout);

                        char show_cmd[8192];
                        snprintf(show_cmd, sizeof(show_cmd), "\"%s\" show --root \"%s\" %s",
                                 argv[0], root, items[sel - 1]);
                        system(show_cmd);

                        printf("\nPress Enter to continue, e to edit in editor: ");
                        fflush(stdout);
                        if (fgets(choice, sizeof(choice), stdin) != NULL) {
                            if (choice[0] == 'e' || choice[0] == 'E') {
                                char edit_cmd[8192];
                                snprintf(edit_cmd, sizeof(edit_cmd), "\"%s\" edit --root \"%s\" %s",
                                         argv[0], root, items[sel - 1]);
                                system(edit_cmd);
                            }
                        }

                        if (t != NULL)
                            *t = '\t';
                    }
                }
            }
        }
    }
#else
    {
        char cmd[65536];
        size_t cmd_len;
        int i;

        /* Build pipe input */
        cmd_len = 0;
        for (i = 0; i < item_count; ++i) {
            size_t needed = strlen(items[i]) + 1;
            if (cmd_len + needed >= sizeof(cmd) - 1)
                break;
            memcpy(cmd + cmd_len, items[i], needed - 1);
            cmd_len += needed - 1;
            cmd[cmd_len] = '\n';
            ++cmd_len;
        }
        cmd[cmd_len] = '\0';

        if (cmd_len == 0) {
            puts("No prompts found.");
            for (i = 0; i < item_count; ++i)
                free(items[i]);
            return 0;
        }

        /* Check for fzf */
        fzf_available = (system("command -v fzf >/dev/null 2>&1") == 0);

        if (fzf_available) {
            /* Use fzf */
            FILE *fzf;
            char fzf_cmd[65536];

            snprintf(fzf_cmd, sizeof(fzf_cmd),
                     "fzf --with-nth=2.. --delimiter=\\'\\t\\' --preview=\"echo {} | cut -f1 | "
                     "xargs -I{} %s show --root '%s' {} --raw 2>/dev/null\"",
                     argv[0], root);

            fzf = popen(fzf_cmd, "w");
            if (fzf != NULL) {
                fwrite(cmd, 1, cmd_len, fzf);
                pclose(fzf);
            }
        } else {
            /* Numbered menu fallback */
            puts("Prompt Library Browser");
            puts("---------------------");
            for (i = 0; i < item_count; ++i) {
                char *tp = strchr(items[i], '\t');
                printf("%3d. %s\n", i + 1, tp != NULL ? tp + 1 : items[i]);
            }
            puts("");
            puts("Enter number to view, q to quit:");

            {
                char choice[16];
                while (1) {
                    printf("> ");
                    if (fgets(choice, sizeof(choice), stdin) == NULL)
                        break;
                    if (choice[0] == 'q' || choice[0] == 'Q')
                        break;

                    int sel = atoi(choice);
                    if (sel < 1 || sel > item_count) {
                        puts("Invalid selection.");
                        continue;
                    }

                    char *t = strchr(items[sel - 1], '\t');
                    if (t != NULL)
                        *t = '\0';

                    printf("\n--- Prompt: %s ---\n", items[sel - 1]);
                    fflush(stdout);

                    char show_cmd[8192];
                    snprintf(show_cmd, sizeof(show_cmd), "%s show --root '%s' %s", argv[0], root,
                             items[sel - 1]);
                    system(show_cmd);

                    printf("\nPress Enter to continue, e to edit in editor: ");
                    fflush(stdout);
                    if (fgets(choice, sizeof(choice), stdin) != NULL) {
                        if (choice[0] == 'e' || choice[0] == 'E') {
                            char edit_cmd[8192];
                            snprintf(edit_cmd, sizeof(edit_cmd), "%s edit --root '%s' %s", argv[0],
                                     root, items[sel - 1]);
                            system(edit_cmd);
                        }
                    }

                    if (t != NULL)
                        *t = '\t';
                }
            }
        }
    }
#endif

    {
        int i;
        for (i = 0; i < item_count; ++i)
            free(items[i]);
    }
    return 0;
}

int pp_cli_run(int argc, char **argv) {
    if (argc <= 1 || is_help_flag(argv[1])) {
        print_help();
        return 0;
    }

    if (is_version_flag(argv[1])) {
        print_version();
        return 0;
    }

    if (strcmp(argv[1], "init") == 0) {
        return run_init(argc, argv);
    }

    if (strcmp(argv[1], "add") == 0) {
        return run_add(argc, argv);
    }

    if (strcmp(argv[1], "list") == 0) {
        return run_list(argc, argv);
    }

    if (strcmp(argv[1], "show") == 0) {
        return run_show(argc, argv);
    }

    if (strcmp(argv[1], "search") == 0) {
        return run_search(argc, argv);
    }

    if (strcmp(argv[1], "optimize") == 0) {
        return run_optimize(argc, argv);
    }

    if (strcmp(argv[1], "folder") == 0) {
        return run_registry_command(argc, argv, "folder");
    }

    if (strcmp(argv[1], "category") == 0) {
        return run_registry_command(argc, argv, "category");
    }

    if (strcmp(argv[1], "export") == 0) {
        return run_export(argc, argv);
    }

    if (strcmp(argv[1], "import") == 0) {
        return run_import(argc, argv);
    }

    if (strcmp(argv[1], "backup") == 0) {
        return run_backup(argc, argv);
    }

    if (strcmp(argv[1], "browse") == 0) {
        return run_browse(argc, argv);
    }

    if (strcmp(argv[1], "edit") == 0) {
        return run_edit(argc, argv);
    }

    if (strcmp(argv[1], "delete") == 0) {
        return run_delete(argc, argv);
    }

    if (is_planned_command(argv[1])) {
        fprintf(stderr, "Command '%s' is planned but not implemented yet. See docs/mvp.md.\n",
                argv[1]);
        return 2;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    fprintf(stderr, "Run 'pp --help' for usage.\n");
    return 1;
}
