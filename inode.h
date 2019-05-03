#ifndef INODE_H
#define INODE_H


#include <vector>

class iNode{
public:
  std::string fileName;
  int size;
  int dataBlock[128];
};


#endif
