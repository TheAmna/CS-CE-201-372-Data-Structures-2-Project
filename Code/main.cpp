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
#include <chrono>
#include <iomanip>
#include <string>

using Clock = std::chrono::high_resolution_clock;

long long usNow(Clock::time_point t0)
{
    return std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - t0).count();
}
void section(const std::string &title)
{
    std::cout << "\n========================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "========================================\n";
}
void pass(const std::string &msg) { std::cout << "  [PASS] " << msg << "\n"; }
void printRects(const std::string &label, const std::vector<Rectangle> &v)
{
    std::cout << "  " << label << " (" << v.size() << " found):\n";
    for (const auto &r : v)
        std::cout << "    (" << r.x1 << "," << r.y1 << ") -> (" << r.x2 << "," << r.y2 << ")\n";
}

// ════════════════════════════════════════════════════════════
//  R-TREE INSERT TESTS
// ════════════════════════════════════════════════════════════
void testRtreeInsert()
{
    section("R-TREE INSERT TESTS");

    std::cout << "\n[R-Insert 1] Basic insert and search\n";
    Rtree t1(4);
    t1.insert({0, 0, 2, 2});
    t1.insert({3, 3, 5, 5});
    t1.insert({1, 1, 4, 4});
    t1.insert({6, 0, 8, 2});
    t1.insert({7, 7, 9, 9});
    std::vector<Rectangle> r1;
    t1.search(t1.getRoot(), {0, 0, 3, 3}, r1);
    printRects("Query (0,0)->(3,3)", r1);
    assert(r1.size() >= 2);
    pass("at least 2 rects overlap query region");

    std::cout << "\n[R-Insert 2] No false positives\n";
    std::vector<Rectangle> r2;
    t1.search(t1.getRoot(), {100, 100, 200, 200}, r2);
    printRects("Far query (100,100)->(200,200)", r2);
    assert(r2.empty());
    pass("0 results for far region");

    std::cout << "\n[R-Insert 3] Force node split (10 inserts, maxChildren=4)\n";
    Rtree t3(4);
    for (int i = 0; i < 10; ++i)
        t3.insert({(float)i, (float)i, (float)i + 1, (float)i + 1});
    std::vector<Rectangle> r3;
    t3.search(t3.getRoot(), {3, 3, 7, 7}, r3);
    printRects("Query (3,3)->(7,7)", r3);
    assert(!r3.empty());
    pass("results returned correctly after split");

    std::cout << "\n[R-Insert 4] Duplicate inserts allowed\n";
    Rtree t4(4);
    t4.insert({1, 1, 3, 3});
    t4.insert({1, 1, 3, 3});
    t4.insert({1, 1, 3, 3});
    std::vector<Rectangle> r4;
    t4.search(t4.getRoot(), {0, 0, 5, 5}, r4);
    printRects("Query (0,0)->(5,5)", r4);
    assert((int)r4.size() == 3);
    pass("all 3 duplicates stored and found");

    std::cout << "\n[R-Insert 5] Bulk insert 50 items\n";
    Rtree t5(4);
    for (int i = 0; i < 50; ++i)
        t5.insert({(float)i, (float)i, (float)i + 1, (float)i + 1});
    std::vector<Rectangle> r5;
    t5.search(t5.getRoot(), {0, 0, 100, 100}, r5);
    std::cout << "  All-space query found: " << r5.size() << " rects\n";
    assert((int)r5.size() == 50);
    pass("all 50 items retrieved");
}

// ════════════════════════════════════════════════════════════
//  R-TREE DELETE TESTS
// ════════════════════════════════════════════════════════════
void testRtreeDelete()
{
    section("R-TREE DELETE TESTS");

    std::cout << "\n[R-Delete 1] Basic delete\n";
    Rtree t1(4);
    t1.insert({0, 0, 2, 2});
    t1.insert({3, 3, 5, 5});
    t1.insert({1, 1, 4, 4});
    t1.insert({6, 0, 8, 2});
    t1.insert({7, 7, 9, 9});
    std::vector<Rectangle> before;
    t1.search(t1.getRoot(), {0, 0, 10, 10}, before);
    std::cout << "  Before: " << before.size() << " rects\n";
    bool ok = t1.remove({3, 3, 5, 5});
    assert(ok);
    std::vector<Rectangle> after;
    t1.search(t1.getRoot(), {0, 0, 10, 10}, after);
    std::cout << "  After:  " << after.size() << " rects\n";
    assert((int)after.size() == (int)before.size() - 1);
    for (const auto &r : after)
        assert(!(r.x1 == 3 && r.y1 == 3 && r.x2 == 5 && r.y2 == 5));
    pass("rect removed, count dropped by 1, not in search results");

    std::cout << "\n[R-Delete 2] Delete non-existent rectangle\n";
    bool notFound = t1.remove({99, 99, 100, 100});
    assert(!notFound);
    pass("returned false for missing rect");

    std::cout << "\n[R-Delete 3] Search confirms deletion\n";
    Rtree t3(4);
    t3.insert({5, 5, 10, 10});
    t3.insert({1, 1, 3, 3});
    t3.insert({7, 2, 9, 4});
    t3.remove({5, 5, 10, 10});
    std::vector<Rectangle> r3;
    t3.search(t3.getRoot(), {0, 0, 15, 15}, r3);
    printRects("Remaining after removing (5,5)->(10,10)", r3);
    assert((int)r3.size() == 2);
    for (const auto &r : r3)
        assert(!(r.x1 == 5 && r.y1 == 5 && r.x2 == 10 && r.y2 == 10));
    pass("2 rects remain, deleted rect gone");

    std::cout << "\n[R-Delete 4] Condense tree after bulk delete\n";
    Rtree t4(4);
    std::vector<Rectangle> bulk;
    for (int i = 0; i < 12; ++i)
    {
        Rectangle r = {(float)i * 2, (float)i * 2, (float)i * 2 + 1, (float)i * 2 + 1};
        bulk.push_back(r);
        t4.insert(r);
    }
    for (int i = 0; i < 6; ++i)
    {
        bool ok2 = t4.remove(bulk[i]);
        assert(ok2);
    }
    std::vector<Rectangle> r4;
    t4.search(t4.getRoot(), {0, 0, 100, 100}, r4);
    std::cout << "  After deleting 6 of 12: " << r4.size() << " remain\n";
    assert((int)r4.size() == 6);
    pass("6 items remain, condense handled correctly");

    std::cout << "\n[R-Delete 5] Delete all items\n";
    Rtree t5(4);
    std::vector<Rectangle> all;
    for (int i = 0; i < 8; ++i)
    {
        Rectangle r = {(float)i, (float)i, (float)i + 1, (float)i + 1};
        all.push_back(r);
        t5.insert(r);
    }
    for (const auto &r : all)
    {
        bool ok2 = t5.remove(r);
        assert(ok2);
    }
    std::vector<Rectangle> r5;
    t5.search(t5.getRoot(), {0, 0, 100, 100}, r5);
    assert(r5.empty());
    pass("tree empty after removing all items");
}

// ════════════════════════════════════════════════════════════
//  B-TREE INSERT TESTS
// ════════════════════════════════════════════════════════════
void testBtreeInsert()
{
    section("B-TREE INSERT TESTS");

    std::cout << "\n[B-Insert 1] Basic insert and search\n";
    Btree t1(2);
    t1.insert(10);
    t1.insert(20);
    t1.insert(5);
    t1.insert(15);
    t1.insert(30);
    assert(t1.search(10) && t1.search(20) && t1.search(5) && t1.search(15) && t1.search(30));
    assert(!t1.search(99));
    pass("all inserted keys found, non-existent key returns false");

    std::cout << "\n[B-Insert 2] Range search returns correct keys\n";
    Btree t2(2);
    for (int i = 1; i <= 10; ++i)
        t2.insert((float)i);
    std::vector<float> r2 = t2.rangeSearch(3, 7);
    std::cout << "  rangeSearch(3,7): ";
    for (float k : r2)
        std::cout << k << " ";
    std::cout << "\n";
    assert((int)r2.size() == 5);
    pass("5 keys found in range [3,7]");

    std::cout << "\n[B-Insert 3] Force root split (10 items, t=2)\n";
    Btree t3(2);
    for (int i = 1; i <= 10; ++i)
        t3.insert((float)i);
    for (int i = 1; i <= 10; ++i)
        assert(t3.search((float)i));
    pass("all 10 keys found after multiple splits");

    std::cout << "\n[B-Insert 4] Duplicate key behaviour\n";
    Btree t4(2);
    t4.insert(5);
    t4.insert(5);
    t4.insert(5);
    assert(t4.search(5));
    pass("duplicate keys inserted without crash");

    std::cout << "\n[B-Insert 5] Range search on 50 items\n";
    Btree t5(3);
    for (int i = 1; i <= 50; ++i)
        t5.insert((float)i);
    std::vector<float> r5 = t5.rangeSearch(20, 30);
    std::cout << "  rangeSearch(20,30) found: " << r5.size() << " keys\n";
    assert((int)r5.size() == 11);
    pass("11 keys found in range [20,30]");
}

// ════════════════════════════════════════════════════════════
//  B-TREE DELETE TESTS
// ════════════════════════════════════════════════════════════
void testBtreeDelete()
{
    section("B-TREE DELETE TESTS");

    std::cout << "\n[B-Delete 1] Basic delete\n";
    Btree t1(2);
    t1.insert(10);
    t1.insert(20);
    t1.insert(5);
    t1.insert(15);
    bool ok = t1.remove(10);
    assert(ok);
    assert(!t1.search(10));
    assert(t1.search(20) && t1.search(5) && t1.search(15));
    pass("key 10 removed, others still present");

    std::cout << "\n[B-Delete 2] Delete non-existent key\n";
    Btree t2(2);
    t2.insert(10);
    t2.insert(20);
    bool notFound = t2.remove(99);
    assert(!notFound);
    pass("returned false for missing key");

    std::cout << "\n[B-Delete 3] Delete triggers borrow from sibling\n";
    Btree t3(2);
    for (int i = 1; i <= 8; ++i)
        t3.insert((float)i);
    t3.remove(1);
    t3.remove(2);
    for (int i = 3; i <= 8; ++i)
        assert(t3.search((float)i));
    pass("remaining keys intact after borrow/merge");

    std::cout << "\n[B-Delete 4] Multiple deletes forcing merges\n";
    Btree t4(2);
    for (int i = 1; i <= 12; ++i)
        t4.insert((float)i);
    for (int i = 1; i <= 6; ++i)
    {
        bool ok2 = t4.remove((float)i);
        assert(ok2);
    }
    for (int i = 7; i <= 12; ++i)
        assert(t4.search((float)i));
    pass("6 remaining keys found after deleting first half");

    std::cout << "\n[B-Delete 5] Delete all keys\n";
    Btree t5(2);
    std::vector<float> keys = {5, 3, 7, 1, 4, 6, 8, 2};
    for (float k : keys)
        t5.insert(k);
    for (float k : keys)
    {
        bool ok2 = t5.remove(k);
        assert(ok2);
    }
    for (float k : keys)
        assert(!t5.search(k));
    pass("all keys removed, none found afterwards");
}

// ════════════════════════════════════════════════════════════
//  COMPARISON ANALYSIS
// ════════════════════════════════════════════════════════════
void runComparison(int n)
{
    std::cout << "\n--- N = " << n << " ---\n";
    std::cout << std::left << std::setw(30) << "Operation"
              << std::right << std::setw(12) << "R-tree(us)"
              << std::setw(12) << "B-tree(us)" << "\n";
    std::cout << std::string(54, '-') << "\n";

    std::vector<Rectangle> rects;
    std::vector<float> keys;
    for (int i = 0; i < n; ++i)
    {
        rects.push_back({(float)i, (float)i, (float)i + 1, (float)i + 1});
        keys.push_back((float)i);
    }

    // INSERT
    Rtree rtree(4);
    auto t0 = Clock::now();
    for (const Rectangle &r : rects)
        rtree.insert(r);
    long long rtIns = usNow(t0);

    Btree btree(2);
    t0 = Clock::now();
    for (float k : keys)
        btree.insert(k);
    long long btIns = usNow(t0);
    std::cout << std::left << std::setw(30) << "Insert" << std::right << std::setw(12) << rtIns << std::setw(12) << btIns << "\n";

    // SEARCH — 100 queries each
    float mid = (float)n / 2;
    Rectangle qbox = {mid - n * 0.05f, mid - n * 0.05f, mid + n * 0.05f, mid + n * 0.05f};
    std::vector<Rectangle> rRes;
    t0 = Clock::now();
    for (int q = 0; q < 100; ++q)
    {
        rRes.clear();
        rtree.search(rtree.getRoot(), qbox, rRes);
    }
    long long rtSrch = usNow(t0);

    t0 = Clock::now();
    for (int q = 0; q < 100; ++q)
        btree.search((float)(n / 2));
    long long btSrch = usNow(t0);
    std::cout << std::left << std::setw(30) << "100x Search" << std::right << std::setw(12) << rtSrch << std::setw(12) << btSrch << "\n";
    std::cout << "  R-tree hits per query: " << rRes.size() << "\n";

    // RANGE SEARCH (B-tree native, R-tree does this via queryBox)
    t0 = Clock::now();
    std::vector<float> bRange = btree.rangeSearch((float)(n / 2 - n / 20), (float)(n / 2 + n / 20));
    long long btRange = usNow(t0);
    std::cout << std::left << std::setw(30) << "B-tree rangeSearch" << std::right << std::setw(12) << "N/A" << std::setw(12) << btRange << "\n";
    std::cout << "  B-tree range keys found: " << bRange.size() << "\n";

    // DELETE 10%
    int del = n / 10;
    t0 = Clock::now();
    for (int i = 0; i < del; ++i)
        rtree.remove(rects[i]);
    long long rtDel = usNow(t0);

    t0 = Clock::now();
    for (int i = 0; i < del; ++i)
        btree.remove(keys[i]);
    long long btDel = usNow(t0);
    std::cout << std::left << std::setw(30) << "Delete 10%" << std::right << std::setw(12) << rtDel << std::setw(12) << btDel << "\n";
}

void testComparison()
{
    section("R-TREE vs B-TREE COMPARISON ANALYSIS");
    std::cout << "\nSame dataset used for both trees.\n";
    std::cout << "R-tree: spatial MBR queries  |  B-tree: 1D key lookups\n";
    runComparison(500);
    runComparison(2000);
    runComparison(5000);
    std::cout << "\n--- Key Observations ---\n";
    std::cout << "  Insert : B-tree faster (scalar compare vs MBR enlargement calc)\n";
    std::cout << "  Search : R-tree returns spatial ranges; B-tree does exact lookup\n";
    std::cout << "  Delete : B-tree borrows/merges; R-tree dissolves + re-inserts\n";
    std::cout << "  Use    : B-tree for sorted 1D keys, R-tree for 2D spatial data\n";
}

// ════════════════════════════════════════════════════════════
//  MAIN
// ════════════════════════════════════════════════════════════
int main()
{
    std::cout << "==========================================\n";
    std::cout << "   R-TREE & B-TREE COMPLETE TEST SUITE\n";
    std::cout << "==========================================\n";

    testRtreeInsert();
    testRtreeDelete();
    testBtreeInsert();
    testBtreeDelete();
    testComparison();

    std::cout << "\n==========================================\n";
    std::cout << "         ALL TESTS PASSED\n";
    std::cout << "==========================================\n";
    return 0;
}