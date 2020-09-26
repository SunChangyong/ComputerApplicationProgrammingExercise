#include <stdio.h>
#include <stdint.h>

#define BITARRAYSIZE 757693       // 定义位数组大小，值为素数

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
	hash = hash % (BITARRAYSIZE - 1);

	return hash;
}

int main() {

}