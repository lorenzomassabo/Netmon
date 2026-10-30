#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  // open the file "/proc/net/tcp" in read-only mode
  FILE *fp = fopen("/proc/net/tcp", "r");
  // return if file is not found
  if (fp == NULL) {
    perror("File not found!");
    return 1;
  }

  int ch; // variable that contains the readed character
  // int is used instead of char to be able to rappresent also the EOF special
  // character

  // Reads characters one by one, fgetc(fp) returns the readed character or
  // "EOF" when the enf of the file is reached or an error occur. The readed
  // character is assigned to ch. The cycle continues till the result is != from
  // EOF
  while ((ch = fgetc(fp)) != EOF) {
    printf("%c", ch); // prints the readed character
  }

  fclose(fp); // file is cloes after the reading ended
  return EXIT_SUCCESS;
}
