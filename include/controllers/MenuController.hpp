#ifndef MENU_CONTROLLER_HPP
#define MENU_CONTROLLER_HPP

#include "Controller.hpp"

class Game;

///
/// \brief Contrôleur du menu de pause
/// \see Controller
///
class MenuController: public Controller
{
  public:
	MenuController(Game *game);
	virtual ~MenuController();

	virtual void onClick (const float x, const float y);
	virtual void onClickRelease (const float x, const float y);
	virtual void onLostFocus ();
	virtual void onGainedFocus ();

	///
	/// \brief Lorsqu'on choisit de retourner au jeu
	///
	void onRetour();

	///
	/// \brief Lorsqu'on choisit de quitter le jeu
	///
	void onQuitter();

  private:
	Game *game;
};

#endif // MENU_CONTROLLER_HPP
