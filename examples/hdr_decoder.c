/**
 * hdr_decoder.c
 * Written by Michael Barker and released to the public domain,
 * as explained at http://creativecommons.org/publicdomain/zero/1.0/
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>

#include <hdr_histogram.h>
#include <hdr_histogram_log.h>

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

static int
single_log_decode(FILE *f, struct hdr_histogram **h)
{
	char	histobuf[64*1024];
	char	*cp;
	int	buflen;
	int	rc;

	cp = fgets(histobuf, sizeof(histobuf), f);
	if (cp == NULL)
		return -1;

	buflen = strlen(histobuf);
	if (histobuf[buflen-1] == '\n')
		histobuf[--buflen] = '\0';

	rc = hdr_log_decode(h, histobuf, buflen);
	if (rc < 0) {
		fprintf(stderr, "failed to decode single log entry: %s\n", hdr_strerror(rc));
	} else {
		hdr_percentiles_print(*h, stdout, 5, 1.0, CLASSIC);
		free(*h);
	}
	return rc;
}

int main(int argc, char** argv)
{
    int rc = 0;
    FILE* f;

    if (argc == 1)
    {
        f = stdin;
    }
    else
    {
        f = fopen(argv[1], "r");
    }

    if (!f)
    {
        fprintf(stderr, "Failed to open file(%s):%s\n", argv[1], strerror(errno));
        return -1;
    }

    struct hdr_log_reader reader;
    if (hdr_log_reader_init(&reader))
    {
        fprintf(stderr, "Failed to init reader\n");
        return -1;
    }

    struct hdr_histogram* h = NULL;
    hdr_timespec timestamp;
    hdr_timespec interval;

    rc = hdr_log_read_header(&reader, f);
    if(rc)
    {
        fprintf(stderr, "Failed to read header: %s\n", hdr_strerror(rc));

	/* try single hdr_log_decode */
	rc = single_log_decode(f, &h);
	goto out;
    }

    while (true)
    {
        rc = hdr_log_read(&reader, f, &h, &timestamp, &interval);

        if (0 == rc)
        {
            hdr_percentiles_print(h, stdout, 5, 1.0, CLASSIC);
	    free(h);
        }
        else if (EOF == rc)
        {
            break;
        }
        else
        {
            fprintf(stderr, "Failed to print histogram: %s\n", hdr_strerror(rc));
            return -1;
        }
    }

out:
    if (f != stdin)
    	fclose(f);

    return 0;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif
