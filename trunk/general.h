BOOL ShowGuide(char *node,LONG line);
BOOL LoadASL(char *filename,const char *title,const char *ifile,const char *pattern,BOOL folders);

#define MAXFILENAME (1024)
/* OS 4 IFace translation defs that don't get defined for cplusplus */
#ifdef __amigaos4__
#define OpenAmigaGuideAsync(...) IAmigaGuide->OpenAmigaGuideAsync(__VA_ARGS__)
#endif

