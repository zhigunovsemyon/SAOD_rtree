#include "rect.h"
#include <cmath>

Rect::Rect() : min{std::numeric_limits<double>::min()}, max{std::numeric_limits<double>::lowest()} {}

Rect::Rect(Point_2D const & min, Point_2D const & max) : min{min}, max{max} {}

Rect::Rect(double x1, double y1, double x2, double y2)
	: min{std::min(x1, x2), std::min(y1, y2)}, max{std::max(x1, x2), std::max(y1, y2)}
{
}

double Rect::distance(Point_2D const & point) const
{
	double dx = std::max({min.x - point.x, 0.0, point.x - max.x});
	double dy = std::max({min.y - point.y, 0.0, point.y - max.y});
	return std::hypot(dx, dy);
}

double Rect::expansionArea(Rect const & other) const
{
	double newMinX = std::min(min.x, other.min.x);
	double newMinY = std::min(min.y, other.min.y);
	double newMaxX = std::max(max.x, other.max.x);
	double newMaxY = std::max(max.y, other.max.y);
	return (newMaxX - newMinX) * (newMaxY - newMinY) - area();
}

void Rect::expand(Rect const & other)
{
	min.x = std::min(min.x, other.min.x), min.y = std::min(min.y, other.min.y);
	max.x = std::max(max.x, other.max.x), max.y = std::max(max.y, other.max.y);
}
