#include <stdio.h>
#include "list.c"

int main(void) {
  list_node *head = allocate_node_with_key(-1);
  list_node *tail = allocate_node_with_key(-1);
  list_node *tmp1 = allocate_node_with_key(1);
  list_node *tmp2 = allocate_node_with_key(2);
  list_node *tmp3 = allocate_node_with_key(3);
  list_node *tmp4 = allocate_node_with_key(4);
  list_node *tmp5 = allocate_node_with_key(5);
  initialize_list_head_tail(head, tail);
  insert_sorted_by_key(head, tmp1);
  insert_sorted_by_key(head, tmp2);
  insert_sorted_by_key(head, tmp3);
  insert_sorted_by_key(head, tmp4);
  insert_sorted_by_key(head, tmp5);
  iterate_print_keys(head);
}