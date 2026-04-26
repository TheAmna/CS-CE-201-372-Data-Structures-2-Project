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

/*
    PAKISTAN CITY SPATIAL SEARCH APPLICATION
    Uses an R-tree to store Pakistani cities as bounding boxes.
    Each city has: name, province, population, area, elevation, type, coordinates.

    MENU:
        1 - Search cities in a region        (R-tree 2D range search)
        2 - Find cities near a location      (R-tree proximity search)
        3 - Search by city type              (R-tree search + type filter)
        4 - Province summary + statistics    (R-tree full scan + grouping)
        5 - Add a new city                   (R-tree insert + CSV save)
        6 - Remove a city                    (R-tree delete + CSV save)
        7 - Compare R-tree vs B-tree         (shows why R-trees exist)
        q - Quit
*/


// ── City struct ───────────────────────────────────────────────────────────────

struct City {
    std::string name;
    std::string province;
    long        population;
    int         area_km2;      // city area in square kilometers
    int         elevation_m;   // height above sea level
    std::string type;          // city / capital / port / industrial / historical
    Rectangle   bbox;          // bounding box stored in R-tree
};

std::vector<City> cities;           // master list of all cities
const std::string CSV_FILE = "input_data.csv";


// ── Print helpers ─────────────────────────────────────────────────────────────

void printLine() {
    std::cout << "  " << std::string(58, '-') << "\n";
}

void printTitle(const std::string& t) {
    std::cout << "\n  [ " << t << " ]\n\n";
}

// format population with commas e.g. 1300000 -> "1,300,000"
std::string fmtPop(long pop) {
    std::string s = std::to_string(pop);
    int insert = (int)s.size() - 3;
    while (insert > 0) {
        s.insert(insert, ",");
        insert -= 3;
    }
    return s;
}

// print one city row
void printCity(const City& c, int rank) {
    if (rank > 0) {
        std::cout << "  " << std::setw(2) << rank << ". ";
    } else {
        std::cout << "   -  ";
    }
    std::cout << std::left
              << std::setw(20) << c.name
              << std::setw(14) << c.province
              << std::setw(12) << c.type
              << "pop: " << std::right << std::setw(13) << fmtPop(c.population)
              << "  elev: " << c.elevation_m << "m\n";
}


// ── Load cities from CSV ──────────────────────────────────────────────────────
// reads pakistan_cities.csv line by line
// columns: name,province,population,area_km2,elevation_m,type,min_lon,min_lat,max_lon,max_lat

bool loadCities(Rtree& tree) {
    std::ifstream file(CSV_FILE);
    if (!file.is_open()) {
        std::cout << "  ERROR: cannot open " << CSV_FILE << "\n";
        return false;
    }

    cities.clear();
    std::string line;
    std::getline(file, line); // skip header row

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
        std::getline(ss, tok, ','); c.bbox.x1     = std::stof(tok); // min_lon
        std::getline(ss, tok, ','); c.bbox.y1     = std::stof(tok); // min_lat
        std::getline(ss, tok, ','); c.bbox.x2     = std::stof(tok); // max_lon
        std::getline(ss, tok, ','); c.bbox.y2     = std::stof(tok); // max_lat

        cities.push_back(c);
        tree.insert(c.bbox); // insert into R-tree
    }
    return true;
}


// ── Save cities back to CSV ───────────────────────────────────────────────────
// called after add or remove so changes persist

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
// R-tree returns rectangles, we match back using coordinates

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
    long totalPop = 0;
    City* largest = nullptr;
    int rank = 1;

    for (const Rectangle& r : results) {
        City* c = findCity(r);
        if (!c) continue;
        printCity(*c, rank++);
        totalPop += c->population;
        if (!largest || c->population > largest->population)
            largest = c;
    }

    printLine();
    std::cout << "  Cities found    : " << results.size() << "\n";
    std::cout << "  Total population: " << fmtPop(totalPop) << "\n";
    if (largest)
        std::cout << "  Largest city    : " << largest->name << "\n";
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 1 - Search cities in a region
//  User picks a preset province OR enters custom coordinates.
//  R-tree search() finds all overlapping cities.
// ─────────────────────────────────────────────────────────────────────────────

void searchRegion(Rtree& tree) {
    printTitle("FUNCTION 1: SEARCH CITIES IN A REGION");

    std::cout << "  Select region:\n";
    std::cout << "    a - All of Pakistan\n";
    std::cout << "    b - Punjab\n";
    std::cout << "    c - Sindh\n";
    std::cout << "    d - KPK\n";
    std::cout << "    e - Balochistan\n";
    std::cout << "    f - Custom coordinates\n";
    std::cout << "  Choice: ";

    char ch;
    std::cin >> ch;
    Rectangle box;
    std::string label;

    if      (ch == 'a') { box = {60.0, 23.0, 78.0, 37.5}; label = "All of Pakistan"; }
    else if (ch == 'b') { box = {70.0, 29.0, 75.5, 33.8}; label = "Punjab"; }
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
    tree.search(tree.getRoot(), box, results); // R-tree does the work

    printResults(results);
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 2 - Find cities near a location
//  User picks a center point + radius in km.
//  Converts km to degrees (1 degree ~ 100km) and builds a search box.
// ─────────────────────────────────────────────────────────────────────────────

void findNearby(Rtree& tree) {
    printTitle("FUNCTION 2: FIND CITIES NEAR A LOCATION");

    std::cout << "  Select center point:\n";
    std::cout << "    a - Islamabad  (73.1, 33.7)\n";
    std::cout << "    b - Karachi    (67.0, 24.9)\n";
    std::cout << "    c - Lahore     (74.3, 31.5)\n";
    std::cout << "    d - Peshawar   (71.6, 34.0)\n";
    std::cout << "    e - Custom GPS coordinates\n";
    std::cout << "  Choice: ";

    char ch;
    std::cin >> ch;
    float lon, lat;
    std::string label;

    if      (ch == 'a') { lon = 73.1f; lat = 33.7f; label = "Islamabad"; }
    else if (ch == 'b') { lon = 67.0f; lat = 24.9f; label = "Karachi"; }
    else if (ch == 'c') { lon = 74.3f; lat = 31.5f; label = "Lahore"; }
    else if (ch == 'd') { lon = 71.6f; lat = 34.0f; label = "Peshawar"; }
    else {
        std::cout << "  Enter longitude latitude: ";
        std::cin >> lon >> lat;
        label = "Custom point";
    }

    std::cout << "  Enter radius in km (e.g. 100): ";
    float km;
    std::cin >> km;

    // convert km to degrees (rough: 1 degree ~ 100km)
    float radius = km / 100.0f;

    // build search box around the center point
    Rectangle searchBox = {
        lon - radius,
        lat - radius,
        lon + radius,
        lat + radius
    };

    std::cout << "\n  Cities within " << (int)km << "km of " << label << ":\n";

    std::vector<Rectangle> results;
    tree.search(tree.getRoot(), searchBox, results); // R-tree does the work

    printResults(results);

    // also find nearest city by center distance
    if (!results.empty()) {
        City* nearest  = nullptr;
        float minDist  = 99999;
        for (const Rectangle& r : results) {
            City* c = findCity(r);
            if (!c) continue;
            float cx   = (c->bbox.x1 + c->bbox.x2) / 2.0f; // center of city bbox
            float cy   = (c->bbox.y1 + c->bbox.y2) / 2.0f;
            float dist = (cx - lon) * (cx - lon) + (cy - lat) * (cy - lat);
            if (dist < minDist) { minDist = dist; nearest = c; }
        }
        if (nearest)
            std::cout << "  Nearest city    : " << nearest->name << "\n";
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 3 - Search by city type
//  Searches all of Pakistan then filters by type.
// ─────────────────────────────────────────────────────────────────────────────

void searchByType(Rtree& tree) {
    printTitle("FUNCTION 3: SEARCH BY CITY TYPE");

    std::cout << "  City types:\n";
    std::cout << "    a - port       (coastal/trade cities)\n";
    std::cout << "    b - capital    (provincial/federal capitals)\n";
    std::cout << "    c - industrial (manufacturing hubs)\n";
    std::cout << "    d - historical (ancient/heritage cities)\n";
    std::cout << "    e - city       (general urban centers)\n";
    std::cout << "  Choice: ";

    char ch;
    std::cin >> ch;
    std::string typeFilter;

    if      (ch == 'a') typeFilter = "port";
    else if (ch == 'b') typeFilter = "capital";
    else if (ch == 'c') typeFilter = "industrial";
    else if (ch == 'd') typeFilter = "historical";
    else                typeFilter = "city";

    // search all of Pakistan first
    std::vector<Rectangle> all;
    tree.search(tree.getRoot(), {60.0, 23.0, 78.0, 37.5}, all);

    // then filter by type
    std::vector<Rectangle> filtered;
    for (const Rectangle& r : all) {
        City* c = findCity(r);
        if (c && c->type == typeFilter)
            filtered.push_back(r);
    }

    std::cout << "\n  " << typeFilter << " cities in Pakistan:\n";
    printResults(filtered);
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 4 - Province summary + statistics
//  Groups all cities by province, shows population, density, elevation.
// ─────────────────────────────────────────────────────────────────────────────

void provinceSummary(Rtree& tree) {
    printTitle("FUNCTION 4: PROVINCE SUMMARY AND STATISTICS");

    // search all of Pakistan
    std::vector<Rectangle> all;
    tree.search(tree.getRoot(), {60.0, 23.0, 78.0, 37.5}, all);

    // group cities by province
    std::map<std::string, std::vector<City*>> byProvince;
    for (const Rectangle& r : all) {
        City* c = findCity(r);
        if (c) byProvince[c->province].push_back(c);
    }

    // print table header
    printLine();
    std::cout << "  " << std::left
              << std::setw(16) << "Province"
              << std::setw(8)  << "Cities"
              << std::setw(18) << "Total Population"
              << std::setw(16) << "Avg Density"
              << "Largest City\n";
    printLine();

    long grandTotal = 0;

    for (auto& entry : byProvince) {
        const std::string& province      = entry.first;
        std::vector<City*>& cityList     = entry.second;

        long totalPop  = 0;
        long totalArea = 0;
        City* largest  = nullptr;

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
                  << std::setw(16) << (std::to_string(density) + "/km2")
                  << (largest ? largest->name : "-") << "\n";

        grandTotal += totalPop;
    }

    printLine();
    std::cout << "  " << std::left
              << std::setw(16) << "TOTAL"
              << std::setw(8)  << all.size()
              << fmtPop(grandTotal) << "\n";
    printLine();

    // show highest and lowest elevation cities
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
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 5 - Add a new city
//  User enters city details, inserts into R-tree, saves to CSV.
// ─────────────────────────────────────────────────────────────────────────────

void addCity(Rtree& tree) {
    printTitle("FUNCTION 5: ADD A NEW CITY");

    City c;

    std::cin.ignore(); // clear leftover newline
    std::cout << "  City name     : "; std::getline(std::cin, c.name);
    std::cout << "  Province      : "; std::getline(std::cin, c.province);
    std::cout << "  Population    : "; std::cin >> c.population;
    std::cout << "  Area (km2)    : "; std::cin >> c.area_km2;
    std::cout << "  Elevation (m) : "; std::cin >> c.elevation_m;
    std::cin.ignore();
    std::cout << "  Type (city/capital/port/industrial/historical): ";
    std::getline(std::cin, c.type);

    float lon, lat;
    std::cout << "  Center longitude (e.g. 67.0): "; std::cin >> lon;
    std::cout << "  Center latitude  (e.g. 25.0): "; std::cin >> lat;

    // build a 0.15 degree box around center (~15km)
    c.bbox = { lon - 0.15f, lat - 0.15f, lon + 0.15f, lat + 0.15f };

    tree.insert(c.bbox);   // insert into R-tree
    cities.push_back(c);   // add to master list
    saveCSV();             // persist to CSV

    std::cout << "\n  Done! " << c.name << " added to R-tree and saved to CSV.\n";
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 6 - Remove a city
//  Shows numbered list, user picks by number.
//  Removes from R-tree using delete, updates CSV.
// ─────────────────────────────────────────────────────────────────────────────

void removeCity(Rtree& tree) {
    printTitle("FUNCTION 6: REMOVE A CITY");

    if (cities.empty()) {
        std::cout << "  No cities loaded.\n";
        return;
    }

    // show numbered list
    std::cout << "  Select city to remove:\n\n";
    for (int i = 0; i < (int)cities.size(); ++i) {
        std::cout << "  " << std::setw(3) << (i + 1) << ". "
                  << std::left << std::setw(22) << cities[i].name
                  << "(" << cities[i].province << ", " << cities[i].type << ")\n";
    }

    std::cout << "\n  Enter number (0 to cancel): ";
    int choice;
    std::cin >> choice;

    if (choice <= 0 || choice > (int)cities.size()) {
        std::cout << "  Cancelled.\n";
        return;
    }

    City toRemove = cities[choice - 1]; // copy before erasing

    bool removed = tree.remove(toRemove.bbox); // R-tree delete

    if (removed) {
        cities.erase(cities.begin() + (choice - 1)); // remove from master list
        saveCSV();                                    // update CSV
        std::cout << "\n  Done! " << toRemove.name << " removed from R-tree and CSV updated.\n";
    } else {
        std::cout << "  ERROR: could not remove from R-tree.\n";
    }
}


// ─────────────────────────────────────────────────────────────────────────────
//  FUNCTION 7 - Compare R-tree vs B-tree
//  Inserts same data into both, runs same query, shows B-tree fails for 2D.
// ─────────────────────────────────────────────────────────────────────────────

void compareWithBtree(Rtree& rtree) {
    printTitle("FUNCTION 7: R-TREE vs B-TREE COMPARISON");

    // build B-tree with same cities (only x1 as 1D key)
    Btree btree(2);
    for (const City& c : cities)
        btree.insert(c.bbox.x1);

    std::cout << "  Same " << cities.size() << " cities loaded into both trees.\n";
    std::cout << "  R-tree stores full bbox {x1, y1, x2, y2}\n";
    std::cout << "  B-tree stores only x1 (longitude) as a 1D key\n";
    printLine();

    // Query A: 1D range — both should agree
    std::cout << "  Query A - 1D: cities with longitude between 66 and 75\n";
    auto bKeys = btree.rangeSearch(66.0f, 75.0f);
    std::vector<Rectangle> rResults;
    rtree.search(rtree.getRoot(), {66.0, -99, 75.0, 99}, rResults);
    std::cout << "  B-tree found : " << bKeys.size()    << " keys\n";
    std::cout << "  R-tree found : " << rResults.size() << " cities\n";
    std::cout << "  Both correct for 1D queries.\n";
    printLine();

    // Query B: 2D range — B-tree cannot handle y dimension
    std::cout << "  Query B - 2D: cities inside box {lon 66-75, lat 24-33}\n";
    std::cout << "  (needs BOTH longitude AND latitude check)\n\n";

    auto bKeys2 = btree.rangeSearch(66.0f, 75.0f); // B-tree only sees x1
    std::vector<Rectangle> rResults2;
    rtree.search(rtree.getRoot(), {66.0, 24.0, 75.0, 33.0}, rResults2); // R-tree checks x AND y

    std::cout << "  R-tree found : " << rResults2.size() << " cities  -- correct (checks x AND y)\n";
    std::cout << "  B-tree found : " << bKeys2.size()    << " keys    -- wrong   (only checked x1)\n";
    printLine();
    std::cout << "  B-tree returned " << (int)(bKeys2.size() - rResults2.size())
              << " extra results because it cannot filter by latitude.\n";
    std::cout << "  This is why R-trees exist -- B-trees cannot do 2D spatial queries.\n";
    printLine();
}


// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {

    // welcome message — plain text, no special characters
    std::cout << "\n";
    std::cout << "  ================================================\n";
    std::cout << "      PAKISTAN CITY SPATIAL SEARCH APPLICATION\n";
    std::cout << "               Powered by R-Tree\n";
    std::cout << "           CS-201 Data Structures II\n";
    std::cout << "  ================================================\n\n";

    // load data from CSV into R-tree
    Rtree tree(4);
    std::cout << "  Loading cities from " << CSV_FILE << "...\n";
    if (!loadCities(tree)) return 1;
    std::cout << "  " << cities.size() << " cities loaded into R-tree successfully.\n";

    // main menu loop
    char choice;
    do {
        std::cout << "\n";
        std::cout << "  ================================================\n";
        std::cout << "                    MAIN MENU\n";
        std::cout << "  ================================================\n";
        std::cout << "    1 - Search cities in a region\n";
        std::cout << "    2 - Find cities near a location\n";
        std::cout << "    3 - Search by city type\n";
        std::cout << "    4 - Province summary and statistics\n";
        std::cout << "    5 - Add a new city\n";
        std::cout << "    6 - Remove a city\n";
        std::cout << "    7 - Compare R-tree vs B-tree\n";
        std::cout << "    q - Quit\n";
        std::cout << "  ================================================\n";
        std::cout << "  Choice: ";
        std::cin >> choice;

        if      (choice == '1') searchRegion(tree);
        else if (choice == '2') findNearby(tree);
        else if (choice == '3') searchByType(tree);
        else if (choice == '4') provinceSummary(tree);
        else if (choice == '5') addCity(tree);
        else if (choice == '6') removeCity(tree);
        else if (choice == '7') compareWithBtree(tree);
        else if (choice != 'q') std::cout << "  Invalid choice. Try again.\n";

    } while (choice != 'q');

    std::cout << "\n  Goodbye!\n\n";
    return 0;
}
