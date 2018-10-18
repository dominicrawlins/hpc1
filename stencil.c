
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Define output file name
#define OUTPUT_FILE "stencil.pgm"

void stencil(const short size, float * restrict image, float * restrict  tmp_image);
void init_image(const short size, float * restrict  image, float * restrict  tmp_image);
void output_image(const char * file_name, const short size, float *image);
double wtime(void);

int main(int argc, char *argv[]) {

  // Check usage
  if (argc != 4) {
    fprintf(stderr, "Usage: %s nx ny niters\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Initiliase problem dimensions from command line arguments
  short nx = atoi(argv[1]);
  short ny = atoi(argv[2]);
  short niters = atoi(argv[3]);

  if(nx != ny){
    fprintf(stderr, "nx and ny do not equal");
    exit(EXIT_FAILURE);
  }

  short size = nx;

  // Allocate the image
  float * restrict image = malloc(sizeof(double)*size*size);
  float * restrict tmp_image = malloc(sizeof(double)*size*size);

  // Set the input image
  init_image(size, image, tmp_image);

  // Call the stencil kernel
  double tic = wtime();
  for (short t = 0; t < niters; ++t) {
    stencil(size, image, tmp_image);
    stencil(size, tmp_image, image);
  }
  double toc = wtime();


  // Output
  printf("------------------------------------\n");
  printf(" runtime: %lf s\n", toc-tic);
  printf("------------------------------------\n");

  output_image(OUTPUT_FILE, size, image);
  free(image);
}

void stencil(const short size, float * restrict image, float * restrict tmp_image) {
  //when i=0
  //when j=0
  tmp_image[0] = image[0] * 0.6f;
  tmp_image[0] += image[size] * 0.1f;
  tmp_image[0] += image[0] * 0.1f;
  for (short j = 1; j < size-1; ++j) {
    tmp_image[j] = image[j] * 0.6f;
    tmp_image[j] += image[j  +size] * 0.1f;
    tmp_image[j] += image[j-1] * 0.1f;
    tmp_image[j] += image[j+1] * 0.1f;
  }
  //when j=size-1
  tmp_image[size-1] = image[size-1] * 0.6f;
  tmp_image[size-1] += image[(size-1) +size] * 0.1f;
  tmp_image[size-1] += image[(size-1)-1] * 0.1f;


  for (short i = 1; i < size-1; ++i) {
    //when j=0
    tmp_image[i*size] = image[i*size] * 0.6f;
    tmp_image[i*size] += image[(i-1)*size] * 0.1f;
    tmp_image[i*size] += image[(i+1)*size] * 0.1f;
    tmp_image[i*size] += image[1+i*size] * 0.1f;
    for (short j = 1; j < size-1; ++j) {
      tmp_image[j+i*size] = image[j+i*size] * 0.6f;
      tmp_image[j+i*size] += image[j  +(i-1)*size] * 0.1f;
      tmp_image[j+i*size] += image[j  +(i+1)*size] * 0.1f;
      tmp_image[j+i*size] += image[j-1+i*size] * 0.1f;
      tmp_image[j+i*size] += image[j+1+i*size] * 0.1f;
    }
    //when j=size-1
    tmp_image[(size-1)+i*size] = image[(size-1)+i*size] * 0.6f;
    tmp_image[(size-1)+i*size] += image[(size-1)  +(i-1)*size] * 0.1f;
    tmp_image[(size-1)+i*size] += image[(size-1)  +(i+1)*size] * 0.1f;
    tmp_image[(size-1)+i*size] += image[(size-1)-1+i*size] * 0.1f;
  }
  //when i = size-1
  //when j=0
  tmp_image[(size-1)*size] = image[(size-1)*size] * 0.6f;
  tmp_image[(size-1)*size] += image[((size-1)-1)*size] * 0.1f;
  tmp_image[(size-1)*size] += image[1+(size-1)*size] * 0.1f;
  for (short j = 1; j < size-1; ++j) {
    tmp_image[j+(size-1)*size] = image[j+(size-1)*size] * 0.6f;
    tmp_image[j+(size-1)*size] += image[j  +((size-1)-1)*size] * 0.1f;
    tmp_image[j+(size-1)*size] += image[j-1+(size-1)*size] * 0.1f;
    tmp_image[j+(size-1)*size] += image[j+1+(size-1)*size] * 0.1f;
  }
  //when j=size-1
  tmp_image[(size-1)+(size-1)*size] = image[(size-1)+(size-1)*size] * 0.6f;
  tmp_image[(size-1)+(size-1)*size] += image[(size-1)  +((size-1)-1)*size] * 0.1f;
  tmp_image[(size-1)+(size-1)*size] += image[(size-1)-1+(size-1)*size] * 0.1f;
}

// Create the input image
void init_image(const short size, float * restrict image, float * restrict tmp_image) {
  // Zero everything
  for (short j = 0; j < size; ++j) {
    for (short i = 0; i < size; ++i) {
      image[j+i*size] = 0.0f;
      tmp_image[j+i*size] = 0.0f;
    }
  }

  // Checkerboard
  for (int j = 0; j < 8; ++j) {
    for (int i = 0; i < 8; ++i) {
      for (int jj = j*size/8; jj < (j+1)*size/8; ++jj) {
        for (int ii = i*size/8; ii < (i+1)*size/8; ++ii) {
          if ((i+j)%2)
          image[jj+ii*size] = 100.0f;
        }
      }
    }
  }
}

// Routine to output the image in Netpbm grayscale binary image format
void output_image(const char * file_name, const short size, float *image) {

  // Open output file
  FILE *fp = fopen(file_name, "w");
  if (!fp) {
    fprintf(stderr, "Error: Could not open %s\n", OUTPUT_FILE);
    exit(EXIT_FAILURE);
  }

  // Ouptut image header
  fprintf(fp, "P5 %d %d 255\n", size, size);

  // Calculate maximum value of image
  // This is used to rescale the values
  // to a range of 0-255 for output
  float maximum = 0.0f;
  for (short j = 0; j < size; ++j) {
    for (short i = 0; i < size; ++i) {
      if (image[j+i*size] > maximum)
        maximum = image[j+i*size];
    }
  }

  // Output image, converting to numbers 0-255
  for (short j = 0; j < size; ++j) {
    for (short i = 0; i < size; ++i) {
      fputc((char)(255.0f*image[j+i*size]/maximum), fp);
    }
  }

  // Close the file
  fclose(fp);

}

// Get the current time in seconds since the Epoch
double wtime(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec*1e-6;
}
