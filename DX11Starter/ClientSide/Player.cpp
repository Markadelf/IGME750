#include "Player.h"
#include "ColliderManager.h"
#include "GameUI.h"
#include "FilePathHelper.h"

Player::Player()
{
	m_reversed = false;
	// initialize the input manager
	PlayerInput = new GameInput();
	m_keyFrameRequested = true;
	m_reportActionUsed = false;
	m_usedAction = false;
}


Player::~Player()
{
	delete PlayerInput;
}

void Player::Initialize(const Transform& startingPos, float initialTime, HandleObject handle, float keyPeriod, int isFirstBullet)
{
	m_transform = startingPos;
	m_time = initialTime;
	m_handle = handle;
	m_keyPeriod = keyPeriod;
	m_keyFrameTimer = 0;
	m_reversed = false;
	m_dead = false;
	m_ctr = isFirstBullet;
}

void Player::Reposition(const Transform& pos, float time)
{
	m_transform = pos;
	m_time = time;
}

void Player::acquireAction()
{

	// act on user input for player actions
	for (auto x : PlayerInput->activeKeyMap)
	{
		switch (x.first)
		{
		case input::GameCommands::Shoot:
			if (m_ctr != 0)
			{
				m_actionUsedTime = m_time;
				m_keyFrameRequested = true;
				m_usedAction = true;
				m_reversed = false;
                static std::string path = FilePathHelper::GetPath("Sounds/Bullet.wav");
				PlayerSound.PlaySounds(path, { (0),(0),(0) }, 0.0f);
				break;
			}
			else
			{
				m_ctr++;
				break;
			}
		case input::GameCommands::ReverseTime:
			m_reversed = !m_reversed;
			m_keyFrameRequested = true;
			break;
		}
	}
}

void Player::acquirePosition(float deltaTime)
{
	Vector2 vel;
	// act on user input for player position
	for (auto x : PlayerInput->activeKeyMap)
	{
		switch (x.first)
		{
		case input::GameCommands::PlayerMoveForward:
			vel = vel + Vector2(0, 1);
			break;
		case input::GameCommands::PlayerMoveBack:
			vel = vel + Vector2(0, -1);
			break;
		case input::GameCommands::PlayerMoveLeft:
			vel = vel + Vector2(-1, 0);
			break;
		case input::GameCommands::PlayerMoveRight:
			vel = vel + Vector2(1, 0);
			break;
		}
	}

	if (vel.m_x != 0 || vel.m_y != 0)
	{
		vel = vel.Normalized();
		vel = vel.Rotate(-m_transform.GetRot());
		m_transform.SetPos(m_transform.GetPos() + vel * deltaTime);
	}
}

void Player::Update(float deltaTime)
{
	PlayerInput->acquireInput();
	m_time += deltaTime * (m_reversed ? -1 : 1);
	UpdatePosition(deltaTime);
	if (!m_usedAction)
	{
		m_keyFrameTimer -= deltaTime;
		if (m_keyFrameTimer <= 0)
		{
			m_keyFrameRequested = true;
		}
		acquireAction();
	}
	else
	{
		if (m_time > m_actionUsedTime + m_action.m_duration)
		{
			m_usedAction = false;
			m_keyFrameRequested = true;
			m_reportActionUsed = true;
		}
	}

	GameUI::Get().UpdateGameUI(m_dead, m_time);
}

Transform Player::GetTransform() const
{
	return m_transform;
}

TimeStamp Player::GetTimeStamp() const
{
	return m_time;
}

bool Player::GetReversed() const
{
	return m_reversed;
}

int Player::GetEntityId() const
{
	return m_entityId;
}

HandleObject Player::GetHandle() const
{
	return m_handle;
}

void Player::SetHandle(HandleObject& obj)
{
	m_handle = obj;
}

void Player::Rotate(float amount)
{
	m_transform.SetRot(m_transform.GetRot() + amount);
}

void Player::SetEntityId(int id)
{
	m_entityId = id;
}

void Player::SetTransform(Transform trans)
{
	m_transform = trans;
}

void Player::SetAction(ActionInfo action)
{
	m_action = action;
}

void Player::SetDead(bool val)
{
	m_dead = val;
}

KeyFrameData Player::GetKeyFrame()
{
	KeyFrameData key = KeyFrameData(m_transform, m_time, m_entityId, m_reportActionUsed);
	m_reportActionUsed = false;
	return key;
}

bool Player::StackRequested()
{
	bool ret = m_usedAction ? m_actionUsedTime == m_time : m_keyFrameTimer <= 0;
	if (ret)
	{
		m_keyFrameTimer = m_keyPeriod;
		m_keyFrameRequested = false;
	}
	return ret;
}

void Player::UpdatePosition(float deltaTime)
{
	acquirePosition(deltaTime);
}