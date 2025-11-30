#pragma once

#include "Utils/MathUtil.h"
#include "Asset/Animation/AnimationClip.h"

#include <vector>
#include <string>
#include <map>
#include <memory>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>

// アニメーション対応のメッシュを管理するクラス
class Mesh
{
public:
    Mesh();
    ~Mesh();

    // メッシュファイルを読み込む
    virtual bool Load(const std::string& fileName, class AssetManager* assetMamager, bool isRightHanded = false);
    // メッシュとリソースの開放
    void Unload();

    // メッシュの頂点配列（VAO）を取得
    const std::vector<std::shared_ptr<class VertexArray>>& GetVertexArray() { return mVertexArray; }

    // マテリアルを取得
    std::shared_ptr<class Material> GetMaterial(size_t index);

    // 使用するシェーダー名を取得
    const std::string& GetShaderName() const { return mShaderName; }
    // スペキュラー強度を取得
    float GetSpecPower() const { return mSpecPower; }

    // Assimpで読み込まれたシーンへのポインタ
    const aiScene* GetScene() const { return mScene; }

    // 指定時刻における姿勢行列（各ボーン）を計算する
    void ComputePoseAtTime(float animationTime, const aiAnimation* pAnimation, std::vector<Matrix4>& outTransforms);

    // 読み込まれたアニメーションクリップを取得
    const std::vector<struct AnimationClip>& GetAnimationClips() const { return mAnimationClips; }
    // アニメーションが存在するかどうか
    bool HasAnimation() const { return !mAnimationClips.empty(); }
    
private:
    // メッシュデータの読み込み（ボーンの有無を判断）
    void LoadMeshData();
    // マテリアルの読み込み
    void LoadMaterials(class AssetManager* assetMamager);
    // アニメーションクリップの読み込み
    void LoadAnimations();

    // 通常メッシュの生成（ボーンなし）
    void CreateMesh(const aiMesh* m);
    // スキンメッシュの生成（ボーンあり）
    void CreateMeshBone(const aiMesh* m);
    // ボーン情報の読み込み
    void LoadBones(const aiMesh* m, std::vector<struct VertexBoneData>& bones);

    // ボーン階層を再帰的に計算
    void ComputeBoneHierarchy(float animationTime, const aiNode* pNode, const Matrix4& parentTransform, const aiAnimation* pAnimation);
    // 指定ノード名に一致するアニメーションチャネルを検索
    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& nodeName);

    // 補間計算：スケール / 回転 / 平行移動
    void CalcInterpolatedScaling(Vector3& outVec, float animationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(Quaternion& outVec, float animationTime, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(Vector3& outV, float animationTime, const aiNodeAnim* pNodeAnim);

    // 補間対象のキーインデックスを検索
    unsigned int FindScaling(float animationTime, const aiNodeAnim* pNodeAnim);
    unsigned int FindRotation(float animationTime, const aiNodeAnim* pNodeAnim);
    unsigned int FindPosition(float animationTime, const aiNodeAnim* pNodeAnim);


    // Assimpのシーンデータ
    const aiScene* mScene;
    Assimp::Importer mImporter;

    // ボーン名からインデックスをマップ
    std::map<std::string, unsigned int> mBoneMapping;
    // ボーン数
    unsigned int mNumBones;
    // 各ボーンのオフセット行列や最終変換行列など
    std::vector<struct BoneInfo> mBoneInfo;
    // ルートノードの逆変換行列
    Matrix4 mGlobalInverseTransform;

    // 頂点配列オブジェクト（1メッシュにつき1つ）
    std::vector<std::shared_ptr<class VertexArray>> mVertexArray;
    // マテリアル
    std::vector<std::shared_ptr<class Material>> mMaterials;

    // 読み込まれたアニメーションクリップ
    std::vector<struct AnimationClip> mAnimationClips;

    // 使用するシェーダー名
    std::string mShaderName;
    // スペキュラー係数
    float mSpecPower;
    
};
