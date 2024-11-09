#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256

static unsigned total_bits = 0;
static unsigned bit_count = 0;
static unsigned char bit_buffer = 0;

/* HuffNode stores a node for a BinaryTree & LinkedList */
typedef struct HuffNode {
  char data;
  unsigned frequency;
  struct HuffNode *left, *right, *next;
} HuffNode;

/* LinkedList stores the base structure for a LinkedList */
typedef struct LinkedList {
  unsigned size;
  HuffNode *head;
} LinkedList;

/* Creates a new HuffNode  */
HuffNode *create_node(char data, unsigned freq) {
  HuffNode *newNode = (HuffNode *)malloc(sizeof(HuffNode));
  newNode->data = data;
  newNode->frequency = freq;
  newNode->left = newNode->right = newNode->next = NULL;
  return newNode;
}

/* Inserts a new node in the LinkedList based on its frequency */
void insert_node(LinkedList *list, HuffNode *node) {
  if (list->head == NULL) {
    list->head = node;
    node->next = NULL;
    list->size++;
    return;
  }

  /* Frequency less than head */
  if (node->frequency < list->head->frequency) {
    node->next = list->head;
    list->head = node;
    list->size++;
    return;
  }

  /* Finds the correct position to insert */
  HuffNode *curr = list->head;
  while (curr->next != NULL && curr->next->frequency <= node->frequency)
    curr = curr->next;

  /* Insert node at the correct position */
  node->next = curr->next;
  curr->next = node;
  list->size++;
}

/* Removes the head from a LinkedList */
HuffNode *remove_head(LinkedList *list) {
  if (list->head == NULL)
    return NULL;
  HuffNode *tmp = list->head;
  list->head = tmp->next;
  tmp->next = NULL;
  return tmp;
}

/* Checks if a HuffNode is a leaf */
int is_leaf(HuffNode *node) {
  return (node->left == NULL && node->right == NULL);
}

/* Creates an empty LinkedList */
LinkedList *create_linkedlist() {
  LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
  list->size = 0;
  list->head = NULL;
  return list;
}

/* Frees the memory allocated for a LinkedList */
void free_list(HuffNode *head) {
  while (head != NULL) {
    HuffNode *curr = head;
    head = head->next;
    free(curr);
  }
}

/* Prints a LinkedList */
void print_list(LinkedList *list) {
  HuffNode *curr = list->head;
  while (curr != NULL) {
    printf("node: %c | freq: %d\n", curr->data, curr->frequency);
    curr = curr->next;
  }
}

// Sets the frequency for each char in the file
void char_frequency(FILE *fp, int *frequency) {
  int c, file_position = 0;
  while ((c = fgetc(fp)) != EOF) {
    if (c != '\n') {
      frequency[c]++;
      file_position++;
    }
  }
  printf("File position = %d\n", file_position);
}

/* Creates a frequency list from the input file */
LinkedList *frequency_list(FILE *input_file) {
  LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
  if (list == NULL)
    return NULL;
  list->head = NULL;

  int frequency[MAX_CHARS] = {0};
  char_frequency(input_file, frequency);

  for (int i = 0; i < MAX_CHARS; i++) {
    if (frequency[i] > 0) {
      HuffNode *new_node = create_node(i, frequency[i]);
      if (new_node == NULL) {
        free_list(list->head);
        free(list);
        return NULL;
      }
      insert_node(list, new_node);
    }
  }

  return list;
}

/* Builds a Huffman Tree using a Linked List */
void huffman_tree(LinkedList *list) {
  while (list->head != NULL && list->head->next != NULL) {
    /* Creates a new node (parent) with the summed frequency of the first two */
    HuffNode *left = remove_head(list), *right = remove_head(list);
    HuffNode *parent = create_node('\0', left->frequency + right->frequency);
    parent->left = left, parent->right = right;

    insert_node(list, parent);
  }
}

/* Traverse a tree assigning 0s & 1s for left and right path respectively */
void encode_tree(HuffNode *root, char bitstream[], int pos, int code_length[],
                 unsigned codes[]) {
  if (root == NULL)
    return;

  if (is_leaf(root)) {
    code_length[root->data] = pos;

    unsigned curr_code = 0;
    for (int i = 0; i < pos; i++)
      curr_code = (curr_code << 1) | (bitstream[i] == '1');
    codes[root->data] = curr_code;
    total_bits += (pos * root->frequency);
  }

  bitstream[pos] = '0';
  encode_tree(root->left, bitstream, pos + 1, code_length, codes);

  bitstream[pos] = '1';
  encode_tree(root->right, bitstream, pos + 1, code_length, codes);
}

/* Write a single bit to the output file */
void write_bit(FILE *output, int bit) {
  bit_buffer = (bit_buffer << 1) | (bit & 1);
  bit_count++;

  if (bit_count == 8) {
    fputc(bit_buffer, output);
    bit_buffer = 0;
    bit_count = 0;
  }
}

/* Write any remaining bits in the buffer */
void clear_bits(FILE *output) {
  if (bit_count > 0) {
    bit_buffer <<= (8 - bit_count);
    fputc(bit_buffer, output);
    bit_buffer = 0;
    bit_count = 0;
  }
}

/* Compress the input file using the generated codes */
void compress(FILE *input, FILE *output, const unsigned codes[],
              const int code_length[]) {
  rewind(input);
  int c;

  while ((c = fgetc(input)) != EOF) {
    if (c != '\n') {
      unsigned code = codes[c];
      int length = code_length[c];
      unsigned mask = 1 << (length - 1);

      for (int i = 0; i < length; i++) {
        write_bit(output, (code & mask) ? 1 : 0);
        mask >>= 1;
      }
    }
  }

  clear_bits(output);
}

long get_file_size(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL)
    return -1;

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fclose(fp);
  return size;
}

/* Prints the size of both compressed and uncompressed files */
void print_statistics(LinkedList *list) {
  long uncompressed = get_file_size("text.txt");
  long compressed = get_file_size("compressed.txt");

  printf("\nBits:\n");
  printf("compressed: %d\n", total_bits);
  printf("uncompressed: %d\n", list->head->frequency * 8);
  printf("\nBytes:\n");
  printf("compressed size: %ld\n", compressed);
  printf("uncompressed size: %ld\n", uncompressed);
}

int main(void) {
  FILE *input = fopen("text.txt", "r");
  FILE *output = fopen("compressed.txt", "wb");

  LinkedList *list = frequency_list(input);

  int code_length[MAX_CHARS] = {0};
  unsigned codes[MAX_CHARS] = {0};
  char bitstream[MAX_CHARS];

  huffman_tree(list);
  encode_tree(list->head, bitstream, 0, code_length, codes);
  compress(input, output, codes, code_length);
  print_statistics(list);

  // Cleanup
  free_list(list->head);
  free(list);
  fclose(input);
  fclose(output);

  return 0;
}
