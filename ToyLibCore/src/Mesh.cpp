#include "Mesh.h"
#include "Texture.h"
#include "AssetManager.h"
#include "VertexArray.h"
#include "Bone.h"
#include "Polygon.h"
#include "Material.h"
// Assimp用
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <vector>
#include <memory>
#include <iostream>
#include <cassert>
#include <string>

//const std::string ASSETS_PATH = "GameApp/Assets/";


// aiMatrix4x4からMatrix4へ変換
static void MatrixAi2Gl(Matrix4& mat, const aiMatrix4x4 aim)
{

    mat.mat[0][0] = aim.a1;
    mat.mat[0][1] = aim.b1;
    mat.mat[0][2] = aim.c1;
    mat.mat[0][3] = aim.d1;

    mat.mat[1][0] = aim.a2;
    mat.mat[1][1] = aim.b2;
    mat.mat[1][2] = aim.c2;
    mat.mat[1][3] = aim.d2;

    mat.mat[2][0] = aim.a3;
    mat.mat[2][1] = aim.b3;
    mat.mat[2][2] = aim.c3;
    mat.mat[2][3] = aim.d3;

    mat.mat[3][0] = aim.a4;
    mat.mat[3][1] = aim.b4;
    mat.mat[3][2] = aim.c4;
    mat.mat[3][3] = aim.d4;

}


// コンストラクタ
Mesh::Mesh()
: mScene(nullptr)
, mNumBones(0)
, mSpecPower(1.0f)
{
    
}

Mesh::~Mesh()
{
    mVertexArray.clear();
}

// 階層を辿ってノードの変換行列を得る
void Mesh::ComputeBoneHierarchy(float animationTime, const aiNode* pNode, const Matrix4& parentTransform, const aiAnimation* pAnimation)
{

    std::string nodeName(pNode->mName.data);
    Matrix4 nodeTransformation;
    MatrixAi2Gl(nodeTransformation, pNode->mTransformation);
    
    const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, nodeName);
    
    if (pNodeAnim)
    {
        // スケーリング
        Vector3 scaling;
        CalcInterpolatedScaling(scaling, animationTime, pNodeAnim);
        Matrix4 scalingM = Matrix4::CreateScale(scaling);
        
        // 回転マトリックス
        Quaternion rotationQ;
        CalcInterpolatedRotation(rotationQ, animationTime, pNodeAnim);
        Matrix4 rotationM = Matrix4::CreateFromQuaternion(rotationQ);
        
        // 移動マトリックス
        Vector3 translation;
        CalcInterpolatedPosition(translation, animationTime, pNodeAnim);
        Matrix4 translationM = Matrix4::CreateTranslation(translation);
        
        // ローカルマトリックスを生成
        nodeTransformation = rotationM * translationM * scalingM;;
    }

    
    
    // グローバルマトリックスを生成
    Matrix4 globalTransformation = nodeTransformation * parentTransform;

    if (mBoneMapping.find(nodeName) != mBoneMapping.end())
    {
        unsigned int boneIndex = mBoneMapping[nodeName];
        mBoneInfo[boneIndex].FinalTransformation =  mBoneInfo[boneIndex].BoneOffset * globalTransformation * mGlobalInverseTransform;
    }

    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
    {
        ComputeBoneHierarchy(animationTime, pNode->mChildren[i], globalTransformation, pAnimation);
    }
}

// アニメーションデータを取得
const aiNodeAnim* Mesh::FindNodeAnim(const aiAnimation* pAnimation, const std::string nodeName)
{
    for (unsigned int i = 0 ; i < pAnimation->mNumChannels ; i++)
    {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];
        
        if (std::string(pNodeAnim->mNodeName.data) == nodeName)
        {
            return pNodeAnim;
        }
    }
    return nullptr;
}


void Mesh::CalcInterpolatedPosition(Vector3& outVec, float animationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1)
    {
        outVec.Set(pNodeAnim->mPositionKeys[0].mValue.x,
                pNodeAnim->mPositionKeys[0].mValue.y,
                pNodeAnim->mPositionKeys[0].mValue.z);
        return;
    }
            
    unsigned int positionIndex = FindPosition(animationTime, pNodeAnim);
    unsigned int nextPositionIndex = (positionIndex + 1);
    assert(nextPositionIndex < pNodeAnim->mNumPositionKeys);
    
    float deltaTime = (float)(pNodeAnim->mPositionKeys[nextPositionIndex].mTime - pNodeAnim->mPositionKeys[positionIndex].mTime);
    float factor = (animationTime - (float)pNodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
    factor = std::clamp(factor, 0.0f, 1.0f);
    assert(factor >= 0.0f && factor <= 1.0f);

    const aiVector3D& start = pNodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D& end = pNodeAnim->mPositionKeys[nextPositionIndex].mValue;
    aiVector3D delta = end - start;
    aiVector3D tr = start + factor * delta;
    //tr = tr.Normalize();
    outVec.Set(tr.x, tr.y, tr.z);
}


void Mesh::CalcInterpolatedRotation(Quaternion& outVec, float animationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumRotationKeys == 1)
    {
        outVec.Set(pNodeAnim->mRotationKeys[0].mValue.x,
                pNodeAnim->mRotationKeys[0].mValue.y,
                pNodeAnim->mRotationKeys[0].mValue.z,
                pNodeAnim->mRotationKeys[0].mValue.w);
        return;
    }
    
    unsigned int rotationIndex = FindRotation(animationTime, pNodeAnim);
    unsigned int nextRotationIndex = (rotationIndex + 1);
    assert(nextRotationIndex < pNodeAnim->mNumRotationKeys);
    
    float deltaTime = (float)(pNodeAnim->mRotationKeys[nextRotationIndex].mTime - pNodeAnim->mRotationKeys[rotationIndex].mTime);
    float factor = (animationTime - (float)pNodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
    factor = std::clamp(factor, 0.0f, 1.0f);
    assert(factor >= 0.0f && factor <= 1.0f);

    const aiQuaternion& startRotationQ = pNodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion& endRotationQ   = pNodeAnim->mRotationKeys[nextRotationIndex].mValue;

    
    aiQuaternion q;
    aiQuaternion::Interpolate(q, startRotationQ, endRotationQ, factor);
    q = q.Normalize();
    outVec.Set(q.x, q.y, q.z, q.w);
    outVec.Normalize();

}


void Mesh::CalcInterpolatedScaling(Vector3& outVec, float animationTime, const aiNodeAnim* pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1)
    {
        outVec.Set(pNodeAnim->mScalingKeys[0].mValue.x,
                pNodeAnim->mScalingKeys[0].mValue.y,
                pNodeAnim->mScalingKeys[0].mValue.z);
        return;
    }

    unsigned int scalingIndex = FindScaling(animationTime, pNodeAnim);
    unsigned int nextScalingIndex = (scalingIndex + 1);
    
    assert(nextScalingIndex < pNodeAnim->mNumScalingKeys);
    float deltaTime = (float)(pNodeAnim->mScalingKeys[nextScalingIndex].mTime - pNodeAnim->mScalingKeys[scalingIndex].mTime);
    float factor = (animationTime - (float)pNodeAnim->mScalingKeys[scalingIndex].mTime) / deltaTime;
    factor = std::clamp(factor, 0.0f, 1.0f);
    assert(factor >= 0.0f && factor <= 1.0f);
    
    const aiVector3D& start = pNodeAnim->mScalingKeys[scalingIndex].mValue;
    const aiVector3D& end   = pNodeAnim->mScalingKeys[nextScalingIndex].mValue;
    
    aiVector3D delta = end - start;
    aiVector3D sc = start + factor * delta;
    //sc = sc.Normalize();
    outVec.Set(sc.x, sc.y, sc.z);
}


unsigned int Mesh::FindPosition(float animationTime, const aiNodeAnim* pNodeAnim)
{
    for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
        if (animationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
        {
            return i;
        }
    }
    
    return pNodeAnim->mNumScalingKeys - 2;
}


unsigned int Mesh::FindRotation(float animationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
    {
        if (animationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
        {
            return i;
        }
    }

    return pNodeAnim->mNumScalingKeys - 2;
}


unsigned int Mesh::FindScaling(float animationTime, const aiNodeAnim* pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);
    
    for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
    {
        if (animationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime)
        {
            return i;
        }
    }
    return pNodeAnim->mNumScalingKeys - 2;
}

// ボーン読み込み
void Mesh::LoadBones(const aiMesh* m, std::vector<VertexBoneData>& bones)
{
    for (unsigned int i = 0 ; i < m->mNumBones ; i++)
    {
        unsigned int boneIndex = 0;
        std::string boneName(m->mBones[i]->mName.data);
        
        if (mBoneMapping.find(boneName) == mBoneMapping.end())
        {
            // Allocate an index for a new bone
            boneIndex = mNumBones;
            mNumBones++;
            BoneInfo bi;
            mBoneInfo.push_back(bi);
            
            // オフセットマトリックスのコピー
            Matrix4 mat;
            MatrixAi2Gl(mat, m->mBones[i]->mOffsetMatrix);
            mBoneInfo[boneIndex].BoneOffset = mat;
            mBoneMapping[boneName] = boneIndex;
        }
        else
        {
            boneIndex = mBoneMapping[boneName];
        }
        
        for (unsigned int j = 0 ; j < m->mBones[i]->mNumWeights ; j++)
        {
            unsigned int VertexID = m->mBones[i]->mWeights[j].mVertexId;

            float weight  = m->mBones[i]->mWeights[j].mWeight;
            bones[VertexID].AddBoneData(boneIndex, weight);
        }
    }
}

// Bone付き
void Mesh::CreateMeshBone(const aiMesh* m)
{

    std::vector<float> vertexBuffer;            // ３個ずつ（XYZ）
    std::vector<float> normalBuffer;            // ３個ずつ
    std::vector<float> uvBuffer;                // ２個ずつ
    std::vector<unsigned int> boneIDs;          // ４個ずつ
    std::vector<float> boneWeights;             // ４個ずつ
    std::vector<unsigned int> indexBuffer;
    
    std::vector<VertexBoneData> bones;
    // Boneバッファ
    bones.resize(m->mNumVertices);

    LoadBones(m, bones);
    
    for (unsigned int i = 0 ; i < m->mNumVertices ; i++)
    {
        vertexBuffer.push_back(m->mVertices[i].x);
        vertexBuffer.push_back(m->mVertices[i].y);
        vertexBuffer.push_back(m->mVertices[i].z);
            
        normalBuffer.push_back(m->mNormals[i].x);
        normalBuffer.push_back(m->mNormals[i].y);
        normalBuffer.push_back(m->mNormals[i].z);

        if (m->HasTextureCoords(0))
        {
            uvBuffer.push_back(m->mTextureCoords[0][i].x);
            uvBuffer.push_back(m->mTextureCoords[0][i].y);
        }
        else
        {
            uvBuffer.push_back(0.0f);
            uvBuffer.push_back(0.0f);
        }

        boneIDs.push_back(bones[i].IDs[0]);
        boneIDs.push_back(bones[i].IDs[1]);
        boneIDs.push_back(bones[i].IDs[2]);
        boneIDs.push_back(bones[i].IDs[3]);
        
        boneWeights.push_back(bones[i].Weights[0]);
        boneWeights.push_back(bones[i].Weights[1]);
        boneWeights.push_back(bones[i].Weights[2]);
        boneWeights.push_back(bones[i].Weights[3]);
    }

    // インデックスバッファ取り出し
    for (unsigned int i = 0; i < m->mNumFaces; i++)
    {
        const aiFace& Face = m->mFaces[i];
        assert(Face.mNumIndices == 3);
        indexBuffer.push_back((Face.mIndices[0]));
        indexBuffer.push_back((Face.mIndices[1]));
        indexBuffer.push_back((Face.mIndices[2]));
    }
    
    
    mVertexArray.push_back(std::make_shared<VertexArray>(
            static_cast<unsigned int>(vertexBuffer.size()) / 3,
            vertexBuffer.data(),
            normalBuffer.data(),
            uvBuffer.data(),
            boneIDs.data(),
            boneWeights.data(),
            static_cast<unsigned int>(indexBuffer.size()),
            indexBuffer.data()) );
    
    mVertexArray[mVertexArray.size()-1]->SetTextureID(m->mMaterialIndex);
}

// メッシュをロード（Boneなし）
void Mesh::CreateMesh(const aiMesh* m)
{
    
    std::vector<float> vertexBuffer;            // ３個ずつ（XYZ）
    std::vector<float> normalBuffer;            // ３個ずつ
    std::vector<float> uvBuffer;                // ２個ずつ
    std::vector<unsigned int> indexBuffer;

    for (unsigned int i = 0 ; i < m->mNumVertices ; i++)
    {
        vertexBuffer.push_back(m->mVertices[i].x);
        vertexBuffer.push_back(m->mVertices[i].y);
        vertexBuffer.push_back(m->mVertices[i].z);
        
        normalBuffer.push_back(m->mNormals[i].x);
        normalBuffer.push_back(m->mNormals[i].y);
        normalBuffer.push_back(m->mNormals[i].z);

        if (m->HasTextureCoords(0))
        {
            uvBuffer.push_back(m->mTextureCoords[0][i].x);
            uvBuffer.push_back(m->mTextureCoords[0][i].y);
        }
        else
        {
            uvBuffer.push_back(0.0f);
            uvBuffer.push_back(0.0f);
        }
    }
    
    // インデックスバッファ取り出し
    for (unsigned int i = 0 ; i < m->mNumFaces ; i++)
    {

        const aiFace& Face = m->mFaces[i];
        assert(Face.mNumIndices == 3);
        indexBuffer.push_back(Face.mIndices[0]);
        indexBuffer.push_back(Face.mIndices[1]);
        indexBuffer.push_back(Face.mIndices[2]);
    }

    mVertexArray.push_back(std::make_shared<VertexArray>(
            static_cast<unsigned int>(vertexBuffer.size()) / 3,
            vertexBuffer.data(),
            normalBuffer.data(),
            uvBuffer.data(),
            static_cast<unsigned int>(indexBuffer.size()),
            indexBuffer.data()) );
    
    mVertexArray[mVertexArray.size()-1]->SetTextureID(m->mMaterialIndex);
    
    
}

bool Mesh::Load(const std::string& fileName, AssetManager* assetMamager, bool isRightHanded)
{
    unsigned int ASSIMP_LOAD_FLAGS = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                     aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
                                     aiProcess_OptimizeMeshes;

    if (isRightHanded) {
        ASSIMP_LOAD_FLAGS |= aiProcess_FlipWindingOrder;
    } else {
        ASSIMP_LOAD_FLAGS |= aiProcess_MakeLeftHanded;
    }

    std::string fullName = assetMamager->GetAssetsPath() + fileName;
    mScene = mImporter.ReadFile(fullName, ASSIMP_LOAD_FLAGS);
    if (!mScene) {
        std::cerr << "Assimp Load Error: " << mImporter.GetErrorString() << std::endl;
        return false;
    }

    aiMatrix4x4 inv = mScene->mRootNode->mTransformation;
    inv = inv.Inverse();
    MatrixAi2Gl(mGlobalInverseTransform, inv);

    LoadMeshData();
    LoadMaterials(assetMamager);
    LoadAnimations();

    return true;
}

void Mesh::LoadMeshData()
{
    for (int cnt = 0; cnt < mScene->mNumMeshes; cnt++) {
        aiMesh* m = mScene->mMeshes[cnt];
        if (m->HasBones()) {
            CreateMeshBone(m);
        } else {
            CreateMesh(m);
        }
    }
}

void Mesh::LoadMaterials(AssetManager* assetMamager)
{
    for (unsigned int cnt = 0; cnt < mScene->mNumMaterials; cnt++) {
        aiMaterial* pMaterial = mScene->mMaterials[cnt];
        std::shared_ptr<Material> mat = std::make_shared<Material>();

        aiColor3D color(0.f, 0.f, 0.f);
        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color))
            mat->SetAmbientColor(Vector3(color.r, color.g, color.b));
        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color))
            mat->SetDiffuseColor(Vector3(color.r, color.g, color.b));
        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color))
            mat->SetSpecularColor(Vector3(color.r, color.g, color.b));

        float shininess = 32.0f;
        if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_SHININESS, shininess))
            mat->SetSpecPower(shininess);

        aiString path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
            std::string texPath = path.C_Str();
            if (texPath[0] == '*') {
                int index = std::atoi(texPath.c_str() + 1);
                if (index >= 0 && index < (int)mScene->mNumTextures) {
                    aiTexture* aiTex = mScene->mTextures[index];
                    std::string key = "_EMBED_" + std::to_string(index);

                    const uint8_t* imageData = reinterpret_cast<const uint8_t*>(aiTex->pcData);
                    size_t imageSize = (aiTex->mHeight == 0)
                        ? aiTex->mWidth
                        : aiTex->mWidth * aiTex->mHeight * 4;

                    auto tex = assetMamager->GetEmbeddedTexture(key, imageData, imageSize);
                    if (tex) mat->SetDiffuseMap(tex);
                }
            } else {
                std::string textureFile = texPath;
                auto tex = assetMamager->GetTexture(textureFile);
                if (tex) mat->SetDiffuseMap(tex);
            }
        }

        mMaterials.push_back(mat);
    }
}

void Mesh::LoadAnimations()
{
    mAnimationClips.clear();

    if (!mScene || mScene->mNumAnimations == 0)
    {
        std::cout << "[Mesh] No animations found in scene." << std::endl;
        return;
    }

    std::cout << "[Mesh] Found " << mScene->mNumAnimations << " animation(s)." << std::endl;

    for (unsigned int i = 0; i < mScene->mNumAnimations; i++)
    {
        const aiAnimation* anim = mScene->mAnimations[i];

        AnimationClip clip;
        clip.mAnimation = anim;
        clip.mName = anim->mName.C_Str(); // 名前が空の場合もある
        clip.mDuration = static_cast<float>(anim->mDuration);
        clip.mTicksPerSecond = (anim->mTicksPerSecond != 0.0)
            ? static_cast<float>(anim->mTicksPerSecond)
            : 25.0f; // デフォルトTick値（Assimp推奨）

        mAnimationClips.emplace_back(clip);

    }
}
// データ破棄
void Mesh::Unload()
{
    mScene = nullptr;
    mVertexArray.clear();
}

std::shared_ptr<Material> Mesh::GetMaterial(size_t index)
{
    if (index < mMaterials.size())
    {
        return mMaterials[index];
    }
    return nullptr;
}


void Mesh::ComputePoseAtTime(float animationTime, const aiAnimation* pAnimation, std::vector<Matrix4>& outTransforms)
{
    Matrix4 identity = Matrix4::Identity;
    ComputeBoneHierarchy(animationTime, mScene->mRootNode, identity, pAnimation);

    outTransforms.resize(mNumBones);
    for (unsigned int i = 0; i < mNumBones; i++)
    {
        outTransforms[i] = mBoneInfo[i].FinalTransformation;
    }
}

