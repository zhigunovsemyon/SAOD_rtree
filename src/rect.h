#pragma once
#include <ostream>

struct Point_2D {
	double x, y;

	bool operator==(Point_2D const & oth) const { return (this->x == oth.x) && (this->y == oth.y); }

	bool operator!=(Point_2D const & oth) const { return !(*this == oth); }

	Point_2D(double n) : x{n}, y{n} {}

	Point_2D(double x, double y) : x{x}, y{y} {}

	friend std::ostream & operator<<(std::ostream & ost, Point_2D const & point)
	{
		return ost << '(' << point.x << ',' << point.y << ')';
	}
};

// Структура для представления точки или прямоугольника
struct Rect {
	Point_2D min, max;

	Rect();

	Rect(Point_2D const & min, Point_2D const & max);

	Rect(double x1, double y1, double x2, double y2);

	bool operator!=(Rect const & oth) const { return (this->min != oth.min) || (this->max != oth.max); }

	bool operator==(Rect const & oth) const { return !(oth != *this); }

	// Расширяет прямоугольник чтобы включить другой прямоугольник
	void expand(Rect const & other);

	// Вычисляет площадь прямоугольника
	double area() const { return (max.x - min.x) * (max.y - min.y); }

	// Вычисляет увеличение площади при расширении для другого прямоугольника
	double expansionArea(Rect const & other) const;

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
	double distance(Point_2D const & point) const;

	friend std::ostream & operator<<(std::ostream & ost, Rect const & rect)
	{
		return ost << rect.min << '-' << rect.max;
	}
};
