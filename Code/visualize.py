import sys
import os
import pandas as pd
import geopandas as gpd
import matplotlib
matplotlib.use('Agg')          # non-interactive: saves to file instead of GUI window
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.lines import Line2D
import matplotlib.patheffects as pe

SHAPEFILE = "natural_earth_data/ne_110m_admin_0_countries.shp"
ALL_CITIES_CSV = "pakistan_cities.csv"
OUTPUT_FILE    = "map_output.png"

PROVINCE_COLORS = {
    "Punjab"      : "#e63946",   # red
    "Sindh"       : "#2a9d8f",   # teal
    "KPK"         : "#e9c46a",   # yellow
    "Balochistan" : "#f4a261",   # orange
    "ICT"         : "#6a4c93",   # purple
    "GB"          : "#457b9d",   # steel blue
    "AJK"         : "#2d6a4f",   # forest green
}
DEFAULT_COLOR = "#888888"

# city type → marker shape
TYPE_MARKERS = {
    "capital"    : "*",
    "port"       : "D",
    "industrial" : "s",
    "historical" : "^",
    "city"       : "o",
}

def get_color(province):
    return PROVINCE_COLORS.get(province, DEFAULT_COLOR)

def get_marker(city_type):
    return TYPE_MARKERS.get(city_type, "o")

def scale_size(population, min_s=60, max_s=600):
    """Scale dot size by population between min_s and max_s."""
    pop_min = 90_000
    pop_max = 16_000_000
    ratio = (population - pop_min) / (pop_max - pop_min)
    ratio = max(0.0, min(1.0, ratio))   # clamp to [0,1]
    return min_s + ratio * (max_s - min_s)

def fmt_pop(pop):
    """Format population with commas."""
    return f"{pop:,}"

def smart_label_offset(lon, lat):
    """
    Return (dx, dy, ha) for city label placement.
    Cities on the right edge nudge left, others nudge right.
    Avoids labels running off the map edge.
    """
    if lon > 75.0:
        return -0.15, 0.12, 'right'
    elif lon < 64.0:
        return  0.15, 0.12, 'left'
    else:
        return  0.15, 0.12, 'left'

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 visualize.py <query_result.csv> [title] [function_number]")
        sys.exit(1)
    result_csv   = sys.argv[1]
    map_title    = sys.argv[2] if len(sys.argv) > 2 else "Search Results"
    func_num     = sys.argv[3] if len(sys.argv) > 3 else "1"
    # subtitle per function
    subtitles = {
        "1": "R-Tree Region Search",
        "2": "R-Tree Proximity Search",
        "3": "R-Tree Type Filter Search",
    }
    subtitle = subtitles.get(func_num, "R-Tree Spatial Search")

    if not os.path.exists(SHAPEFILE):
        print(f"ERROR: Shapefile not found: {SHAPEFILE}")
        print("Make sure ne_110m_admin_0_countries.shp is in the same folder.")
        sys.exit(1)
    if not os.path.exists(ALL_CITIES_CSV):
        print(f"ERROR: {ALL_CITIES_CSV} not found.")
        sys.exit(1)
    if not os.path.exists(result_csv):
        print(f"ERROR: Result CSV not found: {result_csv}")
        sys.exit(1)
    # load Pakistan outline from shapefile
    world         = gpd.read_file(SHAPEFILE)
    pakistan_shape = world[world['ADMIN'] == 'Pakistan']
    # load all cities (for grey background dots)
    all_cities = pd.read_csv(ALL_CITIES_CSV)
    all_cities['lon'] = (all_cities['min_lon'] + all_cities['max_lon']) / 2
    all_cities['lat'] = (all_cities['min_lat'] + all_cities['max_lat']) / 2
    # load result cities (highlighted)
    result_cities = pd.read_csv(result_csv)
    result_cities['lon'] = (result_cities['min_lon'] + result_cities['max_lon']) / 2
    result_cities['lat'] = (result_cities['min_lat'] + result_cities['max_lat']) / 2
    fig, ax = plt.subplots(figsize=(13, 11))
    fig.patch.set_facecolor('#1a1a2e')   # dark navy background
    ax.set_facecolor('#1a1a2e')
    pakistan_shape.plot(
        ax        = ax,
        color     = '#16213e',     # dark fill
        edgecolor = '#4a9eff',     # bright blue border
        linewidth = 1.8,
        zorder    = 1
    )

    # ── Draw all cities as faint grey dots (context layer) ───────────────────
    non_result_names = set(all_cities['name']) - set(result_cities['name'])
    bg_cities = all_cities[all_cities['name'].isin(non_result_names)]

    ax.scatter(
        bg_cities['lon'],
        bg_cities['lat'],
        s         = 25,
        color     = '#555577',
        alpha     = 0.6,
        zorder    = 2,
        marker    = 'o',
        linewidths= 0
    )

    # faint labels for background cities
    for _, row in bg_cities.iterrows():
        ax.annotate(
            row['name'],
            (row['lon'], row['lat']),
            xytext    = (0.1, 0.1),
            textcoords= 'offset points',
            fontsize  = 6,
            color     = '#666688',
            alpha     = 0.7,
            zorder    = 2
        )

    # ── Draw result cities (highlighted layer) ────────────────────────────────
    plotted_provinces = set()

    for _, row in result_cities.iterrows():
        color  = get_color(row['province'])
        marker = get_marker(row['type'])
        size   = scale_size(row['population'])
        dx, dy, ha = smart_label_offset(row['lon'], row['lat'])
        # outer glow ring
        ax.scatter(
            row['lon'], row['lat'],
            s          = size * 2.2,
            color      = color,
            alpha      = 0.2,
            zorder     = 3,
            marker     = 'o',
            linewidths = 0
        )
        # main dot
        ax.scatter(
            row['lon'], row['lat'],
            s          = size,
            color      = color,
            edgecolors = 'white',
            linewidths = 1.2,
            zorder     = 4,
            marker     = marker,
            label      = row['province'] if row['province'] not in plotted_provinces else ""
        )
        plotted_provinces.add(row['province'])
        # city name label with dark outline so it's readable on any background
        ax.annotate(
            f"{row['name']}",
            (row['lon'], row['lat']),
            xytext    = (dx, dy),
            textcoords= 'offset points',
            fontsize  = 8.5,
            fontweight= 'bold',
            color     = 'white',
            ha        = ha,
            zorder    = 5,
            path_effects = [
                pe.withStroke(linewidth=2.5, foreground='#1a1a2e')
            ]
        )
        # population sub-label
        ax.annotate(
            fmt_pop(row['population']),
            (row['lon'], row['lat']),
            xytext    = (dx, dy - 8),
            textcoords= 'offset points',
            fontsize  = 6.5,
            color     = color,
            ha        = ha,
            alpha     = 0.9,
            zorder    = 5,
            path_effects = [
                pe.withStroke(linewidth=1.5, foreground='#1a1a2e')
            ]
        )

    # map extent: zoom to Pakistan with padding 
    ax.set_xlim(59.5, 78.5)
    ax.set_ylim(22.5, 38.0)
    # gridlines 
    ax.grid(
        True,
        color     = '#2a2a4a',
        linewidth = 0.5,
        linestyle = '--',
        alpha     = 0.6,
        zorder    = 0
    )
    ax.tick_params(colors='#888899', labelsize=8)
    for spine in ax.spines.values():
        spine.set_edgecolor('#2a2a4a')
    # axis labels
    ax.set_xlabel("Longitude", color='#aaaacc', fontsize=9, labelpad=6)
    ax.set_ylabel("Latitude",  color='#aaaacc', fontsize=9, labelpad=6)
    # provinces (colored dots) 
    province_patches = []
    for prov in sorted(plotted_provinces):
        province_patches.append(
            mpatches.Patch(color=get_color(prov), label=prov)
        )

    # legend: marker shapes for city types
    types_in_results = result_cities['type'].unique()
    type_handles = []
    for t in sorted(types_in_results):
        type_handles.append(
            Line2D([0], [0],
                   marker    = get_marker(t),
                   color     = 'w',
                   markerfacecolor = '#aaaacc',
                   markersize= 7,
                   label     = t,
                   linestyle = 'None')
        )

    # draw two side-by-side legends
    leg1 = ax.legend(
        handles   = province_patches,
        title     = "Province",
        loc       = 'lower left',
        framealpha= 0.25,
        facecolor = '#0f0f1e',
        edgecolor = '#444466',
        labelcolor= 'white',
        title_fontsize = 8,
        fontsize  = 8
    )
    leg1.get_title().set_color('#aaaacc')
    ax.add_artist(leg1)

    if type_handles:
        leg2 = ax.legend(
            handles   = type_handles,
            title     = "City Type",
            loc       = 'lower right',
            framealpha= 0.25,
            facecolor = '#0f0f1e',
            edgecolor = '#444466',
            labelcolor= 'white',
            title_fontsize = 8,
            fontsize  = 8
        )
        leg2.get_title().set_color('#aaaacc')

    # ── Dot size legend ───────────────────────────────────────────────────────
    size_labels = [("1M",  1_000_000),
                   ("5M",  5_000_000),
                   ("16M", 16_000_000)]
    size_handles = [
        Line2D([0], [0],
               marker    = 'o',
               color     = 'w',
               markerfacecolor = '#aaaacc',
               markersize= (scale_size(p) / 60) ** 0.5 * 4,
               label     = lbl,
               linestyle = 'None')
        for lbl, p in size_labels
    ]
    leg3 = ax.legend(
        handles   = size_handles,
        title     = "Population",
        loc       = 'upper right',
        framealpha= 0.25,
        facecolor = '#0f0f1e',
        edgecolor = '#444466',
        labelcolor= 'white',
        title_fontsize = 8,
        fontsize  = 8
    )
    leg3.get_title().set_color('#aaaacc')

    # ── Title ─────────────────────────────────────────────────────────────────
    ax.set_title(
        f"Pakistan Spatial Search  —  {map_title}",
        fontsize   = 15,
        fontweight = 'bold',
        color      = 'white',
        pad        = 14
    )
    ax.text(
        0.5, 1.01,
        f"{subtitle}  |  {len(result_cities)} cities found",
        transform  = ax.transAxes,
        ha         = 'center',
        fontsize   = 9,
        color      = '#8888bb',
        style      = 'italic'
    )

    # ── Stats box (bottom centre) ─────────────────────────────────────────────
    total_pop = result_cities['population'].sum()
    largest   = result_cities.loc[result_cities['population'].idxmax(), 'name']
    stats_text = (
        f"Cities: {len(result_cities)}   |   "
        f"Total pop: {fmt_pop(total_pop)}   |   "
        f"Largest: {largest}"
    )
    fig.text(
        0.5, 0.01,
        stats_text,
        ha         = 'center',
        fontsize   = 8.5,
        color      = '#8888bb',
        style      = 'italic'
    )
    plt.tight_layout(rect=[0, 0.03, 1, 1])
    plt.savefig(OUTPUT_FILE, dpi=180, bbox_inches='tight',
                facecolor=fig.get_facecolor())
    plt.close()
    print(f"  Map saved to {OUTPUT_FILE}")
    # Try common Linux/Chromebook image viewers in order
    viewers = ['eog', 'feh', 'display', 'xdg-open', 'gpicview', 'ristretto']
    opened  = False
    for viewer in viewers:
        ret = os.system(f"which {viewer} > /dev/null 2>&1")
        if ret == 0:
            os.system(f"{viewer} {OUTPUT_FILE} &")
            opened = True
            break

    if not opened:
        print(f"  Could not auto-open image. Please open {OUTPUT_FILE} manually.")
if __name__ == "__main__":
    main()