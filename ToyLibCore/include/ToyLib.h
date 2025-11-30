#pragma once
//
// ToyLib.h
//  - ゲーム側から ToyLib を利用するための統合ヘッダー
//  - 基本的にはこのファイルだけをインクルードすればOK
//
//  ※エンジン内部（ToyLibCore 側）は個別ヘッダを直接インクルードする想定
//

//======================================
// Engine Core
//======================================
#include "Engine/Core/Application.h"
#include "Engine/Core/ApplicationEntry.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Component.h"

//======================================
// Engine Runtime
//======================================
#include "Engine/Runtime/InputSystem.h"
#include "Engine/Runtime/AnimationPlayer.h"
#include "Engine/Runtime/TimeOfDaySystem.h"
#include "Engine/Runtime/SingleInstance.h"

//======================================
// Engine Render
//======================================
#include "Engine/Render/Renderer.h"
#include "Engine/Render/Shader.h"
#include "Engine/Render/LightingManager.h"

//======================================
// Asset
//======================================

// Asset Manager
#include "Asset/AssetManager.h"

// --- Animation Assets ---
#include "Asset/Animation/AnimationClip.h"

// --- Audio Assets ---
#include "Asset/Audio/Music.h"
#include "Asset/Audio/SoundEffect.h"

// --- Font Assets ---
#include "Asset/Font/TextFont.h"

// --- Geometry Assets ---
#include "Asset/Geometry/Bone.h"
#include "Asset/Geometry/Mesh.h"
#include "Asset/Geometry/Polygon.h"
#include "Asset/Geometry/VertexArray.h"

// --- Material Assets ---
#include "Asset/Material/Material.h"
#include "Asset/Material/Texture.h"

//======================================
// Camera
//======================================
#include "Camera/CameraComponent.h"
#include "Camera/FollowCameraComponent.h"
#include "Camera/OrbitCameraComponent.h"

//======================================
// Graphics (Visual Components)
//======================================

// 基底コンポーネント
#include "Graphics/VisualComponent.h"

// --- Mesh 系 ---
#include "Graphics/Mesh/MeshComponent.h"
#include "Graphics/Mesh/SkeletalMeshComponent.h"

// --- Sprite / Billboard 系 ---
#include "Graphics/Sprite/SpriteComponent.h"
#include "Graphics/Sprite/BillboardComponent.h"
#include "Graphics/Sprite/TextSpriteComponent.h"

// --- Effect 系 ---
#include "Graphics/Effect/ParticleComponent.h"
#include "Graphics/Effect/ShadowSpriteComponent.h"
#include "Graphics/Effect/WireframeComponent.h"

//======================================
// Environment (Sky, Weather)
//======================================
#include "Environment/SkyDomeComponent.h"
#include "Environment/SkyDomeMeshGenerator.h"
#include "Environment/WeatherDomeComponent.h"
#include "Environment/WeatherManager.h"
#include "Environment/WeatherOverlayComponent.h"

//======================================
// Movement
//======================================
#include "Movement/MoveComponent.h"
#include "Movement/DirMoveComponent.h"
#include "Movement/FollowMoveComponent.h"
#include "Movement/FPSMoveComponent.h"
#include "Movement/InertiaMoveComponent.h"
#include "Movement/OrbitMoveComponent.h"

//======================================
// Physics
//======================================
#include "Physics/BoundingVolumeComponent.h"
#include "Physics/ColliderComponent.h"
#include "Physics/GravityComponent.h"
#include "Physics/LaserColliderComponent.h"
#include "Physics/PhysWorld.h"

//======================================
// Audio (再生系)
//======================================
#include "Audio/SoundComponent.h"
#include "Audio/SoundMixer.h"

//======================================
// Utils
//======================================
#include "Utils/MathUtil.h"
#include "Utils/JsonHelper.h"
#include "Utils/StringUtil.h"
#include "Utils/IMEUtil.h"

//
// 必要ならここに using namespace ToyLib; を置く選択肢もあるけど、
// 名前衝突を避けるために各ヘッダ側で namespace を切っている前提。
// （ゲーム側で明示的に using する方が安全）
//
/*
#pragma once


// --- Core ---
#include "Engine/Core/Application.h"
#include "Engine/Core/Actor.h"
#include "Engine/Core/Component.h"
#include "Movement/MoveComponent.h"
#include "Engine/Runtime/InputSystem.h"
#include "Engine/Render/Renderer.h"
#include "Engine/Runtime/SingleInstance.h"
#include "Physics/PhysWorld.h"
#include "Engine/Runtime/TimeOfDaySystem.h"

// --- Move Components ---
#include "Movement/FollowMoveComponent.h"
#include "Movement/OrbitMoveComponent.h"
#include "Movement/InertiaMoveComponent.h"
#include "Movement/DirMoveComponent.h"
#include "Movement/FPSMoveComponent.h"

// --- Collider & Volume ---
#include "Physics/ColliderComponent.h"
#include "Physics/BoundingVolumeComponent.h"
#include "Physics/LaserColliderComponent.h"
#include "Physics/GravityComponent.h"

// --- Visual Components ---
#include "Graphics/Mesh/MeshComponent.h"
#include "Graphics/Mesh/SkeletalMeshComponent.h"
#include "Graphics/VisualComponent.h"
#include "Graphics/Sprite/SpriteComponent.h"
#include "Graphics/Effect/ShadowSpriteComponent.h"
#include "Graphics/Sprite/BillboardComponent.h"
#include "Graphics/Effect/ParticleComponent.h"
#include "Graphics/Effect/WireframeComponent.h"
#include "Environment/SkyDomeComponent.h"
#include "Environment/WeatherDomeComponent.h"
#include "Environment/WeatherOverlayComponent.h"
#include "Graphics/Sprite/TextSpriteComponent.h"

// --- Utility ---
#include "Utils/MathUtil.h"
#include "Utils/JsonHelper.h"
#include "Asset/Material/Texture.h"
#include "Engine/Render/Shader.h"
#include "Asset/Geometry/VertexArray.h"
#include "Asset/Geometry/Polygon.h"
#include "Asset/Geometry/Mesh.h"
#include "Asset/Material/Material.h"
#include "Environment/SkyDomeMeshGenerator.h"
#include "Environment/WeatherManager.h"
#include "Asset/AssetManager.h"
#include "Asset/Font/TextFont.h"

// --- Camera / View ---
#include "Camera/CameraComponent.h"
#include "Camera/FollowCameraComponent.h"
#include "Camera/OrbitCameraComponent.h"


// --- Optional system-level ---
#include "Engine/Core/ApplicationEntry.h"

// --- Animation ---
#include "Engine/Runtime/AnimationPlayer.h"

// --- Sound ---
#include "Asset/Audio/SoundEffect.h"
#include "Asset/Audio/Music.h"
#include "Audio/SoundMixer.h"
#include "Audio/SoundComponent.h"
*/
