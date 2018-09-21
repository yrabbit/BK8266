#ifndef UTF8_KOI8_H_INCLUDE
#define UTF8_KOI8_H_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

void URL_UTF8_To_KOI8 (char *pKOI8, const char *pURL);
void KOI8_To_UTF8     (char *pUTF8, const char *pKOI8);

#ifdef __cplusplus
}
#endif

#endif
