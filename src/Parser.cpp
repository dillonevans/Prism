#include <iostream>
#include <vector>
#include "../include/Parser.h"
#include "../include/BinaryOperatorNode.h"
#include "../include/IntegerLiteralNode.h"
#include "../include/CompoundStatementNode.h"
#include "../include/VariableDeclarationNode.h"
#include "../include/BooleanLiteralNode.h"
#include "../include/IfStatementNode.h"
#include "../include/VariableNode.h"
#include "../include/ScopeTreeNode.h"
#include "../include/ReturnNode.h"
#include "../include/TypeCheckingVisitor.h"
#include "../include/FunctionDeclarationNode.h"
#include "../include/FunctionCallNode.h"
#include "../include/WhileNode.h"
#include "../include/ProgramNode.h"
#include "../include/StringSymbolTable.h"
#include "../include/StringLiteralNode.h"

/**
 * Constructor
 */
Parser::Parser(std::string text) : lexer{ text }
{
    //Perform Lexical Analysis
    while (!lexer.hasReachedEOF())
    {
        tokens.push_back(lexer.lex());
    }

    ///Push the Global Scope Symbol Table
    scopeTreeStack.push(new ScopeTreeNode());
    auto printfNode = new FunctionDeclarationNode(IntegerPrimitive, "printf");
    scopeTreeStack.top()->addEntry("printf", IntegerPrimitive, printfNode, true);
};

/**
 * Returns true if the passed Syntax Type is a binary operator
 */
bool Parser::isBinaryOperator(SyntaxType syntaxType)
{
    switch (syntaxType)
    {
        case MultToken:
        case DivideToken:
        case AddToken:
        case MinusToken:
        case LessThanToken:
        case GreaterThanToken:
        case LessThanOrEqualToToken:
        case GreaterThanOrEqualToToken:
        case LogicalOrToken:
        case LogicalAndToken:
        case EqualsToken:
        case AssignmentToken:
            return true;
    }
    return false;
}

/**
 * Prints all of the tokens
 */
void Parser::printTokens()
{
    std::cout << "Printing Tokens...\n";
    for (auto& x : tokens)
    {
        std::cout << x.getText() << "\n";
    }
};

/**
 * Looks ahead in the token table by an offset
 */
SyntaxToken Parser::peek(int offset)
{
    auto index = position + offset;
    return tokens[index];
};

/**
 * Return the current token
 */
SyntaxToken Parser::getCurrentToken()
{
    return peek(0);
};

/**
 * Consume and return the current token
 */
SyntaxToken Parser::getNextToken()
{
    auto current = getCurrentToken();
    position++;
    return current;
};

/**
 * Returns the current token if the passed syntax is what is expected
 */
SyntaxToken Parser::match(SyntaxType expected, std::string text)
{
    auto current = getCurrentToken();
    if (current.getSyntaxType() == expected)
    {
        return getNextToken();
    }
    else
    {
        std::cerr << "Unexpected Token \"" << current.getText() << "\". Expected " << text << ".";
    }
    exit(EXIT_FAILURE);
};

int Parser::getOperatorPrecedence(SyntaxType op)
{
    switch (op)
    {
        case EqualsToken:
            return 5;
        case MultToken:
        case DivideToken:
            return 4;
        case AddToken:
        case MinusToken:
            return 3;
        case LessThanToken:
        case GreaterThanToken:
        case GreaterThanOrEqualToToken:
        case LessThanOrEqualToToken:
            return 3;
        case LogicalAndToken:
            return 2;
        case LogicalOrToken:
            return 1;
        case AssignmentToken:
            return 0;
    }
    return 0;
}

BinaryOperatorType Parser::getBinaryOperatorType(SyntaxType op)
{
    switch (op)
    {
        case AddToken:
            return AdditionOperator;
        case MultToken:
            return MultiplicationOperator;
        case DivideToken:
            return DivisionOperator;
        case MinusToken:
            return SubtractionOperator;
        case GreaterThanToken:
            return GreaterThanOperator;
        case LessThanToken:
            return LessThanOperator;
        case LogicalAndToken:
            return LogicalAndOperator;
        case LogicalOrToken:
            return LogicalOrOperator;
        case AssignmentToken:
            return AssignmentOperator;
        case LessThanOrEqualToToken:
            return LessThanOrEqualToOperator;
        case GreaterThanOrEqualToToken:
            return GreaterThanOrEqualToOperator;
        case EqualsToken:
            return EqualsOperator;
        default:
            std::cout << "TEST";
            exit(EXIT_FAILURE);
    }
};

/**
 * Parses tokens such as literals and function calls.
 */
ASTNode* Parser::parsePrimary()
{
    std::string result;
    ASTNode* tree;
    std::string identifier, text;
    ASTNode* varNode;
    std::vector<ASTNode*> args;
    switch (getCurrentToken().getSyntaxType())
    {
        case IntegerLiteralToken:
            result = match(IntegerLiteralToken, "Integer Literal").getText();
            return new IntegerLiteralNode(std::stoi(result));
        case TrueKeywordToken:
            match(TrueKeywordToken, "true");
            return new BooleanLiteralNode(true);
        case FalseKeywordToken:
            match(FalseKeywordToken, "false");
            return new BooleanLiteralNode(false);
        case LeftParenthesisToken:
            match(LeftParenthesisToken, "(");
            tree = parseExpression(0);
            match(RightParenthesisToken, ")");
            return tree;
        case IdentifierToken:
            identifier = getCurrentToken().getText();
            match(IdentifierToken, "identifier");
            /**
             * Attempt to find the local variable node corresponding to the given identifier.
             * If it is found, return the node. Otherwise, report an error
             */
            varNode = resolve(identifier, scopeTreeStack.top());
            if (varNode)
            {
                std::cout << "Identifier: " << identifier << " was found\n";
                if (getCurrentToken().getSyntaxType() == LeftParenthesisToken)
                {
                    match(LeftParenthesisToken, "(");
                    if (getCurrentToken().getSyntaxType() != RightParenthesisToken)
                    {
                        args.push_back(parseExpression(0));
                    }
                    while (getCurrentToken().getSyntaxType() != RightParenthesisToken)
                    {
                        match(CommaToken, ",");
                        args.push_back(parseExpression(0));
                    }
                    match(RightParenthesisToken, ")");
                    return new FunctionCallNode(identifier, args);
                }
                return varNode;
            }
            else
            {
                std::cerr << "Variable \"" << identifier << "\" does not exist in the current scope";
                exit(EXIT_FAILURE);
            }
        case StringLiteralToken:
            text = match(StringLiteralToken, "String literal").getText();
            StringSymbolTable::addEntry(text);
            return new StringLiteralNode(text);

        default:
            match(IntegerLiteralToken, "Integer Literal");
            exit(EXIT_FAILURE);
    }
    return nullptr;
};

/**
 * Construct an AST via precendence climbing parsing
 */
ASTNode* Parser::parseExpression(int minimumPrecedence)
{
    ASTNode* left = parsePrimary(), * right, * ltemp;
    SyntaxType lookAhead = getCurrentToken().getSyntaxType();

    while ((isBinaryOperator(lookAhead) && getOperatorPrecedence(lookAhead) > minimumPrecedence)
        || (isRightAssociative(lookAhead) && getOperatorPrecedence(lookAhead) == minimumPrecedence)) {
        getNextToken();
        right = parseExpression(getOperatorPrecedence(lookAhead));

        /**
         * This ensures that the left hand side of the tree is loaded into a register,
         * and the right hand side is simply retrieved from RAM when generating the
         * x86-64 code.
         */
        if (lookAhead == AssignmentToken)
        {
            ltemp = left;
            left = right;
            right = ltemp;
        }

        left = new BinaryOperatorNode(left, getBinaryOperatorType(lookAhead), right);
        lookAhead = getCurrentToken().getSyntaxType();
        if (lookAhead == RightParenthesisToken || lookAhead == SemicolonToken)
        {
            return left;
        }
    }
    return left;
};

ASTNode* Parser::parseStatement()
{
    switch (getCurrentToken().getSyntaxType())
    {
        case IfToken:
            return parseIfStatement();
        case LeftCurlyBraceToken:
            return parseCompoundStatement();
        case VarKeywordToken:
        case IntKeywordToken:
        case BoolKeywordToken:
            return parseVariableDeclarationStatement();
        case ReturnKeyword:
            return parseReturnStatement();
        case WhileKeywordToken:
            return parseWhileStatement();
        default:
            return parseExpressionStatement();
    }
    return nullptr;
}

ASTNode* Parser::parseCompoundStatement()
{
    match(LeftCurlyBraceToken, "{");
    std::vector<ASTNode*> statements;

    /**
     * When we encounter a block statement, we push a new scope onto the stack,
     * make it the child of the parent to resolve variables in nested scopes,
     * and increment the scope variable.
     */
    ScopeTreeNode* parent = scopeTreeStack.top(), * child = new ScopeTreeNode();
    scopeTreeStack.push(child);
    parent->addChild(child);
    scope++;

    //While inside the compound statement
    while (getCurrentToken().getSyntaxType() != RightCurlyBraceToken)
    {
        //Add each statement to the statement list
        statements.push_back(parseStatement());
    }
    match(RightCurlyBraceToken, "}");

    //When the scope ends, decrement the scope and pop the stack
    scope--;
    scopeTreeStack.pop();
    return new CompoundStatementNode(statements);
}

ASTNode* Parser::parseIfStatement()
{
    ASTNode* condition, * stmtBody, * elseBody = nullptr;

    //Parse the "if (condition)" portion
    match(IfToken, "if");
    match(LeftParenthesisToken, "(");
    condition = parseExpression(0);
    match(RightParenthesisToken, ")");

    //Parse the "then" portion of the if statement
    stmtBody = parseStatement();
    if (getCurrentToken().getSyntaxType() == ElseToken)
    {
        match(ElseToken, "else");
        elseBody = parseStatement();
    }
    return new IfStatementNode(condition, stmtBody, elseBody);
}

ASTNode* Parser::parseVariableDeclarationStatement()
{
    ASTNode* rhs = nullptr;
    std::string identifier;
    SyntaxToken token = getCurrentToken();
    bool isVarType = false;
    Type t;
    switch (getCurrentToken().getSyntaxType())
    {
        case IntKeywordToken:

            match(IntKeywordToken, "int");
            t = IntegerPrimitive;
            localOffset += 4;
            break;
        case DoubleKeywordToken:
            match(DoubleKeywordToken, "double");
            break;
        case BoolKeywordToken:
            match(BoolKeywordToken, "bool");
            t = BooleanPrimitive;
            break;
        default:
            t = ImplicitVarType;
            match(VarKeywordToken, "var");
            isVarType = true;
            break;
    }
    token = getCurrentToken();
    match(IdentifierToken, "an identifer");
    identifier = token.getText();

    /**
     * Rather than checking the parent scope, we check the local scope
     * so that we can implement variable shadowing
     */
    if (scopeTreeStack.top()->getNode(identifier))
    {
        std::cerr << "Error: Variable \"" << identifier << "\" already exists.\n";
        exit(EXIT_FAILURE);
    }
    //Add the local variable to the current scope
    scopeTreeStack.top()->addEntry(identifier, t, new VariableNode(t, identifier, true), false);
    token = getCurrentToken();

    //Implicitly typed variables require an assignment so as to deduce the type
    if (isVarType)
    {
        match(AssignmentToken, "=");
        rhs = parseExpression(0);
        match(SemicolonToken, ";");
    }
    else
    {
        if (token.getSyntaxType() != AssignmentToken)
        {
            match(SemicolonToken, ";");
        }
        else
        {
            match(AssignmentToken, "=");
            rhs = parseExpression(0);
            match(SemicolonToken, ";");
        }
    }
    return new VariableDeclarationNode(scopeTreeStack.top()->getNode(identifier), rhs, identifier);
}

ASTNode* Parser::parseReturnStatement()
{
    ASTNode* toReturn;
    match(ReturnKeyword, "return");
    toReturn = parseExpression(0);
    match(SemicolonToken, ";");
    return new ReturnNode(toReturn);
}

/**
 * Parses a function declaration including any parameters.
 */
ASTNode* Parser::parseFunctionDeclaration()
{
    Type returnType;
    ASTNode* body;
    FunctionDeclarationNode* functionDeclNode;
    std::string functionIdentifier, parameterIdentifier;
    std::vector<std::pair<VariableDeclarationNode*, Type>> parameterList;

    //Determine the return type of the function
    switch (getCurrentToken().getSyntaxType())
    {
        case IntKeywordToken:
            returnType = IntegerPrimitive;
            match(IntKeywordToken, "int");
            break;
        case BoolKeywordToken:
            returnType = BooleanPrimitive;
            match(BoolKeywordToken, "bool");
            break;
        case VarKeywordToken:
            std::cerr << "Error: \"var\" is not valid for function arguments.\n";
            exit(EXIT_FAILURE);
    }

    //Match and obtain the function identifier
    if (getCurrentToken().getSyntaxType() == IdentifierToken)
    {
        functionIdentifier = getCurrentToken().getText();
    }
    match(IdentifierToken, "an identifier");

    /**
     * When declaring a function, the parameters are made children of the global scope,
     * and the function body adds a new scope to the scope in which the parameters reside.
    */
    functionDeclNode = new FunctionDeclarationNode(returnType, functionIdentifier);
    scopeTreeStack.top()->addEntry(functionIdentifier, returnType, functionDeclNode, true);
    ScopeTreeNode* parent = scopeTreeStack.top(), * child = new ScopeTreeNode();
    scopeTreeStack.push(child);
    parent->addChild(child);
    scope++;

    //Parse the function parameters (if any)
    match(LeftParenthesisToken, "(");
    while (getCurrentToken().getSyntaxType() != RightParenthesisToken)
    {
        match(IntKeywordToken, "int");

        /**
         * Obtain the parameter identifier, add it to the current scope's symbol table,
         * and add it to the function's argument list
         */
        parameterIdentifier = match(IdentifierToken, "an identifier").getText();
        auto node = new VariableNode(IntegerPrimitive, parameterIdentifier, true);
        parameterList.push_back(std::pair<VariableDeclarationNode*, Type>(new VariableDeclarationNode(node, nullptr, parameterIdentifier), IntegerPrimitive));

        scopeTreeStack.top()->addEntry(parameterIdentifier, IntegerPrimitive, node, true);

        /**
         * If a comma is encountered, the next token cannot be a right semicolon
         */
        if (getCurrentToken().getSyntaxType() == CommaToken)
        {
            match(CommaToken, ",");
            //Ensure that the next token is NOT a right parenthesis
            if (getCurrentToken().getSyntaxType() == RightParenthesisToken)
            {
                std::cerr << "Error: Invalid function declaration syntax";
                exit(EXIT_FAILURE);
            }
        }
    }
    match(RightParenthesisToken, ")");

    //Parse the function body
    body = parseCompoundStatement();
    /**
     * At this point, the scope is returned to that of the function parameters,
     * so we pop the stack and decrement the scope to return to the global scope for
     * future function declarations
     */
    scopeTreeStack.pop();
    scope--;
    functionDeclNode->functionBody = body;
    functionDeclNode->parameterList = parameterList;
    functionDeclNode->functionName = functionIdentifier;
    functionDeclNode->stackOffset = (localOffset | 15) + 1;
    localOffset = 0;
    return functionDeclNode;
}

ASTNode* Parser::parseExpressionStatement()
{
    ASTNode* expr = parseExpression(0);
    match(SemicolonToken, ";");
    return expr;
}

ASTNode* Parser::parseWhileStatement()
{
    ASTNode* condition, * body;

    //Parse the "while (condition)" portion
    match(WhileKeywordToken, "while");
    match(LeftParenthesisToken, "(");
    condition = parseExpression(0);
    match(RightParenthesisToken, ")");
    body = parseStatement();
    return new WhileNode(condition, body);

}

ASTNode* Parser::resolve(std::string identifier, ScopeTreeNode* node)
{
    if (!node) { return nullptr; }
    auto variableNode = node->getNode(identifier);
    if (!variableNode) { return resolve(identifier, node->getParentNode()); }
    else { return variableNode; }
}

bool Parser::isRightAssociative(SyntaxType op)
{
    return op == AssignmentToken;
}

/**
 * Parses the program from start to finish by parsing functions as well as
 * global variable declarations.
 */
ASTNode* Parser::parseProgram()
{
    std::vector<ASTNode*> units;
    //While there are still tokens to be read in
    while (getCurrentToken().getSyntaxType() != EOFToken)
    {
        units.push_back(parseFunctionDeclaration());
    }
    return new ProgramNode(units);
}
