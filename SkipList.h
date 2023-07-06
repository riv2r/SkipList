#include <iostream>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_PATH "/home/user/Desktop/SkipList/store/file"

std::mutex mtx;

/*
Node
+-------------------+
+   next[levels-1]  +
+   next[levels-2]  +
+       ....        +
+       next[0]     +
+       levels      +
+       key         +
+       value       +
+-------------------+
*/

template<typename K,typename V>
class Node
{
public:
    Node();
    ~Node();
public:
    Node(K,V,int);
    K getKey() const;
    V getValue() const;
    void setValue(V);
public:
    int levels;
    Node<K,V>** next;
private:
    K key;
    V value;
};

template<typename K,typename V>
Node<K,V>::~Node()
{
    delete[] next;
    next=nullptr;
}

template<typename K,typename V>
Node<K,V>::Node(K k,V v,int ls)
{
    this->key=k;
    this->value=v;
    this->levels=ls;
    this->next=new Node<K,V>*[ls];
    memset(this->next,0,sizeof(Node<K,V>*)*ls);
}

template<typename K,typename V>
K Node<K,V>::getKey() const
{
    return this->key;
}

template<typename K,typename V>
V Node<K,V>::getValue() const
{
    return this->value;
}

template<typename K,typename V>
void Node<K,V>::setValue(V v)
{
    this->value=v;
}


/*
+-----------------------+
+   next[mx_levels-1]   +
+   next[mx_levels-2]   +
+       ....            +
+       next[0]         +
+       mx_levels       +
+       key             +
+       value           +
+-----------------------+
+       cur_level       +
+       cnt             +
+       mx_levels       +
+-----------------------+
*/

template<typename K,typename V>
class SkipList
{
private:
    Node<K,V>* head;
    int mx_levels;  // max levels
    int cnt;        // element count
    int cur_level;  // highest level
    std::ofstream fileWriter;
    std::ifstream fileReader;
public:
    SkipList();
    ~SkipList();
public:
    SkipList(int);
    int getRandomLevel();
    Node<K,V>* createNode(K,V,int);
    bool insertElement(K,V);
    void displayList();
    bool searchElement(K);
    void deleteElement(K);
    void inDisk();
    void outDisk();
};

template<typename K,typename V>
SkipList<K,V>::~SkipList()
{
    if(fileWriter.is_open()) fileWriter.close();
    if(fileReader.is_open()) fileReader.close();
    delete head;
    head=nullptr;
}

template<typename K,typename V>
SkipList<K,V>::SkipList(int mx_ls)
{
    this->mx_levels=mx_ls;
    this->cnt=0;
    this->cur_level=0;
    K k;
    V v;
    this->head=new Node<K,V>(k,v,mx_ls);
}

template<typename K,typename V>
int SkipList<K,V>::getRandomLevel()
{
    int l=(rand()%this->mx_levels)+1;
    return l;
}

template<typename K,typename V>
Node<K,V>* SkipList<K,V>::createNode(K k,V v,int ls)
{
    Node<K,V>* n=new Node<K,V>(k,v,ls);
    return n;
}

template<typename K,typename V>
bool SkipList<K,V>::insertElement(K k,V v)
{
    mtx.lock();
    Node<K,V>* cur=this->head;
    Node<K,V>* update[this->mx_levels];
    memset(update,0,sizeof(Node<K,V>*)*this->mx_levels);

    for(int i=this->cur_level;i>=0;--i)
    {
        while(cur->next[i] && cur->next[i]->getKey()<k) cur=cur->next[i];
        update[i]=cur;
    }
    cur=cur->next[0];

    if(cur && cur->getKey()==k)
    {
        std::cout<<"key:"<<k<<" exists"<<std::endl;
        mtx.unlock();
        return false;
    }

    if(cur==nullptr || cur->getKey()!=k)
    {
        int l=getRandomLevel();
        if(l>this->cur_level)
        {
            for(int i=this->cur_level+1;i<l;++i) update[i]=this->head;
            this->cur_level=l-1;
        }

        Node<K,V>* insertedNode=createNode(k,v,l);

        for(int i=0;i<l;++i)
        {
            insertedNode->next[i]=update[i]->next[i];
            update[i]->next[i]=insertedNode;
        }

        std::cout<<"insert key:"<<k<<" value:"<<v<<" successfully"<<std::endl;
        ++this->cnt;
    }
    mtx.unlock();
    return true;
}

template<typename K,typename V>
void SkipList<K,V>::displayList()
{
    std::cout<<"*****Skip List*****"<<std::endl;
    for(int i=0;i<=this->cur_level;++i)
    {
        Node<K,V>* node=this->head->next[i];
        std::cout<<"level "<<i<<": ";
        while(node)
        {
            std::cout<<node->getKey()<<":"<<node->getValue()<<"; ";
            node=node->next[i];
        }
        std::cout<<std::endl;
    }
}

template<typename K,typename V>
bool SkipList<K,V>::searchElement(K k)
{
    std::cout<<"searching key:"<<k<<std::endl;
    Node<K,V>* cur=this->head;

    for(int i=this->cur_level;i>=0;--i)
    {
        while(cur->next[i] && cur->next[i]->getKey()<k) cur=cur->next[i];
    }

    cur=cur->next[0];

    if(cur && cur->getKey()==k)
    {
        std::cout<<"found key:"<<k<<" value:"<<cur->getValue()<<std::endl;
        return true;
    }

    std::cout<<"not found key:"<<k<<std::endl;
    return false;
}

template<typename K,typename V>
void SkipList<K,V>::deleteElement(K k)
{
    mtx.lock();
    Node<K,V>* cur=this->head;
    Node<K,V>* update[this->mx_levels];
    memset(update,0,sizeof(Node<K,V>*)*this->mx_levels);

    for(int i=this->cur_level;i>=0;--i)
    {
        while(cur->next[i] && cur->next[i]->getKey()<k) cur=cur->next[i];
        update[i]=cur;
    }

    cur=cur->next[0];

    if(cur && cur->getKey()==k)
    {
        for(int i=0;i<this->cur_level;++i)
        {
            if(update[i]->next[i]!=cur) break;
            update[i]->next[i]=cur->next[i];
        }

        while(this->cur_level && this->head->next[this->cur_level]==nullptr) --this->cur_level;
        --this->cnt;
        std::cout<<"delete key:"<<k<<" successfully"<<std::endl;
    }

    mtx.unlock();
    return;
}

template<typename K,typename V>
void SkipList<K,V>::inDisk()
{
    std::cout<<"in disk:"<<std::endl;
    this->fileWriter.open(STORE_PATH);

    Node<K,V>* cur=this->head->next[0];
    while(cur!=nullptr)
    {
        this->fileWriter<<cur->getKey()<<":"<<cur->getValue()<<"\n";
        cur=cur->next[0];
    }

    this->fileWriter.flush();
    this->fileWriter.close();
    return;
}

template<typename K,typename V>
void SkipList<K,V>::outDisk()
{
    std::cout<<"out disk:"<<std::endl;
    fileReader.open(STORE_PATH);

    const char* f=":";
    std::string line;
    while(getline(fileReader,line))
    {
        char* tmp=(char*)line.c_str();
        char* part=strtok(tmp,f);
        int key=stoi(std::string(part));
        part=strtok(nullptr,f);
        std::string value=part;

        this->insertElement(key,value);
    }
    fileReader.close();
}
