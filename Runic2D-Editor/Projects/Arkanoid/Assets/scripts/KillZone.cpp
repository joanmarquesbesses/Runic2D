#include "KillZone.h"
#include "Ball.h"

using namespace Runic2D;

void KillZone::OnCollision(Entity other)
{
	if (other.GetComponent<TagComponent>().Tag == "Ball") {
		if (other.HasComponent<NativeScriptComponent>())
		{
			R2D_INFO("KillZone collided with Ball");
            auto& nsc = other.GetComponent<NativeScriptComponent>();

            Ball* ballScript = dynamic_cast<Ball*>(nsc.Instance);

            if (ballScript)
            {
                ballScript->lives--;

                if (ballScript->lives > 0)
                {
                    ballScript->Restart();
                }
                else
                {
                    // Si arriba a 0, deixem que el Ball::OnUpdate el destrueixi
                    // o ho fem aquí directament.
                    R2D_INFO("GAME OVER");
                }
            }
		}
	}	
}