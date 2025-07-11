#include "rtree.h"
#include "rect.h"

RTreeNode::RTreeNode(bool leaf)
	: bounds{std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest()}, isLeaf(leaf)
{
}

void RTreeNode::recalculateBounds()
{
	bounds = Rect{std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest()};
	if (isLeaf) {
		for (auto const & obj : objects) {
			bounds.expand(obj);
		}
	} else {
		for (auto const & child : children) {
			bounds.expand(child->bounds);
		}
	}
}

bool RTreeNode::removeObject(Rect const & obj)
{
	auto it = std::find(objects.begin(), objects.end(), obj);
	if (it != objects.end()) {
		objects.erase(it);
		recalculateBounds();
		return true;
	}
	return false;
}

void RTreeNode::removeChild(std::shared_ptr<RTreeNode> child)
{
	auto it = std::find(children.begin(), children.end(), child);
	if (it != children.end()) {
		children.erase(it);
		recalculateBounds();
	}
}

void RTreeNode::addObject(Rect const & obj)
{
	objects.push_back(obj);
	bounds.expand(obj);
}

void RTreeNode::addChild(std::shared_ptr<RTreeNode> child)
{
	children.push_back(child);
	bounds.expand(child->bounds);
}

void RTree::searchInRegion(std::shared_ptr<RTreeNode> node, Rect const & region, std::vector<Rect> & results)
{
	if (!node->bounds.intersects(region)) {
		return;
	}

	if (node->isLeaf) {
		for (auto const & obj : node->objects) {
			if (region.intersects(obj)) {
				results.push_back(obj);
			}
		}
	} else {
		for (auto const & child : node->children) {
			searchInRegion(child, region, results);
		}
	}
}

RTree::RTree(size_t maxChildren) : maxChildren(maxChildren), minChildren(maxChildren / 2)
{
	root = std::make_shared<RTreeNode>(true);
}

std::vector<Rect> RTree::searchRegion(Rect const & region)
{
	std::vector<Rect> results;
	searchInRegion(root, region, results);
	return results;
}

Rect RTree::nearestNeighbor(Point_2D const & point)
{
	Rect best{std::numeric_limits<double>::min(), std::numeric_limits<double>::lowest()};
	double bestDist = std::numeric_limits<double>::max();
	nearestNeighbor(root, point, best, bestDist);
	return best;
}

void RTree::insert(Rect const & obj)
{
	auto leaf = chooseLeaf(root, obj);
	leaf->addObject(obj);

	// Если узел переполнен, разделяем его
	if (leaf->objects.size() > maxChildren) {
		splitNode(leaf);
	} else {
		// Иначе обновляем границы родителей
		auto node = leaf;
		auto parent = findParent(root, node);
		while (parent) {
			parent->recalculateBounds();
			node = parent;
			parent = findParent(root, node);
		}
	}
}

void RTree::nearestNeighbor(std::shared_ptr<RTreeNode> node, Point_2D const & point, Rect & best, double & bestDist)
{
	if (!node)
		return;

	double nodeDist = node->bounds.distance(point);
	if (nodeDist >= bestDist)
		return;

	if (node->isLeaf) {
		for (auto const & obj : node->objects) {
			double dist = obj.distance(point);
			if (dist < bestDist) {
				bestDist = dist;
				best = obj;
			}
		}
	} else {
		// Сортируем детей по расстоянию до точки
		std::vector<std::shared_ptr<RTreeNode>> sortedChildren = node->children;
		std::sort(sortedChildren.begin(), sortedChildren.end(),
			  [point](std::shared_ptr<RTreeNode> const & a, std::shared_ptr<RTreeNode> const & b) {
				  return a->bounds.distance(point) < b->bounds.distance(point);
			  });

		for (auto const & child : sortedChildren) {
			nearestNeighbor(child, point, best, bestDist);
		}
	}
}

void RTree::handleUnderflow(std::shared_ptr<RTreeNode> node)
{
	auto parent = findParent(root, node);
	if (!parent) {
		// Если это корень
		if (!node->isLeaf && node->children.size() == 1) {
			root = node->children[0];
		}
		return;
	}

	// Попробуем перераспределить элементы с соседями
	for (auto & sibling : parent->children) {
		if (sibling == node)
			continue;

		if (node->isLeaf && sibling->objects.size() > minChildren) {
			// Перемещаем объект из sibling в node
			node->addObject(sibling->objects.back());
			sibling->objects.pop_back();
			sibling->recalculateBounds();
			node->recalculateBounds();
			parent->recalculateBounds();
			return;
		} else if (!node->isLeaf && sibling->children.size() > minChildren) {
			// Перемещаем ребенка из sibling в node
			node->addChild(sibling->children.back());
			sibling->children.pop_back();
			sibling->recalculateBounds();
			node->recalculateBounds();
			parent->recalculateBounds();
			return;
		}
	}

	// Если перераспределение невозможно, объединяем узлы
	for (auto it = parent->children.begin(); it != parent->children.end(); ++it) {
		if (*it == node) {
			auto sibling = (it == parent->children.begin()) ? *(it + 1) : *(it - 1);

			if (node->isLeaf) {
				sibling->objects.insert(sibling->objects.end(), node->objects.begin(),
							node->objects.end());
			} else {
				sibling->children.insert(sibling->children.end(), node->children.begin(),
							 node->children.end());
			}

			parent->children.erase(it);
			sibling->recalculateBounds();
			parent->recalculateBounds();

			// Если у родителя стало слишком мало детей
			if (parent != root && parent->children.size() < minChildren) {
				handleUnderflow(parent);
			} else if (parent == root && parent->children.size() == 1) {
				root = parent->children[0];
			}
			return;
		}
	}
}

bool RTree::removeObject(std::shared_ptr<RTreeNode> node, Rect const & obj)
{
	if (!node->bounds.intersects(obj)) {
		return false;
	}

	if (node->isLeaf) {
		return node->removeObject(obj);
	} else {
		for (auto & child : node->children) {
			if (removeObject(child, obj)) {
				// Если после удаления у ребенка стало слишком мало элементов
				if ((child->isLeaf && child->objects.size() < minChildren) ||
				    (!child->isLeaf && child->children.size() < minChildren)) {
					handleUnderflow(child);
				}
				node->recalculateBounds();
				return true;
			}
		}
		return false;
	}
}

bool RTree::searchExact(std::shared_ptr<RTreeNode> node, Rect const & obj)
{
	if (!node->bounds.intersects(obj)) {
		return false;
	}

	if (node->isLeaf) {
		for (auto const & o : node->objects) {
			if (o.min.x == obj.min.x && o.min.y == obj.min.y && o.max.x == obj.max.x &&
			    o.max.y == obj.max.y) {
				return true;
			}
		}
		return false;
	} else {
		for (auto const & child : node->children) {
			if (searchExact(child, obj)) {
				return true;
			}
		}
		return false;
	}
}

std::shared_ptr<RTreeNode> RTree::findParent(std::shared_ptr<RTreeNode> current, std::shared_ptr<RTreeNode> child)
{
	if (current->isLeaf) {
		return nullptr;
	}

	for (auto const & c : current->children) {
		if (c == child) {
			return current;
		}
		auto parent = findParent(c, child);
		if (parent) {
			return parent;
		}
	}

	return nullptr;
}

void RTree::splitNode(std::shared_ptr<RTreeNode> node)
{
	// Простая реализация - линейный split (можно улучшить)
	auto newNode = std::make_shared<RTreeNode>(node->isLeaf);

	// Сортируем объекты/детей по оси X
	if (node->isLeaf) {
		std::sort(node->objects.begin(), node->objects.end(),
			  [](Rect const & a, Rect const & b) { return a.min.x < b.min.x; });

		ssize_t splitPos = node->objects.size() / 2;
		newNode->objects.assign(node->objects.begin() + splitPos, node->objects.end());
		node->objects.erase(node->objects.begin() + splitPos, node->objects.end());
	} else {
		std::sort(node->children.begin(), node->children.end(),
			  [](std::shared_ptr<RTreeNode> const & a, std::shared_ptr<RTreeNode> const & b) {
				  return a->bounds.min.x < b->bounds.min.x;
			  });

		ssize_t splitPos = node->children.size() / 2;
		newNode->children.assign(node->children.begin() + splitPos, node->children.end());
		node->children.erase(node->children.begin() + splitPos, node->children.end());
	}

	// Пересчитываем границы
	node->recalculateBounds();
	newNode->recalculateBounds();

	// Если это корень, создаем новый корень
	if (node == root) {
		auto newRoot = std::make_shared<RTreeNode>();
		newRoot->addChild(node);
		newRoot->addChild(newNode);
		root = newRoot;
	} else {
		// Иначе добавляем новый узел в родителя
		auto parent = findParent(root, node);
		parent->addChild(newNode);

		// Если родитель переполнен, разделяем его
		if (parent->children.size() > maxChildren) {
			splitNode(parent);
		}
	}
}

std::shared_ptr<RTreeNode> RTree::chooseLeaf(std::shared_ptr<RTreeNode> node, Rect const & obj)
{
	if (node->isLeaf) {
		return node;
	}

	// Выбираем дочерний узел с минимальным увеличением площади
	double minExpansion = std::numeric_limits<double>::max();
	std::shared_ptr<RTreeNode> bestChild = nullptr;

	for (auto const & child : node->children) {
		double expansion = child->bounds.expansionArea(obj);
		if (expansion < minExpansion ||
		    (expansion == minExpansion && child->bounds.area() < bestChild->bounds.area())) {
			minExpansion = expansion;
			bestChild = child;
		}
	}

	return chooseLeaf(bestChild, obj);
}
