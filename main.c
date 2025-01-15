#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHARS 256
#define LEFT(i) (2 * i + 1)
#define RIGHT(i) (2 * i + 2)
#define PARENT(i) ((i - 1) / 2)

static unsigned total_bits = 0;
static unsigned bit_count = 0;
static unsigned char bit_buffer = 0;

/* HuffNode stores a node for a BinaryTree & Heap */
typedef struct HuffNode {
  char data;
  unsigned frequency;
  struct HuffNode *left, *right, *next;
} HuffNode;

typedef struct Heap {
  HuffNode **array;
  unsigned capacity;
  unsigned size;
} Heap;

void print_statistics(Heap *heap, const char *input_filename,
                      const char *output_filename);

void swap(HuffNode **a, HuffNode **b) {
  HuffNode *tmp = *a;
  *a = *b;
  *b = tmp;
}

Heap *init_heap(unsigned capacity) {
  Heap *heap = (Heap *)malloc(sizeof(Heap));
  heap->array = (HuffNode **)malloc(capacity * sizeof(HuffNode *));
  heap->capacity = capacity;
  heap->size = 0;
  return heap;
}

void min_heapify(Heap *heap, int i) {
  int smallest = i;
  int l = LEFT(i), r = RIGHT(i);

  if (l < heap->size &&
      heap->array[l]->frequency < heap->array[smallest]->frequency)
    smallest = l;

  if (r < heap->size &&
      heap->array[r]->frequency < heap->array[smallest]->frequency)
    smallest = r;

  if (smallest != i) {
    swap(&heap->array[i], &heap->array[smallest]);
    min_heapify(heap, smallest);
  }
}

HuffNode *extract_min(Heap *heap) {
  if (heap->size == 0)
    return NULL;

  HuffNode *min = heap->array[0];
  heap->array[0] = heap->array[heap->size - 1];
  heap->size--;
  min_heapify(heap, 0);

  return min;
}

void insert_heap(Heap *heap, HuffNode *node) {
  if (heap->size >= heap->capacity)
    return;

  /* Insert in the end */
  heap->array[heap->size] = node;
  int i = heap->size;
  heap->size++;

  /* Heapify up */
  while (i > 0 &&
         heap->array[PARENT(i)]->frequency > heap->array[i]->frequency) {
    swap(&heap->array[PARENT(i)], &heap->array[i]);
    i = PARENT(i);
  }
}

void free_heap(Heap *heap) {
  if (heap == NULL)
    return;

  free(heap->array);
  free(heap);
}

/* Creates a new HuffNode  */
HuffNode *create_node(char data, unsigned freq) {
  HuffNode *newNode = (HuffNode *)malloc(sizeof(HuffNode));
  newNode->data = data;
  newNode->frequency = freq;
  newNode->left = newNode->right = newNode->next = NULL;
  return newNode;
}

/* Checks if a HuffNode is a leaf */
int is_leaf(HuffNode *node) {
  return (node->left == NULL && node->right == NULL);
}

// Sets the frequency for each char in the file
void char_frequency(FILE *fp, int *frequency) {
  int c, file_position = 0;
  while ((c = fgetc(fp)) != EOF) {
    frequency[c]++;
    file_position++;
  }
  printf("File position = %d\n", file_position);
}

/* Creates a frequency list from the input file */
Heap *frequency_list(FILE *input_file) {
  Heap *heap = init_heap(MAX_CHARS);
  if (heap == NULL)
    return NULL;

  int frequency[MAX_CHARS] = {0};
  char_frequency(input_file, frequency);

  /* int total_chars = 0; */
  for (int i = 0; i < MAX_CHARS; i++) {
    if (frequency[i] > 0) {
      /* printf("char '%c'\tascii: %d\tfrequency: %d\n", i, i, frequency[i]); */
      /* total_chars++; */
      HuffNode *node = create_node(i, frequency[i]);
      if (node == NULL) {
        free_heap(heap);
        return NULL;
      }
      insert_heap(heap, node);
    }
  }

  /* printf("unique characters: %d\n", total_chars); */
  /* printf("heap size: %d\n", heap->size); */

  return heap;
}

void build_huffman_tree(Heap *heap) {
  /* printf("build_huffman_tree heap size: %d\n", heap->size); */

  while (heap->size > 1) {
    HuffNode *left = extract_min(heap);
    HuffNode *right = extract_min(heap);

    HuffNode *parent = create_node('\0', left->frequency + right->frequency);
    parent->left = left;
    parent->right = right;

    insert_heap(heap, parent);
    /* printf("Heap size after step %d: %d\n", step, heap->size); */
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

    /* printf("char '%c' freq: %u\n", root->data, root->frequency); */

    for (int i = 0; i < pos; i++)
      curr_code = (curr_code << 1) | (bitstream[i] == '1');
    /* printf("%c", bitstream[i]); // print the bits */

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
    if (fputc(bit_buffer, output) == EOF)
      printf("Error writing byte to output file\n");
    bit_buffer = 0;
    bit_count = 0;
  }
}

/* Read a single bit from a file */
int read_bit(FILE *input, unsigned char *bit) {
  static unsigned char buffer; // 1 byte
  static int bits_remaining =
      0; // Tracks how many bits left to process in the current buffer

  if (bits_remaining == 0) {
    if (fread(&buffer, 1, 1, input) != 1)
      return 0;         // EOF or error
    bits_remaining = 8; // 8 new bits to process
  }

  bits_remaining--;
  *bit = (buffer >> bits_remaining) &
         1; // AND with 1 to extract the least significant bit
  return 1;
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

/* Write the compression header with character frequencies */
void write_header(FILE *output, Heap *heap) {
  /* Write number of distinct characters */
  unsigned char num_chars = heap->size;
  fwrite(&num_chars, sizeof(unsigned char), 1, output);

  /* Write each character and its frequency */
  for (int i = 0; i < heap->size; i++)
    if (is_leaf(heap->array[i])) {
      fwrite(&heap->array[i]->data, sizeof(char), 1, output);
      fwrite(&heap->array[i]->frequency, sizeof(unsigned), 1, output);
    }
}

/* Compress the input file using the generated codes */
int compress(const char *input_filename, const char *output_filename) {
  FILE *input = fopen(input_filename, "r");
  if (input == NULL) {
    printf("Error: Could not open input file %s\n", input_filename);
    return -1;
  }

  FILE *output = fopen(output_filename, "wb");
  if (output == NULL) {
    printf("Error: Could not open output file %s\n", output_filename);
    fclose(input);
    return -1;
  }

  printf("Compressing...\n");
  // Initialize the frequency list
  Heap *heap = frequency_list(input);
  if (heap == NULL) {
    fclose(input);
    fclose(output);
    return -1;
  }

  write_header(output, heap);

  // Initialize arrays for encoding
  int code_length[MAX_CHARS] = {0};
  unsigned codes[MAX_CHARS] = {0};
  char bitstream[MAX_CHARS];

  // Build Huffman tree and encode
  build_huffman_tree(heap);
  encode_tree(heap->array[0], bitstream, 0, code_length, codes);

  // Write compressed data
  rewind(input);
  int c;
  while ((c = fgetc(input)) != EOF) {
    unsigned code = codes[c];
    int length = code_length[c];
    unsigned mask = 1 << (length - 1);

    for (int i = 0; i < length; i++) {
      write_bit(output, (code & mask) ? 1 : 0);
      mask >>= 1;
    }
  }
  clear_bits(output);

  print_statistics(heap, input_filename, output_filename);

  free_heap(heap);
  fclose(input);
  fclose(output);
  printf("INFO: Successfully compressed\n");

  return 0;
}

/* Function to handle the decompression */
int decompress(const char *input_filename, const char *output_filename) {
  FILE *input = fopen(input_filename, "rb");
  FILE *output = fopen(output_filename, "w");
  unsigned char num_chars;

  if (fread(&num_chars, sizeof(unsigned char), 1, input) != 1) {
    printf("ERROR: reading input file header.");
    return -1;
  }

  printf("Decompressing...\n");
  Heap *heap = init_heap(2 * num_chars - 1);

  for (int i = 0; i < num_chars; i++) {
    char character;
    unsigned frequency;

    if (fread(&character, sizeof(char), 1, input) != 1 ||
        fread(&frequency, sizeof(unsigned), 1, input) != 1) {
      printf("ERROR: reading char and frequency failed.");
      free_heap(heap);
      fclose(input);
      fclose(output);
      return -1;
    }

    HuffNode *node = create_node(character, frequency);
    insert_heap(heap, node);
  }

  build_huffman_tree(heap);
  HuffNode *root = heap->array[0];
  HuffNode *curr = root;

  /* Read compressed data bit by bit */
  unsigned char bit;
  while (read_bit(input, &bit)) {
    /* If bit equals 0, go left, if 1, go right */
    curr = (bit == 0) ? curr->left : curr->right;

    if (is_leaf(curr)) {
      fputc(curr->data, output); // Write char to output
      curr = root;               // Go back to root for next char
    }
  }

  free_heap(heap);
  fclose(input);
  fclose(output);
  printf("INFO: Successfully decompressed\n");

  return 0;
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

void print_usage(const char *program) {
  printf("Compile: gcc main.c -o %s\n", program);
  printf("Usage: %s [option] [input_file] [output_file]\n", program);
  printf("Options:\n");
  printf("  -c    Compress the input file\n");
  printf("  -d    Decompress the input file\n");
}

/* Prints the size of both compressed and uncompressed files */
void print_statistics(Heap *heap, const char *input_filename,
                      const char *output_filename) {
  long uncompressed = get_file_size(input_filename);
  long compressed = get_file_size(output_filename);

  printf("\nHuffman Coding (Heap as Priority Queue)");
  printf("\nBits:\n");
  printf("uncompressed: %d\n", heap->array[0]->frequency * 8);
  printf("compressed: %d\n", total_bits);
  printf("\nBytes:\n");
  printf("uncompressed size: %ld\n", uncompressed);
  printf("compressed size: %ld\n", compressed);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    print_usage(argv[0]);
    return 1;
  }

  const char *option = argv[1];
  const char *input_file = argv[2];
  const char *output_file = argv[3];

  if (strcmp(option, "-c") == 0) {
    return compress(input_file, output_file);
  } else if (strcmp(option, "-d") == 0) {
    return decompress(input_file, output_file);
  } else {
    print_usage(argv[0]);
    return 1;
  }
}
