#include "Btree.hpp"

//constructor
Btree::Btree(int minDegree){
t = minDegree;
root_node = new BtreeNode(true); //starts with only root node
}

//destructor
Btree::~Btree() {
    deleteTree(root_node);
}


void Btree::deleteTree(BtreeNode* node){
    if (node == nullptr)   // empty node
        return;
    // delete all child nodes
    for (int i = 0; i < node->children.size(); i++){
        deleteTree(node->children[i]);  //recursive call for each child
    }
    // now delete the current node itself
    delete node;
}



//called when a child node is full (has 2t-1 keys) and we need to insert into it
// it is split into two halves and push the middle key up into the parent
//  i= index of the full child in parent->children
void Btree::splitChild(BtreeNode* parent, int i, BtreeNode* child){
    
    BtreeNode* right = new BtreeNode(child->isLeaf); // new node which is the right half of the split
    right->parent = parent;
    float mid = child->keys[t - 1]; //middle key

    // move right half of keys to new node
    for (int j = t; j < 2 * t - 1; j++){
        right->keys.push_back(child->keys[j]);
    }

    //if not a leaf, move right half of children too
    if (!child->isLeaf){
        for (int j = t; j < 2 * t; j++){
            right->children.push_back(child->children[j]); // move child pointer
            child->children[j]->parent = right;
        }

        //resize the original child to keep only left half
        child->children.resize(t);
    }

    //original child keys (left half only)
    child->keys.resize(t - 1);

    parent->children.insert(parent->children.begin() + i + 1, right); // insert right into parent's children list at position i+1
    parent->keys.insert(parent->keys.begin() + i, mid); //middle key promoted
}
