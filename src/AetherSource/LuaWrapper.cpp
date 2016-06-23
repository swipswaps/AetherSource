#include <algorithm>
#define NOMINMAX

//#include <functional>
#include "LuaWrapper.h"
#include "StringHelper.h"
#include "../robot/Source/Robot.h"

#define MINIMUM_FPS 6

// Wrap the lua function wrapper into a namespace for disambiguation and future-proofing in case we add more lua classes.
// TODO: Make a version of this that wraps lambdas and use those instead of class member methods.
namespace NLuaWrapper
{
	typedef int (CLuaWrapper::*signature)(lua_State *L);

	// This template wraps a member function into a C-style "free" function compatible with lua.
	template <signature function>

	int dispatch(lua_State *L)
	{
		CLuaWrapper *pointer = *static_cast<CLuaWrapper**>(lua_getextraspace(L));

		return ((*pointer).*function)(L);
	}
}

CLuaWrapper::CLuaWrapper() :
	m_iFPS(MINIMUM_FPS),
	m_pLuaState(NULL)
{
	m_pDirectXWrapper = new CDirectXWrapper();
}

CLuaWrapper::~CLuaWrapper()
{
	Close();

	delete m_pDirectXWrapper;
}

bool CLuaWrapper::IsOpen()
{
	return (m_pLuaState != NULL);
}

bool CLuaWrapper::Close()
{
	if (IsOpen())
	{
		lua_close(m_pLuaState);
		m_pLuaState = NULL;

		return true;
	}

	return false;
}

bool CLuaWrapper::Open(const char *fn)
{
	Close();

	m_pLuaState = luaL_newstate();
	luaL_openlibs(m_pLuaState);

	// TODO: use std::bind to register lua functions and remove the wrapper.
	//lua_pushcclosure(m_pLuaState, std::bind(&CLuaWrapper::LuaGetDisplayWidth, this, std::placeholders::_1), 0);
	//lua_setglobal(m_pLuaState, "GetDisplayWidth");

	*static_cast<CLuaWrapper**>(lua_getextraspace(m_pLuaState)) = this;
	
	lua_register(m_pLuaState, "GetDisplayWidth", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetDisplayWidth>);
	lua_register(m_pLuaState, "GetDisplayHeight", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetDisplayHeight>);
	lua_register(m_pLuaState, "GetDisplayFrequency", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetDisplayFrequency>);
	lua_register(m_pLuaState, "SetResolution", &NLuaWrapper::dispatch<&CLuaWrapper::LuaSetResolution>);
	lua_register(m_pLuaState, "GetWidth", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetWidth>);
	lua_register(m_pLuaState, "GetHeight", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetHeight>);
	lua_register(m_pLuaState, "GetFPS", &NLuaWrapper::dispatch<&CLuaWrapper::LuaGetFPS>);
	lua_register(m_pLuaState, "LoadImage", &NLuaWrapper::dispatch<&CLuaWrapper::LuaLoadImage>);
	lua_register(m_pLuaState, "ReleaseImage", &NLuaWrapper::dispatch<&CLuaWrapper::LuaReleaseImage>);
	lua_register(m_pLuaState, "DrawSprite", &NLuaWrapper::dispatch<&CLuaWrapper::LuaDrawSprite>);
	lua_register(m_pLuaState, "LoadFont", &NLuaWrapper::dispatch<&CLuaWrapper::LuaLoadFont>);
	lua_register(m_pLuaState, "ReleaseFont", &NLuaWrapper::dispatch<&CLuaWrapper::LuaReleaseFont>);
	lua_register(m_pLuaState, "DrawText", &NLuaWrapper::dispatch<&CLuaWrapper::LuaDrawText>);
	lua_register(m_pLuaState, "LoadSound", &NLuaWrapper::dispatch<&CLuaWrapper::LuaLoadSound>);
	lua_register(m_pLuaState, "PlaySound", &NLuaWrapper::dispatch<&CLuaWrapper::LuaPlaySound>);

	if (luaL_dofile(m_pLuaState, fn) != LUA_OK)
	{
		Close();

		return false;
	}
	
	return true;
}

void CLuaWrapper::SetFPS(int iFPS)
{
	m_iFPS = std::max(iFPS, MINIMUM_FPS);
}

void CLuaWrapper::OnDestroy()
{
	if (!IsOpen()) return;

	lua_getglobal(m_pLuaState, "OnDestroy");

	lua_call(m_pLuaState, 0, 0);
}

void CLuaWrapper::OnUpdate(double deltaTime)
{
	if (!IsOpen()) return;

	UpdateKeyboard();

	lua_getglobal(m_pLuaState, "OnUpdate");
	lua_pushnumber(m_pLuaState, deltaTime);
	lua_call(m_pLuaState, 1, 0);
}

void CLuaWrapper::OnRender(double deltaTime)
{
	if (!IsOpen()) return;

	// Clear render target
	m_pDirectXWrapper->Clear();

	// Execute draw commands from Lua
	lua_getglobal(m_pLuaState, "OnRender");
	lua_pushnumber(m_pLuaState, deltaTime);
	lua_call(m_pLuaState, 1, 0);

	// Check if sprite batch is still active
	if (m_previousRenderCall.find("DrawSprite") != std::string::npos)
	{
		// End sprite batch
		m_pDirectXWrapper->EndSpriteBatch();
	}

	// Render to texture
	m_pDirectXWrapper->Render();

	// Reset tracked render call
	m_previousRenderCall = "";
}

void CLuaWrapper::UpdateKeyboard()
{
	static Robot::KeyState s;
	Robot::Keyboard::GetState(s);

	lua_newtable(m_pLuaState);

	lua_pushliteral(m_pLuaState, "Shift");
	lua_pushboolean(m_pLuaState, s[Robot::Key::KeyShift]);
	lua_settable(m_pLuaState, -3);

	lua_pushliteral(m_pLuaState, "Control");
	lua_pushboolean(m_pLuaState, s[Robot::Key::KeyControl]);
	lua_settable(m_pLuaState, -3);

	lua_pushliteral(m_pLuaState, "Alt");
	lua_pushboolean(m_pLuaState, s[Robot::Key::KeyAlt]);
	lua_settable(m_pLuaState, -3);

	lua_pushliteral(m_pLuaState, "System");
	lua_pushboolean(m_pLuaState, s[Robot::Key::KeySystem]);
	lua_settable(m_pLuaState, -3);

	lua_setglobal(m_pLuaState, "Keyboard");
}

int CLuaWrapper::LuaGetWidth(lua_State *L)
{
	lua_pushinteger(L, GetWidth());

	return 1;
}

int CLuaWrapper::LuaGetHeight(lua_State *L)
{
	lua_pushinteger(L, GetHeight());

	return 1;
}

int CLuaWrapper::LuaGetFPS(lua_State *L)
{
	lua_pushinteger(L, GetFPS());

	return 1;
}

// TODO: Change these to accept a table with the arguments instead

int CLuaWrapper::LuaSetResolution(lua_State *L)
{
	SetResolution((int)luaL_checkinteger(L, 1), (int)luaL_checkinteger(L, 2));
	SetFPS((int)luaL_checkinteger(L, 3));

	return 0;
}

int CLuaWrapper::LuaGetDisplayWidth(lua_State *L)
{
	HDC hDC = GetDC(NULL);
	int width = GetDeviceCaps(hDC, HORZRES);
	ReleaseDC(NULL, hDC);

	lua_pushinteger(L, width);

	return 1;
}

int CLuaWrapper::LuaGetDisplayHeight(lua_State *L)
{
	HDC hDC = GetDC(NULL);
	int height = GetDeviceCaps(hDC, VERTRES);
	ReleaseDC(NULL, hDC);

	lua_pushinteger(L, height);

	return 1;
}

int CLuaWrapper::LuaGetDisplayFrequency(lua_State *L)
{
	HDC hDC = GetDC(NULL);
	int frequency = GetDeviceCaps(hDC, VREFRESH);
	ReleaseDC(NULL, hDC);

	lua_pushinteger(L, frequency);

	return 1;
}

int CLuaWrapper::LuaLoadImage(lua_State *L)
{
	const char *filePath = luaL_checkstring(L, 1);

	DX::Image image = m_pDirectXWrapper->LoadImage(to_wstring(filePath).c_str());

	lua_pushlightuserdata(L, image);

	return 1;
}

int CLuaWrapper::LuaReleaseImage(lua_State * L)
{
	DX::Image image = (DX::Image)lua_touserdata(L, 1);

	m_pDirectXWrapper->ReleaseImage(image);

	return 0;
}

// TODO: Change this to accept a table with arguments
int CLuaWrapper::LuaDrawSprite(lua_State *L)
{
	if (m_previousRenderCall != __FUNCTION__)
	{
		// Begin sprite batch
	    m_pDirectXWrapper->BeginSpriteBatch();
	}

	lua_settop(L, 1);
	luaL_checktype(L, 1, LUA_TTABLE);

	// Now to get the data out of the table
	// 'unpack' the table by putting the values onto
	// the stack first. Then convert those stack values
	// into an appropriate C type.
	lua_getfield(L, 1, "image");
	lua_getfield(L, 1, "x");
	lua_getfield(L, 1, "y");

	// Get arguments
	DX::Image image = (DX::Image)lua_touserdata(L, -3);
	double xPosition = luaL_checknumber(L, -2);
	double yPosition = luaL_checknumber(L, -1);
	
	// Draw sprite
    m_pDirectXWrapper->DrawSprite(image, float(xPosition), float(yPosition));

	//
	lua_pop(L, 3);

	//
	m_previousRenderCall = __FUNCTION__;

	return 0;
}

int CLuaWrapper::LuaLoadFont(lua_State * L)
{
	const char *fontFamily = luaL_checkstring(L, 1);

	DX::Font font = m_pDirectXWrapper->LoadFont(to_wstring(fontFamily).c_str());

	lua_pushlightuserdata(L, font);

	return 1;
}

int CLuaWrapper::LuaReleaseFont(lua_State * L)
{
	DX::Font font = (DX::Font)lua_touserdata(L, 1);

	m_pDirectXWrapper->ReleaseFont(font);

	return 0;
}

// TODO: Change this to accept a table with arguments
int CLuaWrapper::LuaDrawText(lua_State * L)
{
	if (m_previousRenderCall.find("DrawSprite") != std::string::npos)
	{
		// End previous sprite batch
		m_pDirectXWrapper->EndSpriteBatch();
	}

	lua_settop(L, 1);
	luaL_checktype(L, 1, LUA_TTABLE);

	// Now to get the data out of the table
	// 'unpack' the table by putting the values onto
	// the stack first. Then convert those stack values
	// into an appropriate C type.
	lua_getfield(L, 1, "text");
	lua_getfield(L, 1, "font");
	lua_getfield(L, 1, "size");
	lua_getfield(L, 1, "x");
	lua_getfield(L, 1, "y");
	lua_getfield(L, 1, "color");

	// Get arguments
	const char *text = luaL_checkstring(L, -6);
	DX::Font font = (DX::Font)lua_touserdata(L, -5);
	double size = luaL_checknumber(L, -4);
	double x = luaL_checknumber(L, -3);
	double y = luaL_checknumber(L, -2);
	long long color = luaL_optinteger(L, -1, 0xFFFFFFFF);

	m_pDirectXWrapper->DrawText(to_wstring(text).c_str(), font, float(size), float(x), float(y), UINT32(color));

	lua_pop(L, 6);

	m_previousRenderCall = __FUNCTION__;

	return 0;
}

int CLuaWrapper::LuaLoadSound(lua_State *L)
{
	return 0;
}

int CLuaWrapper::LuaPlaySound(lua_State *L)
{
	return 0;
}
