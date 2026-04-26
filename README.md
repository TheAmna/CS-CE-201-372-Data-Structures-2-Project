# CS-CE-201-372-Data-Structures-2-Project

## Team Members 
- [Adina Naveed](https://github.com/adinanaveed)
- [Humna Khan](https://github.com/humna0809)
- [Amna Ali](https://github.com/TheAmna)
  
## Project Overview

Research analysis on the data structure R-tree. We implemented the R-tree from scratch in C++ along with a B-tree for comparison, assessed the time complexities of all core operations, and built a real-world application that searches Pakistani cities using spatial queries.


## Motivation to use R trees

An R-tree is a height-balanced spatial index that stores data as Minimum Bounding Rectangles (MBRs). B-trees only indexes one dimension, an R-tree can answer 2D range queries like "find all cities inside this region". At each level of the tree, a parent MBR tightly encloses all of its children, allowing entire subtrees to be skipped during search if they don't overlap the query box.


## Interim Demo

https://github.com/user-attachments/assets/595db9af-1605-4ce4-ba3b-8f9f2c13852e


## Application

We built a user interactive C++ terminal application using 20 Pakistani cities as spatial data. Each city is stored as a bounding box in the R-tree. Once the search query is done, map visualization is handled separately in Python using GeoPandas and Natural Earth data. The app supports:

- Searching cities by province or custom region
- Finding cities near a GPS point within a given radius
- Filtering cities by type (port, capital, industrial, etc.)
- Province-level population and density statistics

  <img width="400" height="400" alt="image" src="https://github.com/user-attachments/assets/d8a6fbe9-2534-4f71-8a3d-7b7000d83f26" />
*Image showing the result from searching region Punjab*



## Challenges

1. When a node overflows, the split algorithm has to divide rectangles into two groups in a way that minimizes MBR overlap. So we tried searching up the logic, found quite interesting articles on it.
2. C++ has limited libraries for map rendering, so we moved the visualization layer to Python and utlised libraries such as Pandas, Geopandas amd Matplotlib to visualise the maps.
3. The R-tree has additional internal operations beyond the usual insert, search, and delete — specifically `condenseTree` and `reinsertSubtree` for deletion which required deeper reading of Guttman's original 1984 paper to implement correctly.




## Key Learnings

1. Tree quality depends heavily on the split heuristic. A bad split produces large overlapping MBRs that force the search to visit more branches, degrading performance toward O(n).
2. Deletion in an R-tree works differently from a B-tree — instead of borrowing or merging like B-trees do, R-trees dissolve underfull nodes and re-insert their entries, which keeps the spatial quality of the tree intact.
