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

#define M_WRITE 1
#define M_READ  2
#define M_FSYNC 4

void syntax(char *me, int full) {
    fprintf (stderr, "%s [-hrwskm] [-z <size>] [-b <#>] -f <file>\n", me);
    if (full) {
        fprintf(stderr, "\n\t-h\t\tThis help.\n");
        fprintf(stderr, "\t-r\t\tREAD in the file.\n");
        fprintf(stderr, "\t-w\t\tWRITE in the file\n");
        fprintf(stderr, "\t-s\t\tFSYNC after each pass.\n");
        fprintf(stderr, "\t-k\t\tUnits are in KB.\n");
        fprintf(stderr, "\t-m\t\tUnits are in MB.\n");
        fprintf(stderr, "\t-z <size>\tCreate the file with the size*unit size.\n");
        fprintf(stderr, "\t-b <#>\t\tUse that block size for READ/WRITE.\n");
        fprintf(stderr, "\t-f <file>\tTarget file.\n");
    }
    return;
}


int main(int ac, char **av) {
    char *iofile = NULL;
    unsigned short nBlock = 128;
    unsigned short oMode = 0;
    int rc, c, fd, ctime, otime;
    int nbytes = 0;
    void *rbuf = 0;
    int oFlags, iOut;
    unsigned long offset = 0;
    unsigned long int fsize = 0;
    unsigned long int cRead, cWrite;
    unsigned long int cCycle;
    unsigned long int sizeCoef = 1; /* defaults to bytes */
    unsigned long int ioFileSize = 0;

    while ((c = getopt (ac, av, "hrwskmz:b:f:")) != -1) {
        switch (c)
        {
            case 'k':
                sizeCoef = 1024;
                break;
            case 'm':
                sizeCoef = 1024 * 1024;
                break;
            case 'z':
                ioFileSize = 1024 * 1024 * atoi(optarg);
                break;
            case 'r':
                oMode |= M_READ;
                break;
            case 'w':
                oMode |= M_WRITE;
                break;
            case 's':
                oMode |= M_FSYNC;
                break;
            case 'b':
                nBlock = atoi(optarg);
                break;
            case 'f':
                iofile = optarg;
                break;
            case 'h':
                syntax(av[0], 1);
                return 0;
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
                syntax(av[0], 0);
                return 1;
            default:
                syntax(av[0], 0);
                return 1;
        } 
    }

    if (iofile == NULL) {
        syntax(av[0], 0);
        return 1;
    }
    fprintf(stdout, "[-] Going to use %s with %dK blocks..\n", iofile, nBlock);

    /* if -z, we need to check if the file exist first */
    if (ioFileSize > 0 && access(iofile, F_OK) != -1) {
        fprintf(stdout, "[!] Error, you used -z but the specified file already exists.");
        return 1;
    }

    if (ioFileSize > 0) {
        /* create the file */
        fprintf(stdout, "[!] TBD!");
        return 1;
    }

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
    cCycle = cRead = cWrite = 0;
    iOut = 0;

    fprintf(stdout, "\tMB in\tMB out\tsec elapsed\t# cycle\n");
    while (1) {
    
        /* current time */
        ctime = time(NULL);
        if ((ctime - otime) >= 1) {
            if (iOut % 20) {
                fprintf(stdout, "\tMB in\tMB out\tsec elapsed\t# cycle\n");
            }
            fprintf(stdout, "\t%4u\t%6u\t%11.d\t%7.d\n", (cRead/1024/1024), (cWrite/1024/1024), (ctime - otime), cCycle);
            cCycle = cRead = cWrite = 0;
            otime = ctime;
            iOut++;
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
        cCycle++;
        //fprintf(stdout, "\t* W <pos=%d> <size=%d>\n", offset, nbytes);
        free(rbuf);
    }


    return 0;
}

