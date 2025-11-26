#pragma once

#include "MathUtils.h"
#include <string>

// テクスチャを管理
class Texture
{
public:
	Texture();
	~Texture();
	
    // テクスチャ読み込み（SDL2_image）
	bool Load(const std::string& fileName, class AssetManager* assetManager);
    // 埋め込みテクスチャ読み込み
    bool LoadFromMemory(const void* data, int size);
    bool LoadFromMemory(const void* data, int width, int height);

    void CreateForRendering(int w, int h, unsigned int format);
    bool CreateAlphaCircle(int size, float centerX, float centerY, Vector3 color, float blendPow = 1.0f);
    bool CreateRadialRays(int size, int numRays, float fadePow, float rayStrength, float intensityScale);
    
    // 削除
	void Unload();
    // 描画用にBind
	void SetActive(int unit);
	// 幅と高さ取得
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }

    
    // シャドウマップ用テクスチャ生成
    void CreateShadowMap(int width, int height);
    unsigned int GetTextureID() const { return mTextureID; }
    
private:
    // OpenGLが管理するID
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
    
};


