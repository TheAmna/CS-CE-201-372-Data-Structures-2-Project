#include "Rtree.hpp"
#include "Btree.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <map>
#include <cstdlib>   // for system()

/*
    PAKISTAN CITY SPATIAL SEARCH APPLICATION
    ─────────────────────────────────────────
    Uses an R-tree to store Pakistani cities as bounding boxes.
    Each city has: name, province, population, area, elevation, type, coordinates.

    MENU:
        1 → Search cities in a region        (R-tree 2D range search)
        2 → Find cities near a location      (R-tree proximity search)
        3 → Search by city type              (R-tree search + type filter)
        4 → Province summary + statistics    (R-tree full scan + grouping)
        5 → Add a new city                   (R-tree insert + CSV save)
        6 → Remove a city                    (R-tree delete + CSV save)
        7 → Compare R-tree vs B-tree         (shows why R-trees exist)
        q → Quit
*/


// ── City struct ───────────────────────────────────────────────────────────────

struct City {
    std::string name;
    std::string province;
    long        population;
    int         area_km2;
    int         elevation_m;
    std::string type;
    Rectangle   bbox;
};

std::vector<City> cities;
const std::string CSV_FILE = "pakistan_cities.csv";


// ── Print helpers ─────────────────────────────────────────────────────────────

void printLine() { std::cout << "  " << std::string(58, '-') << "\n"; }

void printTitle(const std::string& t) {
    std::cout << "\n  ╔" << std::string(t.size() + 2, '=') << "╗\n";
    std::cout << "  ║ " << t << " ║\n";
    std::cout << "  ╚" << std::string(t.size() + 2, '=') << "╝\n\n";
}

// format population with commas e.g. 1300000 → "1,300,000"
std::string fmtPop(long pop) {
    std::string s = std::to_string(pop);
    int ins = (int)s.size() - 3;
    while (ins > 0) { s.insert(ins, ","); ins -= 3; }
    return s;
}

// print one city row
void printCity(const City& c, int rank = 0) {
    if (rank > 0) std::cout << "  " << std::setw(2) << rank << ". ";
    else          std::cout << "   •  ";
    std::cout << std::left  << std::setw(18) << c.name
              << std::setw(14) << ("(" + c.province + ")")
              << std::setw(12) << c.type          // was 10 — fixed formatting gap
              << "pop: " << std::right << std::setw(13) << fmtPop(c.population)
              << "  elev: " << c.elevation_m << "m\n";
}

// ── small helper: ask "repeat or main menu?" ─────────────────────────────────
// prompt  = e.g. "Search another region?"
// returns true  → user wants to repeat the current function
// returns false → user wants to go back to main menu
bool askRepeat(const std::string& prompt) {
    char ans;
    std::cout << "\n";
    printLine();
    std::cout << "  " << prompt << "\n";
    std::cout << "    y → Yes, do it again\n";
    std::cout << "    m → Back to main menu\n";
    std::cout << "  Choice: ";
    std::cin >> ans;
    return (ans == 'y' || ans == 'Y');
}


// ── Export query results to CSV for Python visualizer ────────────────────────
// writes name,province,population,area_km2,elevation_m,type,min_lon,min_lat,max_lon,max_lat
// Python reads this and plots the highlighted cities on the map

void exportQueryResult(const std::vector<Rectangle>& results,
                       const std::string& filename = "query_result.csv") {
    std::ofstream f(filename);
    f << "name,province,population,area_km2,elevation_m,type,"
      << "min_lon,min_lat,max_lon,max_lat\n";
    for (const Rectangle& r : results) {
        City* c = nullptr;
        for (City& city : cities)
            if (city.bbox.equals(r)) { c = &city; break; }
        if (!c) continue;
        f << c->name        << ","
          << c->province    << ","
          << c->population  << ","
          << c->area_km2    << ","
          << c->elevation_m << ","
          << c->type        << ","
          << c->bbox.x1     << ","
          << c->bbox.y1     << ","
          << c->bbox.x2     << ","
          << c->bbox.y2     << "\n";
    }
    f.close();
}

// ── Launch Python visualizer ──────────────────────────────────────────────────
// func: "1" = region, "2" = nearby, "3" = type
// title: shown on the map e.g. "Punjab" or "100km of Karachi"

void showMap(const std::vector<Rectangle>& results,
             const std::string& title,
             const std::string& func) {
    if (results.empty()) {
        std::cout << "  (No results to map)\n";
        return;
    }

    // write result CSV for Python
    exportQueryResult(results);

    // build the shell command
    // title may contain spaces so wrap in quotes
    std::string cmd = "python3 visualize.py query_result.csv \""
                      + title + "\" " + func;

    std::cout << "  Generating map...\n";
    int ret = system(cmd.c_str());

    if (ret == 0)
        std::cout << "  Map saved as map_output.png\n";
    else
        std::cout << "  Map generation failed. Is visualize.py in the same folder?\n";
}

// ── Ask "View on map?" helper ─────────────────────────────────────────────────
// called after results print in functions 1, 2, 3
// returns true if user wants the map

bool askMap() {
    char ans;
    std::cout << "  View results on map? (y/n): ";
    std::cin >> ans;
    return (ans == 'y' || ans == 'Y');
}


// ── Load cities from CSV ──────────────────────────────────────────────────────

bool loadCities(Rtree& tree) {
    std::ifstream file(CSV_FILE);
    if (!file.is_open()) {
        std::cout << "  ERROR: cannot open " << CSV_FILE << "\n";
        return false;
    }

    cities.clear();
    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string tok;
        City c;

        std::getline(ss, c.name,     ',');
        std::getline(ss, c.province, ',');
        std::getline(ss, tok, ','); c.population  = std::stol(tok);
        std::getline(ss, tok, ','); c.area_km2    = std::stoi(tok);
        std::getline(ss, tok, ','); c.elevation_m = std::stoi(tok);
        std::getline(ss, c.type,     ',');
        std::getline(ss, tok, ','); c.bbox.x1     = std::stof(tok);
        std::getline(ss, tok, ','); c.bbox.y1     = std::stof(tok);
        std::getline(ss, tok, ','); c.bbox.x2     = std::stof(tok);
        std::getline(ss, tok, ','); c.bbox.y2     = std::stof(tok);

        cities.push_back(c);
        tree.insert(c.bbox);
    }
    return true;
}


// ── Save cities back to CSV ───────────────────────────────────────────────────

void saveCSV() {
    std::ofstream file(CSV_FILE);
    file << "name,province,population,area_km2,elevation_m,type,min_lon,min_lat,max_lon,max_lat\n";
    for (const City& c : cities) {
        file << c.name        << ","
             << c.province    << ","
             << c.population  << ","
             << c.area_km2    << ","
             << c.elevation_m << ","
             << c.type        << ","
             << c.bbox.x1     << ","
             << c.bbox.y1     << ","
             << c.bbox.x2     << ","
             << c.bbox.y2     << "\n";
    }
}


// ── Match result rectangle back to City ──────────────────────────────────────

City* findCity(const Rectangle& r) {
    for (City& c : cities)
        if (c.bbox.equals(r)) return &c;
    return nullptr;
}


// ── Print search results with summary stats ───────────────────────────────────

void printResults(const std::vector<Rectangle>& results) {
    if (results.empty()) {
        std::cout << "  No cities found.\n";
        return;
    }

    printLine();
    long  totalPop = 0;
    City* largest  = nullptr;

    for (const Rectangle& r : results) {
        City* c = findCity(r);
        if (!c) continue;
        printCity(*c);
        totalPop += c->population;
        if (!largest || c->population > largest->population)
            largest = c;
    }

    printLine();
    std::cout << "  Cities found    : " << results.size()    << "\n";
    std::cout << "  Total population: " << fmtPop(totalPop)  << "\n";
    if (largest)
        std::cout << "  Largest city    : " << largest->name << "\n";
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 1 — Search cities in a region
// ─────────────────────────────────────────────────────────────────────────────

void searchRegion(Rtree& tree) {
    do {
        printTitle("FUNCTION 1: SEARCH CITIES IN A REGION");

        std::cout << "  Select region:\n";
        std::cout << "    a → All of Pakistan\n";
        std::cout << "    b → Punjab\n";
        std::cout << "    c → Sindh\n";
        std::cout << "    d → KPK\n";
        std::cout << "    e → Balochistan\n";
        std::cout << "    f → Custom coordinates\n";
        std::cout << "  Choice: ";

        char ch; std::cin >> ch;
        Rectangle   box;
        std::string label;

        if      (ch == 'a') { box = {60.0, 23.0, 78.0, 37.5}; label = "All of Pakistan"; }
        else if (ch == 'b') { box = {70.0, 29.0, 75.5, 33.5}; label = "Punjab"; }   // tightened max_lat to 33.5 (was 33.8 which included Islamabad)
        else if (ch == 'c') { box = {66.0, 23.5, 71.0, 28.5}; label = "Sindh"; }
        else if (ch == 'd') { box = {69.5, 33.5, 74.5, 36.0}; label = "KPK"; }
        else if (ch == 'e') { box = {60.0, 25.0, 70.0, 32.0}; label = "Balochistan"; }
        else {
            std::cout << "  Enter min_lon min_lat max_lon max_lat: ";
            std::cin >> box.x1 >> box.y1 >> box.x2 >> box.y2;
            label = "Custom region";
        }

        std::cout << "\n  Searching: " << label << "...\n";

        std::vector<Rectangle> results;
        tree.search(tree.getRoot(), box, results);
        printResults(results);

        if (!results.empty() && askMap())
            showMap(results, label, "1");

    } while (askRepeat("Search another region?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 2 — Find cities near a location
// ─────────────────────────────────────────────────────────────────────────────

void findNearby(Rtree& tree) {
    do {
        printTitle("FUNCTION 2: FIND CITIES NEAR A LOCATION");

        std::cout << "  Select center point:\n";
        std::cout << "    a → Islamabad  (73.1, 33.7)\n";
        std::cout << "    b → Karachi    (67.0, 24.9)\n";
        std::cout << "    c → Lahore     (74.3, 31.5)\n";
        std::cout << "    d → Peshawar   (71.6, 34.0)\n";
        std::cout << "    e → Custom GPS coordinates\n";
        std::cout << "  Choice: ";

        char ch; std::cin >> ch;
        float lon, lat;
        std::string label;

        if      (ch == 'a') { lon = 73.1f; lat = 33.7f; label = "Islamabad"; }
        else if (ch == 'b') { lon = 67.0f; lat = 24.9f; label = "Karachi";   }
        else if (ch == 'c') { lon = 74.3f; lat = 31.5f; label = "Lahore";    }
        else if (ch == 'd') { lon = 71.6f; lat = 34.0f; label = "Peshawar";  }
        else {
            std::cout << "  Enter longitude latitude: ";
            std::cin >> lon >> lat;
            label = "Custom point";
        }

        std::cout << "  Enter radius in km (e.g. 100): ";
        float km; std::cin >> km;

        // 1 degree ≈ 100 km (rough approximation)
        float radius = km / 100.0f;

        Rectangle searchBox = {
            lon - radius,
            lat - radius,
            lon + radius,
            lat + radius
        };

        std::cout << "\n  Cities within " << (int)km << "km of " << label << ":\n";

        std::vector<Rectangle> results;
        tree.search(tree.getRoot(), searchBox, results);
        printResults(results);

        // find nearest city by center-point distance
        if (!results.empty()) {
            City*  nearest = nullptr;
            float  minDist = 99999.0f;
            for (const Rectangle& r : results) {
                City* c = findCity(r);
                if (!c) continue;
                float cx   = (c->bbox.x1 + c->bbox.x2) / 2.0f;
                float cy   = (c->bbox.y1 + c->bbox.y2) / 2.0f;
                float dist = (cx - lon) * (cx - lon) + (cy - lat) * (cy - lat);
                if (dist < minDist) { minDist = dist; nearest = c; }
            }
            if (nearest)
                std::cout << "  Nearest city    : " << nearest->name << "\n";
        }

        if (!results.empty() && askMap()) {
            std::string mapTitle = std::to_string((int)km) + "km of " + label;
            showMap(results, mapTitle, "2");
        }

    } while (askRepeat("Search another location?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 3 — Search by city type
// ─────────────────────────────────────────────────────────────────────────────

void searchByType(Rtree& tree) {
    do {
        printTitle("FUNCTION 3: SEARCH BY CITY TYPE");

        std::cout << "  City types available:\n";
        std::cout << "    a → port       (coastal/trade cities)\n";
        std::cout << "    b → capital    (provincial/federal capitals)\n";
        std::cout << "    c → industrial (manufacturing hubs)\n";
        std::cout << "    d → historical (ancient/heritage cities)\n";
        std::cout << "    e → city       (general urban centers)\n";
        std::cout << "  Choice: ";

        char ch; std::cin >> ch;
        std::string typeFilter;

        if      (ch == 'a') typeFilter = "port";
        else if (ch == 'b') typeFilter = "capital";
        else if (ch == 'c') typeFilter = "industrial";
        else if (ch == 'd') typeFilter = "historical";
        else                typeFilter = "city";

        // full Pakistan scan then attribute filter
        std::vector<Rectangle> all;
        tree.search(tree.getRoot(), {60.0, 23.0, 78.0, 37.5}, all);

        std::vector<Rectangle> filtered;
        for (const Rectangle& r : all) {
            City* c = findCity(r);
            if (c && c->type == typeFilter)
                filtered.push_back(r);
        }

        std::cout << "\n  " << typeFilter << " cities in Pakistan:\n";
        printResults(filtered);

        if (!filtered.empty() && askMap())
            showMap(filtered, typeFilter + " cities", "3");

    } while (askRepeat("Search another city type?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 4 — Province summary + statistics
// ─────────────────────────────────────────────────────────────────────────────

void provinceSummary(Rtree& tree) {
    do {
        printTitle("FUNCTION 4: PROVINCE SUMMARY & STATISTICS");

        std::vector<Rectangle> all;
        tree.search(tree.getRoot(), {60.0, 23.0, 78.0, 37.5}, all);

        std::map<std::string, std::vector<City*>> byProvince;
        for (const Rectangle& r : all) {
            City* c = findCity(r);
            if (c) byProvince[c->province].push_back(c);
        }

        printLine();
        std::cout << "  " << std::left
                  << std::setw(16) << "Province"
                  << std::setw(8)  << "Cities"
                  << std::setw(18) << "Total Population"
                  << std::setw(16) << "Avg Density"
                  << "Largest City\n";
        printLine();

        long grandTotal = 0;

        for (auto& [province, cityList] : byProvince) {
            long  totalPop  = 0;
            long  totalArea = 0;
            City* largest   = nullptr;

            for (City* c : cityList) {
                totalPop  += c->population;
                totalArea += c->area_km2;
                if (!largest || c->population > largest->population)
                    largest = c;
            }

            long density = (totalArea > 0) ? totalPop / totalArea : 0;

            std::cout << "  " << std::left
                      << std::setw(16) << province
                      << std::setw(8)  << cityList.size()
                      << std::setw(18) << fmtPop(totalPop)
                      << std::setw(16) << (std::to_string(density) + "/km²")
                      << (largest ? largest->name : "-") << "\n";

            grandTotal += totalPop;
        }

        printLine();
        std::cout << "  " << std::left
                  << std::setw(16) << "TOTAL"
                  << std::setw(8)  << all.size()
                  << fmtPop(grandTotal) << "\n";
        printLine();

        City* highest = nullptr;
        City* lowest  = nullptr;
        for (const Rectangle& r : all) {
            City* c = findCity(r);
            if (!c) continue;
            if (!highest || c->elevation_m > highest->elevation_m) highest = c;
            if (!lowest  || c->elevation_m < lowest->elevation_m)  lowest  = c;
        }
        if (highest) std::cout << "  Highest city: " << highest->name << " (" << highest->elevation_m << "m)\n";
        if (lowest)  std::cout << "  Lowest city : " << lowest->name  << " (" << lowest->elevation_m  << "m)\n";

    } while (askRepeat("View province summary again?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 5 — Add a new city
// ─────────────────────────────────────────────────────────────────────────────

void addCity(Rtree& tree) {
    do {
        printTitle("FUNCTION 5: ADD A NEW CITY");

        City c;

        std::cin.ignore();
        std::cout << "  City name     : "; std::getline(std::cin, c.name);
        std::cout << "  Province      : "; std::getline(std::cin, c.province);
        std::cout << "  Population    : "; std::cin >> c.population;
        std::cout << "  Area (km²)    : "; std::cin >> c.area_km2;
        std::cout << "  Elevation (m) : "; std::cin >> c.elevation_m;

        std::cin.ignore();
        std::cout << "  Type (city/capital/port/industrial/historical): ";
        std::getline(std::cin, c.type);

        float lon, lat;
        std::cout << "  Center longitude (e.g. 67.0): "; std::cin >> lon;
        std::cout << "  Center latitude  (e.g. 25.0): "; std::cin >> lat;

        c.bbox = { lon - 0.15f, lat - 0.15f,
                   lon + 0.15f, lat + 0.15f };

        tree.insert(c.bbox);
        cities.push_back(c);
        saveCSV();

        std::cout << "\n  ✓ " << c.name << " added to R-tree\n";
        std::cout << "  ✓ Saved to " << CSV_FILE << "\n";

    } while (askRepeat("Add another city?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 6 — Remove a city
// ─────────────────────────────────────────────────────────────────────────────

void removeCity(Rtree& tree) {
    do {
        printTitle("FUNCTION 6: REMOVE A CITY");

        if (cities.empty()) {
            std::cout << "  No cities loaded.\n";
            break;  // nothing to remove, exit the loop immediately
        }

        std::cout << "  Select city to remove:\n\n";
        for (int i = 0; i < (int)cities.size(); ++i) {
            std::cout << "  " << std::setw(3) << (i + 1) << ". "
                      << std::left << std::setw(20) << cities[i].name
                      << "(" << cities[i].province << ", " << cities[i].type << ")\n";
        }

        std::cout << "\n  Enter number (0 to cancel): ";
        int choice; std::cin >> choice;

        if (choice <= 0 || choice > (int)cities.size()) {
            std::cout << "  Cancelled.\n";
            // still ask if they want to try again or go back
        } else {
            City toRemove = cities[choice - 1];
            bool removed  = tree.remove(toRemove.bbox);

            if (removed) {
                cities.erase(cities.begin() + (choice - 1));
                saveCSV();
                std::cout << "\n  ✓ " << toRemove.name << " removed from R-tree\n";
                std::cout << "  ✓ CSV updated\n";
            } else {
                std::cout << "  ERROR: could not remove from R-tree\n";
            }
        }

    } while (askRepeat("Remove another city?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 7 — Compare R-tree vs B-tree
// ─────────────────────────────────────────────────────────────────────────────

void compareWithBtree(Rtree& rtree) {
    do {
        printTitle("FUNCTION 7: R-TREE vs B-TREE COMPARISON");

        Btree btree(2);
        for (const City& c : cities)
            btree.insert(c.bbox.x1);

        std::cout << "  Same " << cities.size() << " cities loaded into both trees.\n";
        std::cout << "  R-tree stores full bbox {x1,y1,x2,y2}\n";
        std::cout << "  B-tree stores only x1 (longitude) as a 1D key\n\n";

        // ── Query A: 1D range ────────────────────────────────────────────
        std::cout << "  Query A — 1D: cities with longitude (x1) between 66 and 75\n";
        std::cout << "  (Both trees should agree on this)\n";

        auto bKeys = btree.rangeSearch(66.0f, 75.0f);

        std::vector<Rectangle> rResults;
        rtree.search(rtree.getRoot(), {66.0, -99, 75.0, 99}, rResults);

        std::cout << "  B-tree found : " << bKeys.size()    << " keys\n";
        std::cout << "  R-tree found : " << rResults.size() << " cities\n";

        // ── Query B: 2D range ────────────────────────────────────────────
        std::cout << "\n  Query B — 2D: cities inside box {66,24,75,33}\n";
        std::cout << "  (x: 66-75  AND  y: 24-33 — needs BOTH dimensions)\n";

        auto bKeys2 = btree.rangeSearch(66.0f, 75.0f);

        std::vector<Rectangle> rResults2;
        rtree.search(rtree.getRoot(), {66.0, 24.0, 75.0, 33.0}, rResults2);

        std::cout << "\n  R-tree found : " << rResults2.size() << " cities  ✓ correct (checks x AND y)\n";
        std::cout << "  B-tree found : " << bKeys2.size()     << " keys    ✗ wrong  (only checked x1)\n";
        printLine();
        std::cout << "  The B-tree returned " << (bKeys2.size() - rResults2.size())
                  << " extra results because it cannot filter by latitude.\n";
        std::cout << "  This is exactly why R-trees were invented —\n";
        std::cout << "  traditional structures cannot handle multi-dimensional data.\n";
        printLine();

    } while (askRepeat("Run the comparison again?"));
}


// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {

    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════════════════╗\n";
    std::cout << "  ║       PAKISTAN CITY SPATIAL SEARCH APP          ║\n";
    std::cout << "  ║            Powered by R-Tree                    ║\n";
    std::cout << "  ║         CS-201 Data Structures II               ║\n";
    std::cout << "  ╚══════════════════════════════════════════════════╝\n\n";

    Rtree tree(4);
    std::cout << "  Loading cities from " << CSV_FILE << "...\n";
    if (!loadCities(tree)) return 1;
    std::cout << "  ✓ " << cities.size() << " cities loaded into R-tree\n";

    char choice;
    do {
        std::cout << "\n";
        std::cout << "  ════════════════════════════════════════════════\n";
        std::cout << "                    MAIN MENU\n";
        std::cout << "  ════════════════════════════════════════════════\n";
        std::cout << "    1 → Search cities in a region\n";
        std::cout << "    2 → Find cities near a location\n";
        std::cout << "    3 → Search by city type\n";
        std::cout << "    4 → Province summary + statistics\n";
        std::cout << "    5 → Add a new city\n";
        std::cout << "    6 → Remove a city\n";
        std::cout << "    7 → Compare R-tree vs B-tree\n";
        std::cout << "    q → Quit\n";
        std::cout << "  ════════════════════════════════════════════════\n";
        std::cout << "  Choice: ";
        std::cin >> choice;

        if      (choice == '1') searchRegion(tree);
        else if (choice == '2') findNearby(tree);
        else if (choice == '3') searchByType(tree);
        else if (choice == '4') provinceSummary(tree);
        else if (choice == '5') addCity(tree);
        else if (choice == '6') removeCity(tree);
        else if (choice == '7') compareWithBtree(tree);
        else if (choice != 'q') std::cout << "  Invalid choice, try again.\n";

    } while (choice != 'q');

    std::cout << "\n  Goodbye!\n\n";
    return 0;
}