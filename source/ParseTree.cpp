#include "ParseTree.h"

void DisplaySyntaxParseTreeNodeAsText(ostream& out, const SyntaxParseTree::Node* node, const string& prefix)
{
    if (node == nullptr) {
        return;
    }

    out << prefix << node->value.name << "(" << node->value.i << ";" << node->value.j << ")" << std::endl;

    DisplaySyntaxParseTreeNodeAsText(out, node->left, prefix + "    L!");
    DisplaySyntaxParseTreeNodeAsText(out, node->right, prefix + "    R!");
}

void DisplaySyntaxParseTreeAsText(ostream& out, const SyntaxParseTree& tree)
{
    DisplaySyntaxParseTreeNodeAsText(out, tree.GetRoot(), "");
}

SyntaxParseTree::SyntaxParseTree()
{
    m_root = nullptr;
}

SyntaxParseTree::SyntaxParseTree(const SyntaxParseTree& tree)
{
    if (tree.m_root) {
        m_root = new Node(NullPosition);
        DeepCopyNode(tree.m_root, m_root);
    } else {
        m_root = nullptr;
    }
}

SyntaxParseTree::~SyntaxParseTree()
{
    FreeNode(m_root);

    m_root = nullptr;
}

bool SyntaxParseTree::Empty() const
{
    return m_root == nullptr;
}

const SyntaxParseTree::Node* SyntaxParseTree::GetRoot() const
{
    return m_root;
}

SyntaxParseTree::Node* SyntaxParseTree::GetRoot()
{
    return m_root;
}

void SyntaxParseTree::FreeNode(Node* node)
{
    if (node == nullptr) {
        return;
    }

    FreeNode(node->left);
    FreeNode(node->right);

    delete node;
}

SyntaxParseTree::Node* SyntaxParseTree::InsertChildNode(SyntaxParseTree::Node* parentNode, const NonTerminalPositionType& value)
{
    if (parentNode == nullptr) {
        if (m_root == nullptr) {
            m_root = new Node(value);

            return m_root;
        }

        return nullptr;
    }

    if (parentNode->left == nullptr) {
        parentNode->left = new Node(value);

        return parentNode->left;
    }

    if (parentNode->right == nullptr) {
        parentNode->right = new Node(value);

        return parentNode->right;
    }

    return nullptr;
}

SyntaxParseTree& SyntaxParseTree::operator=(const SyntaxParseTree& tree)
{
    FreeNode(m_root);

    if (tree.m_root) {
        m_root = new Node(NullPosition);
        DeepCopyNode(tree.m_root, m_root);
    } else {
        m_root = nullptr;
    }

    return *this;
}

void SyntaxParseTree::DeepCopyNode(const Node* from, Node* where)
{
    if (where == nullptr) {
        return;
    }

    where->value = from->value;

    if (from->left) {
        where->left = new Node(NullPosition);
    }

    if (from->right) {
        where->right = new Node(NullPosition);
    }

    DeepCopyNode(from->left, where->left);
    DeepCopyNode(from->right, where->right);
}
