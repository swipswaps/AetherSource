#pragma once

#include <string>
#include <lua.hpp>

#include "DirectXWrapper.h"
#include "../robot/Source/Robot.h"

class CLuaWrapper
{
protected:
	void UpdateKeyboard();

	int LuaGetWidth(lua_State *L);
	int LuaGetHeight(lua_State *L);
	int LuaGetFPS(lua_State *L);
	int LuaSetResolution(lua_State *L);
	int LuaGetDisplayWidth(lua_State *L);
	int LuaGetDisplayHeight(lua_State *L);
	int LuaGetDisplayFrequency(lua_State *L);
	int LuaLoadTexture(lua_State *L);
	int LuaClearScreen(lua_State *L);
	int LuaReleaseTexture(lua_State *L);
	int LuaDrawSprite(lua_State *L);
	int LuaLoadFont(lua_State *L);
	int LuaReleaseFont(lua_State *L);
	int LuaDrawText(lua_State *L);
	int LuaLoadSound(lua_State *L);
	int LuaPlaySound(lua_State *L);

	int m_iFPS;

	std::string m_strPreviousRenderCall;

	lua_State *m_pLuaState;

	Robot::KeyState m_keyState;

	CDirectXWrapper *m_pDirectXWrapper; 
public:
	CLuaWrapper();
	~CLuaWrapper();

	bool IsOpen();

	bool Close();
	bool Open(const char *fn);

	int GetWidth() { return m_pDirectXWrapper->GetWidth(); }
	int GetHeight() { return m_pDirectXWrapper->GetHeight(); }
	void SetResolution(int iWidth, int iHeight) { m_pDirectXWrapper->SetResolution(iWidth, iHeight); }
	int GetFPS() { return m_iFPS; }
	void SetFPS(int iFPS);
	
	HBITMAP Capture() { return m_pDirectXWrapper->Capture(); }

	void OnDestroy();
	void OnUpdate(double deltaTime);
	void OnRender(double deltaTime);
};
