#include <stdio.h>
#include <stdint.h>

#define BITARRAYSIZE 757693       // ����λ�����С��ֵΪ����

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
	hash = hash % (BITARRAYSIZE - 1);

	return hash;
}

int main() {

}