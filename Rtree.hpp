<<<<<<< HEAD
#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include <limits>

struct Rectangle {
    float x1, y1, x2, y2;

    float area() const {
        return (x2 - x1) * (y2 - y1);
    }

    bool overlaps(const Rectangle& other) const {
        return !(x1 > other.x2 || x2 < other.x1 || y1 > other.y2 || y2 < other.y1);
    }

    bool equals(const Rectangle& other) const {
        return x1 == other.x1 && y1 == other.y1 &&
               x2 == other.x2 && y2 == other.y2;
    }

    Rectangle unionWith(const Rectangle& other) const {
        return { std::min(x1, other.x1), std::min(y1, other.y1),
                 std::max(x2, other.x2), std::max(y2, other.y2) };
    }
};

struct RtreeNode {
    bool isLeaf;
    Rectangle mbr;
    std::vector<Rectangle> items;
    std::vector<RtreeNode*> children;
    RtreeNode* parent = nullptr;

    RtreeNode(bool leaf) : isLeaf(leaf), mbr{0, 0, 0, 0} {}
};

class Rtree {
private:
    RtreeNode* root_node;
    int maxChildren;
    int minChildren;

    RtreeNode* chooseLeaf(RtreeNode* node, const Rectangle& rect);
    void        adjustTree(RtreeNode* node, RtreeNode* splitNode = nullptr);
    RtreeNode*  splitNode(RtreeNode* node);
    float       getEnlargement(const Rectangle& container, const Rectangle& newItem);
    Rectangle   computeMBR(RtreeNode* node);
    void        deleteTree(RtreeNode* node);
    //helpers for deleting
    RtreeNode* findLeaf(RtreeNode* node, const Rectangle& rect);
    void       condenseTree(RtreeNode* leaf);
 
public:
    Rtree(int maxChildren);
    void insert(const Rectangle& rect);
    void search(RtreeNode* node, const Rectangle& queryBox, std::vector<Rectangle>& results);
    bool remove(const Rectangle& rect);
    RtreeNode* getRoot() { return root_node; }
    ~Rtree();
};
=======
#include <algorithm>
#include <vector>
#include <string>
#pragma once

struct Rectangle {
    float x1,y1,x2,y2;
    float area() const{
        return (x2-x1)*(y2-y1);
    }
    bool overlaps(const Rectangle& other) const{
        return !(x1>other.x2|| x2<other.x1 || y1>other.y2 || y2<other.y1)
    }
};

struct RtreeNode{
    bool isLeaf;
    Rectangle mbr;
    std::vector<Rectangle> items;
    std::vector <RtreeNode*>children;
    RtreeNode(bool leaf): isLeaf(leaf){}
};

class Rtree{
    private:
    RtreeNode* root_node;
    int maxChildren;
    int minChildren;
    RtreeNode* choodeLeaf(RtreeNode* node,cinst Rectangle& rect); //finding the best sibling to put a new rectangle
    void adjustTree(RtreeNode* node ); //update MBRs after an insertion
    void splitNode( RtreeNode* node); //splitting after it overflows
    float getEnlargement(const Rectangle& container, const Rectangle& newItem);//helper to calculate how much AREA WILL GROW WHEN WE ADD A RECTANGLE.


    public:
    Rtree(int maxChildren);
    void insert(const Rectangle& rect);
    void search(RtreeNode* node , const Rectangle& queryBox, std::vector<Rectangle>& results); //comparison
    RtreeNode* getRoot(){return root_node;}
    ~Rtree();
};
>>>>>>> 0738b64275c20ae975c39d90295f77708ab3c513
