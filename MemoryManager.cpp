#include "MemoryManager.h"

// Node functions
Node::Node(size_t length, size_t offset, char* ptr, bool isBlock) {
    this->length = length;
    this->offset = offset;
    this->ptr = ptr;
    this->isBlock = isBlock;
}


// Memory Manager functions
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator) {
    this->wordSize = wordSize;
    this->allocator = allocator;
}

MemoryManager::~MemoryManager() {
    shutdown();
}

void MemoryManager::initialize(size_t sizeInWords) {
    if (sizeInWords > 65536) {
        return;
    }

    if (memoryBlock != nullptr) {
        shutdown();
    }

    this->memBlockWordSize = sizeInWords;

    totalSize = sizeInWords*wordSize;
    memoryBlock = new char[totalSize];

    //memoryMap.clear();
    Node initialHole = {sizeInWords, 0, memoryBlock, false};
    memoryMap.push_back(initialHole);

}

void MemoryManager::shutdown() {
    if (this->memoryBlock != nullptr) {
        delete[] memoryBlock;
        memoryBlock = nullptr;
    }

    memoryMap.clear();
    totalSize = 0;

    // anything else?

}

void* MemoryManager::allocate(size_t sizeInBytes) {
    if (memoryBlock == nullptr) {
        return nullptr;
    }

    size_t newSizeInWords = 0;
    if (sizeInBytes % wordSize != 0) {
        newSizeInWords = (sizeInBytes/wordSize) + 1;
    }
    else {
        newSizeInWords = sizeInBytes/wordSize;
    }
    
    int offsetVal = allocator(newSizeInWords, getList());

    // no suitable hole found 
    if (offsetVal == -1) {
        return nullptr;
    }

    for (auto& node : memoryMap) {
        // find hole at offset
        if (node.offset == offsetVal && !node.isBlock) {
            // split node
            node.isBlock = true;
            if (newSizeInWords < node.length) {
                Node newHole(node.length - newSizeInWords, offsetVal + newSizeInWords, node.ptr + newSizeInWords*wordSize, false);
                node.length = newSizeInWords;
                memoryMap.insert(memoryMap.begin() + (&node - &memoryMap[0]) + 1, newHole);
            }
            return node.ptr;
        }
    }

    return nullptr;
}

void MemoryManager::free(void *address) {
    if (memoryBlock == nullptr) {
        return;
    }

    bool found = false;
    for (auto& node : memoryMap) {
        // find node at address and free
        if (node.ptr == address && node.isBlock) {
            node.isBlock = false;
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }


    // merge holes and delete other nodes
    for (size_t i = 0; i < (memoryMap.size() - 1); ) {
        // check if current node and next node are holes, if so merge
        if (i + 1 < memoryMap.size() && !memoryMap.at(i).isBlock && !memoryMap.at(i+1).isBlock) {
            memoryMap.at(i).length += memoryMap.at(i+1).length;
            memoryMap.erase(memoryMap.begin() + i + 1);
        }
        else {
            // move to next node if no merge performed
            i++; 
        }
    }

}

void MemoryManager::setAllocator(std::function<int(int, void *)> allocator) {
    this->allocator = allocator;
}

int MemoryManager::dumpMemoryMap(char *filename) {
    return 0;
}

void* MemoryManager::getList() {
    if (memoryBlock == nullptr) {
        return nullptr;
    }

    // get number of holes and hole list
    size_t numHoles = 0;  
    for (const auto& node : memoryMap) {
        if (!node.isBlock) {
            numHoles++;            
        }
    }

    uint16_t* holesArr = new uint16_t[(numHoles*2) + 1];
    holesArr[0] = numHoles;

    int ind = 1;
    for (const auto& node : memoryMap) {
        if (!node.isBlock) {
            holesArr[ind++] = static_cast<uint16_t>(node.offset);
            holesArr[ind++] = static_cast<uint16_t>(node.length);       
        }
    }

    // may need to add little endian conversion if not automatic
    /*for (int i = 1; i < ind; i++) {
        holesArr[i] = (holesArr[i] >> 8) | (holesArr[i] << 8);
    } */

    return holesArr;

}

void* MemoryManager::getBitmap() {
    if (memoryBlock == nullptr) {
        return nullptr;
    }

    for (const auto& node : memoryMap) {
        size_t start = node.offset/wordSize;
        size_t end = start + node.length;
    }

}

unsigned MemoryManager::getWordSize() {
    if (this->memoryBlock == nullptr) {
        return -1;
    }
    return this->wordSize;
}

void* MemoryManager::getMemoryStart() {
    if (this->memoryBlock == nullptr) {
        return nullptr;
    }
    return this->memoryBlock;
}

unsigned MemoryManager::getMemoryLimit() {
    if (this->memoryBlock == nullptr) {
        return -1;
    }
    return this->totalSize;
}


// other functions
int bestFit(int sizeInWords, void *list) {
    uint16_t* holes = static_cast<uint16_t*>(list);
    if (holes == nullptr || holes[0] == 0) {
        return -1;
    }

    int bestSize = SIZE_MAX;
    int bestOffset = -1;

    for (size_t i = 1; i < (holes[0] * 2); i +=2) {
        uint16_t offset = holes[i];
        uint16_t length = holes[i + 1];

        // check if holes fits size and is smaller than curr hole size
        if (length >= sizeInWords && length < bestSize) {
            bestSize = length;
            bestOffset = offset;
        }
    }

    return bestOffset;
}

int worstFit(int sizeInWords, void *list) {
    uint16_t* holes = static_cast<uint16_t*>(list);
    if (holes == nullptr || holes[0] == 0) {
        return -1;
    }

    int bestSize = 0;
    int bestOffset = -1;

    for (size_t i = 1; i < (holes[0] * 2); i +=2) {
        uint16_t offset = holes[i];
        uint16_t length = holes[i + 1];

        // check if holes fits size and is smaller than curr hole size
        if (length >= sizeInWords && length > bestSize) {
            bestSize = length;
            bestOffset = offset;
        }
    }

    return bestOffset;
}