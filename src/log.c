
#include <string.h>

int printk_hex(unsigned char *buff, unsigned int count, char *str_val)
{
	static const char hex[] = "0123456789abcdef";
	unsigned int i, j;
	char str[16];
	//char str_val[75];
	unsigned int index = 0;
	unsigned int str_index = 0;
	unsigned int cnt;

	if(count % 16)
	{
		cnt = count / 16 + 1;
	}
	else
	{
		cnt = count / 16;
	}

	for(i = 0; i < cnt; i++)
	{
		index = 0;
		str_index = 0;
		str_val[index++] = hex[(i & 0xf000) >> 12];
		str_val[index++] = hex[(i & 0xf00) >> 8];
		str_val[index++] = hex[(i & 0xf0) >> 4];
		str_val[index++] = hex[i & 0xf];
		str_val[index++] = ' ';
		str_val[index++] = ' ';

		for(j = 0; j < 16; j++)
		{
			if(j + (i << 4) < count)
			{
				str_val[index++] = hex[(buff[j + (i << 4)] & 0xf0) >> 4];
				str_val[index++] = hex[buff[j + (i << 4)] & 0xf];
				str_val[index++] = ' ';
				if((buff[j] >= 0x20) && (buff[j] <= 0x7e))
				{
					str[str_index++] = buff[j];
				}
				else
				{
					str[str_index++] = '.';
				}
			}
			else
			{
				str_val[index++] = ' ';
				str_val[index++] = ' ';
				str_val[index++] = ' ';
				str[str_index++] = ' ';
			}
		}
		str_val[index++] = ' ';
		str_val[index++] = ' ';
		memcpy(&str_val[index], str, str_index);
		index += str_index;
		str_val[index++] = '\n';
		str_val[index++] = '\0';
	}
	return 0;
}
