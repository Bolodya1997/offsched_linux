#include <linux/kernel.h>

#define DATA_SIZE (1024 * 3)

int offsched_log(const char *entry);

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

	offsched_log("*** OFFSCHED LOG ***");
}

int offsched_log(const char *entry)
{
	int i, tmp;

	for (i = 0; log.pos + i < DATA_SIZE; i++) {
		log.data[log.pos + i] = entry[i];

		if (entry[i] == 0)
			break;
	}

	log.pos += i;

	/* skip to new line */
	tmp = (log.pos - 4) % 0x10;
	if (tmp)
		log.pos += 0x10 - tmp;

	return i;
}
EXPORT_SYMBOL_GPL(offsched_log);
