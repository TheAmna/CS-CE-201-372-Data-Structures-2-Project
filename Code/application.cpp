#include "Rtree.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

/*
    PAKISTAN SPATIAL APPLICATION
    Loads city data from pakistan_cities.csv into an R-tree.
    Each city is stored as a small bounding box (rectangle) around
    its real lat/lon coordinates:
        x = longitude (east-west)
        y = latitude  (north-south)
    TWO DEMO FUNCTIONS:
        1. searchRegion     → find all cities inside a bounding box
        2. findNearbyCities → find all cities within radius of a point
    TO ADD GEOPANDAS LATER:
        Results are exported to results.csv at the end.
        Just run a Python script on that file — no C++ changes needed.
*/


struct City {
    std::string name;
    std::string province;
    long        population;
    Rectangle   bbox;
};
std::vector<City> cities; // filled by loadCities()

bool loadCities(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Error opening file " << filename << "\n";
        return false;
    }
    std::string line;
    std::getline(file, line); // skip header
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        City c;
        std::getline(ss, c.name,     ',');
        std::getline(ss, c.province, ',');
        std::getline(ss, token, ','); c.population = std::stol(token);
        std::getline(ss, token, ','); c.bbox.x1    = std::stof(token); // min_lon
        std::getline(ss, token, ','); c.bbox.y1    = std::stof(token); // min_lat
        std::getline(ss, token, ','); c.bbox.x2    = std::stof(token); // max_lon
        std::getline(ss, token, ','); c.bbox.y2    = std::stof(token); // max_lat
        cities.push_back(c);
    }
    return true;
}

//helper
void printCity(const City& c) {
    std::cout << "  " << c.name
              << " (" << c.province << ")"
              << "  pop: " << c.population
              << "  [" << c.bbox.x1 << "," << c.bbox.y1
              << " -> " << c.bbox.x2 << "," << c.bbox.y2 << "]\n";
}

City* findCity(const Rectangle& r) { // match result rectangle back to a City
    for (City& c : cities)
        if (c.bbox.equals(r)) return &c;
    return nullptr;
}

void printDivider() { std::cout << "────────────────────────────────────────\n"; }

/*  Finds all cities whose bounding box overlaps the query region.
    Real-world use:
        "Which cities are in Punjab province?"
        "What cities are in this flood-affected zone?"
        "Show all cities along the Sindh coast"*/
void searchRegion(Rtree& tree, Rectangle queryBox, const std::string& label) {
    printDivider();
    std::cout << "FUNCTION 1: Region Search — " << label << "\n";
    std::cout << "Box: [" << queryBox.x1 << "," << queryBox.y1
              << " -> "   << queryBox.x2 << "," << queryBox.y2 << "]\n";
    printDivider();
    std::vector<Rectangle> results;
    tree.search(tree.getRoot(), queryBox, results); // R-tree does the work
    if (results.empty()) { std::cout << "No cities found.\n"; return; }
    std::cout << "Cities found (" << results.size() << "):\n";
    for (const Rectangle& r : results) {
        City* c = findCity(r);
        if (c) printCity(*c);
    }
}


// ── FUNCTION 2: findNearbyCities 
/*
    Builds a box around a GPS point and finds all overlapping cities.
    Real-world use:
        "What cities are near this earthquake epicenter?"
        "I am at this GPS location, what cities are within 100km?"

    radius in degrees:  0.5 ≈ 50km,  1.0 ≈ 100km,  1.5 ≈ 150km*/
void findNearbyCities(Rtree& tree, float lon, float lat, float radius, const std::string& label) {
    printDivider();
    std::cout << "FUNCTION 2: Nearby Search — " << label << "\n";
    std::cout << "Point: (" << lon << ", " << lat << ")  radius: "
              << radius << " deg (~" << (int)(radius * 100) << "km)\n";
    printDivider();
    Rectangle searchBox = { lon - radius, lat - radius,
                            lon + radius, lat + radius };
    std::vector<Rectangle> results;
    tree.search(tree.getRoot(), searchBox, results); // R-tree does the work
    if (results.empty()) { std::cout << "No cities found nearby.\n"; return; }
    std::cout << "Nearby cities (" << results.size() << "):\n";
    for (const Rectangle& r : results) {
        City* c = findCity(r);
        if (c) printCity(*c);
    }
}


// ── Export to CSV (plug in GeoPandas later without changing any C++) ──────────

void exportToCSV(Rtree& tree, const std::string& filename) {
    std::vector<Rectangle> all;
    tree.search(tree.getRoot(), {60.0, 23.0, 78.0, 37.5}, all);
    std::ofstream file(filename);
    file << "name,province,population,min_lon,min_lat,max_lon,max_lat\n";
    for (const Rectangle& r : all) {
        City* c = findCity(r);
        if (c)
            file << c->name << "," << c->province << "," << c->population << ","
                 << c->bbox.x1 << "," << c->bbox.y1 << ","
                 << c->bbox.x2 << "," << c->bbox.y2 << "\n";
    }
    file.close();
    std::cout << "\nExported to " << filename << " (ready for GeoPandas)\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "   PAKISTAN SPATIAL SEARCH (R-TREE)\n";
    std::cout << "========================================\n";
    // load from CSV file
    if (!loadCities("pakistan_cities.csv")) return 1;
    // insert all cities into R-tree
    Rtree tree(4);
    std::cout << "\nLoading " << cities.size() << " cities into R-tree...\n";
    for (const City& c : cities) {
        tree.insert(c.bbox);
        std::cout << "  inserted: " << c.name << "\n";
    }
    std::cout << "Done.\n\n";
    // ── DEMO: Function 1 ─────────────────────────────────────────────
    searchRegion(tree, {60.0, 23.0, 78.0, 37.5}, "All of Pakistan");
    searchRegion(tree, {70.0, 29.0, 75.5, 33.0}, "Punjab Province");
    searchRegion(tree, {66.0, 24.0, 70.0, 27.0}, "Sindh / Southern Pakistan");
    // ── DEMO: Function 2 ─────────────────────────────────────────────
    findNearbyCities(tree, 73.06, 33.72, 0.5, "Islamabad capital area");
    findNearbyCities(tree, 66.99, 24.86, 0.8, "Karachi coast");
    findNearbyCities(tree, 73.00, 34.50, 1.5, "Northern Pakistan KPK/GB");
    // ── Export for GeoPandas ──────────────────────────────────────────
    exportToCSV(tree, "results.csv");
    // ── Interactive mode ──────────────────────────────────────────────
    std::cout << "\n========================================\n";
    std::cout << "          INTERACTIVE MODE\n";
    std::cout << "========================================\n";
    std::cout << "Pakistan lon: 60.8-77.8   lat: 23.5-37.1\n";
    char choice;
    do {
        std::cout << "\n1 → Search by region\n";
        std::cout << "2 → Search by point + radius\n";
        std::cout << "q → Quit\n";
        std::cout << "Choice: ";
        std::cin >> choice;
        if (choice == '1') {
            float x1, y1, x2, y2;
            std::cout << "Enter min_lon min_lat max_lon max_lat: ";
            std::cin >> x1 >> y1 >> x2 >> y2;
            searchRegion(tree, {x1, y1, x2, y2}, "Custom region");
        } else if (choice == '2') {
            float lon, lat, radius;
            std::cout << "Enter lon lat radius(degrees): ";
            std::cin >> lon >> lat >> radius;
            findNearbyCities(tree, lon, lat, radius, "Custom point");
        }
    } while (choice != 'q');
    std::cout << "\nGoodbye!\n";
    return 0;
}