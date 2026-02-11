#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <Magick++.h>
#include "defs.h"

using namespace std;
using namespace Magick;
using namespace std::filesystem;
using namespace std::chrono;

Grid3X3 GridBase;

vector<PointData> CoordsList;
const path TilesOutDir = "C:\\TilesOut";
int IconHalfSize = 40;
int IconSizeAfterResize = -1;
double OriginShift = 20037508.342789244;

//Image Ring ( "C:\\QGIS\\Ring.png" );

Image TempTile(Geometry(256, 256), Color("rgba(0,0,0,0.0)"));

void Init()
{
	TempTile.backgroundColor(Color("rgba(0,0,0,0.0)"));
	TempTile.magick("PNG");
}

bool IsFullyTransparent(const Magick::Image& image, int XOffset, int YOffset)
{
	if (!image.alpha())
		return false;

	const size_t width = 256;
	const size_t height = 256;

	const Magick::Quantum* pixels = image.getConstPixels(XOffset, YOffset, width, height);

	if (!pixels)
		return false;

	const size_t channels = image.channels();

	// In typical RGBA, alpha is the last channel
	const size_t alphaOffset = channels - 1;

	const Magick::Quantum transparent = 0; // See note below

	for (size_t i = 0; i < width * height; ++i)
	{
		const Magick::Quantum alpha = pixels[i * channels + alphaOffset];

		if (alpha != transparent)
		{
			return false; // Found visible pixel
		}
	}

	return true;
}

// Step 1: normalize EPSG:3857 meters to [0..1]
PointData Normalize(double x, double y)
{
	double xNorm = (x + OriginShift) / (2 * OriginShift);
	double yNorm = (OriginShift - y) / (2 * OriginShift); // top-left origin
	return PointData(xNorm, yNorm);
}

// Step 2: to pixel space and tile indices at zoom
//static ( double px, double py, int tx, int ty, double offX, double offY )
void ToPixelsAndTiles(double xNorm, double yNorm, int zoom, int& tx, int& ty, double& offX, double& offY)
{
	double worldSize = 256 * std::pow(2, zoom);
	double px = xNorm * worldSize;
	double py = yNorm * worldSize;

	tx = (int)std::floor(px / 256.0);
	ty = (int)std::floor(py / 256.0);

	offX = px - (tx * 256);
	offY = py - (ty * 256);
}

void HandleTileOverlay(Image& SingleTile, int TileX, int TileY, int ZoomLevel)
{
	path TilePath = TilesOutDir / to_string(ZoomLevel) / to_string(TileX);
	create_directories(TilePath);
	TilePath = TilePath / (to_string(TileY) + string(".png"));

	TempTile.erase();

	string MyPath(TilePath.generic_string());

	if (exists(TilePath))
	{
		Image Overlay(MyPath);

		TempTile.composite(Overlay, 0, 0, OverCompositeOp);
	}

	TempTile.composite(SingleTile, 0, 0, OverCompositeOp);

	TempTile.write(MyPath);
	//cout << MyPath << endl;
}

void ProcessTile(const PointData& MyPoint, int ZoomLevel)
{
	PointData Normalized = Normalize(MyPoint.XX, MyPoint.YY);
	Normalized.IconID = MyPoint.IconID;

	int tx = 0, ty = 0;
	double offX = 0, offY = 0;
	ToPixelsAndTiles(Normalized.XX, Normalized.YY, ZoomLevel, tx, ty, offX, offY);

	int X_Offset = 256 + (int)offX - IconHalfSize;
	int Y_Offset = 256 + (int)offY - IconHalfSize;

	GridBase.GridOverlay(X_Offset, Y_Offset, *GetIconByID(Normalized.IconID));

	//int D = 5;
	//if ( D == 5 )
	//{
	//	GridBase.SaveToFile ();
	//	D++;
	//}

	if (GridBase.IsGridTileUsed(0))
		HandleTileOverlay(*GridBase.SubTiles[0], tx - 1, ty - 1, ZoomLevel);
	if (GridBase.IsGridTileUsed(1))
		HandleTileOverlay(*GridBase.SubTiles[1], tx, ty - 1, ZoomLevel);
	if (GridBase.IsGridTileUsed(2))
		HandleTileOverlay(*GridBase.SubTiles[2], tx + 1, ty - 1, ZoomLevel);

	if (GridBase.IsGridTileUsed(3))
		HandleTileOverlay(*GridBase.SubTiles[3], tx - 1, ty, ZoomLevel);
	if (GridBase.IsGridTileUsed(4))
		HandleTileOverlay(*GridBase.SubTiles[4], tx, ty, ZoomLevel);
	if (GridBase.IsGridTileUsed(5))
		HandleTileOverlay(*GridBase.SubTiles[5], tx + 1, ty, ZoomLevel);

	if (GridBase.IsGridTileUsed(6))
		HandleTileOverlay(*GridBase.SubTiles[6], tx - 1, ty + 1, ZoomLevel);
	if (GridBase.IsGridTileUsed(7))
		HandleTileOverlay(*GridBase.SubTiles[7], tx, ty + 1, ZoomLevel);
	if (GridBase.IsGridTileUsed(8))
		HandleTileOverlay(*GridBase.SubTiles[8], tx + 1, ty + 1, ZoomLevel);
}

PointData SplitToPointData(const std::string& input)
{
	std::string_view sv(input);

	size_t first = sv.find(';');
	size_t second = sv.find(';', first + 1);

	if (first == std::string_view::npos || second == std::string_view::npos)
	{
		cout << "Invalid input format" << endl;
	}

	PointData result;

	result.XX = std::stod(std::string(sv.substr(0, first)));
	result.YY = std::stod(std::string(sv.substr(first + 1, second - first - 1)));
	result.IconID = std::stoi(std::string(sv.substr(second + 1)));

	return result;
}

void FillCoordinates()
{
	path filepath = "C:\\QGIS\\X_Y.csv";

	ifstream file;
	file.open(filepath);

	if (!file)
	{
		std::cerr << "Could not open " << filepath << "\n";
		return;
	}

	std::string line;
	while (std::getline(file, line))
	{
		CoordsList.push_back(SplitToPointData(line));
	}
}

int main(int argc, char** argv)
{
	//if (argc < 2)
	//{
	//	std::cerr << "Usage: " << argv[0] << " <integer>\n";
	//	return 1;
	//}

	//int ZoomLevel = std::atoi(argv[1]);
	int ZoomLevel = 10;

	cout << "Working...  ZOOM:  " << ZoomLevel << endl;

	auto start = high_resolution_clock::now();

	InitializeMagick(*argv);

	Init();

	ResizeImages(ZoomLevel);
	IconHalfSize = Conical_SPP.size().width() / 2;
	IconSizeAfterResize = Conical_SPP.size().width();
	FillCoordinates();

	for (size_t i = 0; i < CoordsList.size(); i++)
	{
		PointData MyPoint = CoordsList[i];

		ProcessTile(MyPoint, ZoomLevel);
	}

	auto end = high_resolution_clock::now();

	duration<double> elapsed = end - start; // seconds as double

	std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

	std::cout << "\a Press Enter to exit...";
	std::cin.get(); // waits for user to press Enter
	return 0;
	//try
	//{
	//	Image image ( "C:\\QGIS\\Ring.png" );

	//	image.write ( "outputzzz.png" );

	//	std::cout << "Saved output.png successfully." << std::endl;
	//}
	//catch ( Exception& error_ )
	//{
	//	std::cout << "Caught exception: " << error_.what () << std::endl;
	//	return 1;
	//}
}