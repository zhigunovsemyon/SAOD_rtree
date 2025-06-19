#pragma once
#include "rect.h"
#include <memory>
#include <vector>

// Узел R-дерева
class RTreeNode {
public:
	Rect bounds;
	bool isLeaf;
	std::vector<std::shared_ptr<RTreeNode>> children;
	std::vector<Rect> objects; // Только для листовых узлов

	RTreeNode(bool leaf = false) : isLeaf(leaf) {}

	// Добавляет дочерний узел (для нелистовых узлов)
	void addChild(std::shared_ptr<RTreeNode> child);

	// Добавляет объект (для листовых узлов)
	void addObject(Rect const & obj);

	// Удаляет дочерний узел
	void removeChild(std::shared_ptr<RTreeNode> child);

	// Удаляет объект
	bool removeObject(Rect const & obj);

	// Пересчитывает границы узла
	void recalculateBounds();
};

// R-дерево
class RTree {
private:
	std::shared_ptr<RTreeNode> root;
	size_t maxChildren;
	size_t minChildren;

	// Вспомогательная функция для выбора поддерева для вставки
	std::shared_ptr<RTreeNode> chooseLeaf(std::shared_ptr<RTreeNode> node, Rect const & obj);

	// Разделение узла при переполнении
	void splitNode(std::shared_ptr<RTreeNode> node);

	// Поиск родителя узла
	std::shared_ptr<RTreeNode> findParent(std::shared_ptr<RTreeNode> current, std::shared_ptr<RTreeNode> child);

	// Рекурсивный поиск объектов в регионе
	void searchInRegion(std::shared_ptr<RTreeNode> node, Rect const & region, std::vector<Rect> & results);

	// Рекурсивный поиск точного совпадения
	bool searchExact(std::shared_ptr<RTreeNode> node, Rect const & obj);

	// Рекурсивное удаление объекта
	bool removeObject(std::shared_ptr<RTreeNode> node, Rect const & obj);

	// Обработка нехватки элементов в узле
	void handleUnderflow(std::shared_ptr<RTreeNode> node);

	// Поиск ближайшего соседа
	void nearestNeighbor(std::shared_ptr<RTreeNode> node, Point_2D const & point, Rect & best, double & bestDist);

public:
	RTree(size_t maxChildren = 4);

	// Вставка объекта
	void insert(Rect const & obj);

	// Удаление объекта
	bool remove(Rect const & obj) { return removeObject(root, obj); }

	// Поиск по региону
	std::vector<Rect> searchRegion(Rect const & region);

	// Поиск точного совпадения
	bool searchExact(Rect const & obj) { return searchExact(root, obj); }

	// Поиск ближайшего соседа
	Rect nearestNeighbor(Point_2D const & point);
};
