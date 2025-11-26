#pragma once


// --- Core ---
#include "Application.h"
#include "Actor.h"
#include "Component.h"
#include "MoveComponent.h"
#include "InputSystem.h"
#include "Renderer.h"
#include "SingleInstance.h"
#include "PhysWorld.h"

// --- Move Components ---
#include "FollowMoveComponent.h"
#include "OrbitMoveComponent.h"
#include "InertiaMoveComponent.h"
#include "DirMoveComponent.h"
#include "FPSMoveComponent.h"

// --- Collider & Volume ---
#include "ColliderComponent.h"
#include "BoundingVolumeComponent.h"
#include "LaserColliderComponent.h"
#include "GravityComponent.h"

// --- Visual Components ---
#include "MeshComponent.h"
#include "SkeletalMeshComponent.h"
#include "VisualComponent.h"
#include "SpriteComponent.h"
#include "ShadowSpriteComponent.h"
#include "BillboardComponent.h"
#include "ParticleComponent.h"
#include "WireframeComponent.h"
#include "SkyDomeComponent.h"
#include "WeatherDomeComponent.h"
#include "WeatherOverlayComponent.h"

// --- Utility ---
#include "MathUtils.h"
#include "JsonHelper.h"
#include "Texture.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Font.h"
#include "Polygon.h"
#include "Mesh.h"
#include "Material.h"
#include "SkyDomeMeshGenerator.h"
#include "WeatherManager.h"
#include "AssetManager.h"

// --- Camera / View ---
#include "CameraComponent.h"
#include "FollowCameraComponent.h"
#include "OrbitCameraComponent.h"


// --- Optional system-level ---
#include "ApplicationEntry.h"

// --- Animation ---
#include "AnimationPlayer.h"

// --- Sound ---
#include "SoundEffect.h"
#include "Music.h"
#include "SoundMixer.h"
#include "SoundComponent.h"
