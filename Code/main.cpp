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
// #include "Btree.hpp"
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

void printResults(const std::string& label, std::vector<Rectangle>& results) {
    std::cout << label << " -- found " << results.size() << " rect(s):\n";
    for (auto& r : results)
        std::cout << "  [(" << r.x1 << "," << r.y1
                  << ") -> (" << r.x2 << "," << r.y2 << ")]\n";
}

int main() {

    // ════════════════════════════════════════
    //  INSERT TESTS
    // ════════════════════════════════════════

    std::cout << "========================================\n";
    std::cout << "         INSERT TESTS\n";
    std::cout << "========================================\n";

    // ── Insert Test 1: basic insert and search ───────────────────────
    std::cout << "\n[Insert Test 1] Basic insert and search\n";
    Rtree tree1(4);
    tree1.insert({0, 0, 2, 2});
    tree1.insert({3, 3, 5, 5});
    tree1.insert({1, 1, 4, 4});
    tree1.insert({6, 0, 8, 2});
    tree1.insert({7, 7, 9, 9});

    Rectangle q1 = {0, 0, 3, 3};
    std::vector<Rectangle> r1;
    tree1.search(tree1.getRoot(), q1, r1);
    printResults("Query (0,0)->(3,3)", r1);
    assert(r1.size() >= 2 && "expected at least 2 overlapping rects");
    std::cout << "  PASS\n";

    // ── Insert Test 2: no false positives ────────────────────────────
    std::cout << "\n[Insert Test 2] No false positives\n";
    std::vector<Rectangle> r2;
    Rectangle farQ = {100, 100, 200, 200};
    tree1.search(tree1.getRoot(), farQ, r2);
    printResults("Far query (100,100)->(200,200)", r2);
    assert(r2.empty() && "expected 0 results for far query");
    std::cout << "  PASS\n";

    // ── Insert Test 3: force node split (insert > maxChildren) ───────
    std::cout << "\n[Insert Test 3] Force node split with 10 inserts\n";
    Rtree tree3(4);
    for (int i = 0; i < 10; ++i)
        tree3.insert({(float)i, (float)i, (float)i+1, (float)i+1});

    Rectangle q3 = {3, 3, 7, 7};
    std::vector<Rectangle> r3;
    tree3.search(tree3.getRoot(), q3, r3);
    printResults("Query (3,3)->(7,7)", r3);
    assert(!r3.empty() && "expected results after split");
    std::cout << "  PASS\n";

    // ── Insert Test 4: duplicate inserts ─────────────────────────────
    std::cout << "\n[Insert Test 4] Duplicate inserts\n";
    Rtree tree4(4);
    tree4.insert({1, 1, 3, 3});
    tree4.insert({1, 1, 3, 3});
    tree4.insert({1, 1, 3, 3});

    Rectangle q4 = {0, 0, 5, 5};
    std::vector<Rectangle> r4;
    tree4.search(tree4.getRoot(), q4, r4);
    printResults("Query (0,0)->(5,5)", r4);
    assert(r4.size() == 3 && "expected all 3 duplicates");
    std::cout << "  PASS\n";

    // ── Insert Test 5: large bulk insert ─────────────────────────────
    std::cout << "\n[Insert Test 5] Bulk insert 50 items\n";
    Rtree tree5(4);
    for (int i = 0; i < 50; ++i)
        tree5.insert({(float)i, (float)i, (float)i+1, (float)i+1});

    Rectangle q5 = {0, 0, 100, 100};
    std::vector<Rectangle> r5;
    tree5.search(tree5.getRoot(), q5, r5);
    printResults("Query all (0,0)->(100,100)", r5);
    assert((int)r5.size() == 50 && "expected all 50 items");
    std::cout << "  PASS\n";

    // ════════════════════════════════════════
    //  DELETE TESTS
    // ════════════════════════════════════════

    std::cout << "\n========================================\n";
    std::cout << "         DELETE TESTS\n";
    std::cout << "========================================\n";

    // ── Delete Test 1: basic delete ──────────────────────────────────
    std::cout << "\n[Delete Test 1] Basic delete\n";
    Rtree dtree1(4);
    dtree1.insert({0, 0, 2, 2});
    dtree1.insert({3, 3, 5, 5});
    dtree1.insert({1, 1, 4, 4});
    dtree1.insert({6, 0, 8, 2});
    dtree1.insert({7, 7, 9, 9});

    Rectangle dq1 = {0, 0, 10, 10};
    std::vector<Rectangle> before1;
    dtree1.search(dtree1.getRoot(), dq1, before1);
    printResults("Before delete", before1);

    bool removed = dtree1.remove({3, 3, 5, 5});
    assert(removed && "remove should return true for existing rect");

    std::vector<Rectangle> after1;
    dtree1.search(dtree1.getRoot(), dq1, after1);
    printResults("After delete (3,3)->(5,5)", after1);
    assert(after1.size() == before1.size() - 1 && "count should drop by 1");

    for (auto& r : after1)
        assert(!(r.x1==3 && r.y1==3 && r.x2==5 && r.y2==5)
               && "deleted rect must not appear");
    std::cout << "  PASS\n";

    // ── Delete Test 2: delete non-existent rect ───────────────────────
    std::cout << "\n[Delete Test 2] Delete non-existent rectangle\n";
    bool notFound = dtree1.remove({99, 99, 100, 100});
    assert(!notFound && "remove should return false for missing rect");
    std::cout << "  PASS: returned false correctly\n";

    // ── Delete Test 3: delete then search confirms removal ────────────
    std::cout << "\n[Delete Test 3] Search confirms deletion\n";
    Rtree dtree3(4);
    dtree3.insert({5, 5, 10, 10});
    dtree3.insert({1, 1, 3,  3 });
    dtree3.insert({7, 2, 9,  4 });

    dtree3.remove({5, 5, 10, 10});

    Rectangle dq3 = {0, 0, 15, 15};
    std::vector<Rectangle> dr3;
    dtree3.search(dtree3.getRoot(), dq3, dr3);
    printResults("After removing (5,5)->(10,10)", dr3);
    assert((int)dr3.size() == 2 && "expected 2 remaining");
    for (auto& r : dr3)
        assert(!(r.x1==5 && r.y1==5 && r.x2==10 && r.y2==10)
               && "removed rect must not appear");
    std::cout << "  PASS\n";

    // ── Delete Test 4: condense tree (delete triggers underfull) ──────
    std::cout << "\n[Delete Test 4] Condense tree after bulk delete\n";
    Rtree dtree4(4);
    std::vector<Rectangle> bulk;
    for (int i = 0; i < 12; ++i) {
        Rectangle r = {(float)i*2, (float)i*2,
                       (float)i*2+1, (float)i*2+1};
        bulk.push_back(r);
        dtree4.insert(r);
    }
    for (int i = 0; i < 6; ++i) {
        bool ok = dtree4.remove(bulk[i]);
        assert(ok && "each inserted rect should be removable");
    }
    Rectangle dq4 = {0, 0, 100, 100};
    std::vector<Rectangle> dr4;
    dtree4.search(dtree4.getRoot(), dq4, dr4);
    printResults("After deleting 6 of 12", dr4);
    assert((int)dr4.size() == 6 && "expected 6 remaining");
    std::cout << "  PASS\n";

    // ── Delete Test 5: delete all items ──────────────────────────────
    std::cout << "\n[Delete Test 5] Delete all items\n";
    Rtree dtree5(4);
    std::vector<Rectangle> all;
    for (int i = 0; i < 8; ++i)
    {
        Rectangle r = {(float)i, (float)i, (float)i + 1, (float)i + 1};
        all.push_back(r);
        dtree5.insert(r);
    }
    for (const Rectangle& r : all) {
        bool ok = dtree5.remove(r);
        assert(ok && "every rect should be removable");
    }
    Rectangle dq5 = {0, 0, 100, 100};
    std::vector<Rectangle> dr5;
    dtree5.search(dtree5.getRoot(), dq5, dr5);
    assert(dr5.empty() && "tree must be empty after removing all");
    std::cout << "  PASS: tree is empty\n";

    // ════════════════════════════════════════
    std::cout << "\n========================================\n";
    std::cout << "   ALL INSERT AND DELETE TESTS PASSED\n";
    std::cout << "========================================\n";

    return 0;
}
