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
    tc.Scale = { 0.5f, 0.5f, 1.0f };
    // Calculem la rotació aquí. El script només haurà de moure's "endavant"
    tc.Rotation.z = atan2(direction.y, direction.x);

    // 2. Sprite
    auto& src = entity.AddComponent<Runic2D::SpriteRendererComponent>();
    src.Color = { 1.0f, 1.0f, 1.0f, 1.0f };
    if (s_ProjectileTexture) src.Texture = s_ProjectileTexture;

	auto& anim = entity.AddComponent<Runic2D::AnimationComponent>();
    if (s_ProjectileTexture)
    {
        Runic2D::AnimationProfile profile;
        profile.Name = "Travel";   
        profile.AtlasTexture = s_ProjectileTexture;
        profile.TileSize = { 32.0f, 32.0f }; // Mida de cada frame (ajusta al teu PNG!)
        profile.StartFrame = 0;
        profile.FrameCount = 4;           // Quants frames té l'animació      // Quants frames hi ha per fila a la textura
        profile.FrameTime = 0.1f;         // Velocitat
        profile.Loop = true;              // Volem que giri constantment mentre vola

        anim.Profiles.push_back(profile);

        auto animAsset = Runic2D::Animation2D::CreateFromAtlas(
            profile.AtlasTexture,
            profile.TileSize,
            { 0.0f, 0.0f }, 
            profile.FrameCount,
            profile.FrameTime
        );

        anim.Animations[profile.Name] = animAsset;
        anim.CurrentStateName = "Travel";
        anim.CurrentAnimation = animAsset;
        anim.Playing = true;
        anim.Loop = true;
    }

    // 3. Físiques (Configuració)
    auto& rb = entity.AddComponent<Runic2D::Rigidbody2DComponent>();
    rb.Type = Runic2D::Rigidbody2DComponent::BodyType::Dynamic;
    rb.FixedRotation = true;
    rb.GravityScale = 0.0f;

    auto& bc = entity.AddComponent<Runic2D::CircleCollider2DComponent>();
    bc.IsSensor = true;
    bc.Radius = 0.25f;

    // 4. INSTANCIACIÓ FÍSICA 
    s_Scene->InstantiatePhysics(entity);

    // 5. Script
    auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
    nsc.Bind<Projectile>();

    return entity;
}