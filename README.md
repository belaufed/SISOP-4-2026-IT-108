# Praktikum Sistem Operasi Modul 4

| Nama         | NRP        |
| ------------ | ---------- |
| Nadia Iqlima | 5027251108 |


# Soal 1 - Kenz Rescue

## Deskripsi Masalah

Pada soal ini, diberikan sebuah file ZIP bernama `amba_files.zip` yang berisi beberapa file `.txt`. Tugas utama pada soal ini adalah membuat filesystem sederhana menggunakan FUSE yang mampu membaca isi file dari direktori sumber dan menampilkannya melalui mount point.

Filesystem yang dibuat harus mampu:

* Melakukan mounting direktori sumber.
* Menampilkan isi file melalui mount point.
* Mengimplementasikan operasi dasar FUSE seperti:

  * `getattr`
  * `readdir`
  * `open`
  * `read`

Program dibuat menggunakan bahasa C dan library FUSE.


## Struktur Folder

```bash
soal_1/
├── amba_files/
│   ├── 1.txt
│   ├── 2.txt
│   ├── 3.txt
│   ├── 4.txt
│   ├── 5.txt
│   ├── 6.txt
│   └── 7.txt
├── kenz_rescue
├── kenz_rescue.c
└── mnt
```


## Penjelasan Kode

### 1. Deklarasi Source Directory

```c
static const char *source_dir = "amba_files";
```

Digunakan untuk menentukan direktori sumber tempat file asli berada.

---

### 2. Fungsi `x_getattr`

```c
static int x_getattr(const char *path, struct stat *stbuf)
```

Fungsi ini digunakan untuk mengambil atribut file seperti ukuran file, permission, dan informasi metadata lainnya.


### 3. Fungsi `x_readdir`

```c
static int x_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi)
```

Fungsi ini digunakan untuk membaca isi direktori dan menampilkan file-file yang ada.


### 4. Fungsi `x_open`

```c
static int x_open(const char *path, struct fuse_file_info *fi)
```

Digunakan untuk membuka file dari direktori sumber.


### 5. Fungsi `x_read`

```c
static int x_read(const char *path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi)
```

Digunakan untuk membaca isi file dan mengirimkannya ke mount point.


## Cara Menjalankan Program

### 1. Compile Program

```bash
gcc kenz_rescue.c -o kenz_rescue `pkg-config fuse --cflags --libs`
```


### 2. Jalankan FUSE

```bash
./kenz_rescue amba_files mnt
```


### 3. Melihat Isi Mount Point

```bash
ls mnt
```


## Output Program


**Hasil Mount FUSE**

![Output Soal 1](soal_1/output%20mas%20amba.png)

## Kode Full (kenz.rescue.c)
```bash
#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

static const char *source_dir = "amba_files";

static int x_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];

    if (strcmp(path, "/") == 0) {
        sprintf(fpath, "%s", source_dir);
    } else {
        sprintf(fpath, "%s%s", source_dir, path);
    }

    res = lstat(fpath, stbuf);

    if (res == -1)
        return -errno;

    return 0;
}

static int x_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    dp = opendir(source_dir);

    if (dp == NULL)
        return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp)) != NULL) {
        filler(buf, de->d_name, NULL, 0);
    }

    closedir(dp);

    return 0;
}

static int x_open(const char *path, struct fuse_file_info *fi)
{
    char fpath[1000];

    sprintf(fpath, "%s%s", source_dir, path);

    int res = open(fpath, O_RDONLY);

    if (res == -1)
        return -errno;

    close(res);

    return 0;
}

static int x_read(const char *path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];

    sprintf(fpath, "%s%s", source_dir, path);

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
    .open = x_open,
    .read = x_read,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &x_oper, NULL);
}

```

---

# Soal 2 - Poke MOO

## Deskripsi Masalah

Pada soal ini, dibuat sebuah mini database service menggunakan FUSE dan Docker.

Filesystem yang dibuat akan melakukan translasi file terenkripsi pada folder `encrypted_storage` menjadi file normal pada mount point `fuse_mount`.

File terenkripsi menggunakan algoritma XOR dengan key:

```c
0x76
```

Selain implementasi FUSE, pada soal ini juga dilakukan:

* Pembuatan Docker container.
* Pembuatan Docker image.
* Integrasi bind mount antara `fuse_mount` dan container.
* Pembuatan TCP client sederhana.

---

## Struktur Folder

```bash
soal_2/
├── Dockerfile
├── client.c
├── encrypted_storage/
│   └── notes.csv.enc
├── fuse
├── fuse.c
├── fuse_mount
└── server
```

---

# Penjelasan Program FUSE

## 1. Deklarasi Direktori

```c
static const char *source_dir = "encrypted_storage";
```

Digunakan untuk menentukan lokasi file terenkripsi.

---

## 2. Fungsi XOR Decrypt

```c
void decrypt(char *text, int len)
```

Digunakan untuk melakukan decrypt menggunakan XOR key `0x76`.


## 3. Fungsi `x_getattr`

Digunakan untuk membaca atribut file.


## 4. Fungsi `x_readdir`

Digunakan untuk membaca isi direktori dan menampilkan file `.csv` hasil translasi dari file `.enc`.


## 5. Fungsi `x_open`

Digunakan untuk membuka file terenkripsi.


## 6. Fungsi `x_read`

Digunakan untuk membaca file terenkripsi kemudian melakukan decrypt sebelum ditampilkan.


# Docker

## Dockerfile

```dockerfile
FROM ubuntu:latest

WORKDIR /app

COPY server .

RUN chmod +x server

EXPOSE 9000

CMD ["./server"]
```


## Build Docker Image

```bash
sudo docker build -t soal-2-modul-4-sisop .
```


## Menjalankan Container

```bash
sudo docker run -d \
--name db_app \
-p 9000:9000 \
-v $(pwd)/fuse_mount:/app/database \
soal-2-modul-4-sisop
```


# Client TCP

## Compile Client

```bash
gcc client.c -o client
```


## Menjalankan Client

```bash
./client
```


## Output Program

**Struktur Folder**

![Struktur Folder](soal_2/hasil%20tree.png)


**Output FUSE**

![Output FUSE](soal_2/output%20soal2.png)


**Docker Image**

![Docker Image](soal_2/Screenshot%202026-05-13%20111016.png)


**Menjalankan Client**

![Run Client](soal_2/hasil%20run%20client.png)

## Full Kode (Fuse.c)
```bash
#define FUSE_USE_VERSION 28

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

static const char *source_dir = "encrypted_storage";
static const char key = 0x76;

void decrypt(char *text, int len)
{
    for (int i = 0; i < len; i++) {
        text[i] ^= key;
    }
}

static int x_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];

    if (strcmp(path, "/") == 0) {
        sprintf(fpath, "%s", source_dir);
    } else {

        char temp[1000];

        strcpy(temp, path + 1);

        char *dot = strstr(temp, ".csv");

        if (dot != NULL)
            *dot = '\0';

        sprintf(fpath, "%s/%s.csv.enc", source_dir, temp);
    }

    res = lstat(fpath, stbuf);

    if (res == -1)
        return -errno;

    return 0;
}

static int x_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                     off_t offset, struct fuse_file_info *fi)
{
    DIR *dp;
    struct dirent *de;

    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    dp = opendir(source_dir);

    if (dp == NULL)
        return -errno;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    while ((de = readdir(dp)) != NULL) {

        if (strstr(de->d_name, ".csv.enc") != NULL) {

            char name[1000];

            strcpy(name, de->d_name);

            char *pos = strstr(name, ".enc");

            if (pos != NULL)
                *pos = '\0';

            filler(buf, name, NULL, 0);
        }
    }

    closedir(dp);

    return 0;
}

static int x_open(const char *path, struct fuse_file_info *fi)
{
    char fpath[1000];

    char temp[1000];

    strcpy(temp, path + 1);

    char *dot = strstr(temp, ".csv");

    if (dot != NULL)
        *dot = '\0';

    sprintf(fpath, "%s/%s.csv.enc", source_dir, temp);

    int res = open(fpath, O_RDONLY);

    if (res == -1)
        return -errno;

    close(res);

    return 0;
}

static int x_read(const char *path, char *buf, size_t size,
                  off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];

    char temp[1000];

    strcpy(temp, path + 1);

    char *dot = strstr(temp, ".csv");

    if (dot != NULL)
        *dot = '\0';

    sprintf(fpath, "%s/%s.csv.enc", source_dir, temp);

    int fd = open(fpath, O_RDONLY);

    if (fd == -1)
        return -errno;

    char tempbuf[10000];

    int res = pread(fd, tempbuf, size, offset);

    if (res == -1) {
        res = -errno;
    } else {
        decrypt(tempbuf, res);
        memcpy(buf, tempbuf, res);
    }

    close(fd);

    return res;
}

static struct fuse_operations x_oper = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .open = x_open,
    .read = x_read,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &x_oper, NULL);
}

```


# Kendala Selama Pengerjaan

## Soal 1

* Kesulitan melakukan mounting FUSE.
* Error pada path direktori.
* Warning saat compile terkait `snprintf`.
* Kesalahan permission pada mount point.


## Soal 2

* Docker belum pernah digunakan sebelumnya.
* Bind mount Docker sempat gagal.
* File `notes.csv` tidak muncul pada mount point.
* Permission denied saat mounting FUSE.
* Kesulitan melakukan translasi file `.enc` menjadi `.csv`.


# Kesimpulan

Pada praktikum ini dipelajari implementasi filesystem sederhana menggunakan FUSE serta integrasi container menggunakan Docker. Selain itu dipahami juga bagaimana melakukan translasi file terenkripsi menggunakan algoritma XOR dan bagaimana menghubungkan service database sederhana menggunakan socket TCP.
