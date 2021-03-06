#ifndef VDN_H
#define VDN_H

#include "AstNode.h"
#include <string>
#include "Type.h"
#include "VariableNode.h"
class VariableDeclarationNode : public ASTNode
{

private:
    ASTNode* rhs;
    std::string identifier;
    ASTNode* varNode;
    Type type;

public:
    ASTNode* getRHS();
    Type getType();
    std::string getIdentifier();
    ASTNode* getVarNode();

    VariableDeclarationNode(ASTNode* varNode, ASTNode* rhs, std::string identifier);
    void accept(Visitor& v);
};

#endif