#ifndef INCLUDED_SPECIALITEMHORIZONTAL_H
#define INCLUDED_SPECIALITEMHORIZONTAL_H

#include "Item.hpp"

class Board;

///
/// \brief Item spécial horizontal
/// \see Item
///
class SpecialItemHorizontal : public Item
{
public:
	SpecialItemHorizontal(const std::string &name, const unsigned int points);
	virtual ~SpecialItemHorizontal ();

	virtual void create_callback(Board & board, unsigned int pos);
	virtual void destroy_callback(Board & board, unsigned int pos);

private:
	void explode(Board & board, unsigned pos, int offset);
};

#endif /* INCLUDED_SPECIALITEMHORIZONTAL_H */
