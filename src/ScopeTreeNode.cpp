#include "../headers/ScopeTreeNode.h"

void ScopeTreeNode::addChild(std::string identifier, Type t){
    this->symbolTable[identifier] = t;
}

std::list<ScopeTreeNode*> ScopeTreeNode::getChildren(){
    return this->children;
}

std::unordered_map<std::string, Type> ScopeTreeNode::getSymbolTable(){
    return this->symbolTable;
}

ScopeTreeNode* ScopeTreeNode::getParentNode(){
    return this->parent;
}

void ScopeTreeNode::setParent(ScopeTreeNode *parent){
    this->parent = parent;
}
