#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Clock.hpp>

#include "Globals.hpp"

#include "models/Game.hpp"
#include "models/Board.hpp"

#include "models/items/Item.hpp"

#include "models/conditions/CounterCondition.hpp"

Game::Game(unsigned int x, unsigned int y, unsigned int width, unsigned int height,
		   unsigned int cell_size) :
  board(x, y, width, height, cell_size, *this),
  first_selected(false),
  second_selected(false),
  active_input(true),
  score(0),
  combo(0),
  level(1),
  score_goal(0),
  near_gameover(false),
  gameover(false),
  condition(nullptr)
{
  this->initializeGame();
}

Game::~Game()
{
}

void Game::setClickPosition(const float x, const float y)
{
  if (!board.getDimensions().contains(x, y))
  {
	return;
  }

  sf::Vector2u selected(static_cast<unsigned int>(x),
						static_cast<unsigned int>(y));

  unsigned ind_in_board(board.posToInd(selected));

  if (ind_in_board < board.getTotalSize())
  {
	if (!first_selected)
	{
	  first_item = selected;
	  first_selected = true;
	}

	else
	{
	  second_item = selected;
	  second_selected = true;
	}
  }
}

void Game::setReleasePosition(const float x, const float y)
{
  if (!board.getDimensions().contains(x, y) || !first_selected)
  {
	return;
  }

  sf::Vector2u selected(static_cast<unsigned int>(x),
						static_cast<unsigned int>(y));
  unsigned cell_size(board.getCellSize());
  unsigned ind_in_board(board.posToInd(selected));

  if (ind_in_board < board.getTotalSize())
  {
	if (ind_in_board != board.posToInd(first_item) && !second_selected)
	{
	  // Alors, alors ...
	  // Ici, on récupère la position du deuxième item selectionné lors d'un drag&drop
	  // Le principe, c'est de recupérer le vecteur formé par les positions des deux items (delta)
	  // Et, grace a ce vecteur, on peut connaitre la position de l'item qu'il faut sélectionner
	  sf::Vector2i delta(std::move(selected - first_item));

	  // Devrait être dans Board
	  sf::Vector2u offset(1, board.getColsCount());

	  int dir_x((std::abs(delta.x) < std::abs(delta.y)) ? 0 : static_cast<int>(std::abs(delta.x)) / delta.x);
	  int dir_y((dir_x == 0) ? static_cast<int>(std::abs(delta.y)) / delta.y : 0);

	  second_item = first_item + sf::Vector2u(cell_size * dir_x, cell_size * dir_y);
	  second_selected = true;
	}
  }

  // Si on drag en dehors de la fenêtre
  else
  {
	first_selected = false;
  }
}

void Game::executeMovement()
{
  if (first_selected)
  {
	if (second_selected)
	{
	  unsigned ind_first_item(board.posToInd(first_item));
	  unsigned ind_second_item(board.posToInd(second_item));

	  if (board.areNext(ind_first_item, ind_second_item))
	  {
		if (board.checkMovement(ind_first_item, ind_second_item))
		{
		  registerMove(ind_first_item, ind_second_item);
		  condition->step();
		}
		else
		{
		  registerMove(ind_first_item, ind_second_item);
		  registerMove(ind_second_item, ind_first_item);
		}

		board.resetLastScore();
	  }
	  first_selected = second_selected = false;
	}
  }
}

void Game::registerMove(unsigned int source, unsigned int target)
{
  move_registered.push(sf::Vector2u(source, target));
}

void Game::updateGame()
{
  active_input = !board.areItemsMoving();

  if (score > score_goal)
  {
	score_goal *= goal_coef;
	level += 1;

	int tries = std::atoi(static_cast<std::string>(*condition).c_str());

	condition->reset(tries + tries_added * level);
  }

  if (!move_registered.empty())
  {
	sf::Vector2u current_move(move_registered.top());

	if (active_input)
	{
	  move_registered.pop();
	  board.swapItems(current_move.x, current_move.y);
	}

	board.updatePositions();
  }

  board.update();

  if (*condition && !near_gameover)
  {
	near_gameover = true;
	endClock.restart();
  }

  if (near_gameover && endClock.getElapsedTime().asSeconds() > 1)
  {
	if (!board.areItemsMoving() && board.isStable())
	  gameover = true;
  }
}

void Game::addPoints(unsigned int value)
{
  score += value * combo;
}

void Game::setCombo(unsigned int combo)
{
  this->combo = combo;
}

void Game::setActive (bool value)
{
  active_input = value;
}

Board& Game::getBoard ()
{
  return board;
}

unsigned int Game::getScore() const
{
  return score;
}

unsigned int Game::getCombo() const
{
  return combo;
}

unsigned int Game::getGoalScore() const
{
  return score_goal;
}

unsigned int Game::getLevel() const
{
  return level;
}

Condition& Game::getCondition ()
{
  return *condition;
}

const Condition& Game::getCondition () const
{
  return *condition;
}

bool Game::isActive ()
{
  return active_input;
}

bool Game::isGameOver()
{
  return gameover;
}

void Game::initializeGame()
{
  try
  {
	YAML::Node config(YAML::LoadFile("assets/config.yml"));
	config = config["game"]["gameplay"]["parameters"];

	score_goal = config["base_goal"].as<unsigned int>();
	goal_coef = config["goal_coef"].as<float>();
	tries_added = config["tries_added"].as<unsigned int>();

	condition.reset(new CounterCondition(config["base_tries"].as<unsigned int>()));
  }
  catch (YAML::ParserException e)
  {
	std::cerr << "Le fichier de config n'est pas aux normes !" << std::endl;
	std::cerr << e.what() << std::endl;
	exit(EXIT_FAILURE);
  }
}
