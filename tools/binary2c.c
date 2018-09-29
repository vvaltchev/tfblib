/* SPDX-License-Identifier: BSD-2-Clause */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void show_help_and_exit(const char *appname)
{
   printf("Usage:\n");
   printf("    %s <BINARY FILE> <C OUTPUT FILE> <VARIABLE NAME>\n", appname);
   printf("\n");
   exit(1);
}

bool check_var_name(const char *name)
{
   const char *p = name;

   while (*p) {

      if (p == name) {

         if (!isalpha(*p) && *p != '_')
            return false;

      } else {

         if (!isalnum(*p) && *p != '_')
            return false;

      }

      p++;
   }
   return true;
}

void bin2c(FILE *src, FILE *dst, const char *fn, const char *var_name)
{
   static unsigned char readbuf[4096];

   size_t s;
   unsigned fs;
   unsigned bc = 0;
   int rc;
   struct stat statbuf;

   rc = fstat(fileno(src), &statbuf);

   if (rc != 0) {
      perror("fstat of the binary file failed");
      return;
   }

   fs = statbuf.st_size;

   fprintf(dst, "const struct {\n\n");
   fprintf(dst, "    const char *filename;\n");
   fprintf(dst, "    unsigned int data_size;\n");
   fprintf(dst, "    unsigned char data[];\n\n");
   fprintf(dst, "} %s = {\n\n", var_name);
   fprintf(dst, "    \"%s\", /* file name */\n", fn);
   fprintf(dst, "    %u, /* file size */\n\n", fs);
   fprintf(dst, "    {");

   do {

      s = fread(readbuf, 1, sizeof(readbuf), src);

      for (size_t i = 0; i < s; i++) {

         if (!(bc % 8))
            fprintf(dst, "\n       /* 0x%08x */ ", bc);

         bc++;
         fprintf(dst, "0x%02x%s", readbuf[i], bc < fs ? ", " : "");
      }

   } while (s);

   fprintf(dst, "\n    }\n");
   fprintf(dst, "};\n\n");
}

int main(int argc, char **argv)
{
   FILE *src = NULL;
   FILE *dst = NULL;
   const char *var_name;

   if (argc != 4)
      show_help_and_exit(argv[0]);

   src = fopen(argv[1], "rb");

   if (!src) {
      perror(argv[1]);
      return 1;
   }

   dst = fopen(argv[2], "wb");

   if (!dst) {
      perror(argv[2]);
      return 1;
   }

   var_name = argv[3];

   if (!check_var_name(var_name)) {
      fprintf(stderr, "Invalid C variable name '%s'\n", var_name);
      goto out;
   }

   fprintf(dst, "/* file: %s */\n", argv[1]);
   bin2c(src, dst, basename(argv[1]), var_name);

out:

   if (src)
      fclose(src);

   if (dst)
      fclose(dst);

   return 0;
}
