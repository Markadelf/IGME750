#include <cmath>
#include "AssetManager.h"
#include "FilePathHelper.h"
#include "Game.h"
#include "Vertex.h"
#include "GameUI.h"
#include "RequestNetworkStructs.h"
#include "JsonParser.h"

Game* Game::GameInstance;

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance) : m_renderer(hInstance)
{
    GameInstance = this;
    m_renderer.SetDraw(SDraw, SOnResize);
    m_renderer.SetUpdate(SUpdate);
    m_renderer.SetControls(SOnMouseDown, SOnMouseUp, SOnMouseMove, SOnMouseWheel);
    networkConnection = nullptr;
    m_state = GameState::MenuOnly;
    m_mouseLock = false;
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
    // Release any (and all!) DirectX objects
    // we've made in the Game class	
    //AssetManager::get().~AssetManager();
    // Delete our simple shader objects, which
    // will clean up their own internal DirectX stuff
    AssetManager::get().ReleaseAllAssetResource();

    delete clientInterface;
    if (networkConnection != nullptr)
    {
        delete networkConnection;
        networkConnection = nullptr;
    }
    // Release any (and all!) DirectX objects
    // we've made in the Game class	
    //AssetManager::get().~AssetManager();
    // Delete our simple shader objects, which
    // will clean up their own internal DirectX stuff
    AssetManager::get().ReleaseAllAssetResource();

    Sound.UnLoadSound(FilePathHelper::GetPath("Sounds/Bullet.wav"));
    Sound.Shutdown();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{

    // Helper methods for loading shaders, creating some basic
    // geometry to draw and some simple camera matrices.
    //  - You'll be expanding and/or replacing these later
    LoadTextures();
    LoadShaders();
    CreateBasicGeometry();
    InitializeConnection();
    InitEmitters();
    //Initialize the Audio Engine
    Sound.Init();
    Sound.LoadSound(FilePathHelper::GetPath("Sounds/Bullet.wav"), false, false, false);

    LoadUI();
    m_renderer.OnResize();
}

void Game::LoadTextures()
{

    //ID3D11ShaderResourceView* image;
    // Add if successful

    ID3D11Device* device = m_renderer.GetDevice();
    ID3D11DeviceContext* context = m_renderer.GetContext();

    AssetManager::get().LoadTexture(L"Textures/paint_albedo.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/paint_roughness.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/paint_normals.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/wood_albedo.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/wood_roughness.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/wood_normals.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/floor_albedo.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/floor_roughness.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/floor_normals.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/Stone_Wall_1_Texture.jpeg", device, context);
    AssetManager::get().LoadTexture(L"Textures/Stone_Wall_1_Bump_Map.jpeg", device, context);
    AssetManager::get().LoadTexture(L"Textures/Stone_Wall_1_Normal_Map.jpeg", device, context);
    AssetManager::get().LoadTexture(L"Textures/particle.jpg", device, context);
    AssetManager::get().LoadTexture(L"Textures/particle2.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/particle4.png", device, context);
    AssetManager::get().LoadTexture(L"Textures/Particle5.png", device, context);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadShaders()
{
    ID3D11Device* device = m_renderer.GetDevice();
    ID3D11DeviceContext* context = m_renderer.GetContext();

    //For now shinniness is being handled in Assetmanager.Will move it to the material once we have everythin up and running with latest renderer.
    AssetManager::get().LoadMaterial(0, 0, "PLAYER3", "Textures/paint_albedo.png", "Textures/paint_roughness.png", "Textures/paint_normals.png");
    AssetManager::get().LoadMaterial(0, 0, "WOOD", "Textures/wood_albedo.png", "Textures/wood_roughness.png", "Textures/wood_normals.png");
    AssetManager::get().LoadMaterial(0, 0, "FLOOR", "Textures/floor_albedo.png", "Textures/floor_roughness.png", "Textures/floor_normals.png");
    AssetManager::get().LoadMaterial(0, 0, "WALL", "Textures/Stone_Wall_1_Texture.jpeg", "Textures/Stone_Wall_1_Bump_Map.jpeg", "Textures/Stone_Wall_1_Normal_Map.jpeg");
}

// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

    ID3D11Device* device = m_renderer.GetDevice();
    ID3D11DeviceContext* context = m_renderer.GetContext();
    // Load in the files and get the handles for each from the meshManager
    int coneHandle = AssetManager::get().LoadMesh("OBJ_Files/8Ball.fbx", device);
	int cubeHandle = AssetManager::get().LoadMesh("OBJ_Files/cube.obj", device);
    int cylinderHandle = AssetManager::get().LoadMesh("OBJ_Files/cylinder.obj", device);
    int sphereHandle = AssetManager::get().LoadMesh("OBJ_Files/sphere.obj", device);
    int duckHandle = AssetManager::get().LoadMesh("OBJ_Files/duck.fbx", device);
    int planeHandle = AssetManager::get().LoadMesh("OBJ_Files/Plane.obj", device);
	//int BulletHandle = AssetManager::get().LoadMesh("OBJ_Files/Bullet.obj", device);

    clientInterface = new ClientManager();
}

void Game::InitializeConnection()
{
    // Local host is 127.0.0.1
    // 129.21.29.156
    //Address serverAddress(127, 0, 0, 1, 30000);
    int ipAddr[4];
    JsonParser ipParser(FilePathHelper::GetPath(std::string("config.json")).c_str());
    ipParser.GetIpAddr(ipAddr);
    Address serverAddress(ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], 30000);
    networkConnection = new ClientHelper(30001, serverAddress);
    networkConnection->SetActiveCallBack(SUserCallback);
    networkConnection->SetClientCallBack(SClientCallback);
}

void Game::LoadUI()
{
    UIManager::get().SetContext(m_renderer.GetContext());

    GameUI::Get().GivePointers(m_renderer.GetDevice(), m_renderer.GetContext());
    GameUI::Get().InitializeUI();
}

void Game::JoinGame()
{
    networkConnection->ResetAcks();

    Buffer* buff = networkConnection->GetNextBuffer(MessageType::GameRequest);
    ClientRequest joinRequest;
    joinRequest.m_request = ClientRequestType::Join;
    joinRequest.Serialize(*buff);
    networkConnection->SendToServer();

}

void Game::InitEmitters()
{
    ID3D11Device* device = m_renderer.GetDevice();
    ID3D11DeviceContext* context = m_renderer.GetContext();
    //	AssetManager::get().GetTextureHandle("Textures/particle.jpg"));			// Texture Handle
        // Set up particles
    AssetManager::get().LoadEmitter("Emitter1",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(1, 0.2f, 0.2f, 0.2f),	// Start color
        XMFLOAT4(1, 0.3f, 0.3f, 0.3f),	// End color
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),         // Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle2.png"));

    // Set up particles
    AssetManager::get().LoadEmitter("Emitter4",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(1, 0.6f, 0.1f, .9f),	// Start color
        XMFLOAT4(1, 0.8f, 0.5f, 0.8f),		// End color
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),			// Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle2.png"));


    // Set up particles
    AssetManager::get().LoadEmitter("Emitter2",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(0, 0.8f, 0.1f, 0.7f),											// Start color
        XMFLOAT4(0, 1.f, 0.1f, 0),			// End color
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),         // Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle4.png"));

    // Set up particles
    AssetManager::get().LoadEmitter("Emitter5",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(1, 0.6f, 0.8f, .9f),	// Start color
        XMFLOAT4(1, 0.2f, 0.5f, 0.8f),		// End color
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),			// Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle2.png"));

    AssetManager::get().LoadEmitter("Emitter3",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(.5f, 0.1f, 1.0f, 0.7f),											// Start color
        XMFLOAT4(1, 0.6f, .8f, 0),
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),         // Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle4.png"));

    // Set up particles
    AssetManager::get().LoadEmitter("Emitter6",
        110,							// Max particles
        80,								// Particles per second
        0.5,							// Particle lifetime
        0.08f,							// Start size
        0.1f,							// End size
        XMFLOAT4(1, 0.1f, 0.1f, .9f),	// Start color
        XMFLOAT4(1, 0.2f, 0.5f, 0.8f),		// End color
        XMFLOAT3(0, 0, 0),				// Start velocity
        XMFLOAT3(1, 1, 1),		        // Velocity randomness range
        XMFLOAT3(0, 0, 0),		       	// Emitter position
        XMFLOAT3(0.1f, 0.1f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),			// Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/particle2.png"));

    AssetManager::get().LoadEmitter("Emitter7",
        110,							// Max particles
        10,								// Particles per second
        5.0,							// Particle lifetime
        1.0f,							// Start size
        3.0f,							// End size
        XMFLOAT4(1, 0.2f, 0.2f, 0.2f),	// Start color
        XMFLOAT4(1, 0.8f, 0.3f, 0.3f),	// End color
        XMFLOAT3(0, 1, 0),				// Start velocity
        XMFLOAT3(0, 1, 0),		        // Velocity randomness range
        XMFLOAT3(0.1f, 0.1f, 0),		       	// Emitter position
        XMFLOAT3(20.0f, 20.0f, 0.1f),		// Position randomness range
        XMFLOAT4(-2, 2, -2, 2),         // Random rotation ranges (startMin, startMax, endMin, endMax)
        XMFLOAT3(0, -1, 0),				// Constant acceleration
        m_renderer.GetDevice(),
        AssetManager::get().GetTextureHandle("Textures/Particle5.png"));

}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
    // Quit if the escape key is pressed
    if (GetAsyncKeyState(VK_ESCAPE))
    {
        if (m_state == GameState::InGame)
        {
            m_mouseLock = false;
            ShowCursor(true);
        }
        else
        {
            m_renderer.Quit();
        }
    }
    if (networkConnection != nullptr)
    {
        networkConnection->Listen();
    }
    if (m_state == GameState::InGame)
    {
        clientInterface->Update(deltaTime);
    }
}

void Game::SUpdate(float deltaTime, float totalTime)
{
    GameInstance->Update(deltaTime, totalTime);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
    m_renderer.Begin();

    if (m_state == GameState::InGame)
    {
        m_renderer.RenderGroup(clientInterface->GetDrawGroup());
    }
    UIManager::get().Render();

    m_renderer.End();
}

void Game::OnResize(int width, int height)
{
    clientInterface->GetDrawGroup().m_camera.SetAspectRatio((float)width / height);
    UIManager::get().SetScreenDimensions(width, height);
}


#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
    if (m_state == GameState::InGame && !m_mouseLock)
    {
        ShowCursor(false);
        m_mouseLock = true;
    }
    // Add any custom code here...
    RECT rect;
    GetClientRect(m_renderer.GethWnd(), &rect);
    POINT ul;
    ul.x = rect.left;
    ul.y = rect.top;

    POINT lr;
    lr.x = rect.right;
    lr.y = rect.bottom;

    MapWindowPoints(m_renderer.GethWnd(), nullptr, &ul, 1);
    MapWindowPoints(m_renderer.GethWnd(), nullptr, &lr, 1);

    rect.left = ul.x;
    rect.top = ul.y;

    rect.right = lr.x;
    rect.bottom = lr.y;
    // Save the previous mouse position, so we have it for the future
    prevMousePos.x = x;
    prevMousePos.y = y;

    // Caputure the mouse so we keep getting mouse move
    // events even if the mouse leaves the window.  we'll be
    // releasing the capture once a mouse button is released
    SetCapture(m_renderer.GethWnd());
    ClipCursor(&rect);
    UIManager::get().OnClick(x, y);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
    // Add any custom code here...

    // We don't care about the tracking the cursor outside
    // the window anymore (we're not dragging if the mouse is up)
    ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
    RECT rcClient;
    POINT ptDiff;
    GetWindowRect(m_renderer.GethWnd(), &rcClient);
    ptDiff.x = (rcClient.right - rcClient.left)/2;
    ptDiff.y = (rcClient.bottom - rcClient.top);



    int dY = (y - prevMousePos.y);
	int dX = (x - (int)(ptDiff.x / 2));

    //dX = dX % 10;
    dY = dY % 10;


    // Save the previous mouse position, so we have it for the future
    prevMousePos.x = x;
    prevMousePos.y = y;

    if (m_state == GameState::InGame && m_mouseLock)
    {
		//GetCursorPos();
        clientInterface->GetPlayer().Rotate(dX / 1800.f);
        SetCursorPos((int)(ptDiff.x / 2), (int)(ptDiff.y / 2));
    }
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
    // Add any custom code here...
}
Renderer* Game::GetRenderer()
{
    return &m_renderer;
}
#pragma endregion

#pragma region Static Callbacks
void Game::SDraw(float deltaTime, float totalTime)
{
    GameInstance->Draw(deltaTime, totalTime);
}

void Game::SOnResize(int width, int height)
{
    GameInstance->OnResize(width, height);
}

void Game::SOnMouseDown(WPARAM buttonState, int x, int y)
{
    GameInstance->OnMouseDown(buttonState, x, y);
}

void Game::SOnMouseUp(WPARAM buttonState, int x, int y)
{
    GameInstance->OnMouseUp(buttonState, x, y);
}

void Game::SOnMouseMove(WPARAM buttonState, int x, int y)
{
    GameInstance->OnMouseMove(buttonState, x, y);
}

void Game::SOnMouseWheel(float wheelDelta, int x, int y)
{
    GameInstance->OnMouseWheel(wheelDelta, x, y);
}

// Network callbacks
void Game::SClientCallback(Buffer& bitBuffer)
{
    HostRequest request;
    request.Deserialize(bitBuffer);

    switch (request.m_request)
    {
    case HostRequestType::Prepare:
        GameInstance->clientInterface->SetNetworkPointer(GameInstance->networkConnection);
        GameInstance->clientInterface->Init(request.m_arg);
        UpdateGameState(GameState::InGame);
        break;
    case HostRequestType::DeclareVictor:
        GameUI::Get().ExitToResults(request.m_arg == GameInstance->clientInterface->GetPlayer().GetEntityId() ? 0 : 1);
        GameInstance->networkConnection->ResetAcks();
        break;
    default:

        break;
    }
}

void Game::SUserCallback(Buffer& bitBuffer)
{
    GameInstance->clientInterface->RecieveData(bitBuffer);
}

void Game::StartGameOffline() {
    GameInstance->clientInterface->SetNetworkPointer(nullptr);
    GameInstance->clientInterface->Init(0);
    UpdateGameState(GameState::InGame);
}

// State Machine
void Game::UpdateGameState(GameState arg)
{
    ID3D11Device* device = GameInstance->m_renderer.GetDevice();
    switch (arg)
    {
    case GameState::InGame:
        GameUI::Get().DisplayHUD();
        GameInstance->m_mouseLock = true;
        ShowCursor(false);
        break;
    case GameState::WaitingForNetwork:
        GameInstance->JoinGame();
        break;
    default:
        ShowCursor(true);
        break;
    }
    GameInstance->m_state = arg;
}
#pragma endregion