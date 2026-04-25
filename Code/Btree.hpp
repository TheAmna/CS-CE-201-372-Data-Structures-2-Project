#pragma once
#include <vector>
#include <algorithm>
//node 
struct BtreeNode {
    bool isLeaf; // if true then no children
    std::vector<float>keys; // keys stored in sorted order
    std::vector<BtreeNode*> children; //children[i] = subtree of keys < keys[i]
    BtreeNode* parent = nullptr;
    //constructor , set isLeaf=leaf
    BtreeNode(bool leaf) : isLeaf(leaf) {}
};
//tree
class Btree {
private:
    BtreeNode* root_node;
    int t;  //minimum degree 
    //to clear up the storage , deletes the tree
    void deleteTree(BtreeNode* node);
    //insert helpers
    void insertNonFull(BtreeNode* node, float key); //insert into a node that has room
    void splitChild(BtreeNode* parent, int i, BtreeNode* child); //split a full child, mid key promoted
    //delete helpers 
    void  removeFromNode(BtreeNode* node, float key); // main recursive delete
    void  removeFromLeaf(BtreeNode* node, int idx); // delete from leaf
    void  removeFromInternal(BtreeNode* node, int idx); // swap with predecessor/successor first
    float getPredecessor(BtreeNode* node, int idx); // largest key in left subtree
    float getSuccessor(BtreeNode* node, int idx); // smallest key in right subtree
    void  fill(BtreeNode* node, int idx); // ensure child has >= t keys before descending
    void  borrowFromPrev(BtreeNode* node, int idx); // get one key from left sibling
    void  borrowFromNext(BtreeNode* node, int idx); //get one key from right sibling
    void  merge(BtreeNode* node, int idx); // merge two siblings into one node
    // range search helper
    void rangeSearchHelper(BtreeNode* node, float low, float high, std::vector<float>& result);
public:
    Btree(int minkey); //constructor
    ~Btree(); //destructor
  
    void insert(float key);
    bool remove(float key);              
    bool search(float key);              
    std::vector<float> rangeSearch(float low, float high); //find all keys in [lo,hi]
    BtreeNode* getRoot() { return root_node; }
};
