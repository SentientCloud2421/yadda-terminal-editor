#pragma once

#include "logger.hpp"

#include <cstddef>
#include <string>

template <unsigned N> struct BTreeNode {
	size_t keys[N] = {0};
	BTreeNode<N> *children[N] = {0};
	std::string buffer = "";
	
	size_t insert(std::string data, size_t index) {
		if (index > buffer.length()) {
			printf("index is outside of bounds!\n");
			return 0;
		} else if (index == buffer.length()) {
			buffer += data;
		} else {
			// split the node and go to the left
			return 0;
		}
		return data.length();
	}
	
	size_t remove(size_t length, size_t index) {
		return 0;
	}
	
	void print() {
		if (!children[0]) {
			printf("%s",buffer.c_str());
			return;
		}
		for (size_t i = 0; i < N; i++) {
			if (children[i]) {
				children[i]->print();
			}
		}
	}
};

template <unsigned N> struct BTree {
private:
	BTreeNode<N> root;

public:
	BTree() {}
	~BTree() {}
	
	size_t insert(std::string data, size_t index) {
		BTreeNode<N> *current = &root;
		while (current->children[0]) {
			for (size_t i = 0; i < N; i++) {
				if (index > current->keys[i]) {
					index -= current->keys[i];
				} else {
					current = current->children[i];
					break;
				}
			}
		}
		return current->insert(data, index);
	}
	
	size_t remove(size_t length, size_t index) {
		return root.remove(length, index);
	}
	
	void print() {
		root.print();
		printf("\n");
	}
};