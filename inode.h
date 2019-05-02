#ifndef INODE_H
#define INODE_H


#include <vector>

class iNode{
  public:
    std::string fileName;
    int size;
    std::vector<int> dataBlock{128};
};

#endif
