#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#define MAX_LENGTH 1024

typedef struct
{
	char** files; 
	char fileName[MAX_LENGTH];
	int fileCount;
} Zipper;

char* getFileName(char* fn)
{
	char* fname = strrchr(fn, '\\');
	if (fname == NULL)
		return fn;
	return fname + 1;;
}

void init(Zipper* zip, char** files, int fileCount, char* fileName)
{
	if (fileCount > 0)
	{
		zip->files = files;
		zip->fileCount = fileCount;
		strcpy(zip->fileName, fileName);
	}
}

int digs(int w)
{
	int count = 0;
	while (w > 0)
	{
		w /= 10;
		count++;
	}
	return count;
}

void getInfo(Zipper* zip)
{
	char byte[1];  
	char* s_info = (char*)malloc(sizeof(char));
	*s_info = NULL;
	char fileInfo[MAX_LENGTH];
	strcpy(fileInfo, zip->fileName);
	strcat(fileInfo, "info.txt");
	FILE* info = fopen(fileInfo, "w");  
	int bytes_size = 0;  
	int pos = 0;
	for (int i = 0; i < zip->fileCount; i++)
	{
		FILE* f = fopen(zip->files[i], "rb");
		if (!f) break;

		fseek(f, 0, SEEK_END);
		int size = ftell(f);

		char* name = getFileName(zip->files[i]);

		char* m_size = (char*)malloc(sizeof(char) * digs(size));
		_itoa(size, m_size, 10);
		fclose(f);

		bytes_size += digs(size);
		bytes_size += strlen(name);
		bytes_size += 4;
		s_info = (char*)realloc(s_info, (sizeof(char) * (bytes_size + 4)));
		s_info[pos] = 0;
		strcat(s_info, m_size);
		strcat(s_info, "||");
		strcat(s_info, name);
		strcat(s_info, "||");
		pos = strlen(s_info);
		m_size = NULL;
		free(m_size);

	}
	bytes_size = strlen(s_info) + 2;
	char* b_buff = (char*)malloc(sizeof(char) * digs(bytes_size));
	_itoa(bytes_size, b_buff, 10);

	
	if (digs(bytes_size) < 5)
	{
		for (int i = 0; i < 5 - digs(bytes_size); i++)
			fputs("0", info);
	}
	fputs(b_buff, info);
	fputs("||", info);
	fputs(s_info, info);

	fclose(info);
};

void InCompress(Zipper* zip)
{
	char byte[1]; 

	getInfo(zip);  

	char fileInfo[MAX_LENGTH];
	strcpy(fileInfo, zip->fileName);
	strcat(fileInfo, "info.txt");
	FILE* f;
	FILE* main = fopen(zip->fileName, "wb");  
	FILE* info = fopen(fileInfo, "rb");  
	while (!feof(info))
	{
		if (fread(byte, 1, 1, info) == 1)
			fwrite(byte, 1, 1, main);
	}
	fclose(info);
	remove(fileInfo);
	for (int i = 0; i < zip->fileCount; i++)
	{
		f = fopen(zip->files[i], "rb");
		if (!f)
		{
			printf("Файл %s не найден!\n", zip->files[i]);
			break;
		}

		while (!feof(f))
			if (fread(byte, 1, 1, f) == 1)
				fwrite(byte, 1, 1, main);

		printf("%s добавлен в архив %s.\n", zip->files[i], zip->fileName);
		fclose(f);
	}
	fclose(main);
}

void Recompress(char* fileName, int isView)
{
	FILE* bin = fopen(fileName, "rb");	
	char info_block_size[5];			
	fread(info_block_size, 1, 5, bin);  
	int _sz = atoi(info_block_size);	

	char* info_block = (char*)malloc(sizeof(char) * _sz);	
	fread(info_block, 1, _sz, bin);		

	char** tokens = (char**)malloc(sizeof(char*));
	char* tok = strtok(info_block, "||");
	int toks = 0;
	while (tok)
	{
		if (strlen(tok) == 0)
			break;
		tokens = (char**)realloc(tokens, sizeof(char*) * (toks + 1));
		tokens[toks] = (char*)malloc(sizeof(char) * strlen(tok));
		strcpy(tokens[toks], tok);
		tok = strtok(NULL, "||");
		toks++;
	}

	if (toks % 2 == 1)
		toks--;  
	int files = toks / 2;  

	char byte[1];   
	for (int i = 0; i < files; i++)
	{
		char* size = tokens[i * 2];
		char* name = tokens[i * 2 + 1];
		int _sz = atoi(size);
		if (isView)
		{
			printf("%d %s\n", i + 1, name);
		}
		else
		{
			printf("%s извлечен.\n", name);
			FILE* curr = fopen(name, "wb");
			for (int r = 1; r <= _sz; r++)
			{
				if (fread(byte, 1, 1, bin) == 1)
					fwrite(byte, 1, 1, curr);
			}
			fclose(curr);
		}
		remove(size);
	}
	fclose(bin);
}


int main(int argv, char* argc[])
{
	setlocale(0, "RU");
	if (argv > 1)
	{
		char** files = (char**)malloc(sizeof(char**)); 
		char path[MAX_LENGTH] = { 0 }; 
		int flag_fs = 0, flag_path = 0;  
		char type[10];
		type[9] = '\0';
		int fileCount = 0;
		int isView = 0;
		memset(type, 0, 9);

		for (int i = 1; i < argv; i++)
		{
			if (strcmp(argc[i], "--create") == 0)
			{
				strcpy(type, "create");
				flag_fs = 1;
				flag_path = 0;
				continue;
			}
			if (strcmp(argc[i], "--extract") == 0)
			{
				strcpy(type, "extract");
				flag_fs = flag_path = 0;
			}
			if (strcmp(argc[i], "--file") == 0)
			{
				flag_path = 1;
				flag_fs = 0;
				continue;
			}
			if (strcmp(argc[i], "--list") == 0)
			{
				strcpy(type, "extract");
				flag_path = 1;
				isView = 1;
				continue;
			}

			if (flag_path)
				strcpy(path, argc[i]);

			if (flag_fs)
			{
				files = (char**)realloc(files, sizeof(char*) * (fileCount + 1));
				files[fileCount] = (char*)malloc(sizeof(char) * strlen(argc[i]));
				strcpy(files[fileCount], argc[i]);
				fileCount++;
			}
		}

		if (strcmp(type, "create") == 0)
		{
			Zipper* zip = (Zipper*)malloc(sizeof(Zipper));
			init(zip, files, fileCount, path);
			InCompress(zip);
		}
		if (strcmp(type, "extract") == 0)
			Recompress(path, isView);
	}
	else
		printf("Not enough args!\n");
	return 0;
}