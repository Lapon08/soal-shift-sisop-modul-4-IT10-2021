#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

static const char *dirpath = "/home/ubuntu/Downloads";
int ismkdir = 0;
int istouch = 0;
int istouch2 = 0;

void logFile(char *level, char *cmd, char *info)
{
	FILE *f = fopen("/home/ubuntu/SinSeiFS.log", "a");
	time_t t;
	struct tm *tmp;
	char timeBuff[100];

	time(&t);
	tmp = localtime(&t);
	strftime(timeBuff, sizeof(timeBuff), "%d%m%y-%H:%M:%S", tmp);

	fprintf(f, "%s::%s:%s::%s\n", level, timeBuff, cmd, info);

	fclose(f);
}

void logEncodeAtbash(char *desc)
{
	FILE *f = fopen("/home/ubuntu/EncodeAtbash.log", "a");
	fprintf(f, "%s\n", desc);

	fclose(f);
}

void rot13(char *pathname)
{
	if ((*(pathname) >= 'a' && *(pathname) < 'n'))
		*(pathname) += 13;
	else if ((*(pathname) >= 'A' && *(pathname) < 'N'))
		*(pathname) += 13;
	else if ((*(pathname) > 'm' && *(pathname) <= 'z'))
		*(pathname) -= 13;
	else if ((*(pathname) > 'M' && *(pathname) <= 'Z'))
		*(pathname) -= 13;
}

void atbash(char *new, char *enc)
{
	if (!((*new >= 0 && *new < 65) || (*new > 90 && *new < 97) || (*new > 122 && *new <= 127)))
	{
		if (*new >= 'A' && *new <= 'Z')
		{
			*enc = 'Z' + 'A' - *new;
		}

		if (*new >= 'a' && *new <= 'z')
		{
			*enc = 'z' + 'a' - *new;
		}
	}

	if (((*new >= 0 && *new < 65) || (*new > 90 && *new < 97) || (*new > 122 && *new <= 127)))
	{
		*enc = *new;
	}
}

void encript2(char *pathname)
{
	if (!strcmp(pathname, "."))
	{
		return;
	}
	else if (!strcmp(pathname, ".."))
	{
		return;
	}

	// ekstensi
	char *ekstensi = strrchr(pathname, '.');
	if (ekstensi == NULL)
	{
		ekstensi = "";
	}
	// DEBUG
	printf("\nEKSTENSI %s", ekstensi);
	int b = strlen(ekstensi);
	int i = 0;
	while (i < strlen(pathname) - b)
	{
		rot13(&pathname[i]);
		i++;
	}
}

void encript1(char *pathname)
{
	if (!strcmp(pathname, "."))
	{
		return;
	}
	else if (!strcmp(pathname, ".."))
	{
		return;
	}

	int size = strlen(pathname);
	char tmp[size];

	for (int i = 0; i < size; i++)
	{
		tmp[i] = pathname[i];
	}

	// ekstensi
	char *ekstensi = strrchr(pathname, '.');
	if (ekstensi == NULL)
	{
		ekstensi = "";
	}
	// DEBUG
	printf("\nEKSTENSI %s", ekstensi);
	int b = strlen(ekstensi);
	int i = 0;
	while (i < strlen(pathname) - b)
	{
		atbash(&tmp[i], &pathname[i]);
		i++;
	}
}

void decript1(char *pathname)
{
	if (!strcmp(pathname, "."))
	{
		return;
	}
	else if (!strcmp(pathname, ".."))
	{
		return;
	}
	int size = strlen(pathname);
	char temp[size];

	for (int i = 0; i < size; i++)
	{
		temp[i] = pathname[i];
	}

	int flag1 = 0, flag2 = 0;
	if ((pathname[0] == 'A' && pathname[1] == 't' && pathname[2] == 'o' && pathname[3] == 'Z' && pathname[4] == '_') || (pathname[0] == 'R' && pathname[1] == 'X' && pathname[2] == '_'))
	{
		flag1 = 1, flag2 = 1;
	}
	// ekstensi
	char *ekstensi = strrchr(pathname, '.');
	if (ekstensi == NULL)
	{
		ekstensi = "";
	}
	// DEBUG
	printf("\nEKSTENSI %s", ekstensi);
	int b = strlen(ekstensi);
	int i = 0;
	// melakukan decrypt setelah AtoZ_{.*}/
	while (i < strlen(pathname) - b)
	{
		if (pathname[i] == '/')
		{
			flag1 = 0;
		}
		if ((flag1 != 1 && pathname[i] != '/') && flag2 == 1)
		{

			atbash(&temp[i], &pathname[i]);
		}
		i++;
	}
}

void decript2(char *pathname)
{
	if (!strcmp(pathname, "."))
	{
		return;
	}
	else if (!strcmp(pathname, ".."))
	{
		return;
	}

	int flag1 = 0, flag2 = 0;
	if (pathname[0] == 'R' && pathname[1] == 'X' && pathname[2] == '_')
	{
		flag1 = 1, flag2 = 1;
	}
	// ekstensi
	char *ekstensi = strrchr(pathname, '.');
	if (ekstensi == NULL)
	{
		ekstensi = "";
	}
	int b = strlen(ekstensi);
	int i = 0;
	// melakukan decrypt setelah RX_{.*}/
	while (i < strlen(pathname) - b)
	{
		if (pathname[i] == '/')
		{
			flag1 = 0;
		}
		if ((flag1 != 1 && pathname[i] != '/') && flag2 == 1)
		{

			rot13(&pathname[i]);
		}
		i++;
	}
}

//Get file attributes.
static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;
	char fpath[1000];
	char desc[1000];
	// log
	strcpy(desc, path);
	// fpath
	strcpy(fpath, dirpath);
	strcat(fpath, path);
	sprintf(fpath, "%s%s", dirpath, path);
	// debug
	printf("\nGETATTR %s ->", fpath);
	if (strstr(path, "AtoZ_") != NULL)
	{
		if (ismkdir)
		{

			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			if (!strstr(filename, "AtoZ_"))
			{
				encript1(filename);
			}

			strcpy(fpath, dirpath);

			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);
			ismkdir = 0;
		}
		else if (istouch)
		{

			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			encript1(filename);
			strcpy(fpath, dirpath);
			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);
			istouch = 0;
		}
		else if (istouch2)
		{
			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			encript1(filename);
			strcpy(fpath, dirpath);
			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);

			istouch2 = 0;
		}
		else
		{
			char *encv1 = strstr(path, "AtoZ_");

			decript1(encv1);
			// debug
			// printf("%s \n", encv1);
			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strcat(fpath, encv1);
			// sprintf(fpath, "%s/%s", dirpath, encv1);
		}
	}
	else if (strstr(path, "RX_") != NULL)
	{

		if (ismkdir)
		{

			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			if (!strstr(filename, "RX_"))
			{
				encript1(filename);
				encript2(filename);
			}

			strcpy(fpath, dirpath);

			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);
			ismkdir = 0;
		}
		else if (istouch)
		{

			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			encript1(filename);
			encript2(filename);
			strcpy(fpath, dirpath);
			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);
			istouch = 0;
		}
		else if (istouch2)
		{
			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			char *filename = temp2[x - 1];
			lenfilename = strlen(filename);
			encript1(filename);
			encript2(filename);
			strcpy(fpath, dirpath);
			strncat(fpath, path, strlen(path) - lenfilename);
			strcat(fpath, filename);

			istouch2 = 0;
		}
		else
		{
			char *encv1 = strstr(path, "RX_");

			decript1(encv1);
			decript2(encv1);
			// debug
			// printf("%s \n", encv1);
			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strcat(fpath, encv1);
			// sprintf(fpath, "%s/%s", dirpath, encv1);
		}
	}
	// debug
	printf("\nGETATTR %s ->", fpath);
	res = lstat(fpath, stbuf);
	logFile("INFO", "GETATTR", desc);
	printf(" %s\n", fpath);
	if (res == -1)
		return -errno;

	return 0;
}

//Read directory
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					   off_t offset, struct fuse_file_info *fi)
{
	char fpath[1000];
	char desc[1000];
	// log
	strcpy(desc, path);
	// fpath
	strcpy(fpath, dirpath);
	strcat(fpath, path);
	// debug
	printf("\nREADDIR %s ->", fpath);
	// log
	logFile("INFO", "READDIR", desc);
	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
	}
	else if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
	}
	else if (strstr(path, "RX_") != NULL)
	{
		char *pathencv2 = strstr(path, "RX_");
		decript2(pathencv2);
		decript1(pathencv2);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, pathencv2);
	}
	//debug
	printf("%s\n", fpath);

	DIR *dp;
	struct dirent *de;
	(void)offset;
	(void)fi;
	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;
	while ((de = readdir(dp)) != NULL)
	{
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;

		// debug
		printf("\n READDIR NAMA FILE %s -> ", de->d_name);

		if (strstr(path, "AtoZ_") != NULL)
		{
			encript1(de->d_name);
		}
		else if (strstr(path, "RX_") != NULL)
		{
			encript1(de->d_name);
			encript2(de->d_name);
		}
		// DEBUG
		printf(" %s\n", de->d_name);

		if (filler(buf, de->d_name, &st, 0))
			break;
	}
	closedir(dp);
	return 0;
}

//Rename a file
static int xmp_rename(const char *from, const char *to)
{
	int res;
	char from1[1000], to1[1000], desc[1000];
	char *ffrom1 = strstr(from, "AtoZ_"), *fto1 = strstr(to, "AtoZ_");

	// from di decrypt, to decrypt trus nama file encrypt
	// logfile
	strcpy(desc, from);
	strcat(desc, "::");
	strcat(desc, to);
	// from
	strcpy(from1, dirpath);
	strcat(from1, from);
	// to
	strcpy(to1, dirpath);
	strcat(to1, to);
	// DEBUG
	printf("\nRENAME %s ->  %s\n", from1, to1);
	if (ffrom1 != NULL && fto1 != NULL)
	{
		char filename[500];
		int lenfilename;

		char temp[500];
		char *temp2[500];

		char *slash;
		int x = 0;
		strcpy(temp, to1);

		slash = strtok(temp, "/");
		while (slash != NULL)
		{
			temp2[x] = slash;
			x++;
			slash = strtok(NULL, "/");
		}

		strcpy(filename, temp2[x - 1]);
		lenfilename = strlen(filename);
		char *encv1 = strstr(to, "AtoZ_");
		decript1(encv1);
		// strlen(encv1);
		strcpy(to1, dirpath);
		strcat(to1, "/");
		strncat(to1, encv1, strlen(to) - lenfilename - 1);
		strcat(to1, filename);
		decript1(ffrom1);
		sprintf(from1, "%s/%s", dirpath, ffrom1);
	}
	else if (fto1 != NULL && ffrom1 == NULL)
	{
		char filename[500];
		int lenfilename;

		char temp[500];
		char *temp2[500];

		char *slash;
		int x = 0;
		strcpy(temp, fto1);

		slash = strtok(temp, "/");
		while (slash != NULL)
		{
			temp2[x] = slash;
			x++;
			slash = strtok(NULL, "/");
		}
		strcpy(filename, temp2[x - 1]);
		lenfilename = strlen(filename);
		decript1(fto1);

		// strlen(encv1);
		strcpy(to1, dirpath);
		strcat(to1, "/");
		strncat(to1, fto1, strlen(fto1) - lenfilename);
		strcat(to1, filename);
	}
	// DEBUG
	printf("\nRENAME %s ->  %s\n", from1, to1);

	res = rename(from1, to1);

	// logfile
	char *tempto = strstr(to, "AtoZ_");
	if (tempto != NULL && !strstr(tempto, "/"))
	{
		char log[1000];
		strcpy(log, from1);
		strcat(log, " -> ");
		strcat(log, to1);
		logEncodeAtbash(log);
	}
	logFile("INFO", "RENAME", desc);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[1000];
	char desc[1000];
	strcpy(desc, path);
	strcpy(fpath, dirpath);
	strcat(fpath, path);
	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
	}
	if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
	}

	int res;

	res = open(fpath, fi->flags);
	logFile("INFO", "OPEN", desc);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}

//Read data from an open file
static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
					struct fuse_file_info *fi)
{
	char desc[1000];
	strcpy(desc, path);

	int fd;
	int res;
	char fpath[1000];

	strcpy(fpath, dirpath);
	strcat(fpath, path);
	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
	}
	if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
	}
	if (strstr(path, "RX_") != NULL)
	{
		char *encv2 = strstr(path, "RX_");
		decript2(encv2);
		decript1(encv2);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv2);
	}
	// DEBUG
	printf("\nREAD %s\n",fpath);
	(void)fi;

	fd = open(fpath, O_RDONLY);
	logFile(0, "READ", desc);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}


//Create a directory.
static int xmp_mkdir(const char *path, mode_t mode)
{
	//logsys(0, "MKDIR", path);
	int res;
	char fpath[1000], desc[1000];
	sprintf(fpath, "%s%s", dirpath, path);
	strcpy(desc, path);

	char filename[500];
	int lenfilename;

	char temp[500];
	char *temp2[500];

	char *slash;
	int x = 0;
	strcpy(temp, fpath);

	slash = strtok(temp, "/");
	while (slash != NULL)
	{
		temp2[x] = slash;
		x++;
		slash = strtok(NULL, "/");
	}

	strcpy(filename, temp2[x - 1]);
	lenfilename = strlen(filename);
	// strncpy(fullpath,temp3,lenfullpath-lenfilename);
	// strcat(fullpath,filename);

	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
		// sprintf(fpath, "%s", path);
	}
	else if (strstr(path, "AtoZ_") != NULL)
	{
		char log[1000];
		strcpy(log, dirpath);
		strcat(log, path);
		char *temppath = strstr(path, "AtoZ_");
		if (strstr(temppath, "/") == NULL)
		{
			logEncodeAtbash(log);
		}
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strncat(fpath, encv1, strlen(encv1) - lenfilename);
		strcat(fpath, filename);
	}
	else if (strstr(path, "RX_") != NULL)
	{
		// atbash +ROT13
		char *encv2 = strstr(path, "RX_");
		decript2(encv2);
		decript1(encv2);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strncat(fpath, encv2, strlen(encv2) - lenfilename);
		strcat(fpath, filename);
	}
	printf("\nMKDIR -> %s\n", fpath);
	printf("\nMKDIR -> %s\n", fpath);
	printf("\nMKDIR -> %s\n", fpath);

	res = mkdir(fpath, mode);
	logFile("INFO", "MKDIR", desc);
	ismkdir = 1;
	if (res == -1)
	{
		ismkdir = 0;
		return -errno;
	}
	return 0;
}

//Remove a file
static int xmp_unlink(const char *path)
{

	int res;
	char fpath[1000], desc[1000];
	// log
	strcpy(desc, path);
	// fpath
	strcpy(fpath, dirpath);
	strcat(fpath, path);

	if (strcmp(path, "/") == 0)
	{
		strcpy(fpath, dirpath);
	}
	else if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
	}
	else if (strstr(path, "RX_") != NULL)
	{
		char *encv1 = strstr(path, "RX_");
		decript2(encv1);
		decript1(encv1);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
	}
	printf("\nUNLINK %s\n", fpath);
	logFile("WARNING", "UNLINK", desc);
	res = unlink(fpath);

	if (res == -1)
		return -errno;
	return 0;
}

//Remove a directory
static int xmp_rmdir(const char *path)
{
	//logsys(1, "RMDIR", path);
	int res;
	char fpath[1000], desc[1000];
	strcpy(desc, path);
	sprintf(fpath, "%s%s", dirpath, path);
	if (strcmp(path, "/") == 0)
	{
		strcpy(fpath, dirpath);
	}
	else if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
		// sprintf(fpath, "%s/%s", dirpath, encv1);
	}
	else if (strstr(path, "RX_") != NULL)
	{
		char *encv1 = strstr(path, "RX_");
		decript2(encv1);
		decript1(encv1);
		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strcat(fpath, encv1);
		// sprintf(fpath, "%s/%s", dirpath, encv1);
	}
	res = rmdir(fpath);
	logFile("WARNING", "RMDIR", desc);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_create(const char *path, mode_t mode,
					  struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
	char desc[1000];
	strcpy(desc, path);
	sprintf(fpath, "%s%s", dirpath, path);

	char filename[500];
	int lenfilename;

	char temp[500];
	char *temp2[500];

	char *slash;
	int x = 0;
	strcpy(temp, fpath);

	slash = strtok(temp, "/");
	while (slash != NULL)
	{
		temp2[x] = slash;
		x++;
		slash = strtok(NULL, "/");
	}

	strcpy(filename, temp2[x - 1]);
	lenfilename = strlen(filename);

	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
		// sprintf(fpath, "%s", path);
	}
	if (strstr(path, "AtoZ_") != NULL)
	{
		char *encv1 = strstr(path, "AtoZ_");
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strncat(fpath, encv1, strlen(encv1) - lenfilename);
		strcat(fpath, filename);
		// sprintf(fpath, "%s/%s", dirpath, encv1);
		istouch = 1;
		istouch2 = 1;
	}
	if (strstr(path, "RX_") != NULL)
	{
		char *encv1 = strstr(path, "RX_");
		decript2(encv1);
		decript1(encv1);

		strcpy(fpath, dirpath);
		strcat(fpath, "/");
		strncat(fpath, encv1, strlen(encv1) - lenfilename);
		strcat(fpath, filename);
		// sprintf(fpath, "%s/%s", dirpath, encv1);
		istouch = 1;
		istouch2 = 1;
	}
	printf("\nCREATE -> %s\n", fpath);
	printf("\nCREATE -> %s\n", fpath);
	printf("\nCREATE -> %s\n", fpath);
	res = open(fpath, fi->flags, mode);
	logFile("INFO", "CREATE", desc);
	if (res == -1)
	{
		istouch = 0;
		istouch2 = 0;
		return -errno;
	}

	fi->fh = res;
	return 0;
}
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	char fpath[1000];
	char desc[1000];
	strcpy(desc, path);
	sprintf(fpath, "%s%s", dirpath, path);
	char filename[500];
	int lenfilename;

	char temp[500];
	char *temp2[500];

	char *slash;
	int x = 0;
	strcpy(temp, fpath);
	slash = strtok(temp, "/");
	while (slash != NULL)
	{
		temp2[x] = slash;
		x++;
		slash = strtok(NULL, "/");
	}

	strcpy(filename, temp2[x - 1]);
	lenfilename = strlen(filename);
	if (strcmp(path, "/") == 0)
	{
		path = dirpath;
		strcpy(fpath, path);
		// sprintf(fpath, "%s", path);
	}
	if (strstr(path, "AtoZ_") != NULL)
	{
		if (istouch2)
		{

			char filename[500];
			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			strcpy(filename, temp2[x - 1]);
			lenfilename = strlen(filename);
			char *encv1 = strstr(path, "AtoZ_");
			decript1(encv1);

			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strncat(fpath, encv1, strlen(encv1) - lenfilename);
			strcat(fpath, filename);
			
		}
		else
		{
			char *encv1 = strstr(path, "AtoZ_");
			decript1(encv1);

			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strncat(fpath, encv1, strlen(encv1) - lenfilename);
			strcat(fpath, filename);
			
		}
	}
	if (strstr(path, "RX_") != NULL)
	{
		if (istouch2)
		{

			char filename[500];
			int lenfilename;

			char temp[500];
			char *temp2[500];

			char *slash;
			int x = 0;
			strcpy(temp, fpath);

			slash = strtok(temp, "/");
			while (slash != NULL)
			{
				temp2[x] = slash;
				x++;
				slash = strtok(NULL, "/");
			}

			strcpy(filename, temp2[x - 1]);
			lenfilename = strlen(filename);
			char *encv1 = strstr(path, "RX_");
			decript2(encv1);
			decript1(encv1);

			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strncat(fpath, encv1, strlen(encv1) - lenfilename);
			strcat(fpath, filename);
			
		}
		else
		{
			char *encv1 = strstr(path, "RX_");
			decript2(encv1);
			decript1(encv1);
			strcpy(fpath, dirpath);
			strcat(fpath, "/");
			strncat(fpath, encv1, strlen(encv1) - lenfilename);
			strcat(fpath, filename);
		
		}
	}

	int res;
	// DEBUG
	printf("\nUTIMENS %s\n", fpath);
	logFile("INFO", "UTIMENS", desc);
	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, fpath, ts, AT_SYMLINK_NOFOLLOW);

	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {

	.getattr = xmp_getattr,
	.readdir = xmp_readdir,
	.read = xmp_read,
	.mkdir = xmp_mkdir,
	.unlink = xmp_unlink,
	.rmdir = xmp_rmdir,
	.rename = xmp_rename,
	.open = xmp_open,
	.create = xmp_create,
	.utimens = xmp_utimens

};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}