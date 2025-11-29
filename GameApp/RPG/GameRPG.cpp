#include "GameRPG.h"
#include "ApplicationEntry.h"
#include "HeroActor.h"

// ToyLibの起動Applicationとして登録
TOYLIB_REGISTER_APP(GameRPG)


GameRPG::GameRPG()
: Application()
{
    SetIMEEnabled(false);
    SetAssetsPath("GameApp/Assets/RPG/");
    
    GetTimeOfDaySystem()->SetTimeScale(6000.f);
}

GameRPG::~GameRPG()
{
    
}

void GameRPG::InitGame()
{
    LoadData();
    


    // スプライト
    auto spActor = CreateActor<Actor>();
    spActor->SetPosition(Vector3(-460.0f, -330.0f, 0.0f));
    spActor->SetScale(1);
    auto spSprite = spActor->CreateComponent<SpriteComponent>(100, VisualLayer::UI);
    spSprite->SetTexture(GetAssetManager()->GetTexture("HealthBar.png"));
    spSprite->SetVisible(true);

    
    // 木（ビルボード）
    auto treeActor = CreateActor<Actor>();
    treeActor->SetPosition(Vector3(0.0f, 5.f, 0.0f));
    treeActor->SetScale(0.02);
    auto treeBillboard = treeActor->CreateComponent<BillboardComponent>(100);
    treeBillboard->SetTexture(GetAssetManager()->GetTexture("tree.png"));
    treeBillboard->SetVisible(true);
    treeActor->CreateComponent<GravityComponent>();
    auto treeCollider = treeActor->CreateComponent<ColliderComponent>();
    treeCollider->GetBoundingVolume()->ComputeBoundingVolume(Vector3(-256, -256, -4), Vector3(256,256,4));
    treeCollider->SetFlags(C_WALL | C_FOOT);
    
    // シャドウ用スプライト
    auto shadow = treeActor->CreateComponent<ShadowSpriteComponent>(10);
    //std::shared_ptr<Texture> shadowTex = std::make_shared<Texture>();
    //shadowTex->CreateAlphaCircle(256, 0.5f, 0.4f, Vector3(0.2f, 0.2f, 0.2f), 2.0f);
    //shadow->SetTexture(shadowTex);
    shadow->SetVisible(true);
    shadow->SetOffsetPosition(Vector3(0.0f, -4.9f, 0.0f));
    shadow->SetOffsetScale(0.03f);
    
    
    
    // 焚き火
    auto fireActor = CreateActor<Actor>();
    auto fireMesh = fireActor->CreateComponent<MeshComponent>();
    fireMesh->SetMesh(GetAssetManager()->GetMesh("campfile.x"));
  
    fireActor->SetPosition(Vector3(-15, 0, 15));
    fireActor->SetScale(0.03f);
    auto fireCollider = fireActor->CreateComponent<ColliderComponent>();
    fireCollider->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("campfile.x")->GetVertexArray());
    fireCollider->SetDisp(true);
    fireCollider->SetFlags(C_GROUND | C_WALL | C_FOOT);
    fireActor->CreateComponent<GravityComponent>();
    
    // 炎
    auto particleActor = CreateActor<Actor>();
    particleActor->SetPosition(Vector3(-15, 0, 15));
    auto particleComp = particleActor->CreateComponent<ParticleComponent>();
    particleComp->SetTexture(GetAssetManager()->GetTexture("fire.png"));
    particleComp->CreateParticles(Vector3(0, 0, 0),
                                  10,
                                  1000,
                                  0.3f,
                                  5.5,
                                  ParticleComponent::P_SMOKE);
    particleComp->SetAddBlend(true);

    
    // 先行者
    auto sksActor = CreateActor<Actor>();
    auto sksMesh = sksActor->CreateComponent<MeshComponent>();
    sksMesh->SetMesh(GetAssetManager()->GetMesh("sks.x"));

    sksActor->SetPosition(Vector3(-45, -100, 25));
    sksActor->SetScale(0.0015f);
    auto sksCollider = sksActor->CreateComponent<ColliderComponent>();
    sksCollider->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("sks.x")->GetVertexArray());
    sksCollider->SetDisp(true);
    sksCollider->SetFlags(C_GROUND | C_WALL | C_FOOT);
    sksActor->CreateComponent<GravityComponent>();
    
    
    /*
    auto sunlightActor = CreateActor<Actor>();
    auto sunlight = sunlightActor->CreateComponent<SpriteComponent>(100, VisualLayer::UI);
    std::shared_ptr<Texture> lightTex(new Texture);
    lightTex->CreateRadialRays(1024, 16, 3.f, 2.0f, 0.15f);
    sunlight->SetTexture(lightTex);
    */
}

void GameRPG::LoadData()
{
    
    auto hero = CreateActor<HeroActor>();
    
    // stan
    auto stanActor = CreateActor<Actor>();
    auto stanMesh = stanActor->CreateComponent<SkeletalMeshComponent>();
    auto stanCllider = stanActor->CreateComponent<ColliderComponent>();
    stanMesh->SetMesh(GetAssetManager()->GetMesh("stan.gltf", true));
    stanMesh->SetToonRender(true, 1.015f);
    
    stanActor->SetPosition(Vector3(-3,0,10));
    stanActor->SetScale(0.5f);
    Quaternion q = Quaternion(Vector3::UnitY, Math::ToRadians(-30));
    stanActor->SetRotation(q);
    
    stanCllider->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("stan.gltf")->GetVertexArray());
    stanCllider->GetBoundingVolume()->AdjustBoundingBox(Vector3(0.0f, 0, 0), Vector3(0.5, 1.f, 0.6));
    stanCllider->GetBoundingVolume()->CreateVArray();
    stanCllider->SetDisp(true);
    stanCllider->SetFlags(C_WALL | C_ENEMY | C_FOOT | C_GROUND);
    

    
    auto stanMove = stanActor->CreateComponent<FollowMoveComponent>();
    stanMove->SetTarget(hero);
    stanMove->SetFollowSpeed(1);
    stanActor->CreateComponent<GravityComponent>();

    
    
    // 建物
    auto towerActor = CreateActor<Actor>();
    auto towerMesh = towerActor->CreateComponent<MeshComponent>();
    towerMesh->SetMesh(GetAssetManager()->GetMesh("house.x"));
    
    auto towerCollider = towerActor->CreateComponent<ColliderComponent>();
    towerCollider->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("house.x")->GetVertexArray());
    towerCollider->SetDisp(true);
    towerCollider->SetFlags(C_WALL | C_GROUND | C_FOOT);
    towerCollider->GetBoundingVolume()->AdjustBoundingBox(Vector3(0,0,0), Vector3(0.9, 0.9, 0.9));
    towerActor->SetPosition(Vector3(15, 0, 15));
    towerActor->SetScale(0.003f);
    q = Quaternion(Vector3::UnitY, Math::ToRadians(150));
    towerActor->SetRotation(q);
    towerActor->CreateComponent<GravityComponent>();
    


    // レンガ
    auto brickActor = CreateActor<Actor>();
    auto brickMesh = brickActor->CreateComponent<MeshComponent>();
    brickMesh->SetMesh(GetAssetManager()->GetMesh("brick.x"));
    
    brickActor->SetPosition(Vector3(-15, 00, -15));
    brickActor->SetScale(5.f);
    auto brickCollider = brickActor->CreateComponent<ColliderComponent>();
    brickCollider->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("brick.x")->GetVertexArray());
    brickCollider->SetFlags(C_GROUND | C_WALL | C_FOOT);
    brickActor->CreateComponent<GravityComponent>();
    

    // レンガ２
    auto brickActor2 = CreateActor<Actor>();
    auto brickMesh2 = brickActor2->CreateComponent<MeshComponent>();
    brickMesh2->SetMesh(GetAssetManager()->GetMesh("brick.x"));
    
    brickActor2->SetPosition(Vector3(-10, 10, -15));
    brickActor2->SetScale(5.f);
    auto brickCollider2 = brickActor2->CreateComponent<ColliderComponent>();
    brickCollider2->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("brick.x")->GetVertexArray());
    brickCollider2->SetFlags(C_GROUND | C_WALL | C_FOOT);
    brickActor2->CreateComponent<GravityComponent>();

    
    // レンガ3
    auto brickActor3 = CreateActor<Actor>();
    auto brickMesh3 = brickActor3->CreateComponent<MeshComponent>();
    brickMesh3->SetMesh(GetAssetManager()->GetMesh("brick.x"));
    
    brickActor3->SetPosition(Vector3(-5, 20, -10));
    brickActor3->SetScale(5.f);
    auto brickCollider3 = brickActor3->CreateComponent<ColliderComponent>();
    brickCollider3->GetBoundingVolume()->ComputeBoundingVolume(GetAssetManager()->GetMesh("brick.x")->GetVertexArray());
    brickCollider3->SetFlags(C_GROUND | C_WALL | C_FOOT);
    brickActor3->CreateComponent<GravityComponent>();




    // 地面
    auto b = CreateActor<Actor>();
    auto g = b->CreateComponent<MeshComponent>(false);
    g->SetMesh(GetAssetManager()->GetMesh("ground2.x"));
    b->SetPosition(Vector3(0,0,0));
    b->SetScale(1);
    g->SetToonRender(false, 1.0);
    
    auto groundMesh = GetAssetManager()->GetMesh("ground2.x");
    auto va = groundMesh->GetVertexArray();
    auto vaList = groundMesh->GetVertexArray();
    for (auto va : vaList)
    {
        b->ComputeWorldTransform();
        const auto& polys = va->GetWorldPolygons(b->GetWorldTransform());
        GetPhysWorld()->SetGroundPolygons(polys); // or 統合してまとめる
    }
    
    // スカイドーム
    auto skyActor = CreateActor<Actor>();
    auto dome = skyActor->CreateComponent<WeatherDomeComponent>();
    // オーバーレイ
    auto overlay = skyActor->CreateComponent<WeatherOverlayComponent>();
    
    mWeather = std::make_unique<WeatherManager>();
    mWeather->SetWeatherDome(dome);
    mWeather->SetWeatherOverlay(overlay);
    skyActor->SetPosition(Vector3(0.f, -100.f, 0.f));
    
    
    // BGM
    GetSoundMixer()->LoadBGM("MusMus-BGM-112.mp3");
    GetSoundMixer()->PlayBGM();
}

void GameRPG::UpdateGame(float deltaTime)
{
    mWeather->Update(deltaTime);
}

void GameRPG::ShutdownGame()
{
    GetSoundMixer()->StopBGM();
    
}
