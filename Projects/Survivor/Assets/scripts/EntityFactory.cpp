#include "EntityFactory.h"
#include "GameComponents.h"
#include "Projectile.h" 
#include "Enemy.h"
#include "ExperienceOrb.h"
#include "FloatingText.h"

#include "Runic2D/Utils/Random.h"

Runic2D::Scene* EntityFactory::s_Scene = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_ProjectileTexture = nullptr;

Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_BatTexture = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_BatDeathTexture = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_SlimeTexture = nullptr;

Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_WhiteGemTexture = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_GreenGemTexture = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_RedGemTexture = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_PurpleGemTexture = nullptr;

static void AddAnimationToEntity(Entity entity, const std::string& name, Ref<Texture2D> texture, int startFrame, int frameCount, glm::vec2 tileSize, float frameTime, bool loop)
{
    auto& anim = entity.HasComponent<AnimationComponent>() ?
        entity.GetComponent<AnimationComponent>() :
        entity.AddComponent<AnimationComponent>();

    AnimationProfile profile;
    profile.Name = name;
    profile.AtlasTexture = texture;

    profile.TileSize = tileSize;

    profile.StartFrame = startFrame;
    profile.FrameCount = frameCount;
    profile.FramesPerRow = (int)(texture->GetWidth() / profile.TileSize.x);
    profile.FrameTime = frameTime;
    profile.Loop = loop;

    anim.Profiles.push_back(profile);

    int col = startFrame % profile.FramesPerRow;
    int row = startFrame / profile.FramesPerRow;
    glm::vec2 startPos = { col * profile.TileSize.x, row * profile.TileSize.y };

    anim.Animations[name] = Runic2D::Animation2D::CreateFromAtlas(
        texture,
        profile.TileSize,
        startPos,
        frameCount,
        profile.FramesPerRow,
        frameTime
    );

    if (anim.CurrentStateName.empty()) {
        anim.CurrentStateName = name;
        anim.CurrentAnimation = anim.Animations[name];
        anim.Playing = true;
        anim.Loop = loop;
    }
}

void EntityFactory::Init(Runic2D::Scene* scene)
{
    s_Scene = scene;
	std::string path = Project::GetAssetDirectory().string();
    s_ProjectileTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/projectiles/wizard/WizzardProjectile.png"));

	s_BatTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/Enemies/Bat/Run.png"));
    s_BatDeathTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/Enemies/Bat/Death.png"));
	s_SlimeTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/Enemies/Slime/Slime.png"));

	s_WhiteGemTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/EXP/EXP1.png"));
    s_GreenGemTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/EXP/EXP10.png"));
    s_RedGemTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/EXP/EXP50.png"));
	s_PurpleGemTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/EXP/EXP100.png"));
}

void EntityFactory::Shutdown()
{
    s_ProjectileTexture = nullptr;
	s_BatTexture = nullptr;
	s_BatDeathTexture = nullptr;
	s_WhiteGemTexture = nullptr;
	s_GreenGemTexture = nullptr;
	s_RedGemTexture = nullptr;
	s_PurpleGemTexture = nullptr;

    s_Scene = nullptr;
}

Runic2D::Entity EntityFactory::CreatePlayerProjectile(glm::vec2 position, glm::vec2 direction)
{
    if (!s_Scene) return {};

    auto entity = s_Scene->CreateEntity("Fireball");

    // 1. Transform
    auto& tc = entity.GetComponent<Runic2D::TransformComponent>();
    tc.Translation = { position.x, position.y, 0.0f };
    tc.Scale = { 1.0f, 1.0f, 1.0f };
    tc.Rotation.z = atan2(direction.y, direction.x);

    // 2. Sprite
    auto& src = entity.AddComponent<Runic2D::SpriteRendererComponent>();
    src.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (s_ProjectileTexture) src.Texture = s_ProjectileTexture;

	// 3. Animació
    auto& anim = entity.AddComponent<Runic2D::AnimationComponent>();

    Runic2D::AnimationProfile travelAnim;
    travelAnim.Name = "Travel";
    travelAnim.AtlasTexture = s_ProjectileTexture; 
    travelAnim.TileSize = { 192.0f, 192.0f };

    travelAnim.StartFrame = 0;
    travelAnim.FrameCount = 22;
    travelAnim.FramesPerRow = 5; 

    travelAnim.FrameTime = 0.01f;
    travelAnim.Loop = true;

    anim.Profiles.push_back(travelAnim);

    anim.CurrentStateName = "Travel";
    anim.Playing = true;
    anim.Loop = true;

    anim.CurrentAnimation = Runic2D::Animation2D::CreateFromAtlas(
        travelAnim.AtlasTexture,
        travelAnim.TileSize,
        { 0.0f, 0.0f },
        travelAnim.FrameCount,
        travelAnim.FramesPerRow,
        travelAnim.FrameTime
    );
    anim.Animations["Travel"] = anim.CurrentAnimation;

    // 3. Físiques (Configuració)
    auto& rb = entity.AddComponent<Runic2D::Rigidbody2DComponent>();
    rb.Type = Runic2D::Rigidbody2DComponent::BodyType::Dynamic;
    rb.FixedRotation = true;
    rb.GravityScale = 0.0f;

    auto& bc = entity.AddComponent<Runic2D::CircleCollider2DComponent>();
    bc.IsSensor = true;
    bc.Radius = 0.35f;
    bc.EnableContactEvents = false;
	bc.EnableSensorEvents = true;

    s_Scene->InstantiatePhysics(entity);

	//5. Prjectile properties
    auto& projData = entity.AddComponent<ProjectileComponent>();
    projData.Speed = 12.0f;
    projData.LifeTime = 1.5f;
    projData.Damage = 10.0f;
    projData.Direction = direction;

    // 6. Script
    auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
    nsc.Bind<Projectile>();

    return entity;
}

Runic2D::Entity EntityFactory::CreateBat(glm::vec2 pos, float difficultyMult)
{
    Entity entity = CreateBaseEnemy(pos, "Bat");

    auto& tc = entity.GetComponent<TransformComponent>();
	tc.Scale = { 1.75f, 1.75f, 1.0f };

    auto& stats = entity.AddComponent<EnemyStatsComponent>();
    stats.Health = 10.0f * difficultyMult;    
    stats.MaxHealth = stats.Health;
    stats.Speed = 2.0f + (difficultyMult * 0.1f); 
    stats.Damage = 5.0f * difficultyMult;
    stats.XPDrop = 10;

    auto& src = entity.AddComponent<SpriteRendererComponent>();
    src.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    src.Texture = s_BatTexture; 

	AddAnimationToEntity(entity, "Fly", s_BatTexture, 0, 8, { 64.0f, 64.0f }, 0.1f, true);
	AddAnimationToEntity(entity, "Death", s_BatDeathTexture, 0, 11, { 64.0f, 64.0f }, 0.1f, false);

    auto& coll = entity.AddComponent<CircleCollider2DComponent>();
    coll.Radius = 0.15f;
	coll.Offset = { 0.0f, -0.1f };
    coll.CategoryBits = PhysicsLayers::Enemy | PhysicsLayers::EnemyBat;
	coll.MaskBits = coll.MaskBits = PhysicsLayers::Default | PhysicsLayers::Player | PhysicsLayers::Projectile | PhysicsLayers::EnemyBat;
    coll.IsSensor = false;
	coll.EnableSensorEvents = true;
	coll.EnableContactEvents = true;
    coll.Density = 1.0f;
    coll.Friction = 0.0f;    
    coll.Restitution = 0.0f;  
    s_Scene->InstantiatePhysics(entity);

    return entity;
}

Runic2D::Entity EntityFactory::CreateSlime(glm::vec2 pos, float difficultyMult)
{
    Entity entity = CreateBaseEnemy(pos, "Slime");

    auto& tc = entity.GetComponent<TransformComponent>();
    tc.Scale = { 0.5f, 0.5f, 1.0f };

    auto& stats = entity.AddComponent<EnemyStatsComponent>();
    stats.Health = 10.0f * difficultyMult;
    stats.MaxHealth = stats.Health;
    stats.Speed = 1.0f + (difficultyMult * 0.1f);
    stats.Damage = 5.0f * difficultyMult;
    stats.XPDrop = 10;

    auto& src = entity.AddComponent<SpriteRendererComponent>();
    src.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    src.Texture = s_SlimeTexture;

    AddAnimationToEntity(entity, "Walk", s_SlimeTexture, 4, 4, { 32, 25 }, 0.15f, true);
    AddAnimationToEntity(entity, "Death", s_SlimeTexture, 17, 5, { 32, 25 }, 0.15f, false);

    auto& coll = entity.AddComponent<CircleCollider2DComponent>();
    coll.Radius = 0.30f;
    coll.Offset = { 0.0f, -0.1f };
    coll.CategoryBits = PhysicsLayers::Enemy | PhysicsLayers::EnemySlime;
    coll.MaskBits = coll.MaskBits = PhysicsLayers::Default | PhysicsLayers::Player | PhysicsLayers::Projectile | PhysicsLayers::EnemySlime;
    coll.IsSensor = false;
    coll.EnableSensorEvents = true;
    coll.EnableContactEvents = true;
    coll.Density = 1.0f;
    coll.Friction = 0.0f;
    coll.Restitution = 0.0f;
    s_Scene->InstantiatePhysics(entity);

    return entity;
}

Runic2D::Entity EntityFactory::CreateExperienceGem(glm::vec2 pos, int amount)
{
    Entity entity = s_Scene->CreateEntity("ExperienceGem");

    auto& tc = entity.GetComponent<TransformComponent>();
    tc.Translation = { pos.x, pos.y, -1.0f };
    tc.Scale = { 0.35f, 0.35f, 1.0f };

    // 2. Sprite (Lògica visual segons valor)
    auto& src = entity.AddComponent<SpriteRendererComponent>();
    if (amount < 2) src.Texture = s_WhiteGemTexture;      
    else if (amount < 11) src.Texture = s_GreenGemTexture; 
    else if (amount < 51) src.Texture = s_RedGemTexture;
    else src.Texture = s_PurpleGemTexture;                    

    auto& xpComp = entity.AddComponent<ExperienceComponent>();
    xpComp.Amount = amount;

    auto& coll = entity.AddComponent<CircleCollider2DComponent>();
    coll.Radius = 0.5f;
    coll.CategoryBits = PhysicsLayers::Item;
    coll.MaskBits = coll.MaskBits = PhysicsLayers::Player;
    coll.IsSensor = true; 
    coll.EnableSensorEvents = true;
	coll.EnableContactEvents = false;

    auto& rb = entity.AddComponent<Rigidbody2DComponent>();
    rb.Type = Rigidbody2DComponent::BodyType::Dynamic;
    rb.GravityScale = 0.0f;
    rb.FixedRotation = true;

    s_Scene->InstantiatePhysics(entity);

    // 5. Script
    entity.AddComponent<NativeScriptComponent>().Bind<ExperienceOrb>();

    return entity;
}

Entity EntityFactory::CreateDamageText(glm::vec2 pos, float damageAmount, bool isCritical)
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(0) << damageAmount;
    std::string text = ss.str();

    Entity entity = s_Scene->CreateEntity("DamageText");
    
    float scale = isCritical ? 1.0f : 0.5f;

    float charWidthConstant = 0.6f;
    float approximateWidth = text.length() * charWidthConstant * scale;
    float centerOffset = approximateWidth / 2.0f;

    float randomY = Runic2D::Random::Range(0.0f, 0.5f);
    auto& tc = entity.GetComponent<TransformComponent>();
    tc.Translation = { pos.x - centerOffset, pos.y + randomY, 0.1f };
    tc.Scale = { scale, scale, 1.0f };

    auto& textComp = entity.AddComponent<TextComponent>();
    textComp.TextString = text;

    if (isCritical) textComp.Color = { 1.0f, 0.8f, 0.0f, 1.0f }; 
    else textComp.Color = { 1.0f, 1.0f, 1.0f, 1.0f };            

    textComp.Kerning = 0.0f;
    textComp.LineSpacing = 0.0f;

    entity.AddComponent<NativeScriptComponent>().Bind<FloatingText>();

    return entity;
}

Runic2D::Entity EntityFactory::CreateBaseEnemy(glm::vec2 pos, std::string name)
{
    auto entity = s_Scene->CreateEntity(name);

    auto& tc = entity.GetComponent<TransformComponent>();
    tc.Translation = { pos.x, pos.y, 0.0f };

    auto& rb = entity.AddComponent<Rigidbody2DComponent>();
    rb.Type = Rigidbody2DComponent::BodyType::Dynamic;
    rb.FixedRotation = true;
	rb.GravityScale = 0.0f;

    entity.AddComponent<NativeScriptComponent>().Bind<Enemy>();

    return entity;
}
