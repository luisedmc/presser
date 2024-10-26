#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HuffNode_ {
  char data;
  int frequency;

  struct HuffNode_ *left;
  struct HuffNode_ *right;

  struct HuffNode_ *next; // to be used in the list
} HuffNode;

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

void free_tree(HuffNode *root) {
  if (root != NULL) {
    free_tree(root->left);
    free_tree(root->right);
    free(root);
  }
}

void traversal(HuffNode *root) {
  if (root != NULL) {
    printf("Char: %c\tFreq: %d\n", root->data, root->frequency);
    traversal(root->left);
    traversal(root->right);
  }
}

bool is_leaf(HuffNode *node) {
  return (node->left == NULL && node->right == NULL);
}

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

// char_frequency sets the frequency for each char in the file
void char_frequency(int *freq) {
  FILE *fp = file_handler();

  int c;
  while ((c = fgetc(fp)) != EOF)
    freq[c]++;

  fclose(fp);
}

int main(void) {
  HuffNode *root = NULL;
  int freq[256] = {0};

  char_frequency(freq);

  for (int i = 0; i < 256; i++) {
    if (freq[i] > 0) {
      if (root == NULL)
        root = create_node(i, freq[i]);
      else
        root = insert(root, i, freq[i]);
    }
  }

  traversal(root);

  printf("root: %c\t%d\n", root->data, root->frequency);

  free(root);

  return 0;
}
