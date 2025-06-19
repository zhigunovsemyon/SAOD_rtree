#include "rect.h"
#include "rtree.h"
#include <iostream>

int main()
{
	RTree tree;

	// Добавление объектов
	tree.insert(Rect(1, 1, 2, 2));
	tree.insert(Rect(3, 3, 4, 4));
	tree.insert(Rect(5, 5, 6, 6));
	tree.insert(Rect(7, 7, 8, 8));
	tree.insert(Rect(9, 9, 10, 10));

	// Поиск по региону
	Rect region{2, 2, 5, 5};
	std::cout << "Objects in region " << region << ":\n";
	auto results = tree.searchRegion(region);
	for (auto const & rect : results) {
		std::cout << rect << '\n';
	}

	// Поиск точного совпадения
	Rect exact(3, 3, 4, 4);
	std::cout << "Exact search for " << exact << ": " << (tree.searchExact(exact) ? "Found" : "Not found") << "\n";

	// Поиск ближайшего соседа
	Point_2D point{4.5, 4.5};
	auto nearest = tree.nearestNeighbor(point);
	std::cout << "Nearest to " << point << ": " << nearest << '\n';

	// Удаление объекта
	tree.remove(exact);
	std::cout << "After removal, exact search for " << exact << ": "
		  << (tree.searchExact(exact) ? "Found" : "Not found") << "\n";

	return 0;
}
