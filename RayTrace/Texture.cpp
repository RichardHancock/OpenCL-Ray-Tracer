#include "Texture.h"

#include <assert.h>

#include "misc/Log.h"

Texture::Texture(std::string filename, SDL_Renderer* renderer, bool keepSurface)
	: keepSurface(keepSurface)
{
	texture = nullptr;
	load(filename, renderer);
}
Texture::Texture(SDL_Surface* surface, SDL_Renderer* renderer, bool keepSurface)
	: keepSurface(keepSurface)
{
	texture = nullptr;
	load(surface, renderer);
}
Texture::~Texture()
{
	if (texture != nullptr)
		SDL_DestroyTexture(texture);

	if (keepSurface)
	{
		SDL_FreeSurface(surface);
	}
}

bool Texture::load(std::string filename, SDL_Renderer* renderer)
{
	Log::logI("Loading Texture: " + filename);

	SDL_Surface *image = IMG_Load(filename.c_str());
	if (!image)
	{
		// We'll do a quick check here because it's easy to get filenames or directories wrong
		Log::logE("Can't find image named " + filename + " " + std::string(IMG_GetError()));
		return false;
	}

	currentRenderer = renderer;
	Texture::createTexture(*image); //Need error check here
	surface = image;
	if (!keepSurface)
	{
		SDL_FreeSurface(image);
	}
	return true;
}

bool Texture::load(SDL_Surface* image, SDL_Renderer* renderer)
{
	currentRenderer = renderer;
	Texture::createTexture(*image); //Need error check here
	if (!keepSurface)
	{
		SDL_FreeSurface(image);
	}
	else {
		this->surface = image;
	}

	return true;
}

bool Texture::createTexture(SDL_Surface &image)
{
	texture = SDL_CreateTextureFromSurface(currentRenderer, &image);

	// This is done because a float* and int* conflict.
	int tempDimensionsX;
	int tempDimensionsY;
	SDL_QueryTexture(texture, NULL, NULL, &tempDimensionsX, &tempDimensionsY);

	dimensions.x = (float)tempDimensionsX;
	dimensions.y = (float)tempDimensionsY;

	return (texture ? true : false);
}

void Texture::draw(Vec2 pos)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	// Query the texture to get its original width and height
	destRect.w = (int)dimensions.x;
	destRect.h = (int)dimensions.y;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position of the rectangle we specify
	SDL_RenderCopy(currentRenderer, texture, NULL, &destRect);
}

void Texture::draw(Vec2 pos, float rotation)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	// Query the texture to get its original width and height
	destRect.w = (int)dimensions.x;
	destRect.h = (int)dimensions.y;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position and rotation we specify
	SDL_RenderCopyEx(currentRenderer, texture, NULL, &destRect, rotation, NULL, SDL_FLIP_NONE);
}

void Texture::draw(Vec2 pos, float rotation, Vec2 pivot)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	// Query the texture to get its original width and height
	destRect.w = (int)dimensions.x;
	destRect.h = (int)dimensions.y;

	SDL_Point pivotPoint;
	pivotPoint.x = (int)pivot.x;
	pivotPoint.y = (int)pivot.y;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position and rotation we specify
	SDL_RenderCopyEx(currentRenderer, texture, NULL, &destRect, rotation, &pivotPoint, SDL_FLIP_NONE);
}

void Texture::draw(Vec2 pos, SDL_Rect spriteMapPos)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	// Query the texture to get its original width and height
	destRect.w = destRect.h = spriteMapPos.w;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position of the rectangle we specify
	SDL_RenderCopy(currentRenderer, texture, &spriteMapPos, &destRect);
}

void Texture::drawScaled(Vec2 pos, Vec2 scaledDimensions)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	destRect.w = (int)scaledDimensions.x;
	destRect.h = (int)scaledDimensions.y;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position of the rectangle we specify
	SDL_RenderCopy(currentRenderer, texture, NULL, &destRect);
}
void Texture::drawScaled(Vec2 pos, Vec2 scaledDimensions, SDL_Rect spriteMapPos)
{
	SDL_Rect destRect;

	destRect.x = (int)pos.x;
	destRect.y = (int)pos.y;

	destRect.w = (int)scaledDimensions.x;
	destRect.h = (int)scaledDimensions.y;

	// Here we are telling the renderer to copy the texture to our screen,
	// at the position of the rectangle we specify
	SDL_RenderCopy(currentRenderer, texture, &spriteMapPos, &destRect);
}

Vec2 Texture::getDimensions()
{
	return dimensions;
}

SDL_Renderer* Texture::getRenderer()
{
	return currentRenderer;
}

void Texture::setColourTint(SDL_Colour c)
{
	//Set the texture colour tint or log an error.
	if (SDL_SetTextureColorMod(texture, c.r, c.g, c.b) != 0)
	{
		Log::logW("SDL_SetTextureColorMod failed in setColourTint");
	}
}