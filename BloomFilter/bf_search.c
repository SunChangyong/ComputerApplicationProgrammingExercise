#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#define STRINGNUMS 1270574       // 定义需要存储的字符串个数

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE inline __attribute__((always_inline))

inline uint32_t rotl32(uint32_t x, int8_t r)
{
	return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64(uint64_t x, int8_t r)
{
	return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32_t getblock32(const uint32_t* p, int i)
{
	return p[i];
}

FORCE_INLINE uint64_t getblock64(const uint64_t* p, int i)
{
	return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint32_t fmix32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	return h;
}

//----------

FORCE_INLINE uint64_t fmix64(uint64_t k)
{
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xff51afd7ed558ccd);
	k ^= k >> 33;
	k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
	k ^= k >> 33;

	return k;
}

// MurmurHash3 哈希函数，返回32字节的无类型整数
void MurmurHash3_x86_32(const void* key, int len,
	uint32_t seed, void* out)
{
	const uint8_t* data = (const uint8_t*)key;
	const int nblocks = len / 4;

	uint32_t h1 = seed;

	const uint32_t c1 = 0xcc9e2d51;
	const uint32_t c2 = 0x1b873593;

	//----------
	// body

	const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);

	for (int i = -nblocks; i; i++)
	{
		uint32_t k1 = getblock32(blocks, i);

		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64;
	}

	//----------
	// tail

	const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);

	uint32_t k1 = 0;

	switch (len & 3)
	{
	case 3: k1 ^= tail[2] << 16;
	case 2: k1 ^= tail[1] << 8;
	case 1: k1 ^= tail[0];
		k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
	};

	//----------
	// finalization

	h1 ^= len;

	h1 = fmix32(h1);

	*(uint32_t*)out = h1;
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
	const double fp = 0.0000001;  // 设置允许错误率
	const int bitSize = (int)(STRINGNUMS * 1.44 * (log(1 / fp) / log(2)));  // 计算最优向量bit位数
	const int byteSize = ceil(bitSize / 8);  // 计算需要分配的字节数
	const int hashFuncNums = (int)(0.693 * bitSize / STRINGNUMS);  // 计算需要的哈希函数个数
	int8_t* bitArray = (int8_t*)calloc(byteSize, 1);  // 分配位数组的空间

	// 创建布隆过滤器
	FILE* fpRead, * fpWrite;
	fpRead = openFile("./dict.txt", "r");
	char tmp[100] = { 0 };
	uint32_t hashVal = 0;
	int bitPos = 0;
	int offset = 0;

	while (!feof(fpRead)) {
		fgets(tmp, 99, fpRead);
		tmp[strlen(tmp) - 1] = '\0';  // 放弃最后的 '\n' 换行符
		for (int i = 0; i < hashFuncNums; ++i) {
			MurmurHash3_x86_32(tmp, strlen(tmp), i, &hashVal);
			hashVal = hashVal % bitSize;
			offset = floor(hashVal / 8);
			bitPos = hashVal % 8;
			bitArray[offset] |= (0x01 << bitPos);
		}
		memset(tmp, '\0', 100);  // 重置tmp数组以复用
	}
	closeFile(fpRead);

	// 搜索布隆过滤器
	fpRead = openFile("./string.txt", "r");
	fpWrite = openFile("./result.txt", "w");
	int count = 0;
	while (!feof(fpRead)) {
		if (fgets(tmp, 99, fpRead) == NULL) continue;
		tmp[strlen(tmp) - 1] = '\0';
		int flag = 1;
		for (int i = 0; i < hashFuncNums; ++i) {
			MurmurHash3_x86_32(tmp, strlen(tmp), i, &hashVal);
			hashVal = hashVal % bitSize;
			offset = floor(hashVal / 8);
			bitPos = hashVal % 8;
			if ((bitArray[offset] & (0x01 << bitPos)) == 0x00)	flag = 0;  // 若有一个哈希函数不满足，则元素不在
		}
		if (flag) {
			fputs(tmp, fpWrite);  // 向目标文件写入字符串
			fputc('\n', fpWrite);
			++count;
		}
		memset(tmp, '\0', 100);
	}
	printf("Found: %d\n", count);
	closeFile(fpRead);
	closeFile(fpWrite);

	free(bitArray);
	finish = (double)clock();
	printf("Time: %.4f ms\n", finish - start);
	getchar();
	return 0;
}