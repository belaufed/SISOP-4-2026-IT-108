#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

static char source_dir[1024];

void fullpath(char fpath[1024], const char *path)
{
    strcpy(fpath, source_dir);
    strcat(fpath, path);
}

static int x_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1024];

    memset(stbuf, 0, sizeof(struct stat));

    // virtual file tujuan.txt
    if (strcmp(path, "/tujuan.txt") == 0)
    {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;

        char content[10000] = "Tujuan Mas Amba: ";

        for (int i = 1; i <= 7; i++)
        {
            char fname[1024];

            strcpy(fname, source_dir);
            strcat(fname, "/");

            char num[20];
            sprintf(num, "%d.txt", i);

            strcat(fname, num);

            FILE *fp = fopen(fname, "r");

            if (fp != NULL)
            {
                char line[1024];

                while (fgets(line, sizeof(line), fp))
                {
                    char *p = strstr(line, "KOORD:");

                    if (p != NULL)
                    {
                        strcat(content, p + 7);
                    }
                }

                fclose(fp);
            }
        }

        strcat(content, "\n");

        stbuf->st_size = strlen(content);

        return 0;
    }

    fullpath(fpath, path);

    res = lstat(fpath, stbuf);

    if (res == -1)
        return -errno;

    return 0;
}

static int x_readdir(const char *path,
                     void *buf,
                     fuse_fill_dir_t filler,
                     off_t offset,
                     struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;
    char fpath[1024];

    (void) offset;
    (void) fi;

    fullpath(fpath, path);

    dp = opendir(fpath);

    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL)
    {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;

        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);

    // tambah virtual file tujuan.txt
    if (strcmp(path, "/") == 0)
    {
        struct stat st;

        memset(&st, 0, sizeof(st));

        st.st_mode = S_IFREG | 0444;
        st.st_nlink = 1;

        filler(buf, "tujuan.txt", &st, 0);
    }

    return 0;
}

static int x_access(const char *path, int mask)
{
    char fpath[1024];

    // virtual file
    if (strcmp(path, "/tujuan.txt") == 0)
        return 0;

    fullpath(fpath, path);

    int res = access(fpath, mask);

    if (res == -1)
        return -errno;

    return 0;
}

static int x_open(const char *path,
                  struct fuse_file_info *fi)
{
    // virtual file
    if (strcmp(path, "/tujuan.txt") == 0)
        return 0;

    char fpath[1024];

    fullpath(fpath, path);

    int res = open(fpath, fi->flags);

    if (res == -1)
        return -errno;

    close(res);

    return 0;
}

static int x_read(const char *path,
                  char *buf,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info *fi)
{
    (void) fi;

    // virtual file tujuan.txt
    if (strcmp(path, "/tujuan.txt") == 0)
    {
        char content[10000] = "Tujuan Mas Amba: ";

        for (int i = 1; i <= 7; i++)
        {
            char fname[1024];

            strcpy(fname, source_dir);
            strcat(fname, "/");

            char num[20];
            sprintf(num, "%d.txt", i);

            strcat(fname, num);

            FILE *fp = fopen(fname, "r");

            if (fp != NULL)
            {
                char line[1024];

                while (fgets(line, sizeof(line), fp))
                {
                    char *p = strstr(line, "KOORD:");

                    if (p != NULL)
                    {
                        strcat(content, p + 7);
                    }
                }

                fclose(fp);
            }
        }

        strcat(content, "\n");

        size_t len = strlen(content);

        if (offset < len)
        {
            if (offset + size > len)
                size = len - offset;

            memcpy(buf, content + offset, size);
        }
        else
        {
            size = 0;
        }

        return size;
    }

    char fpath[1024];

    fullpath(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}

static struct fuse_operations x_oper = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .access = x_access,
    .open = x_open,
    .read = x_read,
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr,
                "Usage: %s <source_dir> <mountpoint>\n",
                argv[0]);

        return 1;
    }

    strcpy(source_dir, argv[1]);

    // force foreground mode
    argv[1] = "-f";

    return fuse_main(argc, argv, &x_oper, NULL);
}
