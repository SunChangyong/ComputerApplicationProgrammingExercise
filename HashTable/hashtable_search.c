#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#define HASHTABLESIZE 757693       // 定义哈希表大小，值为素数

// 哈希表中的元素
struct HashElem {
	char* data;
	struct HashElem* next;
};

// 哈希表
struct HashTable {
	struct HashElem* entry[HASHTABLESIZE];
};

// 初始化哈希表
void initHashTable(struct HashTable* t) {
	int i;
	if (t == NULL) return;

	for (i = 0; i < HASHTABLESIZE; ++i) {
		t->entry[i] = (struct HashElem*)malloc(sizeof(struct HashElem));
		t->entry[i]->data = NULL;
		t->entry[i]->next = NULL;
	}
}

// 递归释放链表的内存
struct HashElem* freeNext(struct HashElem* elem) {
	if (elem == NULL) return NULL;
	elem->next = freeNext(elem->next);  // elem->next被释放后需要置为NULL
	if (elem->data != NULL) {
		free(elem->data);
	} 
	if (elem->next != NULL) {
		free(elem->next);
	} 
	elem->data = NULL;
	elem->next = NULL;
	return NULL;
}

// 释放哈希表内存
void freeHashTable(struct HashTable* t) {
	int i;
	struct HashElem* cur;
	if (t == NULL) return;

	for (i = 0; i < HASHTABLESIZE; ++i) {
		cur = t->entry[i];
		if (cur->data == NULL) continue;
		free(cur->data);
		freeNext(cur->next);  // 递归实现
		cur->data = NULL;
		cur->next = NULL;
	}
}

// DJB2 哈希函数
size_t DJB2Hash(const char* str)
{
	if (!*str)   // 这是由本人添加，以保证空字符串返回哈希值0  
		return 0;
	register size_t hash = 5381;
	size_t ch;
	while (ch = (size_t)*str++)
	{
		hash = hash * 33 ^ ch;
	}
	hash = hash % (HASHTABLESIZE - 1);

	return hash;
}

// 哈希表插入元素
int insertHashTable(struct HashTable* t, const char* value) {
	if (t == NULL || value == NULL) {  // 插入失败
		printf("hashtable is null or value is null!\n");
		return 0;
	}

	int index;
	struct HashElem* entry, * tmp, * cur;
	index = DJB2Hash(value);
	// 哈希表中没有该元素
	if (t->entry[index]->data == NULL) {
		t->entry[index]->data = strdup(value);
		return 1;
	}
	// 哈希表中已存在该元素，采用链地址法解决冲突，使用前向插入法
	else {
		entry = t->entry[index];
		tmp = entry->next;
		cur = (struct HashElem*)malloc(sizeof(struct HashElem));
		entry->next = cur;
		cur->next = tmp;
		cur->data = strdup(value);
		return 1;
	}
}

// 哈希表查找元素
int searchHashTable(const struct HashTable* t, const char* value) {
	if (t == NULL || value == NULL) {  // 查找失败
		printf("hashtable is null or value is null!\n");
		return 0;
	}
	
	int index;
	struct HashElem* entry;
	index = DJB2Hash(value);
	entry = t->entry[index];
	if (entry->data == NULL) return 0;  // 该桶中还没有元素
	while (entry != NULL) {
		if (strcmp(entry->data, value) == 0) return 1;  // 找到对应元素
		entry = entry->next;
	}
	return 0;  // 没有找到对应元素
}

// 打印哈希表中的所有元素
void printHashTable(struct HashTable* t) {
	if (t == NULL) return;
	int i;
	int count = 0;
	struct HashElem* entry;

	for (i = 0; i < HASHTABLESIZE; ++i) {
		entry = t->entry[i];
		if (entry != NULL) {
			if (entry->data != NULL) {
				printf("t->entry[%d].data: %s\n", i, entry->data);
				++count;
			}
			entry = entry->next;
		}
	}
	printf("count: %d\n", count);
}

// 打开文件
FILE* openFile(const char* fileName, const char* mode) {
	FILE* fp = fopen(fileName, mode);

	// 如果文件打开失败
	if (fp == NULL) {
		printf("Fail to open file!\n");
		exit(0);
	}
	return fp;
}

//关闭文件
void closeFile(FILE* fp) {
	fclose(fp);
}

int main() {
	double start, finish;
	start = (double)clock();  // 开始时间
	struct HashTable* t = (struct HashTable*)malloc(sizeof(struct HashTable));
	initHashTable(t);

	FILE* fpRead, * fpWrite;
	fpRead = openFile("./dict.txt", "r");
	char tmp[100] = { 0 };

	while (!feof(fpRead)) {
		fgets(tmp, 99, fpRead);
		tmp[strlen(tmp) - 1] = '\0';  // 放弃最后的 '\n' 换行符
		insertHashTable(t, tmp);
		memset(tmp, '\0', 100);  // 重置tmp数组以复用
	}
	closeFile(fpRead);

	fpRead = openFile("./string.txt", "r");
	fpWrite = openFile("./result.txt", "w");
	int count = 0;
	while (!feof(fpRead)) {
		if (fgets(tmp, 99, fpRead) == NULL) continue;
		tmp[strlen(tmp) - 1] = '\0';
		if (searchHashTable(t, tmp)) {
			fputs(tmp, fpWrite);  // 向目标文件写入字符串
			fputc('\n', fpWrite);
			++count;
		}
		memset(tmp, '\0', 100);
	}
	printf("Found: %d\n", count);
	closeFile(fpRead);
	closeFile(fpWrite);

	freeHashTable(t);
	finish = (double)clock();
	printf("Time: %.4f ms\n", finish - start);
	return 0;
}