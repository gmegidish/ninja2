
/* XXX export */
static unsigned
load_raw_file (filename, p)
       const char *filename;
       void **p;
{
       unsigned length;
       int fd;

       if (!filename || !p) {
               return 0;
       }

       *p = NULL;
       fd = open (filename, O_RDONLY | O_BINARY);
       if (fd < 0) {
               return 0;
       }

       lseek (fd, 0, SEEK_END);
       length = tell (fd);
       lseek (fd, 0, SEEK_SET);

       if (length <= 0) {
               close (fd);
               return 0;
       }

       *p = (void*)malloc (length);
       if (!(*p)) {
               close (fd);
               return 0;
       }

       if (read (fd, *p, length) != length) {
               free (*p);
               *p = NULL;
               return 0;
       }

       close (fd);
       return length;
}

