#pragma once
using namespace Magick;

extern int IconSizeAfterResize;
extern bool IsFullyTransparent(const Magick::Image& image, int XOffset, int YOffset);

struct PointData
{
	PointData()
	{
		XX = YY = IconID = 0;
	}
	PointData(double _x, double _y)
	{
		XX = _x;
		YY = _y;
		IconID = 0;
	}
	double XX;
	double YY;
	int IconID;
};


struct Rect
{
	int x, y, w, h;              // top-left, width, height

	Rect ()
	{
		x = y = w = h = 0;
	}

	Rect ( int _x, int _y, int _w, int _h )
	{
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}
};

class Grid3X3
{
public:
	Image BigTile;

	Image Tile1;
	Image Tile2;
	Image Tile3;
	Image Tile4;
	Image Tile5;
	Image Tile6;
	Image Tile7;
	Image Tile8;
	Image Tile9;


	bool IntersectionList[9] = { 0 };
	Image* SubTiles[9] = { 0 };

	Grid3X3 () :
		BigTile ( Geometry ( 768, 768 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile1 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile2 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile3 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile4 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile5 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile6 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile7 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile8 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) ),
		Tile9 ( Geometry ( 256, 256 ), Color ( "rgba(0,0,0,0.0)" ) )
	{
		BigTile.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );

		Tile1.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile2.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile3.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile4.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile5.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile6.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile7.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile8.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );
		Tile9.backgroundColor ( Color ( "rgba(0,0,0,0.0)" ) );

		SubTiles[0] = &Tile1;
		SubTiles[1] = &Tile2;
		SubTiles[2] = &Tile3;
		SubTiles[3] = &Tile4;
		SubTiles[4] = &Tile5;
		SubTiles[5] = &Tile6;
		SubTiles[6] = &Tile7;
		SubTiles[7] = &Tile8;
		SubTiles[8] = &Tile9;

		memset ( IntersectionList, 0, sizeof ( IntersectionList ) );
	}
	~Grid3X3 ()
	{
	}

	inline bool IsGridTileUsed ( int Index )
	{
		return ( IntersectionList[Index] == true );
	}

	void Clear ()
	{
		BigTile.erase ();

		for ( size_t i = 0; i < 9; i++ )
		{
			if ( IntersectionList[i] == true )
				SubTiles[i]->erase ();
		}

		memset ( IntersectionList, 0, sizeof ( IntersectionList ) );
	}

	void GridOverlay ( int x, int y, const Image& src )
	{
		Clear ();
		BigTile.composite ( src, x, y, OverCompositeOp );
		mark_intersections ( x, y );
	}

	void SaveToFile ()
	{
		BigTile.write ( "ZZZ_GridBitmap.png" );
		std::cout << "Saved Big Tile" << std::endl;
	}

	inline bool intersects ( const Rect& a, const Rect& b )
	{
		// Strict overlap (touching edges is NOT an intersection of pixels)
		return ( a.x < b.x + b.w ) && ( a.x + a.w > b.x ) &&
			( a.y < b.y + b.h ) && ( a.y + a.h > b.y );
	}


	void mark_intersections(int ox, int oy)
	{
		constexpr int GRID_SIZE = 768;
		constexpr int CELLS_PER = 3;
		constexpr int CELL_SIZE = GRID_SIZE / CELLS_PER; // 256
		int Icon_Size = IconSizeAfterResize;			 //CAREFUL with icon size

		Rect overlay{ox, oy, Icon_Size, Icon_Size};

		//int countt = 0;
		// Check each of the 9 cells
		for (int row = 0; row < CELLS_PER; ++row)
		{
			for (int col = 0; col < CELLS_PER; ++col)
			{
				Rect cell{col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE};
				if (intersects(cell, overlay))
				{
					int IntersectIndex = row * CELLS_PER + col;
					if (!IsFullyTransparent(BigTile, col * CELL_SIZE, row * CELL_SIZE))
					{
						SubTiles[IntersectIndex]->copyPixels( BigTile, Geometry(256, 256, col * CELL_SIZE, row * CELL_SIZE), Offset(0, 0));
						IntersectionList[IntersectIndex] = true;
					}

					//countt++;
				}
			}
		}

		//std::cout << countt << std::endl;
	}
};

Image Conical_SPP ( "Icons\\1_Conical_SPP.png" );
Image Can_SPP ( "Icons\\2_Can_SPP.png" );
Image Sphere_SPP ( "Icons\\3_Sphere_SPP.png" );
Image PillarSPP ( "Icons\\4_PillarSPP.png" );
Image Spar_SPP ( "Icons\\5_Spar_SPP.png" );
Image BarrelSPP ( "Icons\\6_BarrelSPP.png" );
Image superbuoy_SPP ( "Icons\\7_superbuoy_SPP.png" );
Image iced_buoySPP ( "Icons\\8_iced_buoySPP.png" );

Image* GetIconByID ( int IconID )
{
	switch (IconID)
	{
		//BOYSPP is According to BOYSHP attribute
		case 1:
		return &Conical_SPP;
		case 2:
		return &Can_SPP;
		case 3:
		return &Sphere_SPP;
		case 4:
		return &PillarSPP;
		case 5:
		return &Spar_SPP;
		case 6:
		return &BarrelSPP;
		case 7:
		return &superbuoy_SPP;
		case 8:
		return &iced_buoySPP;

		default:
		{
			std::cout << "Invalid IconID: " << IconID << std::endl;
			return NULL;

		}
	}
}

void ResizeImages(int ZoomLevel)
{
	const int OriginalIconSize = 80;			// Original icon size (80x80)
	int NewIconSize = OriginalIconSize; // Default to original size

	if (ZoomLevel < 10)
	{
		NewIconSize = 30;
	}
	else if (ZoomLevel < 12)
	{
		NewIconSize = 40;
	}
	else if (ZoomLevel < 15)
	{
		NewIconSize = 50;
	}
	else
	{
		NewIconSize = OriginalIconSize;
	}

	Geometry NewSize(NewIconSize, NewIconSize); // Adjust this size as needed based on the zoom level

	Conical_SPP.resize(NewSize);
	Can_SPP.resize(NewSize);
	Sphere_SPP.resize(NewSize);
	PillarSPP.resize(NewSize);
	Spar_SPP.resize(NewSize);
	BarrelSPP.resize(NewSize);
	superbuoy_SPP.resize(NewSize);
	iced_buoySPP.resize(NewSize);
}