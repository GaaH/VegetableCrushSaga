#include <SFML/Window/Event.hpp>

#include "views/GameView.hpp"
#include "controllers/GameController.hpp"

#include "models/Game.hpp"
#include "models/items/Item.hpp"

#include "Globals.hpp"

GameView::GameView(Controller *controller, Game *game, sf::RenderWindow &window) :
  View(controller),
  game(game),
  window(window),
  combo_mode(false),
  levelup_mode(false)
{
  initializeGUI();
}


GameView::~GameView()
{
}

void GameView::clear()
{
  window.clear(sf::Color::White);
}

void GameView::draw()
{
  window.draw(grass_sprite);
  window.draw(game->getBoard().getBoardShape());
  
  auto& items = game->getBoard().getItems();
  for (auto& item: items)
  {
	if (item.get())
	  window.draw(*item);
  }

  window.draw(points_text);
  window.draw(combo_text);
  window.draw(counter_text);
  window.draw(level_text);

  showComboText();
  showLevelupText();
}

void GameView::display()
{
  window.display();
}

void GameView::sendEvents()
{
  sf::Event event;
  while (window.pollEvent(event))
  {
	switch (event.type)
	{
	  case sf::Event::Closed:
		static_cast<GameController*>(controller)->onQuit();
		running = false;
		break;

	  case sf::Event::KeyPressed:
		if (event.key.code == sf::Keyboard::Escape)
		{
		  static_cast<GameController*>(controller)->onEscape();
		  running = false;
		}
		break;

	  case sf::Event::MouseButtonPressed:
		if (event.mouseButton.button == sf::Mouse::Left)
		{
		  controller->onClick(event.mouseButton.x, event.mouseButton.y);
		}
		break;

	  case sf::Event::MouseButtonReleased:
		if (event.mouseButton.button == sf::Mouse::Left)
		{
		  controller->onClickRelease(event.mouseButton.x, event.mouseButton.y);
		}
		break;

	  case sf::Event::LostFocus:
		controller->onLostFocus();
		break;

	  case sf::Event::GainedFocus:
		controller->onGainedFocus();
		break;

	  default:
		break;
	}
  }
}

void GameView::loop()
{
  unsigned int fps(0);
  sf::Clock clock;

  while (running)
  {
	fps = 1.f / clock.getElapsedTime().asSeconds();
	window.setTitle("Vegetable Crush Saga - [FPS: " + std::to_string(fps) + "]");
	clock.restart();

	sendEvents();

	game->updateGame();

	if (game->isGameOver())
	{
	  globals.captureScreen();
	  globals.setCurrentView("End");
	  running = false;
	}

	points_text.setString("Points: " + std::to_string(game->getScore()) + "/" + std::to_string(game->getGoalScore()));
	combo_text.setString("Combo: x" + std::to_string(game->getCombo()));
	counter_text.setString("Coups: " + static_cast<std::string>(game->getCondition()));
	level_text.setString("Level " + std::to_string(game->getLevel()));

	clear();
	draw();
	display();
  }
}

void GameView::showComboText()
{
  static unsigned int prev_combo = 0;

  unsigned int current_combo = game->getCombo();

  if (current_combo > 1 && prev_combo != current_combo)
  {
	popup_text.setCharacterSize(200.f * (1.5f + (((current_combo < 30) ? current_combo : 30) / 10.f)));

	popup_text.setString("x" + std::to_string(current_combo));
	popup_text.setColor(sf::Color(0, 0, 0, 255));


	popup_text.setPosition(window.getSize().x / 2 - popup_text.getGlobalBounds().width / 2,
						   window.getSize().y / 2 - popup_text.getGlobalBounds().height);

	combo_mode = true;
	combo_clock.restart();

  }

  prev_combo = current_combo;

  // Combo
  if (combo_mode)
  {
	if (combo_clock.getElapsedTime().asSeconds() < 3)
	{
	  window.draw(popup_text);

	  sf::Color color = popup_text.getColor();
	  float delta = (float)(1.f/60.f * 70.f);
	  color.a -= (color.a - delta < 0 ? 0 : delta);

	  popup_text.setColor(color);
	}
	else
	{
	  combo_mode = false;
	}
  }
}

void GameView::showLevelupText()
{
  static unsigned int prev_level = 1;
  unsigned int current_level = game->getLevel();

  if (prev_level != current_level)
  {
	levelup_text.setCharacterSize(150.f);
	levelup_text.setColor(sf::Color::Red);

	levelup_text.setString("LEVEL UP !");

	levelup_text.setPosition(window.getSize().x / 2 - levelup_text.getGlobalBounds().width / 2,
							 window.getSize().y / 4 - levelup_text.getGlobalBounds().height);

	levelup_mode = true;
	levelup_clock.restart();
  }

  prev_level = current_level;

  // Levelup
  if (levelup_mode)
  {
	if (levelup_clock.getElapsedTime().asSeconds() < 6)
	{
	  window.draw(levelup_text);

	  sf::Color color = levelup_text.getColor();
	  float delta = (float)(1.f/60.f * 35.f);
	  color.a -= (color.a - delta < 0 ? 0 : delta);

	  levelup_text.setColor(color);
	}
	else
	{
	  levelup_mode = false;
	}
  }
}

void GameView::initializeGUI()
{
  auto font = globals.getFontManager().getResource("wombo_font");

  popup_text.setFont(*font);
  popup_text.setColor(sf::Color(0, 0, 0, 255));
  popup_text.setCharacterSize(200);
  popup_text.setPosition(window.getSize().x / 2, window.getSize().y / 2);

  levelup_text.setFont(*font);
  levelup_text.setColor(sf::Color::Red);
  levelup_text.setCharacterSize(150);
  levelup_text.setPosition(window.getSize().x / 2, window.getSize().y / 4);

  points_text.setFont(*font);
  points_text.setColor(sf::Color::White);
  points_text.setPosition(20, 0);
  points_text.setCharacterSize(30);
  points_text.setString("Score : 0/0");

  combo_text.setFont(*font);
  combo_text.setColor(sf::Color::White);
  combo_text.setPosition(20, 30);
  combo_text.setCharacterSize(30);
  combo_text.setString("Combo : x0");

  counter_text.setFont(*font);
  counter_text.setColor(sf::Color::White);
  counter_text.setPosition(20, 60);
  counter_text.setCharacterSize(30);
  counter_text.setString("Coups : " + static_cast<std::string>(game->getCondition()));

  level_text.setFont(*font);
  level_text.setColor(sf::Color::White);
  level_text.setPosition(window.getSize().x - 150, 0);
  level_text.setCharacterSize(30);
  level_text.setString("Level 1");

  auto grass_texture = globals.getTextureManager().getResource("window_background");
  grass_texture->setRepeated(true);

  grass_sprite.setTexture(*grass_texture);
  grass_sprite.setPosition(0, 0);
  grass_sprite.setColor(sf::Color(180, 180, 180));

  float grass_width = (float)window.getSize().x / grass_texture->getSize().x;
  float grass_height = (float)window.getSize().y / grass_texture->getSize().y;

  sf::IntRect texture_rect = grass_sprite.getTextureRect();
  texture_rect.width *= grass_width;
  texture_rect.height *= grass_height;

  grass_sprite.setTextureRect(texture_rect);
}
