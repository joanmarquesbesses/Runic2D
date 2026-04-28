# Context del Projecte Runic2D
- **Origen:** Creat originalment com a TFG (Treball de Final de Grau).
- **Estat actual:** Fork de l'original per seguir avançant de forma independent.
- **Objectiu:** Desenvolupar un joc comercial per publicar-lo a Steam en el futur.
- **Prioritat actual:** Implementar un sistema de In-Game UI robust (RectTransform).

# Regles Tècniques
- L'Arquitectura es basa en la separació de DLLs (Motor vs Joc).
- El sistema de reflexió usa EnTT i `ComponentRegistry`.
- Qualsevol canvi en la compilació s'ha de fer a través dels fitxers `.lua` de Premake5.
- **In-Game UI vs Editor UI:** L'Editor fa servir ImGui. PERÒ per al joc (In-Game UI) dissenyarem un sistema propi basat en components d'EnTT (`RectTransform`, `ImageComponent`, etc.) renderitzat pel nostre `Renderer2D`. ImGui NO s'ha d'utilitzar mai per dibuixar elements del joc final.