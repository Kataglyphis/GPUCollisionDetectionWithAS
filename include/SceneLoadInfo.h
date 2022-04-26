#pragma once


struct SceneLoadInfo
{
	const char* objPath;
	const char* mtlBaseDir;
	const char* texturePath;
	const uint32_t textureSize;
	const float scale;
};

static const SceneLoadInfo sunTemple = {
    "../Resources/Models/SunTemple/Model/model.obj",
    "../Resources/Models/SunTemple/Model",
    "../Resources/Models/SunTemple/Textures",
    2048,
    0.1,
    
};

static const SceneLoadInfo vikingRoom = {
    "../Resources/Models/VikingRoom/Model/model.obj",
    "../Resources/Models/VikingRoom/Model",
    "../Resources/Models/VikingRoom/Textures",
    1024,
    0.01,
};

static const SceneLoadInfo cruiser = {
    "../Resources/Models/Cruiser/Model/cruiser.obj",
    "../Resources/Models/Cruiser/Model",
    "../Resources/Models/Cruiser/Textures",
    1024,
    0.04,
};

static const SceneLoadInfo cube = {
    "../Resources/Models/Cube/Model/model.obj",
    "../Resources/Models/Cube/Model",
    "../Resources/Models/Cube/Textures",
    1024,
    0.04,
};

static const SceneLoadInfo spot = {
    "../Resources/Models/Spot/Model/model.obj",
    "../Resources/Models/Spot/Model",
    "../Resources/Models/Cube/Textures",
    1024,
    0.04,
};