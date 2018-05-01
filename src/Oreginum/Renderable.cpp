#include "Renderable.hpp"
#include "Renderer Core.hpp"

void Oreginum::Renderable::add(Renderer_Core::Renderer_Type renderer)
{ id = Renderer_Core::add(renderer, this); }

void Oreginum::Renderable::remove(Renderer_Core::Renderer_Type renderer)
{ if(id != -1) Renderer_Core::remove(renderer, id); }
