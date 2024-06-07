/*
Name: Wang Jonghyuk
Student ID: 20220425
Description: Source code to implement customer information management
             database using hash tables.
*/

#define _GNU_SOURCE 1

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "murmurhash.h"
#include "customer_manager.h"

/*--------------------------------------------------------------------*/

#define UNIT_TABLE_SIZE 1024

//customer information
struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  struct UserInfo* idnext;   // pointer to next customer info
  struct UserInfo* namenext;
  int purchase;              // purchase amount (> 0)
};

//customer database
struct DB {
  struct UserInfo **idTable;   // pointer to hash table
  struct UserInfo **nameTable;
  int curTabSize;            // current table size (max # of buckets)
  int numItems;              // # of stored items, needed to determine
			     // # whether the table should be expanded
			     // # or not
};

/*--------------------------------------------------------------------*/
//uses murmurhash algorithm to return hash from input 'key'
uint32_t
murmurhash (const char *key, uint32_t len, uint32_t seed) {
  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;
  uint32_t r1 = 15;
  uint32_t r2 = 13;
  uint32_t m = 5;
  uint32_t n = 0xe6546b64;
  uint32_t h = 0;
  uint32_t k = 0;
  uint8_t *d = (uint8_t *) key; // 32 bit extract from `key'
  const uint32_t *chunks = NULL;
  const uint8_t *tail = NULL; // tail - last 8 bytes
  int i = 0;
  int l = len / 4; // chunk length

  h = seed;

  chunks = (const uint32_t *) (d + l * 4); // body
  tail = (const uint8_t *) (d + l * 4); // last 8 byte chunk of `key'

  // for each 4 byte chunk of `key'
  for (i = -l; i != 0; ++i) {
    // next 4 byte chunk of `key'
    k = chunks[i];

    // encode next 4 byte chunk of `key'
    k *= c1;
    k = (k << r1) | (k >> (32 - r1));
    k *= c2;

    // append to hash
    h ^= k;
    h = (h << r2) | (h >> (32 - r2));
    h = h * m + n;
  }

  k = 0;

  // remainder
  switch (len & 3) { // `len % 4'
    case 3: k ^= (tail[2] << 16);
    case 2: k ^= (tail[1] << 8);

    case 1:
      k ^= tail[0];
      k *= c1;
      k = (k << r1) | (k >> (32 - r1));
      k *= c2;
      h ^= k;
  }

  h ^= len;

  h ^= (h >> 16);
  h *= 0x85ebca6b;
  h ^= (h >> 13);
  h *= 0xc2b2ae35;
  h ^= (h >> 16);

  return h;
}

//performs a modulo arithmatic on 'uiHash' with 'iBucketCount'
static int hash_function(uint32_t uiHash, int iBucketCount)
{
  return (int)(uiHash % (unsigned int)iBucketCount);
}

/*--------------------------------------------------------------------*/
//create then return a db structure
DB_T
CreateCustomerDB(void)
{
  /* Uncomment and use the following implementation if you want */
  DB_T d;
  
  d = (DB_T) calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curTabSize = UNIT_TABLE_SIZE; // start with 1024 
  d->idTable = (struct UserInfo **) calloc(d->curTabSize,
                sizeof(struct UserInfo*));
  d->nameTable = (struct UserInfo **) calloc(d->curTabSize,
                  sizeof(struct UserInfo*));
  if (d->idTable == NULL || d->nameTable == NULL) {
    fprintf(stderr, "Can't allocate a memory for hash table of" 
    " size %d\n", d->curTabSize);   
    free(d);
    return NULL;
  }
  return d;
}
/*--------------------------------------------------------------------*/
//destroy db and free all allocated memory
void
DestroyCustomerDB(DB_T d)
{
  /* fill out this function */
  if (d == NULL) {
    fprintf(stderr, "DB_T is NULL\n");
    return;
  }
  for (int i = 0; i < d->curTabSize; i++) {
    for (struct UserInfo *p = (d->idTable)[i]; p != NULL; ) {
      struct UserInfo *next = p->idnext;
      free(p->id);
      free(p->name);
      free(p);
      p = next;
    }
  }
  free(d->idTable);
  free(d->nameTable);
  free(d);
  return;
  //assert(0);
}
/*--------------------------------------------------------------------*/

#define MAX_TABLE_SIZE 1048576

//register a customer in db with info (id, name, purchase)
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || id == NULL || name == NULL) {
    fprintf(stderr, "DB_T, input ID or name is NULL\n");
    return -1;
  }
  if (purchase <= 0) {
    fprintf(stderr, "Input value of purchase is zero or negative\n");
    return -1;
  }

  // Dynamically resizing hash table
  if ((d->curTabSize != MAX_TABLE_SIZE) && 
    (d->numItems >= (int)(0.75*(d->curTabSize)))) 
  { 
    d->curTabSize *= 2;
    struct UserInfo **idTemp = realloc(d->idTable,
      d->curTabSize*sizeof(struct UserInfo*));
    if (idTemp == NULL) {
      fprintf(stderr, "Can't allocate a memory for hash table of"
        "size %d\n", d->curTabSize);
      d->curTabSize /= 2;
      return -1;
    }
    struct UserInfo **nameTemp = realloc(d->nameTable,
      d->curTabSize*sizeof(struct UserInfo*));
    if (nameTemp == NULL) {
      fprintf(stderr, "Can't allocate a memory for hash table of"
        "size %d\n", d->curTabSize);
      d->curTabSize /= 2;
      return -1;
    }

    d->idTable = idTemp;
    d->nameTable = nameTemp;
    memset(d->idTable + (d->curTabSize/2), 0,
      sizeof(struct UserInfo*)*(d->curTabSize/2));
    memset(d->nameTable + (d->curTabSize/2), 0,
      sizeof(struct UserInfo*)*(d->curTabSize/2));

    //reallocating items by their new hash key
    for (int i = 0; i < d->curTabSize/2; i++) 
    {
      int newHash;
      struct UserInfo *temp;
      for (struct UserInfo **pp = &(d->idTable)[i]; *pp != NULL; ) {
        newHash = hash_function(murmurhash((*pp)->id,
          strlen((*pp)->id), 0), d->curTabSize);
        if (newHash != i) {
          temp = (*pp)->idnext;
          (*pp)->idnext = (d->idTable)[newHash];
          (d->idTable)[newHash] = *pp;
          *pp = temp;
        }
        else pp = &((*pp)->idnext);
      }
      for (struct UserInfo **pp = &(d->nameTable)[i]; *pp != NULL; ) {
        newHash = hash_function(murmurhash((*pp)->name,
          strlen((*pp)->name), 0), d->curTabSize);
        if (newHash != i) {
          temp = (*pp)->namenext;
          (*pp)->namenext = (d->nameTable)[newHash];
          (d->nameTable)[newHash] = *pp;
          *pp = temp;
        }
        else pp = &((*pp)->namenext);
      }
    }
  }

  int idHash = hash_function(murmurhash(id, strlen(id), 0),
    d->curTabSize);
  int nameHash = hash_function(murmurhash(name, strlen(name), 0),
    d->curTabSize);
  for (struct UserInfo* p = (d->idTable)[idHash]; p != NULL; 
        p = p->idnext) {
    if (strcmp(p->id, id) == 0) {
      fprintf(stderr, "Customer with same ID: %s exists\n", id);
      return -1;
    }
  }
  for (struct UserInfo* p = (d->nameTable)[nameHash]; p != NULL; 
        p = p->namenext) {
    if (strcmp(p->name, name) == 0) {
      fprintf(stderr, "Customer with same name: %s exists\n", name);
      return -1;
    }
  }
  struct UserInfo* new = (struct UserInfo*)
                          malloc(sizeof(struct UserInfo));
  new->id = strdup(id);
  new->name = strdup(name);
  new->purchase = purchase;
  new->idnext = (d->idTable)[idHash];
  (d->idTable)[idHash] = new;
  new->namenext = (d->nameTable)[nameHash];
  (d->nameTable)[nameHash] = new;
  d->numItems++;
  return 0;
}

/*--------------------------------------------------------------------*/
//unregister a customer from db whose id is: 'id'
int
UnregisterCustomerByID(DB_T d, const char *id)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || id == NULL) {
    fprintf(stderr, "DB_T or input ID is NULL\n");
    return -1;
  }
  int idHash = hash_function(murmurhash(id, strlen(id), 0),
    d->curTabSize);
  for (struct UserInfo **pp = &(d->idTable)[idHash]; *pp != NULL;
        pp = &((*pp)->idnext)) {
    if (strcmp((*pp)->id, id) == 0) {
      struct UserInfo *temp = (*pp)->idnext;
      int nameHash = hash_function(murmurhash((*pp)->name,
        strlen((*pp)->name), 0), d->curTabSize);
      for (struct UserInfo **ppname = &(d->nameTable)[nameHash];
        *ppname != NULL; ppname = &((*ppname)->namenext)) {
        if (strcmp((*ppname)->name, (*pp)->name) == 0) {
          *ppname = (*ppname)->namenext;
          break;
        }
      }
      free((*pp)->id);
      free((*pp)->name);
      free(*pp);
      *pp = temp;
      d->numItems--;
      return 0;
    }
  }
  fprintf(stderr, "No customer with ID: %s\n", id);
  return -1;
}

/*--------------------------------------------------------------------*/
//unregister a customer from db whose name is: 'name'
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || name == NULL) {
    fprintf(stderr, "DB_T or input name is NULL\n");
    return -1;
  }
  int nameHash = hash_function(murmurhash(name, strlen(name), 0),
    d->curTabSize);
  for (struct UserInfo **pp = &(d->nameTable)[nameHash]; *pp != NULL;
        pp = &((*pp)->namenext)) {
    if (strcmp((*pp)->name, name) == 0) {
      struct UserInfo *temp = (*pp)->namenext;
      int idHash = hash_function(murmurhash((*pp)->id,
        strlen((*pp)->id), 0), d->curTabSize);
      for (struct UserInfo **ppid = &(d->idTable)[idHash];
        *ppid != NULL; ppid = &((*ppid)->idnext)) {
        if (strcmp((*ppid)->id, (*pp)->id) == 0) {
          *ppid = (*ppid)->idnext;
          break;
        }
      }
      free((*pp)->id);
      free((*pp)->name);
      free(*pp);
      *pp = temp;
      d->numItems--;
      return 0;
    }
  }
  fprintf(stderr, "No customer with name: %s\n", name);
  return -1;
}
/*--------------------------------------------------------------------*/
//gets the value of purchase amount of a customer whose id is: 'id'
int
GetPurchaseByID(DB_T d, const char* id)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || id == NULL) {
    fprintf(stderr, "DB_T or input ID is NULL\n");
    return -1;
  }
  int idHash = hash_function(murmurhash(id, strlen(id), 0),
    d->curTabSize);
  for (struct UserInfo *p = (d->idTable)[idHash]; p != NULL;
      p = p->idnext)
    if (strcmp(p->id, id) == 0) return p->purchase;
  fprintf(stderr, "No customer with ID: %s\n", id);
  return -1;
}
/*--------------------------------------------------------------------*/
//gets the value of purchase amount of a customer whose name is: 'name'
int
GetPurchaseByName(DB_T d, const char* name)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || name == NULL) {
    fprintf(stderr, "DB_T or input name is NULL\n");
    return -1;
  }
  int nameHash = hash_function(murmurhash(name, strlen(name), 0),
    d->curTabSize);
  for (struct UserInfo *p = (d->nameTable)[nameHash]; p != NULL;
      p = p->namenext)
    if (strcmp(p->name, name) == 0) return p->purchase;
  fprintf(stderr, "No customer with name: %s\n", name);
  return -1;
}
/*--------------------------------------------------------------------*/
/* evaluate fp for each customer in the db
   and return the sum of all fp function calls */
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  /* fill out this function */
  //assert(0);
  if (d == NULL || fp == NULL) {
    fprintf(stderr, "DB_T or function pointer is NULL");
    return -1;
  }
  int sum = 0;
  for (int i = 0; i < d->curTabSize; i++) {
    for (struct UserInfo *p = (d->idTable)[i]; p != NULL; p = p->idnext)
      sum += fp(p->id, p->name, p->purchase);
  }
  return sum;
}
