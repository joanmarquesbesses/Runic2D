#include "EntityFactory.h"
#include "Projectile.h" 

Runic2D::Scene* EntityFactory::s_Scene = nullptr;
Runic2D::Ref<Runic2D::Texture2D> EntityFactory::s_ProjectileTexture = nullptr;

void EntityFactory::Init(Runic2D::Scene* scene)
{
    s_Scene = scene;
	std::string path = Project::GetAssetDirectory().string();
    s_ProjectileTexture = Runic2D::ResourceManager::Get<Runic2D::Texture2D>(Project::GetAssetFileSystemPath("textures/projectiles/wizard/WizzardProjectile.png"));
}

void EntityFactory::Shutdown()
{
    s_ProjectileTexture = nullptr;
    s_Scene = nullptr;
}

Runic2D::Entity EntityFactory::CreatePlayerProjectile(glm::vec2 position, glm::vec2 direction)
{
    if (!s_Scene) return {};

    auto entity = s_Scene->CreateEntity("Fireball");

    // 1. Transform
    auto& tc = entity.GetComponent<Runic2D::TransformComponent>();
    tc.Translation = { position.x, position.y, 0.0f };
    tc.Scale = { 1.5f, 1.5f, 1.0f };
    // Calculem la rotació aquí. El script només haurà de moure's "endavant"
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
    bc.Radius = 0.25f;
    bc.EnableContactEvents = false;
	bc.EnableSensorEvents = true;

    s_Scene->InstantiatePhysics(entity);

    // 5. Script
    auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
    nsc.Bind<Projectile>();

    return entity;
}