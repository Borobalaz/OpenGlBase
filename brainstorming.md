# Material systems integration

## PBRScene: Scene, PaintingScene: Scene

### Pros
-

### Cons
- No real time switching between render modes.

---
## RenderMode attribute + Dependency Injection

### Pros

### Cons
- Too convoluted

---
## Different engine for each purpose

### Pros
- Straightforward use

### Cons
- No real time switching between render modes.

---
## Painting as post processing

Probably wouldnt look as paintingy as i would like, also bad performance.

# Renderer

Scene összeszedi a kirajzolandó objektumokat egy DrawContext-be:

DrawContext:
- uniformok
- VAO, VBO, EBO,
- shader

scene: 
"""
vector drawContexts
for each drawable:
  DrawContext dc
  drawable.GatherDrawContext(&dc) --->| minden IDrawable a kapott drawContextbe belerakja a saját cuccait
  drawContexts.add(dc)                | pl.: Mesh a VAO-t, material a shadert, és amúgy mindenki a saját uniformjait.

return drawContexts
"""

DeferredRenderer nem használja a DrawContext shader mezejét
ForwardRenderer azt a shadert használja.