#define KEY char*

void initializeHashTable(double);
struct node* insert(KEY, char*);
struct node* get(KEY);
bool keyExists(KEY);
