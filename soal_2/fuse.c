#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

static const char *dirpath = "encrypted_storage";
static const unsigned char KEY = 0x76;

void xor_encrypt(char *buf, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buf[i] ^= KEY;
    }
}

void build_path(char fpath[1024], const char *path)
{
    strcpy(fpath, dirpath);

    if (strcmp(path, "/") != 0)
    {
        strcat(fpath, path);
        strcat(fpath, ".enc");
    }
}

static int x_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1024];

    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    build_path(fpath, path);

    res = lstat(fpath, stbuf);

    if (res == -1)
        return -errno;

    if (S_ISREG(stbuf->st_mode))
    {
        stbuf->st_mode = S_IFREG | 0644;
    }

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

    (void) offset;
    (void) fi;
    (void) path;

    dp = opendir(dirpath);

    if (dp == NULL)
        return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp)) != NULL)
    {
        if (strstr(de->d_name, ".enc"))
        {
            char name[256];

            strcpy(name, de->d_name);

            name[strlen(name) - 4] = '\0';

            filler(buf, name, NULL, 0);
        }
    }

    closedir(dp);

    return 0;
}

static int x_open(const char *path,
                  struct fuse_file_info *fi)
{
    char fpath[1024];

    build_path(fpath, path);

    int fd = open(fpath, fi->flags);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}

static int x_read(const char *path,
                  char *buf,
                  size_t size,
                  off_t offset,
                  struct fuse_file_info *fi)
{
    char fpath[1024];

    build_path(fpath, path);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    int res = pread(fd, buf, size, offset);

    if (res == -1)
    {
        res = -errno;
    }
    else
    {
        xor_encrypt(buf, res);
    }

    close(fd);

    return res;
}

static int x_write(const char *path,
                   const char *buf,
                   size_t size,
                   off_t offset,
                   struct fuse_file_info *fi)
{
    char fpath[1024];

    build_path(fpath, path);

    int fd = open(fpath, O_WRONLY);

    if (fd == -1)
        return -errno;

    char *enc = malloc(size);

    memcpy(enc, buf, size);

    xor_encrypt(enc, size);

    int res = pwrite(fd, enc, size, offset);

    free(enc);

    if (res == -1)
        res = -errno;

    close(fd);

    return res;
}

static int x_create(const char *path,
                    mode_t mode,
                    struct fuse_file_info *fi)
{
    char fpath[1024];

    build_path(fpath, path);

    int fd = creat(fpath, mode);

    if (fd == -1)
        return -errno;

    close(fd);

    return 0;
}

static int x_unlink(const char *path)
{
    char fpath[1024];

    build_path(fpath, path);

    int res = unlink(fpath);

    if (res == -1)
        return -errno;

    return 0;
}

static struct fuse_operations x_oper = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .open = x_open,
    .read = x_read,
    .write = x_write,
    .create = x_create,
    .unlink = x_unlink,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &x_oper, NULL);
}
