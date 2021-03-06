#include "InputManager.h"

// DEFINITIONS //////////////////////////////////////////////////////////////////////////

// define game commands
namespace input
{
	enum GameCommands { Quit, showFPS, Shoot, PlayerMoveForward, PlayerMoveBack, PlayerMoveLeft, PlayerMoveRight, ReverseTime};
}

// CLASSES //////////////////////////////////////////////////////////////////////////////

// the input manager derived from InputManager class
class GameInput : public input::InputManager
{
private:

protected:
	// initialization
	virtual void setDefaultKeyMap() override;
	
public:
	// constructor
	GameInput();
	// destructor
	~GameInput();
};