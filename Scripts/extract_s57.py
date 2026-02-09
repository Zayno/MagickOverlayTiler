"""
Extract features from S57 ENC files (.000) and output to CSV.
Outputs X;Y;[attributes...] in WGS 84 / Pseudo-Mercator (EPSG:3857).
[--source] will add teh source file to the CSV
Configure LAYER_NAME and ATTRIBUTES below to control what gets exported.
"""

import os
import sys
import csv
from osgeo import ogr, osr

# ============================================================
# CONFIGURATION - Change these to extract different data. Example:
#LAYER_NAME = 'WRECKS'
#ATTRIBUTES = ['OBJNAM', 'CATWRK', 'VALSOU']
# ============================================================
LAYER_NAME = 'BOYSPP'
ATTRIBUTES = ['BOYSHP']
# ============================================================


def extract_features(parent_dir, output_csv, show_source=False):
    # Source CRS: S57 files are in WGS84 (EPSG:4326)
    src_srs = osr.SpatialReference()
    src_srs.ImportFromEPSG(4326)
    src_srs.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)

    # Target CRS: WGS 84 / Pseudo-Mercator (EPSG:3857)
    dst_srs = osr.SpatialReference()
    dst_srs.ImportFromEPSG(3857)
    dst_srs.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)

    transform = osr.CoordinateTransformation(src_srs, dst_srs)

    results = []

    # Walk directory tree looking for .000 files
    for root, dirs, files in os.walk(parent_dir):
        for filename in files:
            if filename.lower().endswith('.000'):
                filepath = os.path.join(root, filename)
                print(f"Processing: {filepath}")

                ds = ogr.Open(filepath)
                if ds is None:
                    print(f"  WARNING: Could not open {filepath}")
                    continue

                layer = ds.GetLayerByName(LAYER_NAME)
                if layer is None:
                    print(f"  No {LAYER_NAME} layer found in {filepath}")
                    ds = None
                    continue

                feat_count = layer.GetFeatureCount()
                print(f"  Found {feat_count} {LAYER_NAME} feature(s)")

                for feature in layer:
                    geom = feature.GetGeometryRef()
                    if geom is None:
                        continue

                    # Get centroid for non-point geometries
                    if geom.GetGeometryType() != ogr.wkbPoint:
                        geom = geom.Centroid()

                    # Clone and transform to EPSG:3857
                    geom_clone = geom.Clone()
                    geom_clone.Transform(transform)

                    x = geom_clone.GetX()
                    y = geom_clone.GetY()

                    # Collect attribute values in list order
                    attr_values = []
                    for attr_name in ATTRIBUTES:
                        val = feature.GetField(attr_name)
                        attr_values.append('' if val is None else val)

                    results.append((x, y, attr_values, filename if show_source else None))

                ds = None

    # Write CSV
    with open(output_csv, 'w', newline='') as f:
        writer = csv.writer(f, delimiter=';')

        header = ['X', 'Y'] + ATTRIBUTES
        if show_source:
            header.append('SOURCE')
        writer.writerow(header)

        for x, y, attr_values, source in results:
            row = [f"{x:.6f}", f"{y:.6f}"] + [str(v) for v in attr_values]
            if show_source:
                row.append(source)
            writer.writerow(row)

    print(f"\nDone. {len(results)} {LAYER_NAME} features written to {output_csv}")


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description=f'Extract {LAYER_NAME} features from S57 ENC files')
    parser.add_argument('parent_dir', help='Parent directory to search for .000 files')
    parser.add_argument('-o', '--output', default='output.csv', help='Output CSV file (default: output.csv)')
    parser.add_argument('--source', action='store_true', help='Include source filename in CSV output')

    args = parser.parse_args()

    if not os.path.isdir(args.parent_dir):
        print(f"Error: '{args.parent_dir}' is not a valid directory")
        sys.exit(1)

    extract_features(args.parent_dir, args.output, args.source)
