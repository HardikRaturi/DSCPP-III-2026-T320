#pragma once
#include <iostream>
using namespace std;

// DSA: Singly Linked List Node for Food
struct FoodNode {
    Food data;
    FoodNode* next;
    FoodNode(const Food& f) : data(f), next(nullptr) {}
};
