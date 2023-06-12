#pragma once

#include "ScreenHandler.h"
#include "Utility.h"
#include "Map.h"
#include "Player.h"
#include "Settings.h"
#include "DataTypes.h"

class RaycastRenderer
{
private:
	int m_FOV{};

	int m_RenderWidth{};
	int m_RenderHeight{};

	int m_ProjectionPlaneCenter{};
	double m_DistanceToProjectionPlane{};

	int m_MaxSliceY{};
	int m_MinSliceY{};

	pointf_t m_Intersections[RENDER_WIDTH]{};

	ScreenHandler* m_ScreenHandler{};

	Map* m_pEngineMap{};
	Player* m_pEnginePlayer{};

public:
	RaycastRenderer(int renderWidth, int renderHeight, Map* m, Player* p);
	~RaycastRenderer();

	bool init();

	uint32_t* getScreenPixels();

	pointf_t* getIntersections();

	void paintScreen(uint32_t c);

	void raycast();

	// The distance variable assumes that the distance has already been corrected for the fisheye effect
	void renderWallSlice(int x, double distance, int cellSpaceColumn, double light, Texture* texture);

	void renderFullFloor();

	void renderFullCeiling();

	void renderVerticalFloorSlice();
};

