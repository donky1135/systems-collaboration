typedef struct {
unsigned length;
unsigned capacity;
char **data;
} array_t;
void al_init(array_t *, unsigned);
void al_destroy(array_t *);
void al_append(array_t *, char*, unsigned);

typedef struct {
unsigned length;
unsigned capacity;
char *data;
} stringBuilder;

void sb_init(stringBuilder *, unsigned);
void sb_destroy(stringBuilder *);
void sb_append(stringBuilder *, char);