#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

constexpr int DISPLAY_WIDTH { 1280 };
constexpr int DISPLAY_HEIGHT { 720 };
constexpr int DISPLAY_SCALE{ 1 };

void Draw();

enum eBoxType
{
	STATIC,
	MOUSE
};

// A quick struct to represent AABBs for manual drawing
struct AABB
{
	Point2D pos		{0.f, 0.f};
	Vector2D size	{150.f, 75.f};

	Point2D Left()		{ return Point2D(pos.x - size.x, pos.y); }
	Point2D Right()		{ return Point2D(pos.x + size.x, pos.y); }
	Point2D Top()		{ return Point2D(pos.x, pos.y - size.y); }
	Point2D Bottom()	{ return Point2D(pos.x, pos.y + size.y); }

	Point2D TopLeft()		{ return pos - size; }
	Point2D BottomRight()	{ return pos + size; }
	Point2D TopRight()		{ return Point2D(pos.x + size.x, pos.y - size.y); }
	Point2D BottomLeft()	{ return Point2D(pos.x - size.x, pos.y + size.y); }
};
AABB aabb[2];

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE );
	Play::LoadBackground("Data\\Backgrounds\\background.png");

	aabb[STATIC].pos = Point2D(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 2);
	aabb[STATIC].size = Vector2D(75.f, 150.f);
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	aabb[MOUSE].pos = Play::GetMousePos();

	Draw();

	return Play::KeyDown( VK_ESCAPE );
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cGrey);

	Play::DrawRect(aabb[STATIC].pos - aabb[STATIC].size, aabb[STATIC].pos + aabb[STATIC].size, Play::cWhite);
	
	// Draw user AABB with visible test results...

	// 1. Encapsulate each 'axis test' as a unique boolean value
	bool left =		(aabb[MOUSE].Left().x < aabb[STATIC].Right().x);
	bool right =	(aabb[MOUSE].Right().x > aabb[STATIC].Left().x);
	bool up =		(aabb[MOUSE].Top().y < aabb[STATIC].Bottom().y);
	bool down =		(aabb[MOUSE].Bottom().y > aabb[STATIC].Top().y);

	// 2. Draw each edge coloured according to its axis test
	Play::DrawLine(aabb[MOUSE].BottomLeft(), aabb[MOUSE].TopLeft(), left ? Play::cGreen : Play::cRed);
	Play::DrawLine(aabb[MOUSE].TopRight(), aabb[MOUSE].BottomRight(), right ? Play::cGreen : Play::cRed);
	Play::DrawLine(aabb[MOUSE].TopLeft(), aabb[MOUSE].TopRight(), up ? Play::cGreen : Play::cRed);
	Play::DrawLine(aabb[MOUSE].BottomRight(), aabb[MOUSE].BottomLeft(), down ? Play::cGreen : Play::cRed);
	
	// 3. Visualise failed 'axis overlap' lines
	if(!up)
	{
		Play::DrawLine(	aabb[MOUSE].Top(), aabb[STATIC].Bottom(), Play::cRed);
	}
	if(!down)
	{
		Play::DrawLine(	aabb[MOUSE].Bottom(), aabb[STATIC].Top(), Play::cRed);
	}
	if (!left)
	{
		Play::DrawLine(aabb[MOUSE].Left(), aabb[STATIC].Right(), Play::cRed);
	}
	if (!right)
	{
		Play::DrawLine(aabb[MOUSE].Right(), aabb[STATIC].Left(), Play::cRed);
	}

	// 4. Print HUD details
	if(left && right)
	{
		Play::DrawDebugText(Point2D(100, 50), "X Axis: Overlapped", Play::cGreen);
	}
	else
	{
		Play::DrawDebugText(Point2D(100, 50), "X Axis: No Overlap", Play::cRed);
	}

	if (up && down)
	{
		Play::DrawDebugText(Point2D(100, 70), "Y Axis: Overlapped", Play::cGreen);
	}
	else
	{
		Play::DrawDebugText(Point2D(100, 70), "Y Axis: No Overlap", Play::cRed);
	}

	if (left && right && up && down)
	{
		Play::DrawDebugText(Point2D(100, 110), "Collision Detected", Play::cGreen);
	}
	else
	{
		Play::DrawDebugText(Point2D(100, 110), "No Collision Detected", Play::cRed);
	}

	Play::PresentDrawingBuffer();
}