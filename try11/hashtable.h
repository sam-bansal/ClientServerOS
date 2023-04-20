#define TABLE_SIZE 100

struct node
{
  char *key;
  int value;
  struct node *next;
};

struct hash_table
{
  struct node **buckets;
};

// hash function
int hash(char *key)
{
  int hash_value = 0;
  for (int i = 0; i < strlen(key); i++)
  {
    hash_value += key[i];
  }
  return hash_value % TABLE_SIZE;
}

// create a new node
struct node *create_node(char *key, int value)
{
  struct node *new_node = malloc(sizeof(struct node));
  new_node->key = strdup(key);
  new_node->value = value;
  new_node->next = NULL;
  return new_node;
}

// create a new hash table
struct hash_table *create_hash_table()
{
  struct hash_table *new_table = malloc(sizeof(struct hash_table));
  new_table->buckets = calloc(TABLE_SIZE, sizeof(struct node *));
  return new_table;
}

// insert a key-value pair into the hash table
void hash_table_insert(struct hash_table *ht, char *key, int value)
{
  int index = hash(key);
  struct node *head = ht->buckets[index];
  struct node *curr = head;
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      curr->value = value;
      return;
    }
    curr = curr->next;
  }
  struct node *new_node = create_node(key, value);
  new_node->next = head;
  ht->buckets[index] = new_node;
}

// get the value associated with a key in the hash table
int hash_table_get(struct hash_table *ht, char *key)
{
  int index = hash(key);
  struct node *curr = ht->buckets[index];
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      return curr->value;
    }
    curr = curr->next;
  }
  return -1; // key not found
}

// delete a key-value pair from the hash table
void hash_table_delete(struct hash_table *ht, char *key)
{
  int index = hash(key);
  struct node *head = ht->buckets[index];
  if (head == NULL)
  {
    return; // key not found
  }
  if (strcmp(head->key, key) == 0)
  {
    ht->buckets[index] = head->next;
    free(head->key);
    free(head);
    return;
  }
  struct node *prev = head;
  struct node *curr = head->next;
  while (curr != NULL)
  {
    if (strcmp(curr->key, key) == 0)
    {
      prev->next = curr->next;
      free(curr->key);
      free(curr);
      return;
    }
    prev = curr;
    curr = curr->next;
  }
}