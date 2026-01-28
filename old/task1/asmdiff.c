// Program to compare the output of 'objdump -d' of two ELF files.
// Run for example:
//   objdump -d prog1 > prog1.txt
//   objdump -d prog2 > prog2.txt
//   asmdiff prog1.txt prog2.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct offset_t
{
    int diff;
    int min_v1;
    int max_v1;
    int min_v2;
    int max_v2;
} offsets[10];
int nr_offsets = 0;

int compare_diff(const void *a, const void *b)
{
    struct offset_t *offset_a = (struct offset_t *)a; 
    struct offset_t *offset_b = (struct offset_t *)b;
    return offset_a->diff - offset_b->diff; 
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Requires two filenames as argument\n");
        return -1;
    }

    FILE *f1 = fopen(argv[1], "r");
    if (f1 == NULL)
    {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
        return -1;
    }
    FILE *f2 = fopen(argv[2], "r");
    if (f2 == NULL)
    {
        fprintf(stderr, "Cannot open %s\n", argv[2]);
        return -1;
    }

    for (int line_nr = 1; ; line_nr++)
    {
        char line1[200];
        char* more1 = fgets(line1, 199, f1);
        char line2[200];
        char* more2 = fgets(line2, 199, f2);
        if (more1 == NULL && more2 == NULL)
            break;
        if (more1 == NULL || more2 == NULL)
        {
            fprintf(stderr, "Line %d: Files have different length\n", line_nr);
            return -1;
        }
        if (strlen(line1) != strlen(line2))
        {
            printf("Line %d: Difference:\n1: %s2: %s\n", line_nr, line1, line2);
        }
        else if (strcmp(line1, line2) != 0)
        {
            char *s1 = line1 + 31;
            char *s2 = line2 + 31;
            while (*s1 != '\0' && *s2 != '\0')
                if (strncmp(s1, "0x", 2) == 0 && strncmp(s2, "0x", 2) == 0)
                {
                    s1 += 2;
                    int v1 = 0;
                    for (;;)
                        if ('0' <= *s1 && *s1 <= '9')
                            v1 = 16 * v1 + *s1++ - '0';
                        else if ('a' <= *s1 && *s1 <= 'f')
                            v1 = 16 * v1 + *s1++ + (10 - 'a');
                        else
                            break;
                    s2 += 2;
                    int v2 = 0;
                    for (;;)
                        if ('0' <= *s2 && *s2 <= '9')
                            v2 = 16 * v2 + *s2++ - '0';
                        else if ('a' <= *s2 && *s2 <= 'f')
                            v2 = 16 * v2 + *s2++ + (10 - 'a');
                        else
                            break;
                    int diff = v2 - v1;
                    if (diff != 0)
                    {
                        int i = 0;
                        for (; i < nr_offsets; i++)
                            if (offsets[i].diff == diff)
                            {
                                if (v1 < offsets[i].min_v1)
                                    offsets[i].min_v1 = v1;
                                if (v1 > offsets[i].max_v1)
                                    offsets[i].max_v1 = v1;
                                if (v2 < offsets[i].min_v2)
                                    offsets[i].min_v2 = v2;
                                if (v2 > offsets[i].max_v2)
                                    offsets[i].max_v2 = v2;
                                break;
                            }
                        if (i == nr_offsets)
                        {
                            offsets[i].diff = diff;
                            offsets[i].min_v1 = v1;
                            offsets[i].max_v1 = v1;
                            offsets[i].min_v2 = v2;
                            offsets[i].max_v2 = v2;
                            nr_offsets++;
                        }
                    }
                }
                else if (*s1 != *s2)
                {
                    printf("line_nr %d: difference:\n1: %s2: %s", line_nr, s1, s2);
                    return -1;
                }
                else
                {
                    s1++;
                    s2++;
                }
        }
    }

    // Sort regions by offset
    qsort(offsets, nr_offsets, sizeof(struct offset_t), compare_diff);

    int min_v1 = offsets[0].min_v1;
    int min_v2 = offsets[0].min_v2;
    for (int i = 0; i < nr_offsets; i++)
    {
        printf("Offset %3d for %08x to %08x - %08x to %08x\n",
            offsets[i].diff, offsets[i].min_v1, offsets[i].max_v1, offsets[i].min_v2, offsets[i].max_v2);
        if (offsets[i].min_v1 < min_v1)
            min_v1 = offsets[i].min_v1;
        if (offsets[i].min_v2 < min_v2)
            min_v2 = offsets[i].min_v2;
    }

    printf("min_v1 = %08x\nmin_v2 = %08x\n\n", min_v1, min_v2);
    for (int i = 1; i < nr_offsets; i++)
        for (int j = 0; j < i; j++)
        {
            if (   offsets[j].max_v1 < offsets[i].min_v1
                && offsets[j].max_v2 < offsets[i].min_v2)
                ; // Region j is totally before region i
            else if (   offsets[i].max_v1 < offsets[j].min_v1
                     && offsets[i].max_v2 < offsets[j].min_v2)
                ; // Region i is totally before region j
            else
            {
                printf("Regions with offset %d - %d do overlap\n", offsets[i].diff, offsets[j].diff);
                return -1;
            }
        }
    
    printf("None of the regions overlap\n");

    return 0;
}
