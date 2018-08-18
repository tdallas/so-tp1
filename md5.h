#define PATH_LEN 256
#define MD5_LEN 32

#define STR_VALUE(val) #val
#define STR(name) STR_VALUE(name)

int CalcFileMD5(char *file_name, char *md5_sum);