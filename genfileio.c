/**
 * Generate random fixed block-sized IOs on a file
 *
 * Thomas Gouverneur <thomas@espix.net>
 * 02/2015
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>


void syntax(char *me) {
    fprintf (stderr, "%s [-f <file>] [-b <K>]\n", me);
    return;
}


int main(int ac, char **av) {
    char *iofile = NULL;
    unsigned short nBlock = 128;
    int rc, c, fd, ctime, otime;
    int nbytes = 0;
    void *rbuf = 0;
    unsigned long offset = 0;
    unsigned long int fsize = 0;
    unsigned long int cRead, cWrite;

    while ((c = getopt (ac, av, "b:f:")) != -1) {
        switch (c)
        {
            case 'b':
                nBlock = atoi(optarg);
                break;
            case 'f':
                iofile = optarg;
                break;
            case '?':
                if (optopt == 'f') {
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint (optopt)) {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
                }
                syntax(av[0]);
                return 1;
            default:
                syntax(av[0]);
                return 1;
        } 
    }
    if (iofile == NULL) {
        syntax(av[0]);
        return 1;
    }
    fprintf(stdout, "[-] Going to use %s with %dK blocks..\n", iofile, nBlock);

    /* open the file */
    fd = open(iofile, O_RDWR);
    if (fd == -1) {
        perror("[!] Unable to open file for read/write: ");
        return 1;
    }
    fprintf(stdout, "[-] File opened succesfully, going to read/write randomly forever now...\n");

    /* get file size */
    fsize = (int) lseek(fd, 0, SEEK_END);

    if (fsize == -1) {
        perror("[!] Unable to get file size: ");
        close(fd);
        return 1;
    }

    fprintf(stdout, "[-] File size: %u bytes\n", fsize);

    /* init random generator */
    srand (time(NULL));

    otime = ctime = time(NULL);
    cRead = cWrite = 0;

    while (1) {
    
        /* current time */
        ctime = time(NULL);
        if ((ctime - otime) >= 1) {
            fprintf(stdout, "\t[-READ] %u MB in %d seconds\n", (cRead/1024/1024), (ctime - otime));
            fprintf(stdout, "\t[WRITE] %u MB in %d seconds\n", (cWrite/1024/1024), (ctime - otime));
            cRead = cWrite = 0;
            otime = ctime;
        }

        /* how many bytes to read */
        nbytes = nBlock * 1024; 
        
        /* find an offset to seek to */
        offset = rand() % (fsize - nbytes);

        /* let's do it */
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("[!] Cannot seek to needed position: ");
            close(fd);
            return -1;
        }

        /* get memory buffer to read */
        rbuf = (void*) malloc(nbytes);

        rc = read(fd, rbuf, nbytes);
        if (rc == -1) {
            perror("[!] Cannot read from file: ");
            close(fd);
            return -1;
        }

        //fprintf(stdout, "\t* R <pos=%d> <size=%d>\n", offset, nbytes);
        cRead += rc;

        /* find an offset to write those bytes */
        offset = rand() % (fsize - nbytes);

        /* let's do it */
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("[!] Cannot seek ito needed position: ");
            close(fd);
            return -1;
        }

        rc = write(fd, rbuf, nbytes);
        if (rc == -1) {
            perror("[!] Cannot write to file: ");
            close(fd);
            return -1;
        }

        if (fsync(fd) == -1) {
            perror("[!] fsync() failed: ");
            close(fd);
            return -1;
        }

        cWrite += nbytes;
        //fprintf(stdout, "\t* W <pos=%d> <size=%d>\n", offset, nbytes);
        free(rbuf);
    }


    return 0;
}

