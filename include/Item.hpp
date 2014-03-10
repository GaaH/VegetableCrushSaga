#ifndef INCLUDED_ITEM_HPP
#define INCLUDED_ITEM_HPP

#include <string>
#include <memory>

#include <SFML/Graphics/Sprite.hpp>

class Board;

class Item : public sf::Sprite
{
public:
	static std::unique_ptr<Item> null_item;

	Item(const std::string &name, const unsigned int points);
	virtual ~Item ();

	virtual void create_callback(Board & board, unsigned int pos);
	virtual void destroy_callback(Board & board, unsigned int pos);

	void goTo (const sf::Vector2f & pos);

	std::string getName () const;
	unsigned int getValue () const;

	void destroy ();
	void repair () {destroyed = false;}

	bool isDestroyed () const;
	bool isMoving() const;

	bool operator==(const Item& item) const;
	bool operator!=(const Item& item) const;

	void resetItem(Item* item);

	virtual Item* clone() = 0;

  protected:
	const std::string name;

	unsigned int value;

	bool destroyed;
	bool moving;

  public:
	bool swapped;


};

#endif /* INCLUDED_ITEM_HPP */
