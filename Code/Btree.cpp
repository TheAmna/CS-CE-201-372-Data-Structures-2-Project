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


// insert key 
//  If leaf key just inserted in sorted position
//  If internal first find the right child, split it first if full, then recurse. 
void Btree::insertNonFull(BtreeNode* node, float key) {
    int i = (int)node->keys.size() - 1; // start at rightmost key
 
    if (node->isLeaf) {
        node->keys.push_back(0); // make room
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i]; // shift keys right to make space
            --i;
        }
        node->keys[i + 1] = key; // place key in sorted position
 
    } else {
        // find which child to descend into
        while (i >= 0 && key < node->keys[i])
            --i;
        ++i; // child[i] is where key belongs
 
        if ((int)node->children[i]->keys.size() == 2 * t - 1) {
            splitChild(node, i, node->children[i]); // split full child before descending
            if (key > node->keys[i])
                ++i; // after split, decide which half to go into
        }
 
        insertNonFull(node->children[i], key); // recurse into the right child
    }
}
 
 

void Btree::insert(float key) {
    BtreeNode* r = root_node;
 
    if ((int)r->keys.size() == 2 * t - 1) {
        // root is full
        BtreeNode* newRoot = new BtreeNode(false); // new internal root
        newRoot->children.push_back(r); // old root becomes first child
        r->parent  = newRoot;
        root_node  = newRoot;
        splitChild(newRoot, 0, r); // split old root, push mid key up
        insertNonFull(newRoot, key); // now insert into non-full new root
    } else {
        insertNonFull(r, key); // insert in root
    }
}
 
 
//  search  
//  walk the tree left/right at each node using sorted keys. 
bool Btree::search(float key) {
    BtreeNode* node = root_node;
    while (node) {
        int i = 0;
        while (i < (int)node->keys.size() && key > node->keys[i])
            ++i; // walk right past keys that are too small
 
        if (i < (int)node->keys.size() && node->keys[i] == key)
            return true; // key found
 
        if (node->isLeaf)
            return false; // key not exist
 
        node = node->children[i]; //descend into the right child
    }
    return false;
}
 
 

// collects all keys in [low, high] into results
void Btree::rangeSearchHelper(BtreeNode* node, float low, float high,std::vector<float>& result) {
    if (!node) return; 
 
    int i = 0;
 
    // skip keys that are too small
    while (i < (int)node->keys.size() && node->keys[i] < low) {
        if (!node->isLeaf)
            rangeSearchHelper(node->children[i], low, high, result); // child might have keys >= low
        ++i;
    }
 
    // collect keys and recurse while still in range
    while (i < (int)node->keys.size() && node->keys[i] <= high) {
        if (!node->isLeaf)
            rangeSearchHelper(node->children[i], low, high, result); // check left child of this key
        results.push_back(node->keys[i]);
        ++i;
    }
 
    // check the rightmost child (keys bigger than everything collected so far)
    if (!node->isLeaf && i < (int)node->children.size())
        rangeSearchHelper(node->children[i], low, high, result);
}
 
std::vector<float> Btree::rangeSearch(float low, float high) {
    std::vector<float> result;
    rangeSearchHelper(root_node, low, high, result);
    return result;
}
 
 
// delete helpers

// largest key in the subtree rooted at node->children[idx]
float Btree::getPredecessor(BtreeNode* node, int idx) {
    BtreeNode* cur = node->children[idx];
    while (!cur->isLeaf)
        cur = cur->children.back(); // keep going right until we hit a leaf
    return cur->keys.back();        // rightmost = largest
}
 
// smallest key in the subtree rooted at node->children[idx+1]
float Btree::getSuccessor(BtreeNode* node, int idx) {
    BtreeNode* cur = node->children[idx + 1];
    while (!cur->isLeaf)
        cur = cur->children.front(); // keep going left until we hit a leaf
    return cur->keys.front();        // leftmost = smallest
}
 
// remove key at position idx from a LEAF — safe because fill() was already called
void Btree::removeFromLeaf(BtreeNode* node, int idx) {
    node->keys.erase(node->keys.begin() + idx);
}
 
// remove key at position idx from an INTERNAL node
// we can't just erase it — we must replace with predecessor or successor first
void Btree::removeFromInternal(BtreeNode* node, int idx) {
    float key = node->keys[idx];
 
    if ((int)node->children[idx]->keys.size() >= t) {
        // left child has enough keys → replace with in-order predecessor
        float pred      = getPredecessor(node, idx);
        node->keys[idx] = pred;
        removeFromNode(node->children[idx], pred); // delete predecessor from left subtree
 
    } else if ((int)node->children[idx + 1]->keys.size() >= t) {
        // right child has enough keys → replace with in-order successor
        float succ      = getSuccessor(node, idx);
        node->keys[idx] = succ;
        removeFromNode(node->children[idx + 1], succ); // delete successor from right subtree
 
    } else {
        // both children are at minimum → merge them around key, then delete
        merge(node, idx);
        removeFromNode(node->children[idx], key); // key is now inside the merged child
    }
}
 
// steal one key from left sibling (left sibling has >= t keys)
void Btree::borrowFromPrev(BtreeNode* node, int idx) {
    BtreeNode* child   = node->children[idx];
    BtreeNode* sibling = node->children[idx - 1];
 
    // push parent's separator key down into child (at front)
    child->keys.insert(child->keys.begin(), node->keys[idx - 1]);
 
    // pull sibling's last key up into parent
    node->keys[idx - 1] = sibling->keys.back();
    sibling->keys.pop_back();
 
    // also move sibling's last child if internal
    if (!child->isLeaf) {
        child->children.insert(child->children.begin(), sibling->children.back());
        child->children.front()->parent = child;
        sibling->children.pop_back();
    }
}
 
// steal one key from right sibling (right sibling has >= t keys)
void Btree::borrowFromNext(BtreeNode* node, int idx) {
    BtreeNode* child   = node->children[idx];
    BtreeNode* sibling = node->children[idx + 1];
 
    // push parent's separator key down into child (at back)
    child->keys.push_back(node->keys[idx]);
 
    // pull sibling's first key up into parent
    node->keys[idx] = sibling->keys.front();
    sibling->keys.erase(sibling->keys.begin());
 
    // also move sibling's first child if internal
    if (!child->isLeaf) {
        child->children.push_back(sibling->children.front());
        child->children.back()->parent = child;
        sibling->children.erase(sibling->children.begin());
    }
}
 
// merge children[idx] and children[idx+1] into one node
// pull the separator key down from parent into the merged node
void Btree::merge(BtreeNode* node, int idx) {
    BtreeNode* left  = node->children[idx];
    BtreeNode* right = node->children[idx + 1];
 
    left->keys.push_back(node->keys[idx]); // pull separator key down into left
 
    // copy all of right's keys and children into left
    for (float k : right->keys)         left->keys.push_back(k);
    for (BtreeNode* c : right->children) {
        c->parent = left;
        left->children.push_back(c);
    }
 
    // remove separator key and right child pointer from parent
    node->keys.erase(node->keys.begin() + idx);
    node->children.erase(node->children.begin() + idx + 1);
 
    delete right; // right node is now absorbed into left
}
 
// ensure child at index idx has at least t keys before we descend into it
void Btree::fill(BtreeNode* node, int idx) {
    if (idx > 0 && (int)node->children[idx - 1]->keys.size() >= t)
        borrowFromPrev(node, idx); // left sibling is rich → borrow from it
 
    else if (idx < (int)node->children.size() - 1 &&
             (int)node->children[idx + 1]->keys.size() >= t)
        borrowFromNext(node, idx); // right sibling is rich → borrow from it
 
    else {
        // both siblings are at minimum → must merge
        if (idx == (int)node->children.size() - 1)
            merge(node, idx - 1); // last child → merge with left sibling
        else
            merge(node, idx);     // otherwise merge with right sibling
    }
}
 
 

//  Remove key from the subtree rooted at node.
//  Before descending into any child, we call fill() to ensure
//  that child has >= t keys — so we never get stuck during deletion.
void Btree::removeFromNode(BtreeNode* node, float key) {
    // find first index where keys[idx] >= key
    int idx = 0;
    while (idx < (int)node->keys.size() && node->keys[idx] < key)
        ++idx;
 
    bool keyIsHere = (idx < (int)node->keys.size() && node->keys[idx] == key);
 
    if (node->isLeaf) {
        if (keyIsHere)
            removeFromLeaf(node, idx); // key is right here in the leaf → erase it
        return; // if not found in leaf, it's not in the tree
    }
 
    if (keyIsHere) {
        removeFromInternal(node, idx); // key is in this internal node → special handling
    } else {
        // key might be in child[idx] → make sure that child has enough keys first
        if ((int)node->children[idx]->keys.size() < t)
            fill(node, idx); // borrow or merge to ensure child is safe to descend into
 
        // fill/merge may have changed positions, so check bounds
        if (idx >= (int)node->children.size())
            removeFromNode(node->children[idx - 1], key);
        else
            removeFromNode(node->children[idx], key);
    }
}
 
 

//  Returns true if key was found and deleted.
//  Returns false if key was not in the tree.
bool Btree::remove(float key) {
    if (!search(key)) return false; // key not in tree 
 
    removeFromNode(root_node, key);
 
    // shrink tree height if root is now empty
    if (root_node->keys.empty() && !root_node->isLeaf) {
        BtreeNode* oldRoot = root_node;
        root_node          = root_node->children[0]; // only child becomes new root
        root_node->parent  = nullptr;
        oldRoot->children.clear();
        delete oldRoot;
    }
 
    return true;
}
 
