#include "Block.h"

#include "Score.h"

using namespace Runic2D;

void Block::OnCreate()
{
	auto texture = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/blocksTileset.png"));
	GetComponent<SpriteRendererComponent>().Texture = texture;
	GetComponent<SpriteRendererComponent>().Color = glm::vec4{1.0f};

	int randomInt = std::rand() % (int)BlockType::Count;
	Initialize(static_cast<BlockType>(randomInt));

	m_ScoreEntity = GetScene()->FindEntityByName("Score");
	if (!m_ScoreEntity)
	{
		R2D_CORE_ERROR("Block could not find 'Score' entity!");
	}
}

void Block::Initialize(BlockType type)
{
	m_Type = type;

	// Configurem vides segons el tipus
	m_MaxLives = 6;

	m_Lives = m_MaxLives;
	UpdateVisuals();
}

void Block::UpdateVisuals()
{
	auto& src = GetComponent<SpriteRendererComponent>();
	if (!src.Texture) return;

	int indexX;
	
	if (m_IsDying)
	{
		int animationStartOffset = 5; 
		indexX = animationStartOffset + m_CurrentFrame;
	}
	else
	{
		// Lògica normal de vides
		indexX = m_MaxLives - m_Lives;
	}

	int indexY = (int)m_Type;

	float pixelX = tileWidth + (indexX * tileWidth);
	float pixelY = heightOffset + (indexY * tileHeight );

	float texWidth = (float)src.Texture->GetWidth();
	float texHeight = (float)src.Texture->GetHeight();

	src.SubTexture = SubTexture2D::CreateFromPixelCoords(
		src.Texture,
		pixelX, pixelY,
		tileWidth, tileHeight
	);
}

void Block::OnDestroy()
{
}

void Block::OnUpdate(Timestep ts)
{
	if (m_IsDying)
	{
		m_AnimationTime += ts;

		if (m_AnimationTime >= m_FrameDuration)
		{
			m_AnimationTime = 0.0f;
			m_CurrentFrame++;

			if (m_CurrentFrame >= m_TotalAnimFrames)
			{
				Destroy();
			}
			else
			{
				UpdateVisuals();
			}
		}
	}
}

void Block::Die()
{
	if (m_IsDying) return;
	m_IsDying = true;

	GetScene()->SetCollisionEnabled(GetEntity(), false);

	if (HasComponent<Rigidbody2DComponent>())
	{
		auto& rb2d = GetComponent<Rigidbody2DComponent>();
		b2BodyId bodyId = rb2d.RuntimeBody;

		b2Body_SetType(bodyId, b2_dynamicBody);

		b2Body_SetAwake(bodyId, true);
	}
}

void Block::OnCollision(Entity other)
{
	if (!other.HasComponent<NativeScriptComponent>())
		return;

	if (other.GetComponent<NativeScriptComponent>().ClassName == "Ball")
	{
		TakeDamage();
	}
}

void Block::TakeDamage()
{
	m_Lives--;

	if (m_ScoreEntity && m_ScoreEntity.HasComponent<NativeScriptComponent>())
	{
		auto& nsc = m_ScoreEntity.GetComponent<NativeScriptComponent>();
		if (nsc.Instance)
		{
			// Nota: dynamic_cast és una mica lent, però segur. 
			// Si estàs segur al 100% que és Score, podries fer static_cast.
			Score* scoreScript = dynamic_cast<Score*>(nsc.Instance);

			if (scoreScript)
			{
				if (m_Lives <= 0)
				{
					scoreScript->AddScore(100);
					Die(); // Destruir bloc
					AudioEngine::Play("Projects/Arkanoid/Assets/sounds/DestroyBlock.mp3");
				}
				else
				{
					scoreScript->AddScore(10);
					UpdateVisuals();
					AudioEngine::Play("Projects/Arkanoid/Assets/sounds/HitBlock.mp3");
				}
			}
		}
	}
	else
	{
		// Si no hi ha score, el bloc mor igualment (lògica de joc robusta)
		if (m_Lives <= 0) Die();
		else UpdateVisuals();
	}
}