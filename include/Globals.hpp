#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <yaml-cpp/yaml.h>

#include "View.hpp"
#include "Factory.hpp"
#include "RessourceManager.hpp"

class Item;

typedef RessourcesManager<sf::Texture, std::string> TexturesManager;

class Globals
{
public:
	Globals ();
	~Globals ();

	void init ();
	void gameLoop();
	void shutdown();

	View* getCurrentView () const;
	void setCurrentView (std::string name);

	const Factory<Item>& getItemFactory () const;
	TexturesManager& getTexturesManager ();
	sf::RenderWindow& getWindow ();

	void addView(std::string name, View* view);
	View* getView(std::string name);

	void captureScreen();
	const sf::Sprite& getLastCapture() const;

private:
	bool loadTextures(YAML::Node node, const std::string & path);
	bool loadItems (YAML::Node node);
	static Item* createItem(std::string name, unsigned int value);

	std::unique_ptr<sf::RenderWindow> window;
	View *current_view;

	Factory<Item> item_factory;
	TexturesManager textures_manager;

	std::map<std::string, View*> view_map;

	bool running;

	sf::Texture gameTexture;
	sf::Sprite gameSprite;
};

extern Globals globals;

#endif /* GLOBALS_HPP */
