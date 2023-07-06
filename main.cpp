#include "SkipList.h"

using namespace std;

int main()
{
    SkipList<int,string> sl(6);

    sl.insertElement(100,"99");
    sl.insertElement(1000,"999");
    sl.insertElement(10,"9");
    sl.insertElement(1,"0");

    sl.displayList();

    sl.searchElement(1);
    
    sl.deleteElement(1);

    sl.insertElement(1,"0");

    sl.inDisk();
    
    SkipList<int,string> newsl(6);
    newsl.outDisk();
    newsl.displayList();

    return 0;
}