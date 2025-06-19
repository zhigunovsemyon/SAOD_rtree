#pragma once
#include <cmath>
#include <ostream>

struct Point_2D {
	double x, y;

	bool operator==(Point_2D const & oth) const { return (this->x == oth.x) && (this->y == oth.y); }

	bool operator!=(Point_2D const & oth) const { return !(*this == oth); }

	Point_2D(double n) : x{n}, y{n} {}

	Point_2D(double x, double y) : x{x}, y{y} {}

	friend std::ostream & operator<< (std::ostream & ost, Point_2D const & point)
	{
		return ost << '(' << point.x << ',' << point.y << ')';
	}
};

// Структура для представления точки или прямоугольника
struct Rect {
	Point_2D min, max;

	Rect() : min{std::numeric_limits<double>::min()}, max{std::numeric_limits<double>::lowest()} {}

	Rect(double x1, double y1, double x2, double y2)
		: min{std::min(x1, x2), std::min(y1, y2)}, max{std::max(x1, x2), std::max(y1, y2)}
	{
	}

	bool operator!=(Rect const & oth) const { return (this->min != oth.min) || (this->max != oth.max); }

	bool operator==(Rect const & oth) const { return !(oth != *this); }

	// Расширяет прямоугольник чтобы включить другой прямоугольник
	void expand(Rect const & other)
	{
		min.x = std::min(min.x, other.min.x), min.y = std::min(min.y, other.min.y);
		max.x = std::max(max.x, other.max.x), max.y = std::max(max.y, other.max.y);
	}

	// Вычисляет площадь прямоугольника
	double area() const { return (max.x - min.x) * (max.y - min.y); }

	// Вычисляет увеличение площади при расширении для другого прямоугольника
	double expansionArea(Rect const & other) const
	{
		double newMinX = std::min(min.x, other.min.x);
		double newMinY = std::min(min.y, other.min.y);
		double newMaxX = std::max(max.x, other.max.x);
		double newMaxY = std::max(max.y, other.max.y);
		return (newMaxX - newMinX) * (newMaxY - newMinY) - area();
	}

	// Проверяет пересечение с другим прямоугольником
	bool intersects(Rect const & other) const
	{
		return !(other.min.x > max.x || other.max.x < min.x || other.min.y > max.y || other.max.y < min.y);
	}

	// Проверяет содержит ли прямоугольник точку
	bool contains(Point_2D const & point) const
	{
		return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
	}

	// Проверяет содержит ли прямоугольник другой прямоугольник
	bool contains(Rect const & other) const
	{
		return other.min.x >= min.x && other.max.x <= max.x && other.min.y >= min.y && other.max.y <= max.y;
	}

	// Вычисляет расстояние до точки
	double distance(Point_2D const & point) const
	{
		double dx = std::max({min.x - point.x, 0.0, point.x - max.x});
		double dy = std::max({min.y - point.y, 0.0, point.y - max.y});
		return std::hypot(dx,dy);
	}

	friend std::ostream & operator<< (std::ostream & ost, Rect const & rect)
	{
		return ost << rect.min << '-' << rect.max;
	}
};


