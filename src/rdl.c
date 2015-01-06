#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <dlfcn.h>

const char *EVENT_TABLE = "EVENTS";

struct event {
    char *name;
    void (*func)(void);
};

struct plugin {
    char *path;
    void *dl_handle;
    size_t n_events;
    struct event *events;
    struct plugin *next, **prev;
};

static struct plugin* load_all_plugins(void);
static void system_error(char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    struct plugin *plugins;
    struct plugin *curr;
    size_t i;
    printf("Starting rdl.\n");
    plugins = load_all_plugins();
    if (plugins == NULL) {
        fprintf(stderr, "Didn't load any plugins\n");
    } else {
        for (curr = plugins; curr != NULL; curr = curr->next) {
            printf("Plugin: %s\n", curr->path);
            for (i = 0; i < curr->n_events; i++) {
                printf("  Event: %s\n", curr->events[i].name);
                if (strcmp(curr->events[i].name, "init") == 0) {
                    (curr->events[i].func)();
                }
            }
        }
    }

    printf("Exiting rdl.\n");
    return 0;
}

/* Return the directory the process exe file. Does not include
 * trailing slash. */
static char *get_exe_dir()
{
    const size_t BUF_SIZE = 4096;
    char *dir;
    char buf[BUF_SIZE];
    ssize_t result;
    char *last_slash;

    dir = NULL;
    result = 0;
    
    result = readlink("/proc/self/exe", buf, BUF_SIZE);
    if (result < 1) {
        system_error("Could not open exe link in /proc/self");
    } else if (result == BUF_SIZE) {
        fprintf(stderr, "Directory path name too long.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Fun string hax. */
    last_slash = strrchr(buf, '/');
    *last_slash = '\0';
    dir = strdup(buf);
    return dir;
}

/* Get list of .so files in a directory. */
static void get_so_files(const char *dir, glob_t *plugins)
{
    const char fpat[] = "*.so";
    char *pat;
    size_t dirlen, patlen;
    int glob_err;

    dirlen = strlen(dir);
    patlen = dirlen + sizeof(fpat) + 1;
    pat = malloc(patlen);
    snprintf(pat, patlen,  "%s/%s", dir, fpat);
    puts(pat);
    
    glob_err = glob(pat, 0, NULL, plugins);
    switch (glob_err) {
    case GLOB_ABORTED:
        system_error("Read error looking for plugins.");
        break;                  /* For consistancy */
    case GLOB_NOSPACE:
        fprintf(stderr, "Out of memory\n");
        abort();
        break;                  /* For consitancy */

    case GLOB_NOMATCH:
        fprintf(stderr, "No plugins found!\n");
        break;
    };
    return;
}

/*
 * Load events form the event table.
 */
static void load_events(struct plugin *plugin, const char **event_table)
{
    size_t table_size;
    size_t i;
    struct event *curr_event;

    for (table_size = 0; event_table[table_size] != NULL; table_size++);
    
    plugin->events = malloc(table_size * sizeof(struct event));
    if (plugin->events == 0) {
        system_error("");
    }
        
    for (i = 0; i < table_size; i++) {
        curr_event = &plugin->events[plugin->n_events];
        curr_event->name = strdup(event_table[i]);
        curr_event->func = dlsym(plugin->dl_handle, curr_event->name);
        if (curr_event->func == NULL) {
            fprintf(stderr, "Could not load event handler '%s': %s\n", curr_event->name, dlerror());
        } else {
            plugin->n_events++;
        }
    }
    plugin->events = realloc(plugin->events, plugin->n_events * sizeof(struct event));
    if (plugin->events == NULL)
        system_error(""); /* Shrinking an array should work right? */
}

static struct plugin* load_plugin(const char *path)
{
    struct plugin *plugin;
    const char **event_table;

    plugin = malloc(sizeof *plugin);
    if (plugin == NULL) {
        system_error("");
    }
    
    plugin->path = strdup(path);
    plugin->dl_handle = NULL;
    plugin->n_events = 0;
    plugin->events = NULL;
    plugin->next = NULL; plugin->prev = NULL;

    plugin->dl_handle = dlopen(path, RTLD_LAZY);
    if (plugin->dl_handle == NULL) {
        fprintf(stderr, "%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    dlerror();
    event_table = dlsym(plugin->dl_handle, EVENT_TABLE);
    if (!event_table) {
        fprintf(stderr, "Failed to load event table for: %s\n%s", path, dlerror());
        free(plugin);
    } else {
        load_events(plugin, event_table);
    }
    return plugin;
}

static struct plugin* load_all_plugins(void)
{
    char *exe_dir;
    glob_t dir_glob;
    size_t i;
    struct plugin *first, *last, *curr;
    
    exe_dir = get_exe_dir();
    get_so_files(exe_dir, &dir_glob);

    first = NULL;
    last = NULL;

    printf("Found plugins:\n");
    for (i = 0; i < dir_glob.gl_pathc; i++) {
        printf(" * %s\n", dir_glob.gl_pathv[i]);
        curr = load_plugin(dir_glob.gl_pathv[i]);
        
        if (first == NULL) {
            first = curr;
            last = curr;
        } else {
            last->next = curr;
            curr->prev = &(last->next);
            last = curr;
        }
    }

    
    globfree(&dir_glob);
    free(exe_dir);
    return first;
}
