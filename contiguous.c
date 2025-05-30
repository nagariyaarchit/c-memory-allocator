#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "contiguous.h"
#include <string.h>

struct contiguous {
  struct cnode *first;
  void *upper_limit;
};

struct cnode {
  size_t nsize;
  struct cnode *prev;
  struct cnode *next;
  struct contiguous *block;
};

const int SIZEOF_CONTIGUOUS = sizeof(struct contiguous);
const int SIZEOF_CNODE = sizeof(struct cnode);



static const char STAR_STR[] = "*";
static const char NULL_STR[] = "NULL";

// maybe_null(void *p) return a pointer to "NULL" or "*",
//   indicating if p is NULL or not.
static const char *maybe_null(void *p) {
  return p ? STAR_STR : NULL_STR;
}

// gapsize(n0, n1) determine the size (in bytes) of the gap between n0 and n1.
static size_t gapsize(struct cnode *n0, struct cnode *n1) {
  assert(n0);
  assert(n1);
  void *v0 = n0;
  void *v1 = n1;
  return (v1 - v0) - n0->nsize - sizeof(struct cnode);
}

// print_gapsize(n0, n1) print the size of the gap between n0 and n1,
//     if it's non-zero.
static void print_gapsize(struct cnode *n0, struct cnode *n1) {
  assert(n0);
  assert(n1);
  size_t gap = gapsize(n0, n1);
  
  if (gap != 0) { 
    printf("%zd byte gap\n", gap);
  }
}


// pretty_print_block(chs, size) Print size bytes, starting at chs,
//    in a human-readable format: printable characters other than backslash
//    are printed directly; other characters are escaped as \xXX
static void pretty_print_block(unsigned char *chs, int size) {
  assert(chs);
  for (int i = 0; i < size; i++) {
    printf(0x20 <= chs[i] && chs[i] < 0x80 && chs[i] != '\\'
           ? "%c" : "\\x%02X", chs[i]);
  }
  printf("\n");
}

// print_node(node) Print the contents of node and all nodes that
//    follow it.  Return a pointer to the last node.
static struct cnode *print_node(struct cnode *node) {
  while (node != NULL) {
    void *raw = node + 1;     // point at raw data that follows.
    printf("struct cnode\n");
    printf("    nsize: %ld\n", node->nsize);
    printf("    prev: %s\n", maybe_null(node->prev));
    printf("    next: %s\n",  maybe_null(node->next));

    printf("%zd byte chunk: ", node->nsize);
    
    pretty_print_block(raw, node->nsize);
    
    if (node->next == NULL) {
      return node;
    } else {
      print_gapsize(node, node->next);
      node = node->next;
    }
  }
  return NULL;
}



static void print_hr(void) {
    printf("----------------------------------------------------------------\n");
}

// print_debug(block) print a long message showing the content of block.
void print_debug(struct contiguous *block) {
  assert(block);
  void *raw = block;

  print_hr();
  printf("struct contiguous\n");
  printf("    first: %s\n", maybe_null(block->first));

  if (block->first == NULL) {
    size_t gap = block->upper_limit - raw - sizeof(struct contiguous);
    printf("%zd byte gap\n", gap);           
  } else {
    void *block_first = block->first;
    size_t gap = block_first - raw - sizeof(struct contiguous);
    if (gap) {
      printf("%zd byte gap\n", gap);
    }
  }
 
  struct cnode *lastnode = print_node(block->first);
  
  if (lastnode != NULL) {
    print_gapsize(lastnode, block->upper_limit);
  }

  print_hr();
}



struct contiguous *make_contiguous(size_t size) {
  assert(size >= sizeof(struct contiguous));
  void *memchad  = malloc(size);
  char *temp = memchad;
  struct contiguous *x = memchad;
  x->first = NULL;
  x->upper_limit = temp + size;
  
  temp = temp + sizeof(struct contiguous);
  for (int i = 0; i < size - sizeof(struct contiguous); i++) {
    *temp = '$';
    temp += 1;
  }
  return x;
}


void destroy_contiguous(struct contiguous *block) {
  assert(block);
  if (block->first != NULL) {
    printf("Destroying non-empty block!\n");
  }
  free(block);
}


void cfree(void *p) {
  assert(p);
  char *given = p;
  char *address_given = given - sizeof(struct cnode);
  void *address = address_given;
  struct cnode *node = address;
  struct cnode *prev = node->prev;
  struct cnode *next = node->next;
  struct contiguous *block = node->block;

  if (prev != NULL) {
    prev->next = next;
  } else {
    block->first = next; 
  }

  if (next != NULL) {
    next->prev = prev;
  }
}


void *cmalloc(struct contiguous *block, int size) {
  assert(block);
  size_t needed = sizeof(struct cnode) + size; 
  void *b = block;
  char *conv_b = b;
  void *total_space = block->upper_limit;
  char *upper_limit = total_space;
  char *start_space = conv_b + sizeof(struct contiguous);
  int gap_first_case = upper_limit - (start_space);

  if (block->first == NULL) {

    if (gap_first_case >= needed) {
      void *start = start_space;
      struct cnode *c = start;
      c->nsize = size;
      c->prev = NULL;
      c->next = NULL;
      c->block = block;
      block->first = c;
      char *chunk = start_space;
      chunk = chunk + sizeof(struct cnode);
      return chunk;
  
    } else {
      return NULL;
    }

  } 
  struct cnode *first = block->first;
  void *first_node = first;
  char *conv_first_node = first_node;
  int gap_second_case = conv_first_node - start_space;

  if (gap_second_case >= needed) {
    void *start = start_space;
    struct cnode *c = start;
    c->nsize = size;
    c->prev = NULL;
    c->next = block->first;
    c->block = block;
    block->first->prev = c;
    block->first = c;
    char *chunk = start_space;
    chunk += sizeof(struct cnode);
    return chunk;
  
  } 
  struct cnode *node = block->first;
  while (node->next != NULL) {

    if (gapsize(node, node->next) >= needed) {
      void *conv_node = node;
      char *convert_node = conv_node;
      convert_node = convert_node + node->nsize + sizeof(struct cnode);
      void *another = convert_node;
      struct cnode *new_node = another;
      new_node->nsize = size;
      new_node->prev = node;
      new_node->next = node->next;
      new_node->block = block;
      node->next->prev = new_node;
      node->next = new_node;
      char *chunk = convert_node;
      chunk += sizeof(struct cnode);
      return chunk;
    }
    node = node->next;
  }
  void *c_node = node;
  char *co_node = c_node;
  co_node = co_node + node->nsize + sizeof(struct cnode);
  int last_gap = upper_limit - co_node;

  if (last_gap >= needed) {
    void *conv_node = co_node;
    struct cnode *new_node = conv_node;
    new_node->nsize = size;
    new_node->prev = node;
    new_node->next = NULL;
    new_node->block = block;
    node->next = new_node;
    char *chunk = conv_node;
    chunk += sizeof(struct cnode);
    return chunk;
  }
  return NULL;
}

