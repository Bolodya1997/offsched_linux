#include <linux/kernel.h>

#define DATA_SIZE (1024 * 3)

int offsched_log_str(const char *str);

volatile bool offsched_flags[10];

volatile unsigned long long offsched_count;

static struct {
	int pos;
	char data[DATA_SIZE];
} log;

void __init offsched_log_init(void)
{
	int i;

	log.pos = 0;

	for (i = 0; i < DATA_SIZE; i++) {
		log.data[i] = 0;
	}

	offsched_log_str("*** OFFSCHED LOG ***");
}

int offsched_log_str(const char *str)
{
	int i;

	for (i = 0; log.pos + i < DATA_SIZE; i++) {
		log.data[log.pos + i] = str[i];

		if (str[i] == 0)
			break;
	}

	log.pos += i;

	return i;
}
EXPORT_SYMBOL_GPL(offsched_log_str);

int offsched_log_raw(void *ptr, int bytes)
{
	int i;
	char *raw = (char *) ptr;

	for (i = 0; log.pos + i < DATA_SIZE && i < bytes; i++) {
		log.data[log.pos + i] = raw[i];
	}

	log.pos += i;

	return i;
}
EXPORT_SYMBOL_GPL(offsched_log_raw);

void offsched_log_nl(void)
{
	int tmp;

	tmp = (log.pos - 4) % 0x10;
	if (tmp)
		log.pos += 0x10 - tmp;
}
EXPORT_SYMBOL_GPL(offsched_log_nl);
