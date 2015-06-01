#include "IntermediateCodeParseTree.h"

#include <map>

using std::map;

typedef void (*IntermediateCodeParseTreeRule)(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateRoot);

void buildCompilationUnit(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateRoot)
{
    intermediateRoot.intermediateRule = "ic-compilation-unit";
    intermediateRoot.token.type = Lexem::LEXEM_UNKNOWN;

    BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right, tokens, intermediateRoot);
}

void buildClass(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateRoot)
{
    IntermediateCodeParseTreeNode newClass;

    newClass.intermediateRule = "ic-class";
    newClass.token = tokens[syntaxRoot->left->value.j];

    if (syntaxRoot->right->value.name == "<class-part2>") {
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right->left->right, tokens, newClass); // <class-body-part1>
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right->right->right, tokens, intermediateRoot); // <one-more-class> -> <class-part1>
    } else if (syntaxRoot->right->value.name == "<class-body>") {
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right->right, tokens, newClass); // <class-body-part1>
    }

    intermediateRoot.children.push_back(newClass);
}

void buildClassBody(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someClass)
{
    if (syntaxRoot->left && syntaxRoot->left->value.name == "<class-member-declaration>") {
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->left, tokens, someClass);
    }

    if (syntaxRoot->right && syntaxRoot->right->value.name == "<class-body-part1>") {
        buildClassBody(syntaxRoot->right, tokens, someClass);
    }
}

void buildQualifiedName(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& qualifiedName)
{
    for (int i = syntaxRoot->value.j, iend = syntaxRoot->value.j + syntaxRoot->value.i; i <= iend; i += 2) {
        IntermediateCodeParseTreeNode partOfName;
        partOfName.intermediateRule = "ic-part-of-qualified-name";
        partOfName.token = tokens[i];
        partOfName.positionInCode = i;

        qualifiedName.children.push_back(partOfName);
    }
}

void buildExpression(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someMethod)
{
    if (syntaxRoot->left == nullptr) {
        IntermediateCodeParseTreeNode newToken;
        newToken.intermediateRule = "ic-expr-token";
        newToken.token = tokens[syntaxRoot->value.j];
        newToken.positionInCode = syntaxRoot->value.j;

        someMethod.children.push_back(newToken);

        return;
    }

    if (syntaxRoot->left->value.name == "<opening-parentheses>") {
        IntermediateCodeParseTreeNode newOpeningOperator;
        newOpeningOperator.intermediateRule = "ic-expr-priority-operator";
        newOpeningOperator.token = tokens[syntaxRoot->left->value.j];

        someMethod.children.push_back(newOpeningOperator);

        buildExpression(syntaxRoot->right->left, tokens, someMethod);

        IntermediateCodeParseTreeNode newClosingOperator;
        newClosingOperator.intermediateRule = "ic-expr-priority-operator";
        newClosingOperator.token = tokens[syntaxRoot->right->right->value.j];

        someMethod.children.push_back(newClosingOperator);

        return;
    }

    if (syntaxRoot->left->value.name == "<expression>") {
        buildExpression(syntaxRoot->left, tokens, someMethod);

        IntermediateCodeParseTreeNode newExprOperator;
        newExprOperator.intermediateRule = "ic-expr-operator";
        newExprOperator.token = tokens[syntaxRoot->right->value.j];

        someMethod.children.push_back(newExprOperator);

        buildExpression(syntaxRoot->right->right, tokens, someMethod);

        return;
    }

    if (syntaxRoot->left->value.name == "<non-qualified-identifier>") {
        IntermediateCodeParseTreeNode newQualifiedName;
        newQualifiedName.intermediateRule = "ic-expr-qualified-name";
        newQualifiedName.positionInCode = syntaxRoot->left->value.j;

        buildQualifiedName(syntaxRoot, tokens, newQualifiedName);

        someMethod.children.push_back(newQualifiedName);

        return;
    }

    throw "buildExpression : Intermediate rule for " + syntaxRoot->left->value.name + " node is unknown";
}

void addStatementExpressionParameters(const ParseTree::Node* parametersListNode, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& statementExpr)
{
    if (parametersListNode->value.name == "<closing-parentheses>") {
        return;
    }

    if (parametersListNode->value.name == "<statement-expression-partA1>") {
        addStatementExpressionParameters(parametersListNode->right, tokens, statementExpr);

        return;
    }

    if (parametersListNode->value.name == "<statement-expression-partA2>") {
        addStatementExpressionParameters(parametersListNode->left, tokens, statementExpr);

        if (parametersListNode->right->value.name == "<statement-expression-partA3>") {
            addStatementExpressionParameters(parametersListNode->right->right, tokens, statementExpr);
        }

        return;
    }

    if (parametersListNode->value.name == "<statement-expression-partB>") {
        IntermediateCodeParseTreeNode newOperator;
        newOperator.intermediateRule = "ic-statement-expr-assignment";
        newOperator.token = tokens[parametersListNode->left->value.j];

        statementExpr.children.push_back(newOperator);

        addStatementExpressionParameters(parametersListNode->right, tokens, statementExpr);

        return;
    }

    if (parametersListNode->value.name == "<expression>") {
        IntermediateCodeParseTreeNode newParameter;
        newParameter.intermediateRule = "ic-statement-expr-parameter";
        newParameter.token.type = Lexem::LEXEM_UNKNOWN;
        newParameter.positionInCode = parametersListNode->value.j;

        buildExpression(parametersListNode, tokens, newParameter);

        statementExpr.children.push_back(newParameter);

        return;
    }

    throw "addStatementExpressionParameters : Intermediate rule for " + parametersListNode->value.name + " node is unknown";
}

void buildDeclarationStatement(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someMethodBody)
{
    if (syntaxRoot->left->value.name == "<local-variable-declaration>") {
        IntermediateCodeParseTreeNode newVariable;
        newVariable.intermediateRule = "ic-variable-declaration";
        newVariable.token = tokens[syntaxRoot->left->value.j + 1];
        newVariable.positionInCode = syntaxRoot->left->value.j;

        IntermediateCodeParseTreeNode newVariableType;
        newVariableType.intermediateRule = "ic-variable-type";
        newVariableType.token = tokens[syntaxRoot->left->value.j];

        newVariable.children.push_back(newVariableType);

        if (syntaxRoot->left->right->value.name == "<variable-declarator-with-initialization>") {
            IntermediateCodeParseTreeNode newVariableValue;
            newVariableValue.intermediateRule = "ic-variable-declaration-value";

            buildExpression(syntaxRoot->left->right->right->right, tokens, newVariableValue); // <variable-declarator-with-initialization-part1> -> <expression>


            newVariable.children.push_back(newVariableValue);
        }

        someMethodBody.children.push_back(newVariable);

        return;
    }
    
    if (syntaxRoot->left->value.name == "<statement-expression>") {
        IntermediateCodeParseTreeNode newStatementExpr;
        newStatementExpr.intermediateRule = "ic-statement-expr";
        newStatementExpr.token.type = Lexem::LEXEM_UNKNOWN;

        if (syntaxRoot->left->right->value.name == "<increment-decrement>") {
            IntermediateCodeParseTreeNode newQualifiedName;
            newQualifiedName.intermediateRule = "ic-statement-expr-qualified-name";
            newQualifiedName.token.type = Lexem::LEXEM_UNKNOWN;
            newQualifiedName.positionInCode = syntaxRoot->left->left->value.j;

            buildQualifiedName(syntaxRoot->left->left, tokens, newQualifiedName);

            newStatementExpr.children.push_back(newQualifiedName);

            IntermediateCodeParseTreeNode newOperator;
            newOperator.intermediateRule = "ic-statement-expr-increment-decrement";
            newOperator.token = tokens[syntaxRoot->left->right->value.j];

            newStatementExpr.children.push_back(newOperator);
        } else if (syntaxRoot->left->left->value.name == "<increment-decrement>") {
            IntermediateCodeParseTreeNode newOperator;
            newOperator.intermediateRule = "ic-statement-expr-increment-decrement";
            newOperator.token = tokens[syntaxRoot->left->left->value.j];

            newStatementExpr.children.push_back(newOperator);

            IntermediateCodeParseTreeNode newQualifiedName;
            newQualifiedName.intermediateRule = "ic-statement-expr-qualified-name";
            newQualifiedName.token.type = Lexem::LEXEM_UNKNOWN;
            newQualifiedName.positionInCode = syntaxRoot->left->right->value.j;

            buildQualifiedName(syntaxRoot->left->right, tokens, newQualifiedName);

            newStatementExpr.children.push_back(newQualifiedName);
        } else  if (syntaxRoot->left->left->value.name == "<identifier>") {
            IntermediateCodeParseTreeNode newStatementExprQualifiedName;
            newStatementExprQualifiedName.intermediateRule = "ic-statement-expr-qualified-name";
            newStatementExprQualifiedName.token.type = Lexem::LEXEM_UNKNOWN;
            newStatementExprQualifiedName.positionInCode = syntaxRoot->left->left ->value.j;

            buildQualifiedName(syntaxRoot->left->left, tokens, newStatementExprQualifiedName);

            IntermediateCodeParseTreeNode newStatementExprParameters;
            newStatementExprParameters.intermediateRule = "ic-statement-expr-parameters";
            newStatementExprParameters.token.type = Lexem::LEXEM_UNKNOWN;
            newStatementExprParameters.positionInCode = syntaxRoot->left->right->value.j;

            addStatementExpressionParameters(syntaxRoot->left->right, tokens, newStatementExprParameters);

            newStatementExpr.children.push_back(newStatementExprQualifiedName);
            newStatementExpr.children.push_back(newStatementExprParameters);
        } else {
            throw "buildDeclarationStatement : Unknown statement construction";
        }

        someMethodBody.children.push_back(newStatementExpr);

        return;
    }

    throw "buildDeclarationStatement : Intermediate rule for " + syntaxRoot->left->value.name + " node is unknown";
}

void buildEmbeddedStatement(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someMethodBody)
{
    if (syntaxRoot->right == nullptr) {
        return;
    }

    if (syntaxRoot->left->value.name == "<opening-bracket>") {
        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right, tokens, someMethodBody);

        return;
    }

    if (syntaxRoot->left->value.name == "<while-keyword>") {
        IntermediateCodeParseTreeNode newWhile;
        newWhile.intermediateRule = "ic-while";
        newWhile.token.type = Lexem::LEXEM_UNKNOWN;

        IntermediateCodeParseTreeNode newWhileCondition;
        newWhileCondition.intermediateRule = "ic-while-condition";
        newWhileCondition.token.type = Lexem::LEXEM_UNKNOWN;
        newWhileCondition.positionInCode = syntaxRoot->right->right->left->value.j;
        
        buildExpression(syntaxRoot->right->right->left, tokens, newWhileCondition);

        IntermediateCodeParseTreeNode newWhileBody;
        newWhileBody.intermediateRule = "ic-while-body";
        newWhileBody.token.type = Lexem::LEXEM_UNKNOWN;
        newWhileBody.positionInCode = syntaxRoot->right->right->right->right->value.j;

        BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->right->right->right->right, tokens, newWhileBody);

        newWhile.children.push_back(newWhileCondition);
        newWhile.children.push_back(newWhileBody);

        someMethodBody.children.push_back(newWhile);

        return;
    }

    throw "buildEmbeddedStatement : Intermediate rule for " + syntaxRoot->left->value.name + " node is unknown";
}

void buildMethodBody(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someMethod)
{
    if (syntaxRoot->value.name == "<maybe-empty-block>") {
        if (syntaxRoot->value.i <= 1) {
            return;
        }

        syntaxRoot = syntaxRoot->right;
    }

    BuildIntermediateCodeParseTreeFromSyntaxParseTree(syntaxRoot->left, tokens, someMethod);

    if (syntaxRoot->right->value.name == "<maybe-empty-block-part1>") {
        buildMethodBody(syntaxRoot->right, tokens, someMethod);
    }
}

void addMethodParameters(const ParseTree::Node* parametersListNode, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someMethod)
{
    if (parametersListNode->value.name == "<closing-parentheses>") {
        return;
    }

    if (parametersListNode->value.name == "<method-header-part4a>") {
        addMethodParameters(parametersListNode->left, tokens, someMethod);
        return;
    }

    if (parametersListNode->value.name == "<formal-parameter-list>") {
        addMethodParameters(parametersListNode->left, tokens, someMethod);
        addMethodParameters(parametersListNode->right->right, tokens, someMethod);

        return;
    }

    if (parametersListNode->value.name == "<method-header-part4b>") {
        addMethodParameters(parametersListNode->left, tokens, someMethod);

        return;
    }

    if (parametersListNode->value.name == "<fixed-parameter>") {
        IntermediateCodeParseTreeNode newFixedParameterType;
        newFixedParameterType.intermediateRule = "ic-method-parameter-type";
        newFixedParameterType.token = tokens[parametersListNode->left->value.j];

        IntermediateCodeParseTreeNode newFixedParameterName;
        newFixedParameterName.intermediateRule = "ic-method-parameter-name";
        newFixedParameterName.token = tokens[parametersListNode->right->value.j];
        newFixedParameterName.positionInCode = parametersListNode->right->value.j;

        someMethod.children.push_back(newFixedParameterType);
        someMethod.children.push_back(newFixedParameterName);

        return;
    }

    throw "addMethodParameters : Intermediate rule for " + parametersListNode->value.name + " node is unknown";
}

void buildClassMethod(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& someClass)
{
    IntermediateCodeParseTreeNode newMethod;
    newMethod.intermediateRule = "ic-method";
    newMethod.token = tokens[syntaxRoot->left->right->right->left->value.j];

    IntermediateCodeParseTreeNode newMethodModifier;
    newMethodModifier.intermediateRule = "ic-method-modifier";
    newMethodModifier.token = tokens[syntaxRoot->left->left->value.j];
    newMethod.children.push_back(newMethodModifier);

    IntermediateCodeParseTreeNode newMethodType;
    newMethodType.intermediateRule = "ic-method-type";
    newMethodType.token = tokens[syntaxRoot->left->right->left->value.j];
    newMethod.children.push_back(newMethodType);

    addMethodParameters(syntaxRoot->left->right->right->right->right, tokens, newMethod);

    IntermediateCodeParseTreeNode newMethodBody;
    newMethodBody.intermediateRule = "ic-method-body";
    newMethodBody.token.type = Lexem::LEXEM_UNKNOWN;

    buildMethodBody(syntaxRoot->right, tokens, newMethodBody);

    newMethod.children.push_back(newMethodBody);

    someClass.children.push_back(newMethod);
}

void BuildIntermediateCodeParseTreeFromSyntaxParseTree(const ParseTree::Node* syntaxRoot, const vector<Lexem>& tokens, IntermediateCodeParseTreeNode& intermediateRoot)
{
    if (!syntaxRoot) {
        return;
    }

    map<string, IntermediateCodeParseTreeRule> rules;
    rules["<compilation-unit>"] = buildCompilationUnit;
    rules["<class-part1>"] = buildClass;
    rules["<class-body-part1>"] = buildClassBody;
    rules["<class-member-declaration>"] = buildClassMethod;
    rules["<declaration-statement>"] = buildDeclarationStatement;
    rules["<embedded-statement>"] = buildEmbeddedStatement;
    rules["<maybe-empty-block-part1>"] = buildMethodBody;
    rules["<maybe-empty-block>"] = buildMethodBody;

    map<string, IntermediateCodeParseTreeRule>::iterator intermediateRule = rules.find(syntaxRoot->value.name);

    if (intermediateRule == rules.end()) {
        throw "buildIntermediateCodeParseTreeFromSyntaxParseTree : Intermediate rule for " + syntaxRoot->value.name + " node is unknown";
    }

    (intermediateRule->second)(syntaxRoot, tokens, intermediateRoot);
}

