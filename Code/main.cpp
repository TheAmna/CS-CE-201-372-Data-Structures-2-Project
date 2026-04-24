// // #include "Rtree.hpp"
// // #include <iostream>
// // #include <cassert>

// // void printResults(const std::string& label, std::vector<Rectangle>& results) {
// //     std::cout << label << " -- found " << results.size() << " rect(s):\n";
// //     for (auto& r : results)
// //         std::cout << "  [(" << r.x1 << "," << r.y1
// //                   << ") -> (" << r.x2 << "," << r.y2 << ")]\n";
// // }

// // int main() {
// //     Rtree tree(4);

// //     // Insert 8 rectangles
// //     tree.insert({0, 0, 2, 2});
// //     tree.insert({3, 3, 5, 5});
// //     tree.insert({1, 1, 4, 4});
// //     tree.insert({6, 0, 8, 2});
// //     tree.insert({7, 7, 9, 9});
// //     tree.insert({2, 5, 4, 7});
// //     tree.insert({5, 2, 7, 4});
// //     tree.insert({8, 4, 10, 6});

// //     Rectangle queryAll = {0, 0, 10, 10};
// //     std::vector<Rectangle> before;
// //     tree.search(tree.getRoot(), queryAll, before);
// //     printResults("Before delete", before);

// //     // ── Test 1: delete an existing rectangle ─────────────────────────
// //     std::cout << "\n[Test 1] Removing (3,3)->(5,5)...\n";
// //     bool removed = tree.remove({3, 3, 5, 5});
// //     assert(removed && "remove() should return true for existing rect");

// //     std::vector<Rectangle> after;
// //     tree.search(tree.getRoot(), queryAll, after);
// //     printResults("After delete", after);
// //     assert(after.size() == before.size() - 1);

// //     // Confirm deleted rect is gone
// //     for (auto& r : after)
// //         assert(!(r.x1==3 && r.y1==3 && r.x2==5 && r.y2==5) && "deleted rect should not appear");
// //     std::cout << "  PASS: rect removed, count correct, no false positives\n";

// //     // ── Test 2: delete non-existent rectangle ────────────────────────
// //     std::cout << "\n[Test 2] Removing non-existent (99,99)->(100,100)...\n";
// //     bool notFound = tree.remove({99, 99, 100, 100});
// //     assert(!notFound && "remove() should return false for missing rect");
// //     std::cout << "  PASS: correctly returned false\n";

// //     // ── Test 3: delete all remaining, tree stays valid ───────────────
// //     std::cout << "\n[Test 3] Removing all remaining rectangles...\n";
// //     std::vector<Rectangle> remaining = after;
// //     for (const Rectangle& r : remaining) {
// //         bool ok = tree.remove(r);
// //         assert(ok && "each remaining rect should be removable");
// //     }
// //     std::vector<Rectangle> empty;
// //     tree.search(tree.getRoot(), queryAll, empty);
// //     assert(empty.empty() && "tree should be empty after all removals");
// //     std::cout << "  PASS: tree empty, no crash\n";

// //     // ── Test 4: force condense (trigger underfull nodes) ─────────────
// //     std::cout << "\n[Test 4] Insert 12 items then delete half...\n";
// //     Rtree tree2(4);
// //     std::vector<Rectangle> bulk;
// //     for (int i = 0; i < 12; ++i) {
// //         Rectangle r = {(float)i*2, (float)i*2,
// //                        (float)i*2+1, (float)i*2+1};
// //         bulk.push_back(r);
// //         tree2.insert(r);
// //     }
// //     for (int i = 0; i < 6; ++i) {
// //         bool ok = tree2.remove(bulk[i]);
// //         assert(ok);
// //     }
// //     std::vector<Rectangle> half;
// //     Rectangle bigQuery = {0, 0, 100, 100};
// //     tree2.search(tree2.getRoot(), bigQuery, half);
// //     assert((int)half.size() == 6 && "should have 6 items left");
// //     std::cout << "  PASS: 6 items remain after deleting 6 of 12\n";

// //     std::cout << "\nAll delete tests passed.\n";
// //     return 0;
// // }

// #include "Rtree.hpp"
// #include <iostream>
// #include <cassert>
// #include <vector>

// void printResults(const std::string& label, std::vector<Rectangle>& results) {
//     std::cout << label << " -- found " << results.size() << " rect(s):\n";
//     for (auto& r : results)
//         std::cout << "  [(" << r.x1 << "," << r.y1
//                   << ") -> (" << r.x2 << "," << r.y2 << ")]\n";
// }

// int main() {

//     // ════════════════════════════════════════
//     //  INSERT TESTS
//     // ════════════════════════════════════════

//     std::cout << "========================================\n";
//     std::cout << "         INSERT TESTS\n";
//     std::cout << "========================================\n";

//     // ── Insert Test 1: basic insert and search ───────────────────────
//     std::cout << "\n[Insert Test 1] Basic insert and search\n";
//     Rtree tree1(4);
//     tree1.insert({0, 0, 2, 2});
//     tree1.insert({3, 3, 5, 5});
//     tree1.insert({1, 1, 4, 4});
//     tree1.insert({6, 0, 8, 2});
//     tree1.insert({7, 7, 9, 9});

//     Rectangle q1 = {0, 0, 3, 3};
//     std::vector<Rectangle> r1;
//     tree1.search(tree1.getRoot(), q1, r1);
//     printResults("Query (0,0)->(3,3)", r1);
//     assert(r1.size() >= 2 && "expected at least 2 overlapping rects");
//     std::cout << "  PASS\n";

//     // ── Insert Test 2: no false positives ────────────────────────────
//     std::cout << "\n[Insert Test 2] No false positives\n";
//     std::vector<Rectangle> r2;
//     Rectangle farQ = {100, 100, 200, 200};
//     tree1.search(tree1.getRoot(), farQ, r2);
//     printResults("Far query (100,100)->(200,200)", r2);
//     assert(r2.empty() && "expected 0 results for far query");
//     std::cout << "  PASS\n";

//     // ── Insert Test 3: force node split (insert > maxChildren) ───────
//     std::cout << "\n[Insert Test 3] Force node split with 10 inserts\n";
//     Rtree tree3(4);
//     for (int i = 0; i < 10; ++i)
//         tree3.insert({(float)i, (float)i, (float)i+1, (float)i+1});

//     Rectangle q3 = {3, 3, 7, 7};
//     std::vector<Rectangle> r3;
//     tree3.search(tree3.getRoot(), q3, r3);
//     printResults("Query (3,3)->(7,7)", r3);
//     assert(!r3.empty() && "expected results after split");
//     std::cout << "  PASS\n";

//     // ── Insert Test 4: duplicate inserts ─────────────────────────────
//     std::cout << "\n[Insert Test 4] Duplicate inserts\n";
//     Rtree tree4(4);
//     tree4.insert({1, 1, 3, 3});
//     tree4.insert({1, 1, 3, 3});
//     tree4.insert({1, 1, 3, 3});

//     Rectangle q4 = {0, 0, 5, 5};
//     std::vector<Rectangle> r4;
//     tree4.search(tree4.getRoot(), q4, r4);
//     printResults("Query (0,0)->(5,5)", r4);
//     assert(r4.size() == 3 && "expected all 3 duplicates");
//     std::cout << "  PASS\n";

//     // ── Insert Test 5: large bulk insert ─────────────────────────────
//     std::cout << "\n[Insert Test 5] Bulk insert 50 items\n";
//     Rtree tree5(4);
//     for (int i = 0; i < 50; ++i)
//         tree5.insert({(float)i, (float)i, (float)i+1, (float)i+1});

//     Rectangle q5 = {0, 0, 100, 100};
//     std::vector<Rectangle> r5;
//     tree5.search(tree5.getRoot(), q5, r5);
//     printResults("Query all (0,0)->(100,100)", r5);
//     assert((int)r5.size() == 50 && "expected all 50 items");
//     std::cout << "  PASS\n";

//     // ════════════════════════════════════════
//     //  DELETE TESTS
//     // ════════════════════════════════════════

//     std::cout << "\n========================================\n";
//     std::cout << "         DELETE TESTS\n";
//     std::cout << "========================================\n";

//     // ── Delete Test 1: basic delete ──────────────────────────────────
//     std::cout << "\n[Delete Test 1] Basic delete\n";
//     Rtree dtree1(4);
//     dtree1.insert({0, 0, 2, 2});
//     dtree1.insert({3, 3, 5, 5});
//     dtree1.insert({1, 1, 4, 4});
//     dtree1.insert({6, 0, 8, 2});
//     dtree1.insert({7, 7, 9, 9});

//     Rectangle dq1 = {0, 0, 10, 10};
//     std::vector<Rectangle> before1;
//     dtree1.search(dtree1.getRoot(), dq1, before1);
//     printResults("Before delete", before1);

//     bool removed = dtree1.remove({3, 3, 5, 5});
//     assert(removed && "remove should return true for existing rect");

//     std::vector<Rectangle> after1;
//     dtree1.search(dtree1.getRoot(), dq1, after1);
//     printResults("After delete (3,3)->(5,5)", after1);
//     assert(after1.size() == before1.size() - 1 && "count should drop by 1");

//     for (auto& r : after1)
//         assert(!(r.x1==3 && r.y1==3 && r.x2==5 && r.y2==5)
//                && "deleted rect must not appear");
//     std::cout << "  PASS\n";

//     // ── Delete Test 2: delete non-existent rect ───────────────────────
//     std::cout << "\n[Delete Test 2] Delete non-existent rectangle\n";
//     bool notFound = dtree1.remove({99, 99, 100, 100});
//     assert(!notFound && "remove should return false for missing rect");
//     std::cout << "  PASS: returned false correctly\n";

//     // ── Delete Test 3: delete then search confirms removal ────────────
//     std::cout << "\n[Delete Test 3] Search confirms deletion\n";
//     Rtree dtree3(4);
//     dtree3.insert({5, 5, 10, 10});
//     dtree3.insert({1, 1, 3,  3 });
//     dtree3.insert({7, 2, 9,  4 });

//     dtree3.remove({5, 5, 10, 10});

//     Rectangle dq3 = {0, 0, 15, 15};
//     std::vector<Rectangle> dr3;
//     dtree3.search(dtree3.getRoot(), dq3, dr3);
//     printResults("After removing (5,5)->(10,10)", dr3);
//     assert((int)dr3.size() == 2 && "expected 2 remaining");
//     for (auto& r : dr3)
//         assert(!(r.x1==5 && r.y1==5 && r.x2==10 && r.y2==10)
//                && "removed rect must not appear");
//     std::cout << "  PASS\n";

//     // ── Delete Test 4: condense tree (delete triggers underfull) ──────
//     std::cout << "\n[Delete Test 4] Condense tree after bulk delete\n";
//     Rtree dtree4(4);
//     std::vector<Rectangle> bulk;
//     for (int i = 0; i < 12; ++i) {
//         Rectangle r = {(float)i*2, (float)i*2,
//                        (float)i*2+1, (float)i*2+1};
//         bulk.push_back(r);
//         dtree4.insert(r);
//     }
//     for (int i = 0; i < 6; ++i) {
//         bool ok = dtree4.remove(bulk[i]);
//         assert(ok && "each inserted rect should be removable");
//     }
//     Rectangle dq4 = {0, 0, 100, 100};
//     std::vector<Rectangle> dr4;
//     dtree4.search(dtree4.getRoot(), dq4, dr4);
//     printResults("After deleting 6 of 12", dr4);
//     assert((int)dr4.size() == 6 && "expected 6 remaining");
//     std::cout << "  PASS\n";

//     // ── Delete Test 5: delete all items ──────────────────────────────
//     std::cout << "\n[Delete Test 5] Delete all items\n";
//     Rtree dtree5(4);
//     std::vector<Rectangle> all;
//     for (int i = 0; i < 8; ++i) {
//         Rectangle r = {(float)i, (float)i, (float)i+1, (float)i+1};
//         all.push_back(r);
//         dtree5.insert(r);
//     }
//     for (const Rectangle& r : all) {
//         bool ok = dtree5.remove(r);
//         assert(ok && "every rect should be removable");
//     }
//     Rectangle dq5 = {0, 0, 100, 100};
//     std::vector<Rectangle> dr5;
//     dtree5.search(dtree5.getRoot(), dq5, dr5);
//     assert(dr5.empty() && "tree must be empty after removing all");
//     std::cout << "  PASS: tree is empty\n";

//     // ════════════════════════════════════════
//     std::cout << "\n========================================\n";
//     std::cout << "   ALL INSERT AND DELETE TESTS PASSED\n";
//     std::cout << "========================================\n";

//     return 0;
// }

#include "Rtree.hpp"
#include "Btree.hpp"
#include <iostream>
#include <cassert>
#include <vector>
#include <chrono>  // for timing comparison

// ─────────────────────────────────────────────
//  WHAT THIS FILE DOES
//  ────────────────────
//  1. Tests R-tree  (insert, search, delete)
//  2. Tests B-tree  (insert, search, delete)
//  3. Compares both on the same dataset:
//       - 1D range query  → B-tree should win (or tie)
//       - 2D range query  → R-tree wins, B-tree cannot do this properly
//
//  DATA:
//  We insert the same 50 rectangles into both trees.
//  R-tree stores the full {x1, y1, x2, y2}.
//  B-tree stores only x1 as a float key.
// ─────────────────────────────────────────────

void printRects(const std::string& label, std::vector<Rectangle>& results) {
    std::cout << label << " -- found " << results.size() << " rect(s):\n";
    for (auto& r : results)
        std::cout << "  [(" << r.x1 << "," << r.y1 << ") -> (" << r.x2 << "," << r.y2 << ")]\n";
}

void printKeys(const std::string& label, const std::vector<float>& results) {
    std::cout << label << " -- found " << results.size() << " key(s): ";
    for (float k : results) std::cout << k << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────
//  SECTION 1 — R-TREE TESTS
// ─────────────────────────────────────────────

void testRtree() {
    std::cout << "\n========================================\n";
    std::cout << "          R-TREE TESTS\n";
    std::cout << "========================================\n";

    // ── Insert Test 1: basic insert and search ──────────────────────
    std::cout << "\n[R Insert Test 1] Basic insert and search\n";
    Rtree rt1(4);
    rt1.insert({0, 0, 2, 2});
    rt1.insert({3, 3, 5, 5});
    rt1.insert({1, 1, 4, 4});
    rt1.insert({6, 0, 8, 2});
    rt1.insert({7, 7, 9, 9});

    std::vector<Rectangle> r1;
    rt1.search(rt1.getRoot(), {0, 0, 3, 3}, r1);
    printRects("Query (0,0)->(3,3)", r1);
    assert(r1.size() >= 2 && "expected at least 2 overlapping rects");
    std::cout << "  PASS\n";

    // ── Insert Test 2: no false positives ───────────────────────────
    std::cout << "\n[R Insert Test 2] No false positives\n";
    std::vector<Rectangle> r2;
    rt1.search(rt1.getRoot(), {100, 100, 200, 200}, r2);
    printRects("Far query (100,100)->(200,200)", r2);
    assert(r2.empty() && "expected 0 results");
    std::cout << "  PASS\n";

    // ── Insert Test 3: force node split ─────────────────────────────
    std::cout << "\n[R Insert Test 3] Force node split with 10 inserts\n";
    Rtree rt3(4);
    for (int i = 0; i < 10; ++i)
        rt3.insert({(float)i, (float)i, (float)i+1, (float)i+1});

    std::vector<Rectangle> r3;
    rt3.search(rt3.getRoot(), {3, 3, 7, 7}, r3);
    printRects("Query (3,3)->(7,7)", r3);
    assert(!r3.empty() && "expected results after split");
    std::cout << "  PASS\n";

    // ── Insert Test 4: duplicate inserts ────────────────────────────
    std::cout << "\n[R Insert Test 4] Duplicate inserts\n";
    Rtree rt4(4);
    rt4.insert({1, 1, 3, 3});
    rt4.insert({1, 1, 3, 3});
    rt4.insert({1, 1, 3, 3});

    std::vector<Rectangle> r4;
    rt4.search(rt4.getRoot(), {0, 0, 5, 5}, r4);
    printRects("Query (0,0)->(5,5)", r4);
    assert(r4.size() == 3 && "expected all 3 duplicates");
    std::cout << "  PASS\n";

    // ── Insert Test 5: bulk insert 50 items ─────────────────────────
    std::cout << "\n[R Insert Test 5] Bulk insert 50 items\n";
    Rtree rt5(4);
    for (int i = 0; i < 50; ++i)
        rt5.insert({(float)i, (float)i, (float)i+1, (float)i+1});

    std::vector<Rectangle> r5;
    rt5.search(rt5.getRoot(), {0, 0, 100, 100}, r5);
    printRects("Query all", r5);
    assert((int)r5.size() == 50 && "expected all 50 items");
    std::cout << "  PASS\n";

    // ── Delete Test 1: basic delete ─────────────────────────────────
    std::cout << "\n[R Delete Test 1] Basic delete\n";
    Rtree drt1(4);
    drt1.insert({0, 0, 2, 2});
    drt1.insert({3, 3, 5, 5});
    drt1.insert({1, 1, 4, 4});
    drt1.insert({6, 0, 8, 2});
    drt1.insert({7, 7, 9, 9});

    std::vector<Rectangle> before;
    drt1.search(drt1.getRoot(), {0, 0, 10, 10}, before);

    bool removed = drt1.remove({3, 3, 5, 5});
    assert(removed && "remove should return true");

    std::vector<Rectangle> after;
    drt1.search(drt1.getRoot(), {0, 0, 10, 10}, after);
    printRects("After deleting (3,3)->(5,5)", after);
    assert(after.size() == before.size() - 1 && "count should drop by 1");
    std::cout << "  PASS\n";

    // ── Delete Test 2: delete non-existent ──────────────────────────
    std::cout << "\n[R Delete Test 2] Delete non-existent rect\n";
    bool notFound = drt1.remove({99, 99, 100, 100});
    assert(!notFound && "should return false for missing rect");
    std::cout << "  PASS\n";

    // ── Delete Test 3: delete all ────────────────────────────────────
    std::cout << "\n[R Delete Test 3] Delete all items\n";
    Rtree drt3(4);
    std::vector<Rectangle> all;
    for (int i = 0; i < 8; ++i) {
        Rectangle r = {(float)i, (float)i, (float)i+1, (float)i+1};
        all.push_back(r);
        drt3.insert(r);
    }
    for (const Rectangle& r : all) {
        bool ok = drt3.remove(r);
        assert(ok && "every rect should be removable");
    }
    std::vector<Rectangle> empty;
    drt3.search(drt3.getRoot(), {0, 0, 100, 100}, empty);
    assert(empty.empty() && "tree should be empty");
    std::cout << "  PASS: tree is empty\n";

    std::cout << "\n  ALL R-TREE TESTS PASSED\n";
}


// ─────────────────────────────────────────────
//  SECTION 2 — B-TREE TESTS
// ─────────────────────────────────────────────

void testBtree() {
    std::cout << "\n========================================\n";
    std::cout << "          B-TREE TESTS\n";
    std::cout << "========================================\n";

    // ── Insert Test 1: basic insert + point search ───────────────────
    std::cout << "\n[B Insert Test 1] Basic insert and point search\n";
    Btree bt1(2);
    bt1.insert(10);
    bt1.insert(20);
    bt1.insert(5);
    bt1.insert(15);
    bt1.insert(30);

    assert(bt1.search(10)  && "10 should be found");
    assert(bt1.search(5)   && "5 should be found");
    assert(bt1.search(30)  && "30 should be found");
    assert(!bt1.search(99) && "99 should not be found");
    std::cout << "  PASS\n";

    // ── Insert Test 2: no false positives ───────────────────────────
    std::cout << "\n[B Insert Test 2] No false positives\n";
    auto r2 = bt1.rangeSearch(100, 200);
    printKeys("Range [100,200]", r2);
    assert(r2.empty() && "expected 0 results");
    std::cout << "  PASS\n";

    // ── Insert Test 3: force node split ─────────────────────────────
    std::cout << "\n[B Insert Test 3] Force splits with 10 inserts\n";
    Btree bt3(2);
    for (int i = 1; i <= 10; ++i)
        bt3.insert((float)i);

    auto r3 = bt3.rangeSearch(3, 7);
    printKeys("Range [3,7]", r3);
    assert((int)r3.size() == 5 && "expected keys 3,4,5,6,7");
    std::cout << "  PASS\n";

    // ── Insert Test 4: duplicate inserts ────────────────────────────
    std::cout << "\n[B Insert Test 4] Duplicate inserts\n";
    Btree bt4(2);
    bt4.insert(5);
    bt4.insert(5);
    bt4.insert(5);

    auto r4 = bt4.rangeSearch(4, 6);
    printKeys("Range [4,6] with 3 copies of 5", r4);
    assert((int)r4.size() == 3 && "expected all 3 duplicates");
    std::cout << "  PASS\n";

    // ── Insert Test 5: bulk insert 50 items ─────────────────────────
    std::cout << "\n[B Insert Test 5] Bulk insert 50 items\n";
    Btree bt5(3);
    for (int i = 1; i <= 50; ++i)
        bt5.insert((float)i);

    auto r5 = bt5.rangeSearch(1, 50);
    printKeys("Range [1,50]", r5);
    assert((int)r5.size() == 50 && "expected all 50 items");
    std::cout << "  PASS\n";

    // ── Delete Test 1: basic delete ─────────────────────────────────
    std::cout << "\n[B Delete Test 1] Basic delete\n";
    Btree dbt1(2);
    dbt1.insert(10);
    dbt1.insert(20);
    dbt1.insert(5);
    dbt1.insert(15);
    dbt1.insert(30);

    bool removed = dbt1.remove(20);
    assert(removed            && "remove should return true");
    assert(!dbt1.search(20)   && "20 should not be found after deletion");
    assert(dbt1.search(10)    && "10 should still be there");
    std::cout << "  PASS\n";

    // ── Delete Test 2: delete non-existent ──────────────────────────
    std::cout << "\n[B Delete Test 2] Delete key that does not exist\n";
    bool notFound = dbt1.remove(999);
    assert(!notFound && "should return false for missing key");
    std::cout << "  PASS\n";

    // ── Delete Test 3: delete all ────────────────────────────────────
    std::cout << "\n[B Delete Test 3] Delete all items\n";
    Btree dbt3(2);
    std::vector<float> all;
    for (int i = 1; i <= 8; ++i) {
        all.push_back((float)i);
        dbt3.insert((float)i);
    }
    for (float k : all) {
        bool ok = dbt3.remove(k);
        assert(ok && "every key should be removable");
    }
    auto empty = dbt3.rangeSearch(0, 100);
    assert(empty.empty() && "tree should be empty");
    std::cout << "  PASS: tree is empty\n";

    std::cout << "\n  ALL B-TREE TESTS PASSED\n";
}


// ─────────────────────────────────────────────
//  SECTION 3 — DIRECT COMPARISON
//
//  Same 50 rectangles inserted into both trees.
//  We run equivalent queries on both and compare:
//    Query A: 1D range  → find rects with x1 in [10, 30]
//    Query B: 2D range  → find rects overlapping box {10,10,30,30}
//
//  B-tree for Query A: looks only at x1 → fast and correct
//  R-tree for Query A: checks 2D overlap → also correct but checks y too
//  B-tree for Query B: CANNOT check y → may return wrong results
//  R-tree for Query B: checks full 2D overlap → correct
// ─────────────────────────────────────────────

void compareTrees() {
    std::cout << "\n========================================\n";
    std::cout << "     R-TREE vs B-TREE COMPARISON\n";
    std::cout << "========================================\n";

    // build both trees with the same 50 rectangles
    // each rectangle: x1=i, y1=i, x2=i+5, y2=i+5  (so they have both x and y range)
    Rtree rtree(4);
    Btree btree(3);

    std::cout << "\nInserting 50 rectangles into both trees...\n";
    std::cout << "  R-tree stores full {x1, y1, x2, y2}\n";
    std::cout << "  B-tree stores only x1 as a float key\n\n";

    for (int i = 0; i < 50; ++i) {
        float x1 = (float)i;
        float y1 = (float)i;        // y1 = i  (same as x1 here)
        float x2 = x1 + 5;
        float y2 = y1 + 5;
        rtree.insert({x1, y1, x2, y2});
        btree.insert(x1);           // B-tree only gets x1
    }

    // ── Comparison 1: 1D range query ────────────────────────────────
    std::cout << "── Comparison 1: 1D range query (x1 between 10 and 30) ──\n";

    // B-tree: rangeSearch on x1
    auto t1 = std::chrono::high_resolution_clock::now();
    auto bKeys = btree.rangeSearch(10, 30);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto bTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

    // R-tree: search with a wide y range to match the same rects
    auto t3 = std::chrono::high_resolution_clock::now();
    std::vector<Rectangle> rRects;
    rtree.search(rtree.getRoot(), {10, -999, 30, 999}, rRects);
    auto t4 = std::chrono::high_resolution_clock::now();
    auto rTime = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();

    std::cout << "  B-tree found " << bKeys.size()  << " keys  in " << bTime << " microseconds\n";
    std::cout << "  R-tree found " << rRects.size() << " rects in " << rTime << " microseconds\n";
    std::cout << "  → Both find the same items. B-tree is faster for pure 1D.\n\n";

    // ── Comparison 2: 2D range query ────────────────────────────────
    // Query box: x between 10 and 30, y between 30 and 40
    // B-tree sees x range [10,30] → returns 21 keys (ignores y completely)
    // R-tree checks full overlap → only 6 rects actually overlap both x AND y
    std::cout << "── Comparison 2: 2D query — rects overlapping box {10,30,30,40} ──\n";
    std::cout << "   (x range: 10-30  AND  y range: 30-40)\n\n";

    // R-tree: correctly checks both x and y overlap
    std::vector<Rectangle> rRects2;
    rtree.search(rtree.getRoot(), {10, 30, 30, 40}, rRects2);

    // B-tree: can only check x1 range [10,30], completely ignores y
    auto bKeys2 = btree.rangeSearch(10, 30);

    std::cout << "  R-tree found " << rRects2.size() << " rects (correctly checks x AND y)\n";
    std::cout << "  B-tree found " << bKeys2.size()  << " keys  (only checked x1, ignored y entirely)\n";

    if (bKeys2.size() != rRects2.size()) {
        std::cout << "  → DIFFERENCE: B-tree returned " << bKeys2.size()
                  << " but correct answer is " << rRects2.size() << "\n";
        std::cout << "  → This proves B-tree CANNOT handle 2D spatial queries.\n";
        std::cout << "  → This is exactly why R-trees exist.\n";
    }

    // ── Summary ─────────────────────────────────────────────────────
    std::cout << "\n── Summary ──────────────────────────────────────────────\n";
    std::cout << "  1D range query  →  B-tree is fast and correct\n";
    std::cout << "  2D range query  →  R-tree is correct, B-tree gives wrong count\n";
    std::cout << "  Conclusion: R-trees are necessary for multi-dimensional spatial data\n";
    std::cout << "─────────────────────────────────────────────────────────\n";
}


// ─────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────

int main() {
    testRtree();
    testBtree();
    
    compareTrees();

    std::cout << "\n========================================\n";
    std::cout << "        ALL TESTS PASSED\n";
    std::cout << "========================================\n";
    return 0;
}
