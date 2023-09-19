#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"
#include <cmath>

constexpr float DISPLAY_WIDTH{ 1280 };
constexpr float DISPLAY_HEIGHT{ 720 };
constexpr int DISPLAY_SCALE{ 1 };
const int TEMP_RADIUS{ 48 };
int count_collision{ 0 };

struct Paddle
{
	const Vector2D AABB{ 100.f, 20.f };
	const Vector2D PADDLE_START_POS{ DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 100.0f };
	const Vector2D PADDLE_START_VELOCITY{ 10.0f, 0.0f };
};

struct Ball
{
	const Vector2D AABB{ 50.f, 50.f };
	const Vector2D BALL_START_POS{ DISPLAY_WIDTH / 2 - 250, DISPLAY_HEIGHT / 2 };
	const Vector2D BALL_START_VELOCITY{ 5.0f, 5.0f };
	const Vector2D BALL_ACCELERATION_DEFAULT{ 0.0f, 0.0f };
};

struct Chest
{
	const int CHEST_SIZE = 96;
	const int nCHEST_ROWS = 3;
	const int nCHEST_COLS = 12;
	const Vector2D AABB{ 50.f, 50.f };
	const int SCORE{ 100 };
};

struct Coin
{
	const int SCORE{ 300 };
	const Vector2D AABB{ 50.f, 50.f };
	const Vector2D START_VELOCITY{ 0.0f, 3.0f };
};

struct GameState
{
	int score{ 0 };
	Paddle paddle;
	Ball ball;
	Chest chests;
	Coin coins;
};

GameState gState;

enum GameObjectType
{
	TYPE_BALL = 0,
	TYPE_PADDLE,
	TYPE_CHEST,
	TYPE_COIN,
	TYPE_TOOLS
};

void Draw();
void DrawPaddle();
void DrawBall();
void DrawChest();
void DrawCoin();
void DrawGUI();
void UpdateBall();
void ResetBall();
void UpdatePaddle();
void UpdateCoin();
void CreateBreakoutChests();
void UpdateChests();
void ItemDrop(GameObject& chestObj);
bool AABBCollision(const Point2D& aPos, const Vector2D& aAABB, const Point2D& bPos, const Vector2D& bAABB);
void ApplyReflection(GameObject& objA, const Point2D& aAABB, GameObject& objB, const Point2D& bAABB);
Vector2D GetNearestEdge(const Point2D& aPos, const Vector2D& aAABB, const Point2D& bPos, const Vector2D& bAABB);


void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CentreAllSpriteOrigins();
	Play::CreateGameObject(TYPE_BALL, gState.ball.BALL_START_POS, TEMP_RADIUS, "ball");
	Play::CreateGameObject(TYPE_PADDLE, gState.paddle.PADDLE_START_POS, TEMP_RADIUS, "spanner");

	CreateBreakoutChests();

	GameObject& ballObj = Play::GetGameObjectByType(TYPE_BALL);
	ballObj.velocity = gState.ball.BALL_START_VELOCITY;
	ballObj.acceleration = gState.ball.BALL_ACCELERATION_DEFAULT;
}

bool MainGameUpdate(float elapsedTime)
{
	UpdatePaddle();
	UpdateBall();
	UpdateChests();
	UpdateCoin();
	Draw();
	ResetBall();
	return Play::KeyDown(VK_ESCAPE);
}

int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

void Draw()
{
	Play::ClearDrawingBuffer(Play::cWhite);
	Play::DrawBackground();
	DrawPaddle();
	DrawBall();
	DrawChest();
	DrawCoin();
	DrawGUI();
	Play::PresentDrawingBuffer();
}

void DrawPaddle()
{
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	Play::DrawObject(paddleObj);
	Play::DrawRect(paddleObj.pos - gState.paddle.AABB, paddleObj.pos + gState.paddle.AABB, Play::cGreen);
}

void DrawBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	Play::DrawObjectRotated(Play::GetGameObjectByType(TYPE_BALL));
	Play::DrawRect(ballObj.pos - gState.ball.AABB, ballObj.pos + gState.ball.AABB, Play::cGreen);
}

void DrawChest()
{
	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };
	for (int i = 0; i < chestIds.size(); ++i)
	{
		GameObject& chestObj{ Play::GetGameObject(chestIds[i]) };
		Play::DrawObject(chestObj);
		Play::DrawRect(chestObj.pos - gState.chests.AABB, chestObj.pos + gState.chests.AABB, Play::cGreen);
	}
}

void DrawCoin()
{
	std::vector<int> coinIds{ Play::CollectGameObjectIDsByType(TYPE_COIN) };
	for (int coinId : coinIds)
	{
		GameObject& coinObj{ Play::GetGameObject(coinId) };
		Play::DrawObject(coinObj);
	}
}

void DrawGUI()
{
	Play::DrawFontText("64px", "High Score: " + std::to_string(gState.score), Point2D(DISPLAY_WIDTH / 2, 100), Play::CENTRE);
}

void UpdateBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };

	if (ballObj.pos.y < gState.ball.AABB.y)
	{
		ballObj.velocity.y *= -1;
	}
	if (Play::IsLeavingDisplayArea(ballObj, Play::HORIZONTAL))
	{
		ballObj.velocity.x *= -1;
	}

	// NOTE:
	//  ballObj.oldPos = ballObj.pos;
	//  ballObj.velocity += ballObj.acceleration;
	//  ballObj.pos += ballObj.velocity;
	// Not needed when using UpdateGameObject function
	Play::UpdateGameObject(ballObj);
}

void ResetBall()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };
	if (Play::KeyDown(VK_SPACE))
	{
		ballObj.pos = gState.ball.BALL_START_POS;
		ballObj.velocity = gState.ball.BALL_START_VELOCITY;
		paddleObj.pos = gState.paddle.PADDLE_START_POS;
		paddleObj.velocity = gState.paddle.PADDLE_START_VELOCITY;
	}
}

void UpdatePaddle()
{
	GameObject& ballObj{ Play::GetGameObjectByType(TYPE_BALL) };
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	if (Play::KeyDown(VK_LEFT))
	{
		paddleObj.velocity = -gState.paddle.PADDLE_START_VELOCITY;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		paddleObj.velocity = gState.paddle.PADDLE_START_VELOCITY;
	}
	else
	{
		paddleObj.velocity = { 0, 0 };
	}
	if (Play::IsLeavingDisplayArea(paddleObj))
	{
		paddleObj.pos = paddleObj.oldPos;
	}
	if (AABBCollision(paddleObj.pos, gState.paddle.AABB, ballObj.pos, gState.ball.AABB))
	{
		ApplyReflection(ballObj, gState.ball.AABB, paddleObj, gState.paddle.AABB);
	}
	Play::UpdateGameObject(paddleObj);
}

bool AABBCollision(const Point2D& aPos, const Vector2D& aAABB, const Point2D& bPos, const Vector2D& bAABB)
{
	return (aPos.x - aAABB.x < bPos.x + bAABB.x
		&& aPos.x + aAABB.x > bPos.x - bAABB.x
		&& aPos.y - aAABB.y < bPos.y + bAABB.y
		&& aPos.y + aAABB.y > bPos.y - bAABB.y);
}

void ApplyReflection(GameObject& objA, const Point2D& aAABB, GameObject& objB, const Point2D& bAABB)
{
	Vector2D collisionEdge = GetNearestEdge(objA.oldPos, aAABB, objB.pos, bAABB);
	Vector2D surfaceNormal = collisionEdge.Perpendicular();
	float dotProduct = objA.velocity.Dot(surfaceNormal);
	Vector2D reflectionVector = objA.velocity - (2.0 * dotProduct * surfaceNormal);
	objA.velocity = -reflectionVector;					// Make ballObj.velocity equal the reflection vector
	objA.pos += objA.velocity;							// Nudge the ballObj.position
	objA.velocity += objA.acceleration;					// Apply any change due to acceleration
}

Vector2D GetNearestEdge(const Point2D& aPos, const Vector2D& aAABB, const Point2D& bPos, const Vector2D& bAABB)
{
	// Logic based method to determine surface vector ( more complete solution would be to use line intersections )
	float left = abs((aPos.x + aAABB.x) - (bPos.x - bAABB.x));
	float right = abs((aPos.x - aAABB.x) - (bPos.x + bAABB.x));
	float top = abs((aPos.y + aAABB.y) - (bPos.y - bAABB.y));
	float bottom = abs((aPos.y - aAABB.y) - (bPos.y + bAABB.y));

	if (left < right && left < top && left < bottom)
	{
		return Vector2D{ -1.0f, 0.0f };
	}
	else if (right < left && right < top && right < bottom)
	{
		return Vector2D{ 1.0f, 0.0f };
	}
	else if (top < bottom && top < left && top < right)
	{
		return Vector2D{ 0.0f, -1.0f };
	}
	else if (bottom < top && bottom < left && bottom < right)
	{
		return Vector2D{ 0.0f, 1.0f };
	}
	else
	{
		return Vector2D{ 0.0f, 0.0f };
	}
}

void CreateBreakoutChests()
{
	const int GAP = round((DISPLAY_WIDTH - (gState.chests.CHEST_SIZE * gState.chests.nCHEST_COLS)) / (gState.chests.nCHEST_COLS + 1));

	for (int j = 0; j < gState.chests.nCHEST_ROWS; ++j)
	{
		for (int i = 0; i < gState.chests.nCHEST_COLS; ++i)
		{
			int x = (i * (gState.chests.CHEST_SIZE + GAP)) + gState.chests.CHEST_SIZE / 2 + GAP;
			int y = (j * (gState.chests.CHEST_SIZE + GAP)) + gState.chests.CHEST_SIZE / 2 + GAP;
			Play::CreateGameObject(TYPE_CHEST, { x, y }, TEMP_RADIUS, "box");
		}
	}
}

void UpdateChests()
{
	GameObject& ballObj = Play::GetGameObjectByType(TYPE_BALL);
	std::vector<int> chestIds{ Play::CollectGameObjectIDsByType(TYPE_CHEST) };

	for (int chestId : chestIds)
	{
		GameObject& chestObj{ Play::GetGameObject(chestId) };

		if (AABBCollision(chestObj.pos, gState.chests.AABB, ballObj.pos, gState.ball.AABB))
		{
			ApplyReflection(ballObj, gState.ball.AABB, chestObj, gState.chests.AABB);
			ItemDrop(chestObj);
			gState.score += gState.chests.SCORE;
			Play::DestroyGameObject(chestObj.GetId());
		}

		Play::UpdateGameObject(chestObj);
	}
}

void ItemDrop(GameObject& chestObj)
{
	Play::CreateGameObject(TYPE_COIN, chestObj.pos, TEMP_RADIUS, "coin");

	std::vector<int> coinIds{ Play::CollectGameObjectIDsByType(TYPE_COIN) };
	for (int coinId : coinIds)
	{
		GameObject& coinObj{ Play::GetGameObject(coinId) };
	}
}

void UpdateCoin()
{
	GameObject& paddleObj{ Play::GetGameObjectByType(TYPE_PADDLE) };

	std::vector<int> coinIds{ Play::CollectGameObjectIDsByType(TYPE_COIN) };
	for (int coinId : coinIds)
	{
		GameObject& coinObj{ Play::GetGameObject(coinId) };
		coinObj.velocity = gState.coins.START_VELOCITY;
		Play::UpdateGameObject(coinObj);

		if (AABBCollision(coinObj.pos, gState.coins.AABB, paddleObj.pos, gState.paddle.AABB))
		{
			gState.score += gState.coins.SCORE;
			Play::DestroyGameObject(coinObj.GetId());
		}
		if (coinObj.pos.y > DISPLAY_HEIGHT + 100)
		{
			Play::DestroyGameObject(coinObj.GetId());
		}
	}
}