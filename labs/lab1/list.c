#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
Allocate a linked list node with a given key
Allocate a node using malloc(),
initialize the pointers to NULL,
set the key value to the key provided by the argument
 */
list_node *allocate_node_with_key(int key) {
  /* Allocate a node using malloc() */
  list_node *node;
  node = (list_node *)malloc(sizeof(list_node));
  if (node == NULL) {
    perror("Allocation failed!");
  }

  /* Setting values Properly */
  node->key = key;
  node->next = NULL;
  node->prev = NULL;

  return node;
}

/*
Initialize the key values of the head/tail list nodes (I used -1 key values)
Also, link the head and tail to point to each other
 */
void initialize_list_head_tail(list_node *head, list_node *tail) {
  /* set head and tail's key value -1 */
  head->key = -1;
  tail->key = -1;

  /* link head and tail to point each other */
  head->prev = tail;
  head->next = tail;
  tail->next = head;
  tail->prev = head;

  return;
}

/*
Insert the *new_node* after the *node*
 */
void insert_node_after(list_node *node, list_node *new_node) {
  /* Save current next_node */
  list_node *tmp_node = NULL;
  tmp_node = node->next;

  node->next = new_node;
  new_node->next = tmp_node;
  tmp_node->prev = new_node;
  new_node->prev = node;

  return;
}

/*
Remove the *node* from the list
You may assume that *node* is neither NULL nor head node, nor tail node
 */
void del_node(list_node *node) {
  list_node *prev_node = node->prev;
  list_node *next_node = node->next;
  prev_node->next = next_node;
  next_node->prev = prev_node;

  /* free node */
  node->prev = NULL;
  node->next = NULL;
  free(node);

  return;
}

/*
Search from the head to the tail (excluding both head and tail,
as they do not hold valid keys), for the node with a given *search_key*
and return the node. If the node with given key is not present, return NULL.
You may assume that the list will only hold nodes with unique key values
(No duplicate keys in the list)
 */
list_node *search_list(list_node *head, int search_key) {
  list_node *current_node = NULL;
  current_node = head->next;
  while (current_node->key != -1) {
    if (current_node->key == search_key) {
      return current_node;
    }
    current_node = current_node->next;
  }
  return NULL;
}

/*
Count the number of nodes in the list (excluding head and tail),
and return the counted value
 */
int count_list_length(list_node *head) {
  list_node *current_node = NULL;
  current_node = head->next;
  int len = 0;
  while (current_node->key != -1) {
    len++;
    current_node = current_node->next;
  }
  return len;
}

/*
Check if the list is empty (only head and tail exist in the list)
Return 1 if empty. Return 0 if list is not empty.
 */
int is_list_empty(list_node *head) {
  if (count_list_length(head) == 0)
    return 1;
  else
    return 0;
}

/*
Loop through the list and print the key values
This function will not be tested, but it will aid you in debugging your list.
You may add calls to the *iterate_print_keys* function in the test.c
at points where you need to debug your list.
But make sure to test your final version with the original test.c code.
 */
void iterate_print_keys(list_node *head) {
  if (is_list_empty(head) == 1) {
    return;
  } else {
    list_node *current_node = NULL;
    current_node = head->next;
    while (current_node->key != -1) {
      printf("%d  ", current_node->key);
      current_node = current_node->next;
    }
  }
  return;
}

/*
Insert a *new_node* at the sorted position so that the keys of the nodes of the
list (including the key of the *new_node*) is always sorted (increasing order)
 */
void insert_sorted_by_key(list_node *head, list_node *new_node) {
  /* Find the proper location for new node */
  list_node *current_node = head;
  if (current_node->next->key == -1) {
    insert_node_after(head, new_node);
    return;
  } else {
    while (current_node->next->key < new_node->key &&
           current_node->next->key != -1) {
      current_node = current_node->next;
    }
    insert_node_after(current_node, new_node);
    return;
  }
}
