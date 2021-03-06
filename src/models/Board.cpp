#include <SFML/Graphics/Texture.hpp>

#include "Globals.hpp"

#include "models/Game.hpp"
#include "models/Board.hpp"

#include "models/items/SpecialItemBomb.hpp"

#include "models/generators/UniformItemGenerator.hpp"

Board::Board (unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int cell_size, Game &game) :
  cell_size(cell_size),
  total_size(width * height),
  cols(width),
  rows(height),
  items(total_size),
  item_generator(new UniformItemGenerator),
  last_move_score(0),
  combo(0),
  game(game)
{
  item_generator->generateSpecial(false);

  dimensions = {x, y, width*cell_size, height*cell_size};

  auto board_shape_texture = globals.getTextureManager().getResource("item_background");
  board_shape_texture->setRepeated(true);

  board_shape.setTexture(*board_shape_texture);
  board_shape.setPosition(x, y);

  item_generator->registerItem("Aubergine");
  item_generator->registerItem("Strawberry");
  item_generator->registerItem("Mushroom");
  item_generator->registerItem("Banana");
  item_generator->registerItem("Watermelon");
  item_generator->registerItem("Carrot");

  float right_scale = (float)cell_size / (float)board_shape_texture->getSize().x;
  sf::IntRect texture_rect = board_shape.getTextureRect();

  texture_rect.width *= cols;
  texture_rect.height *= rows;

  board_shape.setTextureRect(texture_rect);
  board_shape.setScale(right_scale, right_scale);

  randomFill();

  do
  {
	fillBlanks();

	for (unsigned int i(0) ; i < rows; ++i)
	{
	  updateLine(i * cols, (i+1) * cols, 1);
	}

	for (unsigned int i(0) ; i < cols; ++i)
	{
	  updateLine(i, cols * rows + i, cols);
	}

	for (unsigned int i = 0; i < cols; ++i)
	{
	  applyGravity(i);
	}

  } while (!isStable());

  // A ce point, le jeu est stable
  last_move_score = combo = 0;
  std::for_each(items.begin(), items.end(), [](std::unique_ptr<Item>& item)
  {
	item->setSwap(false);
  });

  item_generator->generateSpecial(true);
}

Board::~Board ()
{
}

void Board::randomFill ()
{
  for (unsigned int i(0) ; i < total_size ; ++i)
  {
	changeItem(i % cols, i / cols);
  }
}

const std::vector<std::unique_ptr<Item> >& Board::getItems () const
{
  return items;
}

std::unique_ptr<Item>& Board::getItemAt (const unsigned int ind)
{
  if(ind < total_size)
  {
	if(!items[ind]->isDestroyed())
	  return items[ind];
  }
  return Item::null_item;
}

void Board::removeItemAt (const unsigned int ind)
{
  std::unique_ptr<Item>& item(getItemAt(ind));
  if (item)
  {
	last_move_score += item->getValue();

	item->destroy_callback(*this, ind);
	item->destroy();
  }
}

unsigned int Board::getTotalSize () const
{
  return total_size;
}

unsigned int Board::getRowsCount () const
{
  return rows;
}

unsigned int Board::getColsCount () const
{
  return cols;
}

unsigned int Board::getCellSize () const
{
  return cell_size;
}

const sf::Rect<unsigned int>& Board::getDimensions() const
{
  return dimensions;
}

const sf::Sprite &Board::getBoardShape() const
{
  return board_shape;
}

unsigned Board::posToInd (const sf::Vector2u pos) const
{
  return (pos.x - dimensions.left) / cell_size + (pos.y - dimensions.top) / cell_size * cols;
}

unsigned Board::posToInd (const unsigned x, const unsigned y) const
{
  return (x - dimensions.left) / cell_size + (y - dimensions.top) / cell_size * cols;
}

void Board::swapItems (const unsigned int src, const unsigned int dest)
{
  std::swap(items[src], items[dest]);
  items[src]->setSwap(true);
  items[dest]->setSwap(true);
}

void Board::changeItem(unsigned int x, unsigned int y)
{
  unsigned int pos(x + y*cols);

  items[pos] = std::move(item_generator->generate());
  items[pos]->create_callback(*this, pos);
}

bool Board::areNext(unsigned int source, unsigned int target) const
{
  unsigned int tmp(source % cols);

  if (target >= total_size)
	return false;
  if (tmp == 0 && source == target + 1)
	return false;
  if (tmp == cols - 1 && source == target - 1)
	return false;

  return target == source - 1
	  || target == source + 1
	  || target == source + cols
	  || target == source - cols;
}

void Board::fillBlanks()
{
  for (unsigned int i(0) ; i < total_size ; ++i)
  {
	if (items[i]->isDestroyed())
	{
	  changeItem(i % cols, i / cols);
	}
  }
}

void Board::update ()
{
  do {
	fillBlanks();

	updateRowsAndCols();

	for (unsigned int i = 0; i < cols; ++i)
	  applyGravity(i);

  } while (!isStable());

  if (combo)
	game.setCombo(combo);

  game.addPoints(last_move_score);
  last_move_score = 0;

  updatePositions();
}

void Board::updateLine (const unsigned int begin, const unsigned int end, const unsigned int offset)
{
  Item *previous_item(items[begin].get());
  unsigned int cpt(0);
  unsigned int current(begin + offset);

  while (current < end)
  {
	Item *current_item(items[current].get());
	if (previous_item && current_item)
	{
	  if (*previous_item != *current_item)
	  {
		if (cpt >= 2)
		{
		  markForRemoval(current - offset*(cpt+1), current - offset, offset);
		  if (cpt >3)
		  {
			item_generator->forceGenerationOf(previous_item->getName() + "Special");
		  }
		  else if (cpt > 2)
		  {
			if (offset == 1)
			{
			  item_generator->forceGenerationOf(previous_item->getName() + "Horizontal");
			}
			else
			{
			  item_generator->forceGenerationOf(previous_item->getName() + "Vertical");
			}
		  }
		}
		cpt = 0;
	  }
	  else
	  {
		++cpt;
		if (cpt >= 2 && current == end - offset)
		{
		  markForRemoval(current - offset*(cpt), current, offset);
		  if (cpt >3)
		  {
			item_generator->forceGenerationOf(previous_item->getName() + "Special");
		  }
		  else if (cpt > 2)
		  {
			if (offset == 1)
			{
			  item_generator->forceGenerationOf(previous_item->getName() + "Horizontal");
			}
			else
			{
			  item_generator->forceGenerationOf(previous_item->getName() + "Vertical");
			}
		  }
		}
	  }
	}
	else {cpt = 0;}

	current += offset;
	previous_item = current_item;
  }
}

void Board::markForRemoval (unsigned int begin, const unsigned int end, const unsigned int offset)
{
  ++combo;

  while (begin <= end)
  {
	last_move_score += items[begin]->getValue();

	items[begin]->destroy_callback(*this, begin);
	items[begin]->destroy();

	begin += offset;
  }
}

bool Board::isStable () const
{
  return !std::any_of(items.cbegin(), items.cend(), [](const std::unique_ptr<Item> &item)
  {
	return item->isDestroyed();
  });
}

void Board::updateRowsAndCols ()
{
  if (!areItemsMoving())
  {
	for (unsigned int i(0) ; i < rows; ++i)
	{
	  updateLine(i * cols, (i+1) * cols, 1);
	}

	for (unsigned int i(0) ; i < cols; ++i)
	{
	  updateLine(i, cols * rows + i, cols);
	}
  }
}

bool Board::checkMovement (unsigned ind_first_item, unsigned ind_second_item) const
{
  if (validNeighbor(ind_first_item, ind_second_item)
	  || validNeighbor(ind_second_item, ind_first_item))
  {
	return true;
  }

  return false;
}

bool Board::validNeighbor (int source, int dest) const
{
  const Item & item(*items[source]);
  std::array<unsigned, 4> counts{{0, 0, 0, 0}};
  int dest_tmp(dest - 1);

  // Compte le nombre d'éléments équivalents vers la gauche
  while (dest_tmp >= 0 && dest_tmp % cols != cols - 1 && item == *items[dest_tmp] && dest_tmp != source)
  {
	++counts[0];
	--dest_tmp;
  }

  // Compte le nombre d'éléments équivalents vers la droite
  dest_tmp = dest + 1;
  while (dest_tmp % cols != 0 && item == *items[dest_tmp] && dest_tmp != source)
  {
	++counts[1];
	++dest_tmp;
  }

  // Compte le nombre d'éléments équivalents vers le haut
  dest_tmp = dest - cols;
  while (dest_tmp >= 0 && item == *items[dest_tmp] && dest_tmp != source)
  {
	++counts[2];
	dest_tmp -= cols;
  }

  // Compte le nombre d'éléments équivalents vers le bas
  dest_tmp = dest + cols;
  while (dest_tmp < (int)total_size && item == *items[dest_tmp] && dest_tmp != source)
  {
	++counts[3];
	dest_tmp += cols;
  }

  // Retourne si la somme des horizontaux ou des verticaux est supérieure à 2
  return counts[0] + counts[1] >= 2 || counts[2] + counts[3] >= 2;
}

void Board::applyGravity (const unsigned int col)
{
  for (int i(col + cols * (rows-1)) ; i >= 0; i -= cols)
  {
	// Si l'item i est supprimé, on doit le remplacer par l'item au dessus le plus proche
	if (items[i]->isDestroyed())
	{
	  int next(i - cols);

	  // si next < 0, alors il n'y a pas d'item au dessus de i
	  while (next >= 0 && items[next]->isDestroyed())
	  {
		next -= cols;
	  }

	  if (next >= 0)
	  {
		swapItems(i, next);
	  }
	}
  }
}

void Board::updatePositions ()
{
  for (unsigned int j = 0; j < rows; ++j)
  {
	for (unsigned int i = 0; i < cols; ++i)
	{
	  if (items[i + j*cols].get())
	  {
		float new_x(i * cell_size + cell_size / 2 + dimensions.left);
		float new_y(j * cell_size + cell_size / 2 + dimensions.top);

		items[i + j*cols]->goTo(sf::Vector2f(new_x, new_y));
	  }
	}
  }
}

bool Board::areItemsMoving()
{
  return std::any_of(items.cbegin(), items.cend(), [](const std::unique_ptr<Item>& i)
  { return i->isMoving(); });
}

void Board::resetLastScore()
{
  combo = 0;
  last_move_score = 0;
}
