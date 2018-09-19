
typedef struct {
   const char *filename;
   unsigned int data_size;
   unsigned char data[];
} font_file;

extern const font_file **tfb_font_file_list;
