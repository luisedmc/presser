#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int total_bits = 0;

// HuffNode is a BTreeNode & LinkedListNode
typedef struct HuffNode_ {
  char data;
  int frequency;

  struct HuffNode_ *left;
  struct HuffNode_ *right;

  struct HuffNode_ *next;
} HuffNode;

typedef struct LinkedList_ {
  HuffNode *head;
} LinkedList;

typedef struct CompressedFile_ {
  unsigned char bit_buffer; // 8 bits
  int bit_count;            // how many bits in the buffer, when reaches 8 write
  FILE *output;             // compressed file
} CompressedFile;

// Creates a new node
HuffNode *create_node(char ch, int freq) {
  HuffNode *newNode = (HuffNode *)malloc(sizeof(HuffNode));
  newNode->data = ch;
  newNode->frequency = freq;
  newNode->left = NULL;
  newNode->right = NULL;
  newNode->next = NULL;
  return newNode;
}

// Frees the memory allocated for a Linked List
void free_list(HuffNode *head) {
  HuffNode *curr = NULL;
  while (head != NULL) {
    curr = head;
    head = head->next;
    free(curr);
  }
}

// Prints a Linked List
void print_list(HuffNode *head) {
  HuffNode *curr = head;
  while (curr != NULL) {
    printf("char: %c\t\tfreq: %d\n", curr->data, curr->frequency);
    curr = curr->next;
  }
}

// Returns the size of a Linked List
int list_size(HuffNode *head) {
  int count = 0;

  HuffNode *curr = head;
  while (curr != NULL) {
    count++;
    curr = curr->next;
  }

  return count;
}

// Checks if a node is a leaf
bool is_leaf(HuffNode *node) {
  return (node->left == NULL && node->right == NULL);
}

void print_binary(char ch) {
  for (int i = 7; i >= 0; i--)
    printf("%d", (ch >> i) & 1);
}

FILE *file_handler() {
  // Opens a file called "text.txt"
  FILE *fp = fopen("uncompressed.txt", "r");
  if (fp == NULL) {
    fputs("ERROR: Cannot open text.txt file.\n", stderr);
    return NULL;
  }
  // Calls fseek to set the file position indicator to the EOF
  if (fseek(fp, 0, SEEK_END) != 0) {
    fputs("ERROR: Seek to end of file failed.\n", stderr);
    return NULL;
  }

  /* Function ftell returns the current value of the file position indicator for
   * the stream */
  long fpi = ftell(fp);
  if (fpi == -1L) { // ftell failed
    perror("Tell");
    return NULL;
  }
  /* printf("file position = %ld\n", fpi); // bytes */

  rewind(fp);

  return fp;
}

// Sets the frequency for each char in the file
void char_frequency(int *frequency) {
  FILE *fp = file_handler();

  int c, file_position = 0;
  while ((c = fgetc(fp)) != EOF) {
    if (c != '\n') {
      frequency[c]++;
      file_position++;
    }
  }

  fclose(fp);

  printf("File position = %d\n", file_position);
}

// Inserts a node to the Linked List based on its frequency
void insert_node(LinkedList *list, HuffNode *node) {
  if (list->head == NULL) {
    list->head = node;
    return;
  }

  // First node of the list must have the lowest frequency
  if (node->frequency < list->head->frequency) {
    node->next = list->head;
    list->head = node;
    return;
  }

  // Finds the correct position
  HuffNode *curr = list->head;
  while (curr->next != NULL && curr->next->frequency <= node->frequency) {
    curr = curr->next;
  }

  // Insert node
  node->next = curr->next;
  curr->next = node;
}

// Remove the head from a Linked List
HuffNode *remove_node(LinkedList *list) {
  if (list->head == NULL)
    return NULL;

  HuffNode *tmp = list->head;
  list->head = tmp->next;
  tmp->next = NULL;

  return tmp;
}

// Creates a new node (parent) with the summed frequency of the first two
void sum_frequency(LinkedList *list) {
  // The first and second will also be used as the left and right of the parent
  HuffNode *left = remove_node(list);
  HuffNode *right = remove_node(list);

  // Parent left -> lower frequency | right -> highest frequency
  HuffNode *parent = create_node('\0', left->frequency + right->frequency);
  parent->left = left;
  parent->right = right;

  insert_node(list, parent);
}

/* Builds a Huffman Tree from the Linked List */
void build_huffman_tree(LinkedList *list) {
  while (list->head != NULL && list->head->next != NULL) {
    sum_frequency(list);
  }
}

/* Traverse a tree assigning 0s & 1s for left and right path respectively */
void encode_tree(HuffNode *root, char bitstream[], int pos, int code_length[],
                 unsigned int codes[]) {
  if (root == NULL)
    return;

  if (is_leaf(root)) {
    code_length[root->data] = pos;

    /* Convert bitstream into a number and store it in codes */
    unsigned int curr_code = 0;
    for (int i = 0; i < pos; i++) {
      curr_code = curr_code << 1;
      if (bitstream[i] == '1')
        curr_code |= 1;
    }
    codes[root->data] = curr_code;

    /* Add to total bits: path length * frequency of this character */
    total_bits += (pos * root->frequency);
  }

  /* Assign '0' for left path */
  bitstream[pos] = '0';
  encode_tree(root->left, bitstream, pos + 1, code_length, codes);

  /* Assign '1' for right path */
  bitstream[pos] = '1';
  encode_tree(root->right, bitstream, pos + 1, code_length, codes);
}

int main(void) {
  LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
  list->head = NULL;

  int frequency[256] = {0};
  int code_length[256] = {0};
  unsigned int codes[256] = {0};

  char_frequency(frequency);

  for (int i = 0; i < 256; i++) {
    if (frequency[i] > 0) {
      HuffNode *newNode = create_node(i, frequency[i]);
      insert_node(list, newNode);
    }
  }

  /* print_list(list->head); */

  char bitstream[list_size(list->head)];

  build_huffman_tree(list);
  encode_tree(list->head, bitstream, 0, code_length, codes);

  printf("\ncompressed: %d\n", total_bits);
  printf("uncompressed(8 bits): %d\n", list->head->frequency * 8);

  free_list(list->head);

  return 0;
}
