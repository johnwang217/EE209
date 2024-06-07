/*
Name: Wang Jonghyuk
Student ID: 20220425
Description: Source code to implement customer information management
             database using dynamic resizable array.
*/

#define _GNU_SOURCE 1

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

/*--------------------------------------------------------------------*/

#define UNIT_ARRAY_SIZE 1024

//customer information
struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
};

//customer database
struct DB {
  struct UserInfo *pArray;   // pointer to the array
  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // # whether the array should be expanded
			     // # or not
};

/*--------------------------------------------------------------------*/

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
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 1024 elements
  d->pArray = 
    (struct UserInfo *) malloc (d->curArrSize*sizeof(struct UserInfo));
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
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
  for (int i = 0; i < d->numItems; i++) {
    free((d->pArray)[i].id);
    free((d->pArray)[i].name);
  }
  free(d->pArray);
  free(d);
  return;
  //assert(0);
}
/*--------------------------------------------------------------------*/
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
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp((d->pArray)[i].id, id) == 0 ||
        strcmp((d->pArray)[i].name, name) == 0) {
      fprintf(stderr, "Customer with same ID: %s or name: %s exists",
        id, name);
      return -1;
    }
  }
  if (d->curArrSize == d->numItems) {
    d->curArrSize += UNIT_ARRAY_SIZE;
    struct UserInfo* temp = realloc(d->pArray,
      d->curArrSize*sizeof(struct UserInfo));
    if (temp == NULL) {
      fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize); 
      return -1;
    }
    else d->pArray = temp;
  }
  struct UserInfo new = {strdup(name), strdup(id), purchase};
  (d->pArray)[d->numItems] = new;
  d->numItems++;
  return 0;
}

/*--------------------------------------------------------------------*/
//in the array, shifts all customer from 'index' to end by one 
void ScootOver(DB_T d, int index)
{
  assert(index < d->numItems);
  int i;
  for (i = index; i < d->numItems-1; i++) {
    (d->pArray)[i] = (d->pArray)[i+1];
  }
  return;
}

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
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp((d->pArray)[i].id, id) == 0) {
        free((d->pArray)[i].id);
        free((d->pArray)[i].name);
        ScootOver(d, i);
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
  for (int i = 0; i < d->numItems; i++) {
    if (!strcmp((d->pArray)[i].name, name)) {
        free((d->pArray)[i].id);
        free((d->pArray)[i].name);
        ScootOver(d, i);
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
  for (int i = 0; i < d->numItems; i++) {
    if (strcmp((d->pArray)[i].id, id) == 0) {
      return (d->pArray)[i].purchase;
    }
  }
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
  for (int i = 0; i < d->numItems; i++) {
    if (!strcmp((d->pArray)[i].name, name)){
      return (d->pArray)[i].purchase;
    }
  }
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
  for (int i = 0; i < d->numItems; i++) {
    sum += fp((d->pArray)[i].id, (d->pArray)[i].name,
      (d->pArray)[i].purchase);
  }
  return sum;
}