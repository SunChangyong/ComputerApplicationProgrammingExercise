#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#define HASHTABLESIZE 757693       // �����ϣ���С��ֵΪ����

// ��ϣ���е�Ԫ��
struct HashElem {
	char* data;
	struct HashElem* next;
};

// ��ϣ��
struct HashTable {
	struct HashElem* entry[HASHTABLESIZE];
};

// ��ʼ����ϣ��
void initHashTable(struct HashTable* t) {
	int i;
	if (t == NULL) return;

	for (i = 0; i < HASHTABLESIZE; ++i) {
		t->entry[i] = (struct HashElem*)malloc(sizeof(struct HashElem));
		t->entry[i]->data = NULL;
		t->entry[i]->next = NULL;
	}
}

// �ݹ��ͷ�������ڴ�
struct HashElem* freeNext(struct HashElem* elem) {
	if (elem == NULL) return NULL;
	elem->next = freeNext(elem->next);  // elem->next���ͷź���Ҫ��ΪNULL
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

// �ͷŹ�ϣ���ڴ�
void freeHashTable(struct HashTable* t) {
	int i;
	struct HashElem* cur;
	if (t == NULL) return;

	for (i = 0; i < HASHTABLESIZE; ++i) {
		cur = t->entry[i];
		if (cur->data == NULL) continue;
		free(cur->data);
		freeNext(cur->next);  // �ݹ�ʵ��
		cur->data = NULL;
		cur->next = NULL;
	}
}

// DJB2 ��ϣ����
size_t DJB2Hash(const char* str)
{
	if (!*str)   // �����ɱ�����ӣ��Ա�֤���ַ������ع�ϣֵ0  
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

// ��ϣ�����Ԫ��
int insertHashTable(struct HashTable* t, const char* value) {
	if (t == NULL || value == NULL) {  // ����ʧ��
		printf("hashtable is null or value is null!\n");
		return 0;
	}

	int index;
	struct HashElem* entry, * tmp, * cur;
	index = DJB2Hash(value);
	// ��ϣ����û�и�Ԫ��
	if (t->entry[index]->data == NULL) {
		t->entry[index]->data = strdup(value);
		return 1;
	}
	// ��ϣ�����Ѵ��ڸ�Ԫ�أ���������ַ�������ͻ��ʹ��ǰ����뷨
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

// ��ϣ�����Ԫ��
int searchHashTable(const struct HashTable* t, const char* value) {
	if (t == NULL || value == NULL) {  // ����ʧ��
		printf("hashtable is null or value is null!\n");
		return 0;
	}
	
	int index;
	struct HashElem* entry;
	index = DJB2Hash(value);
	entry = t->entry[index];
	if (entry->data == NULL) return 0;  // ��Ͱ�л�û��Ԫ��
	while (entry != NULL) {
		if (strcmp(entry->data, value) == 0) return 1;  // �ҵ���ӦԪ��
		entry = entry->next;
	}
	return 0;  // û���ҵ���ӦԪ��
}

// ��ӡ��ϣ���е�����Ԫ��
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

// ���ļ�
FILE* openFile(const char* fileName, const char* mode) {
	FILE* fp = fopen(fileName, mode);

	// ����ļ���ʧ��
	if (fp == NULL) {
		printf("Fail to open file!\n");
		exit(0);
	}
	return fp;
}

//�ر��ļ�
void closeFile(FILE* fp) {
	fclose(fp);
}

int main() {
	double start, finish;
	start = (double)clock();  // ��ʼʱ��
	struct HashTable* t = (struct HashTable*)malloc(sizeof(struct HashTable));
	initHashTable(t);

	FILE* fpRead, * fpWrite;
	fpRead = openFile("./dict.txt", "r");
	char tmp[100] = { 0 };

	while (!feof(fpRead)) {
		fgets(tmp, 99, fpRead);
		tmp[strlen(tmp) - 1] = '\0';  // �������� '\n' ���з�
		insertHashTable(t, tmp);
		memset(tmp, '\0', 100);  // ����tmp�����Ը���
	}
	closeFile(fpRead);

	fpRead = openFile("./string.txt", "r");
	fpWrite = openFile("./result.txt", "w");
	int count = 0;
	while (!feof(fpRead)) {
		if (fgets(tmp, 99, fpRead) == NULL) continue;
		tmp[strlen(tmp) - 1] = '\0';
		if (searchHashTable(t, tmp)) {
			fputs(tmp, fpWrite);  // ��Ŀ���ļ�д���ַ���
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