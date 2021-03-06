/*******************************************************************************
   RectSupport.cpp
*******************************************************************************/

#include "RectSupport.h"

namespace br {
namespace uerj {
namespace eng {
namespace efoto {

RectSupport::RectSupport(unsigned int parNcols, unsigned int parNrows)
{
	ncols=parNcols;
	nrows=parNrows;
}

RectSupport::~RectSupport()
{

}

std::string RectSupport::objectType(void)
{
	return "RectSupport";
}

unsigned int RectSupport::getCols() const
{
	return ncols;
}

size_t RectSupport::getRows() const
{
	return nrows;
}

} // namespace efoto
} // namespace eng
} // namespace uerj
} // namespace br
