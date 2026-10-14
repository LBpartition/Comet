#include "gba_utils.h"
#include <dirent.h>
#include <strings.h>
#include <sys/stat.h>

static int gba_pair_cmp_newest_first(const void *a, const void *b)
{
    const ScreenshotPair *pa = (const ScreenshotPair *)a;
    const ScreenshotPair *pb = (const ScreenshotPair *)b;
    return strcmp(pb->timestamp, pa->timestamp);
}

static int scan_gba_dir(const char *dirPath, ScreenshotPair *out, int count, int max)
{
    DIR *dir = opendir(dirPath);
    if (!dir) return count;

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL && count < max) {
        const char *name = ent->d_name;
        size_t len = strlen(name);
        if (len < 5 || strcasecmp(name + len - 4, ".bmp") != 0) continue;

        // Open_AGB_Firm names captures YYYY_MM_DD_HH_MM_SS.bmp.
        if (len != 23 ||
            name[4] != '_' || name[7] != '_' ||
            name[10] != '_' || name[13] != '_' ||
            name[16] != '_' || name[19] != '.') continue;

        ScreenshotPair *p = &out[count++];
        memset(p, 0, sizeof(*p));
        size_t timestampLen = len - 4;
        memcpy(p->timestamp, name, timestampLen);
        p->timestamp[timestampLen] = '\0';
        snprintf(p->topPath, sizeof(p->topPath), "%s/%s", dirPath, name);
    }
    closedir(dir);

    return count;
}

int gba_scan_screenshots(ScreenshotPair *out, int max)
{
    char legacyRoot[320], lumaRoot[320];
    snprintf(legacyRoot, sizeof(legacyRoot), "%s%s", SD_ROOT, GBA_SCREENSHOTS_DIR);
    snprintf(lumaRoot, sizeof(lumaRoot), "%s%s", SD_ROOT, GBA_SCREENSHOTS_LUMA_DIR);

    int count = scan_gba_dir(legacyRoot, out, 0, max);
    count = scan_gba_dir(lumaRoot, out, count, max);
    if (count > 1) qsort(out, count, sizeof(ScreenshotPair), gba_pair_cmp_newest_first);
    return count;
}
