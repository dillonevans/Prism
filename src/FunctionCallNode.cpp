#include "../include/FunctionCallNode.h"
#include "../include/Visitor.h"
FunctionCallNode::FunctionCallNode(std::string identifier, std::vector<ASTNode*> args)
{
    this->identifier = identifier;
    this->arguments = args;
};

void FunctionCallNode::accept(Visitor& v)
{
    v.visitFunctionCallNode(this);
}
std::string FunctionCallNode::getIdentifier()
{
    return this->identifier;
}
