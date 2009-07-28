
//#define DoMethod(obj,...) obj==null ? printf("%s : %d NULL OBJECT!!!\n",__FILE__,__LINE__) : DoMethod(x,...);

class Vector {
	int capacity;
	int size;
	void ** data;
public:
	Vector(int capacity);
	Vector();
	~Vector();
	void ** getData() {
		return data;
	}
	int getSize();
	void add(void * element);
	void *getElementAt(int i);
	void removeElement(void * element);
	void removeElementAt(int i);
	void flush();
};

//#define _DEBUG

#ifdef _DEBUG

class Vector;

class AllocInfo {
public:
        unsigned int address;
        unsigned int size;
        char file[256];
        int line;
        static Vector allocs;
        AllocInfo(unsigned int address,unsigned int size, const char *file, int line);
};

//void AddTrack(DWORD ptr, unsigned int size, const char *file, int line);
void AddTrack(unsigned int ptr, unsigned int size, const char *file, int line) {
        AllocInfo *allocInfo = new AllocInfo(ptr,size,file,line);
        AllocInfo::allocs.add(allocInfo);
}

BOOL RemoveTrack(unsigned int ptr,int size);
void dumpAllocs();

inline void * /*__cdecl*/ operator new(unsigned int size, const char *file, int line) {
        void *ptr = (void *)malloc(size);
        AddTrack((unsigned int)ptr, size, file, line);
        return(ptr);
};
/*inline void * operator new[](unsigned int size, const char *file, int line) {
        void *ptr = (void *)malloc(size);
        AddTrack((unsigned int)ptr, size, file, line);
        return(ptr);
};*/
inline void operator delete(void *ptr, size_t size) {
        RemoveTrack((unsigned int)ptr,size);
        free(ptr);
};
/*inline void operator delete[](void *ptr) {
        RemoveTrack((unsigned int)ptr);
        free(ptr);
};*/

//inline void * malloc_track(unsigned int size) {
inline void * malloc_track(unsigned int size, const char *file, int line) {
        void *ptr = (void *)malloc(size);
        AddTrack((unsigned int)ptr, size, file, line);
        //AddTrack((DWORD)ptr, size, "unknown", -1);
        return(ptr);
};
inline void free_track(void *ptr) {
        RemoveTrack((unsigned int)ptr,-1);
        free(ptr);
};
#endif

#ifdef _DEBUG
        #define DEBUG_NEW new(__FILE__, __LINE__)
        #define malloc( x ) malloc_track( x, __FILE__, __LINE__ )
        #define free free_track
#else
        #define DEBUG_NEW new
#endif
#define new DEBUG_NEW
