#include <functional>
#include <vector>
using namespace std;

struct Node {
    size_t length;
    size_t offset;
    char *ptr;
    bool isBlock;

    Node(size_t length, size_t offset, char* ptr, bool isBlock);
};

class MemoryManager {
    public:
    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void *allocate(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void *getMemoryStart();
    unsigned getMemoryLimit();

    private:
    unsigned wordSize;
    std::function<int(int, void *)> allocator;
    char *memoryBlock;
    vector<Node> memoryMap;
    size_t totalSize;
    
};

int bestFit(int sizeInWords, void *list);
int worstFit(int sizeInWords, void *list);

