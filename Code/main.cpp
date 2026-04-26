// // #include "Rtree.hpp"
// // #include "Btree.hpp"
// // #include <iostream>
// // #include <cassert>
// // #include <vector>
// // #include <chrono>
// // #include <iomanip>
// // #include <string>

// // using Clock = std::chrono::high_resolution_clock;

// // long long usNow(Clock::time_point t0) {
// //     return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-t0).count();
// // }
// // void section(const std::string& title) {
// //     std::cout << "\n========================================\n";
// //     std::cout << "  " << title << "\n";
// //     std::cout << "========================================\n";
// // }
// // void pass(const std::string& msg) { std::cout << "  [PASS] " << msg << "\n"; }
// // void printRects(const std::string& label, const std::vector<Rectangle>& v) {
// //     std::cout << "  " << label << " (" << v.size() << " found):\n";
// //     for (const auto& r : v)
// //         std::cout << "    ("<<r.x1<<","<<r.y1<<") -> ("<<r.x2<<","<<r.y2<<")\n";
// // }

// // // ════════════════════════════════════════════════════════════
// // //  R-TREE INSERT TESTS
// // // ════════════════════════════════════════════════════════════
// // void testRtreeInsert() {
// //     section("R-TREE INSERT TESTS");

// //     std::cout << "\n[R-Insert 1] Basic insert and search\n";
// //     Rtree t1(4);
// //     t1.insert({0,0,2,2}); t1.insert({3,3,5,5}); t1.insert({1,1,4,4});
// //     t1.insert({6,0,8,2}); t1.insert({7,7,9,9});
// //     std::vector<Rectangle> r1;
// //     t1.search(t1.getRoot(), {0,0,3,3}, r1);
// //     printRects("Query (0,0)->(3,3)", r1);
// //     assert(r1.size() >= 2);
// //     pass("at least 2 rects overlap query region");

// //     std::cout << "\n[R-Insert 2] No false positives\n";
// //     std::vector<Rectangle> r2;
// //     t1.search(t1.getRoot(), {100,100,200,200}, r2);
// //     printRects("Far query (100,100)->(200,200)", r2);
// //     assert(r2.empty());
// //     pass("0 results for far region");

// //     std::cout << "\n[R-Insert 3] Force node split (10 inserts, maxChildren=4)\n";
// //     Rtree t3(4);
// //     for (int i=0;i<10;++i) t3.insert({(float)i,(float)i,(float)i+1,(float)i+1});
// //     std::vector<Rectangle> r3;
// //     t3.search(t3.getRoot(), {3,3,7,7}, r3);
// //     printRects("Query (3,3)->(7,7)", r3);
// //     assert(!r3.empty());
// //     pass("results returned correctly after split");

// //     std::cout << "\n[R-Insert 4] Duplicate inserts allowed\n";
// //     Rtree t4(4);
// //     t4.insert({1,1,3,3}); t4.insert({1,1,3,3}); t4.insert({1,1,3,3});
// //     std::vector<Rectangle> r4;
// //     t4.search(t4.getRoot(), {0,0,5,5}, r4);
// //     printRects("Query (0,0)->(5,5)", r4);
// //     assert((int)r4.size()==3);
// //     pass("all 3 duplicates stored and found");

// //     std::cout << "\n[R-Insert 5] Bulk insert 50 items\n";
// //     Rtree t5(4);
// //     for (int i=0;i<50;++i) t5.insert({(float)i,(float)i,(float)i+1,(float)i+1});
// //     std::vector<Rectangle> r5;
// //     t5.search(t5.getRoot(), {0,0,100,100}, r5);
// //     std::cout << "  All-space query found: " << r5.size() << " rects\n";
// //     assert((int)r5.size()==50);
// //     pass("all 50 items retrieved");
// // }

// // // ════════════════════════════════════════════════════════════
// // //  R-TREE DELETE TESTS
// // // ════════════════════════════════════════════════════════════
// // void testRtreeDelete() {
// //     section("R-TREE DELETE TESTS");

// //     std::cout << "\n[R-Delete 1] Basic delete\n";
// //     Rtree t1(4);
// //     t1.insert({0,0,2,2}); t1.insert({3,3,5,5}); t1.insert({1,1,4,4});
// //     t1.insert({6,0,8,2}); t1.insert({7,7,9,9});
// //     std::vector<Rectangle> before;
// //     t1.search(t1.getRoot(), {0,0,10,10}, before);
// //     std::cout << "  Before: " << before.size() << " rects\n";
// //     bool ok = t1.remove({3,3,5,5});
// //     assert(ok);
// //     std::vector<Rectangle> after;
// //     t1.search(t1.getRoot(), {0,0,10,10}, after);
// //     std::cout << "  After:  " << after.size() << " rects\n";
// //     assert((int)after.size()==(int)before.size()-1);
// //     for (const auto& r : after) assert(!(r.x1==3&&r.y1==3&&r.x2==5&&r.y2==5));
// //     pass("rect removed, count dropped by 1, not in search results");

// //     std::cout << "\n[R-Delete 2] Delete non-existent rectangle\n";
// //     bool notFound = t1.remove({99,99,100,100});
// //     assert(!notFound);
// //     pass("returned false for missing rect");

// //     std::cout << "\n[R-Delete 3] Search confirms deletion\n";
// //     Rtree t3(4);
// //     t3.insert({5,5,10,10}); t3.insert({1,1,3,3}); t3.insert({7,2,9,4});
// //     t3.remove({5,5,10,10});
// //     std::vector<Rectangle> r3;
// //     t3.search(t3.getRoot(), {0,0,15,15}, r3);
// //     printRects("Remaining after removing (5,5)->(10,10)", r3);
// //     assert((int)r3.size()==2);
// //     for (const auto& r : r3) assert(!(r.x1==5&&r.y1==5&&r.x2==10&&r.y2==10));
// //     pass("2 rects remain, deleted rect gone");

// //     std::cout << "\n[R-Delete 4] Condense tree after bulk delete\n";
// //     Rtree t4(4);
// //     std::vector<Rectangle> bulk;
// //     for (int i=0;i<12;++i) {
// //         Rectangle r={(float)i*2,(float)i*2,(float)i*2+1,(float)i*2+1};
// //         bulk.push_back(r); t4.insert(r);
// //     }
// //     for (int i=0;i<6;++i) { bool ok2=t4.remove(bulk[i]); assert(ok2); }
// //     std::vector<Rectangle> r4;
// //     t4.search(t4.getRoot(), {0,0,100,100}, r4);
// //     std::cout << "  After deleting 6 of 12: " << r4.size() << " remain\n";
// //     assert((int)r4.size()==6);
// //     pass("6 items remain, condense handled correctly");

// //     std::cout << "\n[R-Delete 5] Delete all items\n";
// //     Rtree t5(4);
// //     std::vector<Rectangle> all;
// //     for (int i=0;i<8;++i) {
// //         Rectangle r={(float)i,(float)i,(float)i+1,(float)i+1};
// //         all.push_back(r); t5.insert(r);
// //     }
// //     for (const auto& r : all) { bool ok2=t5.remove(r); assert(ok2); }
// //     std::vector<Rectangle> r5;
// //     t5.search(t5.getRoot(), {0,0,100,100}, r5);
// //     assert(r5.empty());
// //     pass("tree empty after removing all items");
// // }

// // // ════════════════════════════════════════════════════════════
// // //  B-TREE INSERT TESTS
// // // ════════════════════════════════════════════════════════════
// // void testBtreeInsert() {
// //     section("B-TREE INSERT TESTS");

// //     std::cout << "\n[B-Insert 1] Basic insert and search\n";
// //     Btree t1(2);
// //     t1.insert(10); t1.insert(20); t1.insert(5); t1.insert(15); t1.insert(30);
// //     assert(t1.search(10)&&t1.search(20)&&t1.search(5)&&t1.search(15)&&t1.search(30));
// //     assert(!t1.search(99));
// //     pass("all inserted keys found, non-existent key returns false");

// //     std::cout << "\n[B-Insert 2] Range search returns correct keys\n";
// //     Btree t2(2);
// //     for (int i=1;i<=10;++i) t2.insert((float)i);
// //     std::vector<float> r2 = t2.rangeSearch(3,7);
// //     std::cout << "  rangeSearch(3,7): ";
// //     for (float k : r2) std::cout << k << " ";
// //     std::cout << "\n";
// //     assert((int)r2.size()==5);
// //     pass("5 keys found in range [3,7]");

// //     std::cout << "\n[B-Insert 3] Force root split (10 items, t=2)\n";
// //     Btree t3(2);
// //     for (int i=1;i<=10;++i) t3.insert((float)i);
// //     for (int i=1;i<=10;++i) assert(t3.search((float)i));
// //     pass("all 10 keys found after multiple splits");

// //     std::cout << "\n[B-Insert 4] Duplicate key behaviour\n";
// //     Btree t4(2);
// //     t4.insert(5); t4.insert(5); t4.insert(5);
// //     assert(t4.search(5));
// //     pass("duplicate keys inserted without crash");

// //     std::cout << "\n[B-Insert 5] Range search on 50 items\n";
// //     Btree t5(3);
// //     for (int i=1;i<=50;++i) t5.insert((float)i);
// //     std::vector<float> r5 = t5.rangeSearch(20,30);
// //     std::cout << "  rangeSearch(20,30) found: " << r5.size() << " keys\n";
// //     assert((int)r5.size()==11);
// //     pass("11 keys found in range [20,30]");
// // }

// // // ════════════════════════════════════════════════════════════
// // //  B-TREE DELETE TESTS
// // // ════════════════════════════════════════════════════════════
// // void testBtreeDelete() {
// //     section("B-TREE DELETE TESTS");

// //     std::cout << "\n[B-Delete 1] Basic delete\n";
// //     Btree t1(2);
// //     t1.insert(10); t1.insert(20); t1.insert(5); t1.insert(15);
// //     bool ok = t1.remove(10);
// //     assert(ok); assert(!t1.search(10));
// //     assert(t1.search(20)&&t1.search(5)&&t1.search(15));
// //     pass("key 10 removed, others still present");

// //     std::cout << "\n[B-Delete 2] Delete non-existent key\n";
// //     Btree t2(2);
// //     t2.insert(10); t2.insert(20);
// //     bool notFound = t2.remove(99);
// //     assert(!notFound);
// //     pass("returned false for missing key");

// //     std::cout << "\n[B-Delete 3] Delete triggers borrow from sibling\n";
// //     Btree t3(2);
// //     for (int i=1;i<=8;++i) t3.insert((float)i);
// //     t3.remove(1); t3.remove(2);
// //     for (int i=3;i<=8;++i) assert(t3.search((float)i));
// //     pass("remaining keys intact after borrow/merge");

// //     std::cout << "\n[B-Delete 4] Multiple deletes forcing merges\n";
// //     Btree t4(2);
// //     for (int i=1;i<=12;++i) t4.insert((float)i);
// //     for (int i=1;i<=6;++i) { bool ok2=t4.remove((float)i); assert(ok2); }
// //     for (int i=7;i<=12;++i) assert(t4.search((float)i));
// //     pass("6 remaining keys found after deleting first half");

// //     std::cout << "\n[B-Delete 5] Delete all keys\n";
// //     Btree t5(2);
// //     std::vector<float> keys={5,3,7,1,4,6,8,2};
// //     for (float k : keys) t5.insert(k);
// //     for (float k : keys) { bool ok2=t5.remove(k); assert(ok2); }
// //     for (float k : keys) assert(!t5.search(k));
// //     pass("all keys removed, none found afterwards");
// // }

// // // ════════════════════════════════════════════════════════════
// // //  COMPARISON ANALYSIS
// // // ════════════════════════════════════════════════════════════
// // void runComparison(int n) {
// //     std::cout << "\n--- N = " << n << " ---\n";
// //     std::cout << std::left  << std::setw(30) << "Operation"
// //               << std::right << std::setw(12) << "R-tree(us)"
// //               << std::setw(12) << "B-tree(us)" << "\n";
// //     std::cout << std::string(54,'-') << "\n";

// //     std::vector<Rectangle> rects;
// //     std::vector<float> keys;
// //     for (int i=0;i<n;++i) {
// //         rects.push_back({(float)i,(float)i,(float)i+1,(float)i+1});
// //         keys.push_back((float)i);
// //     }

// //     // INSERT
// //     Rtree rtree(4);
// //     auto t0=Clock::now();
// //     for (const Rectangle& r:rects) rtree.insert(r);
// //     long long rtIns=usNow(t0);

// //     Btree btree(2);
// //     t0=Clock::now();
// //     for (float k:keys) btree.insert(k);
// //     long long btIns=usNow(t0);
// //     std::cout<<std::left<<std::setw(30)<<"Insert"<<std::right<<std::setw(12)<<rtIns<<std::setw(12)<<btIns<<"\n";

// //     // SEARCH — 100 queries each
// //     float mid=(float)n/2;
// //     Rectangle qbox={mid-n*0.05f,mid-n*0.05f,mid+n*0.05f,mid+n*0.05f};
// //     std::vector<Rectangle> rRes;
// //     t0=Clock::now();
// //     for (int q=0;q<100;++q){rRes.clear();rtree.search(rtree.getRoot(),qbox,rRes);}
// //     long long rtSrch=usNow(t0);

// //     t0=Clock::now();
// //     for (int q=0;q<100;++q) btree.search((float)(n/2));
// //     long long btSrch=usNow(t0);
// //     std::cout<<std::left<<std::setw(30)<<"100x Search"<<std::right<<std::setw(12)<<rtSrch<<std::setw(12)<<btSrch<<"\n";
// //     std::cout<<"  R-tree hits per query: "<<rRes.size()<<"\n";

// //     // RANGE SEARCH (B-tree native, R-tree does this via queryBox)
// //     t0=Clock::now();
// //     std::vector<float> bRange=btree.rangeSearch((float)(n/2-n/20),(float)(n/2+n/20));
// //     long long btRange=usNow(t0);
// //     std::cout<<std::left<<std::setw(30)<<"B-tree rangeSearch"<<std::right<<std::setw(12)<<"N/A"<<std::setw(12)<<btRange<<"\n";
// //     std::cout<<"  B-tree range keys found: "<<bRange.size()<<"\n";

// //     // DELETE 10%
// //     int del=n/10;
// //     t0=Clock::now();
// //     for (int i=0;i<del;++i) rtree.remove(rects[i]);
// //     long long rtDel=usNow(t0);

// //     t0=Clock::now();
// //     for (int i=0;i<del;++i) btree.remove(keys[i]);
// //     long long btDel=usNow(t0);
// //     std::cout<<std::left<<std::setw(30)<<"Delete 10%"<<std::right<<std::setw(12)<<rtDel<<std::setw(12)<<btDel<<"\n";
// // }

// // void testComparison() {
// //     section("R-TREE vs B-TREE COMPARISON ANALYSIS");
// //     std::cout << "\nSame dataset used for both trees.\n";
// //     std::cout << "R-tree: spatial MBR queries  |  B-tree: 1D key lookups\n";
// //     runComparison(500);
// //     runComparison(2000);
// //     runComparison(5000);
// //     std::cout << "\n--- Key Observations ---\n";
// //     std::cout << "  Insert : B-tree faster (scalar compare vs MBR enlargement calc)\n";
// //     std::cout << "  Search : R-tree returns spatial ranges; B-tree does exact lookup\n";
// //     std::cout << "  Delete : B-tree borrows/merges; R-tree dissolves + re-inserts\n";
// //     std::cout << "  Use    : B-tree for sorted 1D keys, R-tree for 2D spatial data\n";
// // }

// // // ════════════════════════════════════════════════════════════
// // //  MAIN
// // // ════════════════════════════════════════════════════════════
// // int main() {
// //     std::cout << "==========================================\n";
// //     std::cout << "   R-TREE & B-TREE COMPLETE TEST SUITE\n";
// //     std::cout << "==========================================\n";

// //     testRtreeInsert();
// //     testRtreeDelete();
// //     testBtreeInsert();
// //     testBtreeDelete();
// //     testComparison();

// //     std::cout << "\n==========================================\n";
// //     std::cout << "         ALL TESTS PASSED\n";
// //     std::cout << "==========================================\n";
// //     return 0;
// // }
// #include "Rtree.hpp"
// #include "Btree.hpp"
// #include <iostream>
// #include <string>
// #include <vector>

// // ════════════════════════════════════════════════════════════
// //  R-TREE VISUAL PRINTER
// //  Prints the tree level by level with indentation
// //  so you can see exactly how nodes and MBRs are arranged
// // ════════════════════════════════════════════════════════════

// void printRtree(RtreeNode* node, int depth = 0) {
//     if (!node) return;

//     std::string indent(depth * 4, ' ');
//     std::string bar(depth * 4, '-');

//     if (node->isLeaf) {
//         std::cout << indent << "[LEAF] MBR=("
//                   << node->mbr.x1 << "," << node->mbr.y1 << ")->("
//                   << node->mbr.x2 << "," << node->mbr.y2 << ")\n";
//         for (const Rectangle& r : node->items) {
//             std::cout << indent << "  |-- rect: ("
//                       << r.x1 << "," << r.y1 << ")->("
//                       << r.x2 << "," << r.y2 << ")\n";
//         }
//     } else {
//         std::cout << indent << "[INTERNAL] MBR=("
//                   << node->mbr.x1 << "," << node->mbr.y1 << ")->("
//                   << node->mbr.x2 << "," << node->mbr.y2 << ")  "
//                   << "children=" << node->children.size() << "\n";
//         for (RtreeNode* child : node->children)
//             printRtree(child, depth + 1);
//     }
// }

// void rtreeStep(const std::string& msg) {
//     std::cout << "\n  >>> " << msg << "\n";
// }

// // ════════════════════════════════════════════════════════════
// //  B-TREE VISUAL PRINTER
// //  Prints each level left to right showing keys in each node
// // ════════════════════════════════════════════════════════════

// void printBtree(BtreeNode* node, int depth = 0) {
//     if (!node) return;

//     std::string indent(depth * 4, ' ');

//     std::cout << indent;
//     if (node->isLeaf) std::cout << "[LEAF] keys: ";
//     else              std::cout << "[NODE] keys: ";

//     std::cout << "[ ";
//     for (float k : node->keys) std::cout << k << " ";
//     std::cout << "]\n";

//     for (BtreeNode* child : node->children)
//         printBtree(child, depth + 1);
// }

// void divider(const std::string& title) {
//     std::cout << "\n";
//     std::cout << "╔══════════════════════════════════════════╗\n";
//     std::cout << "║  " << title;
//     int pad = 42 - (int)title.size();
//     for (int i = 0; i < pad; ++i) std::cout << ' ';
//     std::cout << "║\n";
//     std::cout << "╚══════════════════════════════════════════╝\n";
// }

// void step(const std::string& msg) {
//     std::cout << "\n------------------------------------------\n";
//     std::cout << "  STEP: " << msg << "\n";
//     std::cout << "------------------------------------------\n";
// }

// // ════════════════════════════════════════════════════════════
// //  R-TREE WALKTHROUGH
// // ════════════════════════════════════════════════════════════

// void rtreeWalkthrough() {
//     divider("R-TREE STEP BY STEP WALKTHROUGH");

//     Rtree tree(3); // maxChildren=3 so splits happen quickly

//     // ── INSERT 1 ─────────────────────────────────────────────
//     step("Insert rect A: (0,0)->(2,2)");
//     std::cout << "  chooseLeaf  -> root is only node, go there\n";
//     std::cout << "  insert      -> add to leaf items\n";
//     std::cout << "  adjustTree  -> update MBR of leaf\n";
//     tree.insert({0,0,2,2});
//     std::cout << "\n  Tree now:\n";
//     printRtree(tree.getRoot());

//     // ── INSERT 2 ─────────────────────────────────────────────
//     step("Insert rect B: (5,5)->(8,8)");
//     std::cout << "  chooseLeaf  -> root is leaf, go there\n";
//     std::cout << "  insert      -> add to same leaf (still under maxChildren=3)\n";
//     std::cout << "  adjustTree  -> MBR expands to cover both A and B\n";
//     tree.insert({5,5,8,8});
//     std::cout << "\n  Tree now:\n";
//     printRtree(tree.getRoot());

//     // ── INSERT 3 ─────────────────────────────────────────────
//     step("Insert rect C: (1,5)->(3,7)");
//     std::cout << "  chooseLeaf  -> root is leaf, go there\n";
//     std::cout << "  insert      -> add (now at maxChildren=3, still OK)\n";
//     std::cout << "  adjustTree  -> MBR expands again\n";
//     tree.insert({1,5,3,7});
//     std::cout << "\n  Tree now:\n";
//     printRtree(tree.getRoot());

//     // ── INSERT 4 — TRIGGERS SPLIT ────────────────────────────
//     step("Insert rect D: (6,1)->(9,3)  <-- THIS TRIGGERS A SPLIT");
//     std::cout << "  chooseLeaf  -> root is leaf, go there\n";
//     std::cout << "  insert      -> now has 4 items > maxChildren=3\n";
//     std::cout << "  splitNode   -> PickSeeds: find pair with most dead space\n";
//     std::cout << "               -> A(0,0,2,2) and B(5,5,8,8) chosen as seeds\n";
//     std::cout << "               -> PickNext: C and D assigned to closest group\n";
//     std::cout << "  adjustTree  -> root was split, NEW ROOT created above both halves\n";
//     tree.insert({6,1,9,3});
//     std::cout << "\n  Tree now (notice: root is now INTERNAL, children are leaves):\n";
//     printRtree(tree.getRoot());

//     // ── INSERT 5 ─────────────────────────────────────────────
//     step("Insert rect E: (2,2)->(4,4)");
//     std::cout << "  chooseLeaf  -> at root (internal), check each child MBR\n";
//     std::cout << "               -> pick child whose MBR grows LEAST to include E\n";
//     std::cout << "  insert      -> add E to chosen leaf\n";
//     std::cout << "  adjustTree  -> update parent MBR\n";
//     tree.insert({2,2,4,4});
//     std::cout << "\n  Tree now:\n";
//     printRtree(tree.getRoot());

//     // ── SEARCH ───────────────────────────────────────────────
//     step("Search query: (0,0)->(5,5)");
//     std::cout << "  At root (internal):\n";
//     std::cout << "    Check child 1 MBR -- does it overlap (0,0,5,5)? YES -> descend\n";
//     std::cout << "    Check child 2 MBR -- does it overlap (0,0,5,5)? maybe -> check\n";
//     std::cout << "  At each leaf: check every stored rect against query\n";
//     std::cout << "  Only rects that OVERLAP the query box are returned\n";

//     Rectangle qbox = {0,0,5,5};
//     std::vector<Rectangle> results;
//     tree.search(tree.getRoot(), qbox, results);
//     std::cout << "\n  Results found (" << results.size() << "):\n";
//     for (const auto& r : results)
//         std::cout << "    (" << r.x1 << "," << r.y1
//                   << ") -> (" << r.x2 << "," << r.y2 << ")\n";

//     // ── DELETE ───────────────────────────────────────────────
//     step("Delete rect B: (5,5)->(8,8)");
//     std::cout << "  findLeaf    -> search tree for leaf containing B\n";
//     std::cout << "               -> at internal node: only descend if MBR overlaps B\n";
//     std::cout << "               -> at leaf: check items with equals()\n";
//     std::cout << "  remove      -> erase B from leaf items\n";
//     std::cout << "  condenseTree-> walk up from leaf\n";
//     std::cout << "               -> if node has < minChildren, DISSOLVE it\n";
//     std::cout << "               -> collect orphans and re-insert them\n";
//     std::cout << "               -> update MBRs on the way up\n";
//     tree.remove({5,5,8,8});
//     std::cout << "\n  Tree after delete:\n";
//     printRtree(tree.getRoot());

//     // ── VERIFY ───────────────────────────────────────────────
//     step("Search again after delete: (0,0)->(10,10)");
//     std::vector<Rectangle> after;
//     tree.search(tree.getRoot(), {0,0,10,10}, after);
//     std::cout << "  Remaining rects (" << after.size() << "):\n";
//     for (const auto& r : after)
//         std::cout << "    (" << r.x1 << "," << r.y1
//                   << ") -> (" << r.x2 << "," << r.y2 << ")\n";
//     std::cout << "  (B is gone, all others still present)\n";
// }

// // ════════════════════════════════════════════════════════════
// //  B-TREE WALKTHROUGH
// // ════════════════════════════════════════════════════════════

// void btreeWalkthrough() {
//     divider("B-TREE STEP BY STEP WALKTHROUGH");

//     Btree tree(2); // t=2 means max 3 keys per node, splits happen quickly

//     // ── INSERT ───────────────────────────────────────────────
//     step("Insert keys one by one: 10, 20, 5, 15, 25");

//     std::cout << "  Insert 10:\n";
//     tree.insert(10);
//     printBtree(tree.getRoot());

//     std::cout << "\n  Insert 20:\n";
//     tree.insert(20);
//     printBtree(tree.getRoot());

//     std::cout << "\n  Insert 5:\n";
//     tree.insert(5);
//     printBtree(tree.getRoot());

//     std::cout << "\n  Insert 15 -- node now FULL (3 keys = 2t-1)\n";
//     tree.insert(15);
//     printBtree(tree.getRoot());

//     std::cout << "\n  Insert 25 -- TRIGGERS SPLIT\n";
//     std::cout << "  splitChild  -> median key rises to parent\n";
//     std::cout << "  new root    -> tree grows taller by one level\n";
//     tree.insert(25);
//     printBtree(tree.getRoot());

//     // ── SEARCH ───────────────────────────────────────────────
//     step("Search for key 15");
//     std::cout << "  At root: keys are sorted, walk right past keys < 15\n";
//     std::cout << "  Descend into correct child\n";
//     std::cout << "  At leaf: found 15 -> return true\n";
//     bool found = tree.search(15);
//     std::cout << "  Result: " << (found ? "FOUND" : "NOT FOUND") << "\n";

//     step("Search for key 99 (not in tree)");
//     std::cout << "  Walk down to leaf, reach end without match -> return false\n";
//     bool notFound = tree.search(99);
//     std::cout << "  Result: " << (notFound ? "FOUND" : "NOT FOUND") << "\n";

//     // ── RANGE SEARCH ─────────────────────────────────────────
//     step("Range search: find all keys between 10 and 20");
//     std::cout << "  Walk tree, skip keys < 10\n";
//     std::cout << "  Collect keys 10, 15, 20\n";
//     std::cout << "  Stop when keys > 20\n";
//     std::vector<float> range = tree.rangeSearch(10, 20);
//     std::cout << "  Found: ";
//     for (float k : range) std::cout << k << " ";
//     std::cout << "\n";

//     // ── DELETE ───────────────────────────────────────────────
//     step("Delete key 20");
//     std::cout << "  find 20 in tree\n";
//     std::cout << "  20 is in internal node -> replace with predecessor or successor\n";
//     std::cout << "  fix any underfull nodes by borrowing or merging\n";
//     std::cout << "\n  Tree before delete:\n";
//     printBtree(tree.getRoot());
//     tree.remove(20);
//     std::cout << "\n  Tree after delete:\n";
//     printBtree(tree.getRoot());

//     step("Delete key 5 (leaf node deletion)");
//     std::cout << "  find 5 in leaf -> erase directly\n";
//     std::cout << "  check if leaf is underfull -> borrow from sibling if needed\n";
//     std::cout << "\n  Tree before:\n";
//     printBtree(tree.getRoot());
//     tree.remove(5);
//     std::cout << "\n  Tree after:\n";
//     printBtree(tree.getRoot());

//     std::cout << "\n  Remaining keys visible in tree above.\n";
//     std::cout << "  (5 and 20 are gone, 10, 15, 25 remain)\n";
//     bool check1 = tree.search(5);
//     bool check2 = tree.search(20);
//     bool check3 = tree.search(15);
//     std::cout << "  search(5)  -> " << (check1?"FOUND":"NOT FOUND") << "\n";
//     std::cout << "  search(20) -> " << (check2?"FOUND":"NOT FOUND") << "\n";
//     std::cout << "  search(15) -> " << (check3?"FOUND":"NOT FOUND") << "\n";
// }

// // ════════════════════════════════════════════════════════════
// //  MAIN
// // ════════════════════════════════════════════════════════════

// int main() {
//     rtreeWalkthrough();
//     btreeWalkthrough();
//     return 0;
// }
#include "Rtree.hpp"
#include "Btree.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>

using Clock = std::chrono::high_resolution_clock;
long long usNow(Clock::time_point t0) {
    return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-t0).count();
}

// ════════════════════════════════════════════════════════════
//  PRINT HELPERS
// ════════════════════════════════════════════════════════════

void divider(const std::string& title) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║  " << title;
    int pad = 44 - (int)title.size();
    for (int i = 0; i < pad; ++i) std::cout << ' ';
    std::cout << "║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";
}

void step(const std::string& msg) {
    std::cout << "\n--------------------------------------------------\n";
    std::cout << "  STEP: " << msg << "\n";
    std::cout << "--------------------------------------------------\n";
}

// ════════════════════════════════════════════════════════════
//  R-TREE PRINTER
//  Shows tree level by level with MBRs and stored rects
// ════════════════════════════════════════════════════════════

void printRtree(RtreeNode* node, int depth = 0) {
    if (!node) return;
    std::string indent(depth * 4, ' ');
    if (node->isLeaf) {
        std::cout << indent << "[LEAF]     MBR=("
                  << node->mbr.x1<<","<<node->mbr.y1<<")->("
                  << node->mbr.x2<<","<<node->mbr.y2<<")\n";
        for (const Rectangle& r : node->items)
            std::cout << indent << "  |-- rect: ("
                      << r.x1<<","<<r.y1<<")->("
                      << r.x2<<","<<r.y2<<")\n";
    } else {
        std::cout << indent << "[INTERNAL] MBR=("
                  << node->mbr.x1<<","<<node->mbr.y1<<")->("
                  << node->mbr.x2<<","<<node->mbr.y2
                  << ")  children="<<node->children.size()<<"\n";
        for (RtreeNode* child : node->children)
            printRtree(child, depth + 1);
    }
}

// ════════════════════════════════════════════════════════════
//  B-TREE PRINTER
//  Shows each node's keys with indentation per level
// ════════════════════════════════════════════════════════════

void printBtree(BtreeNode* node, int depth = 0) {
    if (!node) return;
    std::string indent(depth * 4, ' ');
    std::cout << indent;
    if (node->isLeaf) std::cout << "[LEAF] keys: [ ";
    else              std::cout << "[NODE] keys: [ ";
    for (float k : node->keys) std::cout << k << " ";
    std::cout << "]\n";
    for (BtreeNode* child : node->children)
        printBtree(child, depth + 1);
}

// ════════════════════════════════════════════════════════════
//  R-TREE VISUAL WALKTHROUGH
// ════════════════════════════════════════════════════════════

void rtreeWalkthrough() {
    divider("R-TREE STEP BY STEP WALKTHROUGH");
    std::cout << "  maxChildren=3 so splits happen quickly and are easy to see\n";

    Rtree tree(3);

    // ── INSERT A ─────────────────────────────────────────────
    step("INSERT rect A: (0,0)->(2,2)");
    std::cout << "  chooseLeaf  -> root is the only node, go there\n";
    std::cout << "  insert      -> add A to leaf items\n";
    std::cout << "  adjustTree  -> leaf MBR updated to (0,0)->(2,2)\n";
    tree.insert({0,0,2,2});
    std::cout << "\n  Tree:\n";
    printRtree(tree.getRoot());

    // ── INSERT B ─────────────────────────────────────────────
    step("INSERT rect B: (5,5)->(8,8)");
    std::cout << "  chooseLeaf  -> root is still a leaf, go there\n";
    std::cout << "  insert      -> add B alongside A (count=2, under max=3)\n";
    std::cout << "  adjustTree  -> MBR expands to cover A and B\n";
    tree.insert({5,5,8,8});
    std::cout << "\n  Tree:\n";
    printRtree(tree.getRoot());

    // ── INSERT C ─────────────────────────────────────────────
    step("INSERT rect C: (1,5)->(3,7)");
    std::cout << "  chooseLeaf  -> root is leaf, go there\n";
    std::cout << "  insert      -> add C (count=3, exactly at max=3, still OK)\n";
    std::cout << "  adjustTree  -> MBR grows again\n";
    tree.insert({1,5,3,7});
    std::cout << "\n  Tree:\n";
    printRtree(tree.getRoot());

    // ── INSERT D — SPLIT ─────────────────────────────────────
    step("INSERT rect D: (6,1)->(9,3)  <-- OVERFLOW: SPLIT HAPPENS HERE");
    std::cout << "  chooseLeaf  -> root is leaf, go there\n";
    std::cout << "  insert      -> count becomes 4 > maxChildren=3  OVERFLOW!\n";
    std::cout << "\n  splitNode START:\n";
    std::cout << "    PickSeeds : scan all pairs, find the pair wasting most dead space\n";
    std::cout << "                A(0,0,2,2) + B(5,5,8,8) waste the most -> become seeds\n";
    std::cout << "    Group 1   : [ A ]\n";
    std::cout << "    Group 2   : [ B ]\n";
    std::cout << "    PickNext  : assign C and D to whichever group grows less\n";
    std::cout << "                C(1,5,3,7) closer to B -> Group 2\n";
    std::cout << "                D(6,1,9,3) closer to A -> Group 1\n";
    std::cout << "    Result    : Group1=[A,D]  Group2=[B,C]\n";
    std::cout << "  splitNode END\n";
    std::cout << "\n  adjustTree  -> reached root with a split\n";
    std::cout << "                 create NEW ROOT above both halves\n";
    std::cout << "                 tree height grows by 1\n";
    tree.insert({6,1,9,3});
    std::cout << "\n  Tree (root is now INTERNAL, two leaf children):\n";
    printRtree(tree.getRoot());

    // ── INSERT E ─────────────────────────────────────────────
    step("INSERT rect E: (2,2)->(4,4)");
    std::cout << "  chooseLeaf  -> root is internal, must pick best child\n";
    std::cout << "    Child 1 MBR=(0,0,9,3): enlargement needed to include E = X\n";
    std::cout << "    Child 2 MBR=(1,5,8,8): enlargement needed to include E = Y\n";
    std::cout << "    Pick child needing LEAST enlargement\n";
    std::cout << "  insert      -> add E to chosen leaf\n";
    std::cout << "  adjustTree  -> recompute MBR of parent\n";
    tree.insert({2,2,4,4});
    std::cout << "\n  Tree:\n";
    printRtree(tree.getRoot());

    // ── SEARCH ───────────────────────────────────────────────
    step("SEARCH query box: (0,0)->(5,5)");
    std::cout << "  At root (internal):\n";
    std::cout << "    Child 1 MBR overlaps (0,0,5,5)? -> check and descend if YES\n";
    std::cout << "    Child 2 MBR overlaps (0,0,5,5)? -> check and descend if YES\n";
    std::cout << "  At each leaf: compare every stored rect against query\n";
    std::cout << "  Only rects that OVERLAP the query box are collected\n";
    std::cout << "  MBRs that do NOT overlap -> entire subtree SKIPPED (pruning)\n";
    Rectangle qbox={0,0,5,5};
    std::vector<Rectangle> results;
    tree.search(tree.getRoot(), qbox, results);
    std::cout << "\n  Results (" << results.size() << " found):\n";
    for (const auto& r : results)
        std::cout << "    ("<<r.x1<<","<<r.y1<<")->(" <<r.x2<<","<<r.y2<<")\n";

    // ── DELETE ───────────────────────────────────────────────
    step("DELETE rect B: (5,5)->(8,8)");
    std::cout << "  findLeaf    -> start at root\n";
    std::cout << "                 at internal: descend only into children\n";
    std::cout << "                 whose MBR overlaps B\n";
    std::cout << "                 at leaf: check items with equals()\n";
    std::cout << "                 -> leaf found containing B\n";
    std::cout << "  remove      -> erase B from leaf items list\n";
    std::cout << "\n  condenseTree START (walk up from leaf):\n";
    std::cout << "    leaf now has 1 item  minChildren=1  -> NOT underfull, just update MBR\n";
    std::cout << "    reach root            -> stop\n";
    std::cout << "  condenseTree END\n";
    std::cout << "  No orphans to re-insert this time\n";
    tree.remove({5,5,8,8});
    std::cout << "\n  Tree after delete:\n";
    printRtree(tree.getRoot());

    // ── SEARCH AFTER DELETE ───────────────────────────────────
    step("SEARCH (0,0)->(10,10) after delete to confirm B is gone");
    std::vector<Rectangle> after;
    tree.search(tree.getRoot(), {0,0,10,10}, after);
    std::cout << "  Remaining rects (" << after.size() << "):\n";
    for (const auto& r : after)
        std::cout << "    ("<<r.x1<<","<<r.y1<<")->("<<r.x2<<","<<r.y2<<")\n";
    std::cout << "  B=(5,5)->(8,8) is gone. All others present.\n";
}

// ════════════════════════════════════════════════════════════
//  B-TREE VISUAL WALKTHROUGH
// ════════════════════════════════════════════════════════════

void btreeWalkthrough() {
    divider("B-TREE STEP BY STEP WALKTHROUGH");
    std::cout << "  t=2: each node holds max 3 keys (2t-1), min 1 key (t-1)\n";
    std::cout << "  splits happen quickly so structure is easy to follow\n";

    Btree tree(2);

    // ── INSERTS ──────────────────────────────────────────────
    step("INSERT keys one by one: 10, 20, 5, 15, 25");

    std::cout << "\n  Insert 10 -> goes into root leaf\n";
    tree.insert(10);
    printBtree(tree.getRoot());

    std::cout << "\n  Insert 20 -> placed right of 10 (sorted order)\n";
    tree.insert(20);
    printBtree(tree.getRoot());

    std::cout << "\n  Insert 5 -> placed left of 10 (sorted order)\n";
    tree.insert(5);
    printBtree(tree.getRoot());

    std::cout << "\n  Insert 15 -> node is FULL (3 keys = 2t-1)\n";
    std::cout << "  insertNonFull detects full child -> splitChild called\n";
    std::cout << "  median key 10 rises to new root\n";
    std::cout << "  left  child gets: [ 5 ]\n";
    std::cout << "  right child gets: [ 15 20 ]\n";
    tree.insert(15);
    printBtree(tree.getRoot());

    std::cout << "\n  Insert 25 -> descend right (25 > root key 10)\n";
    std::cout << "  right child [ 15 20 ] has room -> insert directly\n";
    tree.insert(25);
    printBtree(tree.getRoot());

    // ── SEARCH ───────────────────────────────────────────────
    step("SEARCH for key 15");
    std::cout << "  At root: keys=[ 10 ]\n";
    std::cout << "    15 > 10 -> go to right child\n";
    std::cout << "  At right leaf: keys=[ 15 20 25 ]\n";
    std::cout << "    15 == 15 -> FOUND\n";
    bool f1 = tree.search(15);
    std::cout << "  Result: " << (f1?"FOUND":"NOT FOUND") << "\n";

    step("SEARCH for key 99 (not in tree)");
    std::cout << "  At root: 99 > 10 -> go right\n";
    std::cout << "  At right leaf: 99 > 25 -> no more children -> NOT FOUND\n";
    bool f2 = tree.search(99);
    std::cout << "  Result: " << (f2?"FOUND":"NOT FOUND") << "\n";

    // ── RANGE SEARCH ─────────────────────────────────────────
    step("RANGE SEARCH: all keys between 10 and 20");
    std::cout << "  Walk tree in order\n";
    std::cout << "  Skip keys < 10\n";
    std::cout << "  Collect 10, 15, 20\n";
    std::cout << "  Stop when key > 20\n";
    std::vector<float> range = tree.rangeSearch(10, 20);
    std::cout << "  Found: ";
    for (float k : range) std::cout << k << " ";
    std::cout << "\n";

    // ── DELETE 20 ────────────────────────────────────────────
    step("DELETE key 20 (from leaf)");
    std::cout << "  Tree before:\n";
    printBtree(tree.getRoot());
    std::cout << "\n  find 20 -> in right leaf [ 15 20 25 ]\n";
    std::cout << "  20 is in a leaf -> erase directly\n";
    std::cout << "  leaf still has 2 keys >= minKeys=1 -> no borrow/merge needed\n";
    tree.remove(20);
    std::cout << "\n  Tree after:\n";
    printBtree(tree.getRoot());

    // ── DELETE 5 ─────────────────────────────────────────────
    step("DELETE key 5 (triggers borrow from sibling)");
    std::cout << "  Tree before:\n";
    printBtree(tree.getRoot());
    std::cout << "\n  find 5 -> in left leaf [ 5 ]\n";
    std::cout << "  erase 5 -> left leaf now EMPTY (0 keys < minKeys=1)\n";
    std::cout << "  fill()  -> right sibling [ 15 25 ] has spare keys\n";
    std::cout << "  borrowFromNext:\n";
    std::cout << "    root key 10 drops into left leaf\n";
    std::cout << "    right sibling's first key 15 rises to root\n";
    std::cout << "    left  leaf: [ 10 ]\n";
    std::cout << "    root  key:  [ 15 ]\n";
    std::cout << "    right leaf: [ 25 ]\n";
    tree.remove(5);
    std::cout << "\n  Tree after:\n";
    printBtree(tree.getRoot());

    // ── VERIFY ───────────────────────────────────────────────
    step("VERIFY: confirm deleted keys are gone");
    std::cout << "  search(5)  -> " << (tree.search(5) ?"FOUND":"NOT FOUND") << "\n";
    std::cout << "  search(20) -> " << (tree.search(20)?"FOUND":"NOT FOUND") << "\n";
    std::cout << "  search(10) -> " << (tree.search(10)?"FOUND":"NOT FOUND") << "\n";
    std::cout << "  search(15) -> " << (tree.search(15)?"FOUND":"NOT FOUND") << "\n";
    std::cout << "  search(25) -> " << (tree.search(25)?"FOUND":"NOT FOUND") << "\n";
}

// ════════════════════════════════════════════════════════════
//  COMPARISON ANALYSIS
// ════════════════════════════════════════════════════════════

void runComparison(int n) {
    std::cout << "\n--- N = " << n << " ---\n";
    std::cout << std::left  << std::setw(30) << "Operation"
              << std::right << std::setw(12) << "R-tree(us)"
              << std::setw(12) << "B-tree(us)" << "\n";
    std::cout << std::string(54, '-') << "\n";

    std::vector<Rectangle> rects;
    std::vector<float> keys;
    for (int i = 0; i < n; ++i) {
        rects.push_back({(float)i,(float)i,(float)i+1,(float)i+1});
        keys.push_back((float)i);
    }

    // INSERT
    Rtree rtree(4);
    auto t0 = Clock::now();
    for (const Rectangle& r : rects) rtree.insert(r);
    long long rtIns = usNow(t0);

    Btree btree(2);
    t0 = Clock::now();
    for (float k : keys) btree.insert(k);
    long long btIns = usNow(t0);
    std::cout << std::left  << std::setw(30) << "Insert"
              << std::right << std::setw(12) << rtIns
              << std::setw(12) << btIns << "\n";

    // SEARCH — 100 queries
    float mid = (float)n / 2;
    Rectangle qbox = {mid-n*0.05f, mid-n*0.05f, mid+n*0.05f, mid+n*0.05f};
    std::vector<Rectangle> rRes;
    t0 = Clock::now();
    for (int q = 0; q < 100; ++q) { rRes.clear(); rtree.search(rtree.getRoot(), qbox, rRes); }
    long long rtSrch = usNow(t0);

    t0 = Clock::now();
    for (int q = 0; q < 100; ++q) btree.search((float)(n/2));
    long long btSrch = usNow(t0);
    std::cout << std::left  << std::setw(30) << "100x Search"
              << std::right << std::setw(12) << rtSrch
              << std::setw(12) << btSrch << "\n";
    std::cout << "  R-tree hits per query : " << rRes.size() << "\n";

    // RANGE SEARCH
    t0 = Clock::now();
    std::vector<float> bRange = btree.rangeSearch((float)(n/2-n/20),(float)(n/2+n/20));
    long long btRange = usNow(t0);
    std::cout << std::left  << std::setw(30) << "B-tree rangeSearch"
              << std::right << std::setw(12) << "N/A"
              << std::setw(12) << btRange << "\n";
    std::cout << "  B-tree range keys found: " << bRange.size() << "\n";

    // DELETE 10%
    int del = n / 10;
    t0 = Clock::now();
    for (int i = 0; i < del; ++i) rtree.remove(rects[i]);
    long long rtDel = usNow(t0);

    t0 = Clock::now();
    for (int i = 0; i < del; ++i) btree.remove(keys[i]);
    long long btDel = usNow(t0);
    std::cout << std::left  << std::setw(30) << "Delete 10%"
              << std::right << std::setw(12) << rtDel
              << std::setw(12) << btDel << "\n";
}

void analysisSection() {
    divider("R-TREE vs B-TREE COMPARISON ANALYSIS");

    std::cout << "\n  What is being measured:\n";
    std::cout << "  INSERT     : time to insert N items into each tree\n";
    std::cout << "  100x Search: 100 repeated queries (spatial range vs exact lookup)\n";
    std::cout << "  rangeSearch: B-tree finds all keys in a 1D range\n";
    std::cout << "  Delete 10% : time to delete first N/10 items\n";

    runComparison(500);
    runComparison(2000);
    runComparison(5000);

    std::cout << "\n--------------------------------------------------\n";
    std::cout << "  WHAT THE NUMBERS TELL YOU\n";
    std::cout << "--------------------------------------------------\n";
    std::cout << "  Insert:\n";
    std::cout << "    B-tree compares scalar keys -> simple and fast\n";
    std::cout << "    R-tree computes MBR enlargement at every level -> heavier\n\n";
    std::cout << "  Search:\n";
    std::cout << "    B-tree does exact O(log n) point lookup -> very fast\n";
    std::cout << "    R-tree returns ALL rects in a spatial region -> more results\n";
    std::cout << "    B-tree CANNOT answer spatial range queries at all\n\n";
    std::cout << "  Delete:\n";
    std::cout << "    B-tree borrows/merges siblings -> stays in place\n";
    std::cout << "    R-tree dissolves underfull nodes and re-inserts -> slower\n";
    std::cout << "    but re-insertion improves tree quality over time\n\n";
    std::cout << "  Conclusion:\n";
    std::cout << "    B-tree wins on speed for 1D sorted key operations\n";
    std::cout << "    R-tree is the only option for 2D spatial queries\n";
    std::cout << "    They solve fundamentally different problems\n";
}

// ════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════

int main() {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║   R-TREE & B-TREE  COMPLETE DEMO             ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";

    rtreeWalkthrough();
    btreeWalkthrough();
    analysisSection();

    std::cout << "\n╔══════════════════════════════════════════════╗\n";
    std::cout << "║              DEMO COMPLETE                    ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n";
    return 0;
}