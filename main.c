#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

HuffNode *insert(HuffNode *root, char ch, int freq) {
  if (root == NULL)
    return create_node(ch, freq);

  if (freq < root->frequency)
    root->left = insert(root->left, ch, freq);
  else if (freq > root->frequency) {
    root->right = insert(root->right, ch, freq);
  }

  return root;
}

// Frees the memory allocated for a Binary Tree
void free_tree(HuffNode *root) {
  if (root != NULL) {
    free_tree(root->left);
    free_tree(root->right);
    free(root);
  }
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

// Checks if a node is a leaf
bool is_leaf(HuffNode *node) {
  return (node->left == NULL && node->right == NULL);
}

// Compare the frequency between two nodes
int compare_freq(HuffNode node1, HuffNode node2) {
  return node1.frequency - node2.frequency;
}

FILE *file_handler() {
  // Opens a file called "text.txt"
  FILE *fp = fopen("text.txt", "r");
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
  long int fpi = ftell(fp);
  if (fpi == -1L) { // Success
    perror("Tell");
    return NULL;
  }
  printf("file position = %ld\n", fpi); // bytes

  rewind(fp);

  return fp;
}

// Sets the frequency for each char in the file
void char_frequency(int *frequency) {
  FILE *fp = file_handler();

  int c;
  while ((c = fgetc(fp)) != EOF)
    frequency[c]++;

  fclose(fp);
}

// Adds a node to the Linked List based on its frequency
void insert_node(LinkedList *list, HuffNode *node) {
  if (list->head == NULL) {
    list->head = node;
    return;
  }

  // Check if inserted node->freq is < head->freq
  if (node->frequency < list->head->frequency) {
    node->next = list->head;
    list->head = node;
    return;
  }

  HuffNode *curr = list->head;
  while (curr->next != NULL && curr->next->frequency <= node->frequency) {
    curr = curr->next;
  }

  // Insert node
  node->next = curr->next;
  curr->next = node;
}

int main(void) {
  LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
  list->head = NULL;

  int frequency[256] = {0};

  char_frequency(frequency);

  for (int i = 0; i < 256; i++) {
    if (frequency[i] > 0) {
      HuffNode *newNode = create_node(i, frequency[i]);
      insert_node(list, newNode);
    }
  }

  print_list(list->head);
  free_list(list->head);

  return 0;
}
