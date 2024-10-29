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

void print_levels(HuffNode *root) {
  if (root == NULL) {
    printf("Tree is empty\n");
    return;
  }

  // Create a queue to store nodes
  HuffNode **queue =
      (HuffNode **)malloc(1000 * sizeof(HuffNode *)); // Adjust size as needed
  int front = 0;
  int rear = 0;

  // Enqueue root
  queue[rear++] = root;

  // Keep track of current level
  int level = 0;
  printf("\n--- Level %d ---\n", level);

  while (front < rear) {
    // Calculate nodes in current level
    int level_size = rear - front;

    // Process all nodes of current level
    for (int i = 0; i < level_size; i++) {
      // Dequeue node and print it
      HuffNode *current = queue[front++];
      printf("Char: %c\tFreq: %d\t", current->data, current->frequency);

      // Enqueue left child
      if (current->left != NULL) {
        queue[rear++] = current->left;
      }

      // Enqueue right child
      if (current->right != NULL) {
        queue[rear++] = current->right;
      }
    }

    // Print new line after each level
    printf("\n");
    level++;
    if (front < rear) { // If there are more levels
      printf("\n--- Level %d ---\n", level);
    }
  }

  free(queue);
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
  for (int i = 0; i < 256; i++) {
    if (frequency[i] > 0) {
      printf("%c:%d|", i, frequency[i]);
    }
  }

  printf("\n");
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
  print_list(list->head);
}

// Builds a Huffman Tree from the Linked List
void build_huffman_tree(LinkedList *list) {
  while (list->head != NULL && list->head->next != NULL) {
    sum_frequency(list);
  }
  /* printf("\n\nlist: must have 1 node (which will be used as root)\n"); */
  /* print_list(list->head); */
}

// Traverse a tree assigning 0s for left path and 1s for right
void encode_tree(HuffNode *root, char bitstream[], int pos) {
  if (root == NULL)
    return;

  // Prints path from a leaf (char node)
  if (is_leaf(root)) {
    /* printf("char %c: ", root->data); */
    for (int i = 0; i < pos; i++) {
      printf("%c", bitstream[i]);
    }
    /* printf("\n"); */
    return;
  }

  // Assign '0' for left path
  bitstream[pos] = '0';
  encode_tree(root->left, bitstream, pos + 1);

  // Assign '1' for right path
  bitstream[pos] = '1';
  encode_tree(root->right, bitstream, pos + 1);
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

  char bitstream[list_size(list->head)];

  build_huffman_tree(list);
  encode_tree(list->head, bitstream, 0);
  printf("\n");

  free_list(list->head);

  return 0;
}
