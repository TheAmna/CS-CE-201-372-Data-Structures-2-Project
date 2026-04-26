#include "Rtree.hpp"
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
    Uses an R-tree to store Pakistani cities as bounding boxes.
    Each city has: name, province, population, area, elevation, type, coordinates.

    MENU:
        1 → Search cities in a region        (R-tree 2D range search)
        2 → Find cities near a location      (R-tree proximity search)
        3 → Search by city type              (R-tree search + type filter)
        4 → Province summary + statistics    (R-tree full scan + grouping)
        q → Quit
*/

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
              << std::setw(12) << c.type          
              << "pop: " << std::right << std::setw(13) << fmtPop(c.population)
              << "  elev: " << c.elevation_m << "m\n";
}

// returns true if user wants to repeat the current function
// returns false if user wants to go back to main menu
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

// eport query results to CSV for Python visualizer for mapping 
// name,province,population,area_km2,elevation_m,type,min_lon,min_lat,max_lon,max_lat
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

// launch Python visualizer func: "1" = region, "2" = nearby, "3" = type
void showMap(const std::vector<Rectangle>& results,
             const std::string& title,
             const std::string& func) {
    if (results.empty()) {
        std::cout << "  (No results to map)\n";
        return;
    }
    // write result CSV for Python
    exportQueryResult(results);
    std::string cmd = "python3 visualize.py query_result.csv \""
                      + title + "\" " + func;
    std::cout << "  Generating map...\n";
    int ret = system(cmd.c_str());
    if (ret == 0)
        std::cout << "  Map saved as map_output.png\n";
    else
        std::cout << "  Map generation failed. Is visualize.py in the same folder?\n";
}

// called after results print in functions 1, 2, 3
// returns true if user wants the map
bool askMap() {
    char ans;
    std::cout << "  View results on map? (y/n): ";
    std::cin >> ans;
    return (ans == 'y' || ans == 'Y');
}

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

// save cities back to CSV 
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

// match result rectangle back to City
City* findCity(const Rectangle& r) {
    for (City& c : cities)
        if (c.bbox.equals(r)) return &c;
    return nullptr;
}

// print search results with summary stats 
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

//  FUNCTION 1 — Search cities in a region
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

//  FUNCTION 2 — Find cities near a location
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
        std::cout << "    q → Quit\n";
        std::cout << "  ════════════════════════════════════════════════\n";
        std::cout << "  Choice: ";
        std::cin >> choice;

        if      (choice == '1') searchRegion(tree);
        else if (choice == '2') findNearby(tree);
        else if (choice == '3') searchByType(tree);
        else if (choice == '4') provinceSummary(tree);
        else if (choice != 'q') std::cout << "  Invalid choice, try again.\n";

    } while (choice != 'q');

    std::cout << "\n  Goodbye!\n\n";
    return 0;
}
