#include "Rtree.hpp"
#include <stdexcept>
#include <cmath>
#include <algorithm> //using for min and max
#include <functional>


//constructor and destructor
Rtree::Rtree(int max) : maxChildren(max), minChildren(max / 2) {
    root_node = new RtreeNode(true); // start with an empty leaf
}

Rtree::~Rtree() {
    deleteTree(root_node);
}

//helper to delete
void Rtree::deleteTree(RtreeNode* node) {
    if (!node) return;
    for (RtreeNode* child : node->children)
        deleteTree(child);
    delete node;
}


//helpers to compute mbr
//To keep the tree accurate we have to calculate MBR each time.
Rectangle Rtree::computeMBR(RtreeNode* node) {
    Rectangle mbr = { INFINITY, INFINITY, -INFINITY, -INFINITY }; //used cmath

    if (node->isLeaf) {
        for (const Rectangle& r : node->items) {
            mbr.x1 = std::min(mbr.x1, r.x1);
            mbr.y1 = std::min(mbr.y1, r.y1);
            mbr.x2 = std::max(mbr.x2, r.x2);
            mbr.y2 = std::max(mbr.y2, r.y2);
        }
    } else {
        for (RtreeNode* child : node->children) {
            mbr.x1 = std::min(mbr.x1, child->mbr.x1);
            mbr.y1 = std::min(mbr.y1, child->mbr.y1);
            mbr.x2 = std::max(mbr.x2, child->mbr.x2);
            mbr.y2 = std::max(mbr.y2, child->mbr.y2);
        }
    }
    return mbr;
}


//get Enlargement is used to return the increased area, when we add any new item

float Rtree::getEnlargement(const Rectangle& container, const Rectangle& newItem) {
    // Build the smallest rectangle that covers both
    Rectangle enlarged = {
        std::min(container.x1, newItem.x1),
        std::min(container.y1, newItem.y1),
        std::max(container.x2, newItem.x2),
        std::max(container.y2, newItem.y2)
    };
    // Enlargement = extra area required
    return enlarged.area() - container.area();
}

//In Btree we follow a path but in R-tree we are dealing with 2-D space so we have to find where it best fits.
//At each internal level, pick the child whose MBR needs the LEAST enlargement to contain rect.
RtreeNode* Rtree::chooseLeaf(RtreeNode* node, const Rectangle& rect) {
    // Base case: we've reached a leaf
    if (node->isLeaf)
        return node;

    RtreeNode* bestChild = nullptr;
    float bestEnlarge = INFINITY;  //we to find the minimum thats why starting at infinity.
    float bestArea = INFINITY;
    for (RtreeNode* child : node->children) {
        float enlarge = getEnlargement(child->mbr, rect);
        float area    = child->mbr.area();

        // Pick child needing least enlargement; break ties by smallest area
        if (enlarge < bestEnlarge || (enlarge == bestEnlarge && area < bestArea)) {
            bestEnlarge = enlarge;
            bestArea    = area;
            bestChild   = child;
        }
    }

    // Recurse down the chosen subtree
    return chooseLeaf(bestChild, rect);
}



//  Called when node has overflowed (maxChildren + 1 entries).It will splits into two nodes and returns the NEW sibling.
//  Steps:
//    1. PickSeeds  — find the pair that wastes the most area together
//    2. PickNext   — greedily assign remaining entries
//    3. Return the newly created sibling

RtreeNode* Rtree::splitNode(RtreeNode* node) {
    RtreeNode* sibling = new RtreeNode(node->isLeaf);
    sibling->parent    = node->parent;

    if (node->isLeaf) {
        std::vector<Rectangle> all = node->items; // snapshot all items
        node->items.clear();

        // 1. PickSeeds: pair whose combined MBR wastes the most dead space
        int seed1 = 0, seed2 = 1;
        float worstWaste = -INFINITY;

        for (int i = 0; i < all.size(); ++i) {
            for (int j = i + 1; j < all.size(); ++j) {
                float area1 = all[i].area();
                float area2 = all[j].area();
                float combinedArea = all[i].unionWith(all[j]).area()

                // 2. Calculate the area of that union
                float combinedArea = (maxX - minX) * (maxY - minY);
                // Calculate empty space created by pairing these two
                float waste = combinedArea - area1 - area2;

                if (waste > worstWaste) {
                    worstWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }
        // Assign seeds to the two groups
        node->items.push_back(all[seed1]);
        sibling->items.push_back(all[seed2]);

        // Remove seeds from the pool (iterate backwards to keep indices valid)
        std::vector<Rectangle> pool;
        for (int i = 0; i < (int)all.size(); ++i)
            if (i != seed1 && i != seed2)
                pool.push_back(all[i]);

        // 2. PickNext: assign each remaining entry to the group whose MBR
        //    needs the least enlargement; enforce minChildren constraint
        for (const Rectangle& r : pool) {
            // If one group is too small, force all remaining entries into it
            int remaining = (int)pool.size(); 
            if ((int)node->items.size() + remaining <= minChildren) {
                node->items.push_back(r);
                continue;
            }
            if ((int)sibling->items.size() + remaining <= minChildren) {
                sibling->items.push_back(r);
                continue;
            }

            //computing everytime for boundary
            Rectangle mbrA = computeMBR(node);
            Rectangle mbrB = computeMBR(sibling);
            //seeing how much it will grow by inserting new rectangle
            float dA = getEnlargement(mbrA, r);
            float dB = getEnlargement(mbrB, r);
            //picking the smaller one
            if (dA < dB || (dA == dB && node->items.size() <= sibling->items.size()))
                node->items.push_back(r);
            else
                sibling->items.push_back(r);
        }
    } else {
        std::vector<RtreeNode*> all = node->children;
        node->children.clear();

        // 1. PickSeeds (doing again beacuse data is different)
        int   seed1 = 0, seed2 = 1;
        float worstWaste = -INFINITY;

        for (int i = 0; i < (int)all.size(); ++i) {
            for (int j = i + 1; j < (int)all.size(); ++j) {
                Rectangle combined = all[i]->mbr.unionWith(all[j]->mbr);
                float waste = combined.area() - all[i]->mbr.area() - all[j]->mbr.area();
                if (waste > worstWaste) {
                    worstWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        node->children.push_back(all[seed1]);
        sibling->children.push_back(all[seed2]);
        all[seed1]->parent = node;
        all[seed2]->parent = sibling;

        std::vector<RtreeNode*> pool;
        for (int i = 0; i < (int)all.size(); ++i)
            if (i != seed1 && i != seed2)
                pool.push_back(all[i]);

        // 2. PickNext
        for (RtreeNode* child : pool) {
            int remaining = (int)pool.size();
            if ((int)node->children.size() + remaining <= minChildren) {
                child->parent = node;
                node->children.push_back(child);
                continue;
            }
            if ((int)sibling->children.size() + remaining <= minChildren) {
                child->parent = sibling;
                sibling->children.push_back(child);
                continue;
            }

            Rectangle mbrA = computeMBR(node);
            Rectangle mbrB = computeMBR(sibling);
            float dA = getEnlargement(mbrA, child->mbr);
            float dB = getEnlargement(mbrB, child->mbr);

            if (dA < dB || (dA == dB && node->children.size() <= sibling->children.size())) {
                child->parent = node;
                node->children.push_back(child);
            } else {
                child->parent = sibling;
                sibling->children.push_back(child);
            }
        }
    }

    // Update MBRs for both halves
    node->mbr    = computeMBR(node);
    sibling->mbr = computeMBR(sibling);

    return sibling;
}


//  Walk from node back up to root, updating MBRs.If a split produced newSibling, propagate it upward,
//  potentially splitting parent nodes all the way to the root.
void Rtree::adjustTree(RtreeNode* node, RtreeNode* newSibling) {

    // If we've reached the root, handle root split
    if (node == root_node) {
        if (newSibling) {
            RtreeNode* newRoot  = new RtreeNode(false); //false because it is not a leaf
            newRoot->children.push_back(node);
            newRoone ot->children.push_back(newSibling);
            node->parent       = newRoot;
            newSibling->parent = newRoot;
            newRoot->mbr       = computeMBR(newRoot); //update new Root mbr
            root_node          = newRoot;
        }
        return; 
    }

    RtreeNode* parent = node->parent;
    // Update parent's MBR 
    parent->mbr = computeMBR(parent);

    RtreeNode* propagatedSplit = nullptr; //if a new sibling is created then parent has to split
    if (newSibling) {
        newSibling->parent = parent;
        parent->children.push_back(newSibling);
        // Parent overflows — split it too
        if ((int)parent->children.size() > maxChildren) {
            propagatedSplit = splitNode(parent);
        }
        // Re-sync parent MBR after adding sibling
        parent->mbr = computeMBR(parent);
    }
    // Continue walking up
    adjustTree(parent, propagatedSplit);
}

void Rtree::insert(const Rectangle& rect) {
    RtreeNode* leaf = chooseLeaf(root_node, rect); //finding the best leaf node where least enlargemnet takes place

    leaf->items.push_back(rect);
    leaf->mbr = computeMBR(leaf);

    RtreeNode* sibling = nullptr;
    if ((int)leaf->items.size() > maxChildren)
        sibling = splitNode(leaf);

    adjustTree(leaf, sibling);
}

//searching for the nodes that overlaps the region in queryBox.If not overlaps entire subtree is skipped.
void Rtree::search(RtreeNode* node, const Rectangle& queryBox,
                   std::vector<Rectangle>& results) {
    if (!node) return;

    if (node->isLeaf) {
        for (const Rectangle& r : node->items)
            if (r.overlaps(queryBox))
                results.push_back(r);
    }else{
        for (RtreeNode* child : node->children)
            if (child->mbr.overlaps(queryBox))
                search(child, queryBox, results);
    }
}

RtreeNode* Rtree::findLeaf(RtreeNode* node, const Rectangle& rect) {
    if (!node) return nullptr;

    if (node->isLeaf) {
        // Check if rect exists among this leaf's items
        for (const Rectangle& r : node->items)
            if (r.equals(rect))
                return node;
        return nullptr;
    }

    // Internal node: only descend into children whose MBR
    // contains rect (not just overlaps — rect must fit inside)
    for (RtreeNode* child : node->children) {
        if (child->mbr.overlaps(rect)) {
            RtreeNode* result = findLeaf(child, rect);
            if (result) return result; // found in this subtree
        }
    }
    return nullptr;
}


void Rtree::condenseTree(RtreeNode* leaf) {
    std::vector<Rectangle>  orphanedRects;   // entries to re-insert from dissolved leaf nodes
    std::vector<RtreeNode*> orphanedNodes;   // subtrees to re-insert from dissolved internal nodes
    RtreeNode* node = leaf;

    while (node != root_node) {
        RtreeNode* parent = node->parent;
        bool underfull = false;

        if (node->isLeaf)
            underfull = (int)node->items.size() < minChildren; //fewer than minChildren then it is underfull
        else
            underfull = (int)node->children.size() < minChildren;

        if (underfull) {
            auto& siblings = parent->children;// so that we wont have to type parent->children again and again
            siblings.erase(
                std::remove(siblings.begin(), siblings.end(), node),
                siblings.end()
            ); //erase inbuilt function and remove <algoritm> part

            if (node->isLeaf) {
                for (const Rectangle& r : node->items)
                    orphanedRects.push_back(r);
                delete node;
            } else {
                for (RtreeNode* child : node->children)
                    orphanedNodes.push_back(child);
                node->children.clear();
                delete node;
            }
        } else {
            node->mbr = computeMBR(node);
        }

        node = parent;
    }

    // Recompute root MBR
    if (!root_node->isLeaf && !root_node->children.empty())
        root_node->mbr = computeMBR(root_node);

    // Shrink tree height if root has only one child left
    if (!root_node->isLeaf && root_node->children.size() == 1) {
        RtreeNode* oldRoot = root_node;
        root_node = root_node->children[0];
        root_node->parent = nullptr;
        oldRoot->children.clear();
        delete oldRoot;
    }

    // Re-insert all orphaned leaf rectangles
    for (const Rectangle& r : orphanedRects)
        insert(r);

    // Re-insert orphaned subtree children (as individual leaf rects)
    for (RtreeNode* n : orphanedNodes)
        reinsertSubtree(n);
}

void Rtree::reinsertSubtree(RtreeNode* n) {
    if (!n) return;

    if (n->isLeaf) {
        // If it's a leaf, just insert all its rectangles
        for (const Rectangle& r : n->items) {
            insert(r);
        }
    } else {
        // If it's internal, keep going down until we find leaves
        for (RtreeNode* child : n->children) {
            reinsertSubtree(child);
        }
    }
    // Clean up the node memory after its items are re-inserted
    delete n; 
}


bool Rtree::remove(const Rectangle& rect) {
    // Finding the leaf that holds rect
    RtreeNode* leaf = findLeaf(root_node, rect);
    if (!leaf) return false;

    // Removing the entry from the leaf
    auto& items = leaf->items;
    int removeIdx = -1;
    for (int i = 0; i < (int)items.size(); ++i) {
        if (items[i].equals(rect)) { removeIdx = i; break; }
    }
    if (removeIdx == -1) return false;
    items.erase(items.begin() + removeIdx);

    // Fixing underfull nodes and recompute MBRs
    condenseTree(leaf);

    return true;
}
