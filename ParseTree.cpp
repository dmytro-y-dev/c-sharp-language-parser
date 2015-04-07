#include "ParseTree.h"

void DisplayNodeAsText(ostream& out, const ParseTree::Node* node, const string& prefix)
{
    if (node == nullptr) {
        return;
    }

    out << prefix << node->value.name << "(" << node->value.i << ";" << node->value.j << ")" << std::endl;

    DisplayNodeAsText(out, node->left, prefix + "    L!");
    DisplayNodeAsText(out, node->right, prefix + "    R!");
}

void DisplayTreeAsText(ostream& out, const ParseTree& tree)
{
    DisplayNodeAsText(out, tree.GetRoot(), "");
}

ParseTree::ParseTree()
{
    m_root = nullptr;
}

ParseTree::ParseTree(const ParseTree& tree)
{
    m_root = new Node(NullPosition);
    DeepCopyNode(tree.m_root, m_root);
}

ParseTree::~ParseTree()
{
    FreeNode(m_root);

    m_root = nullptr;
}

bool ParseTree::Empty() const
{
    return m_root == nullptr;
}

const ParseTree::Node* ParseTree::GetRoot() const
{
    return m_root;
}

ParseTree::Node* ParseTree::GetRoot()
{
    return m_root;
}

void ParseTree::FreeNode(Node* node)
{
    if (node == nullptr) {
        return;
    }

    FreeNode(node->left);
    FreeNode(node->right);

    delete node;
}

ParseTree::Node* ParseTree::InsertChildNode(ParseTree::Node* parentNode, const NonTerminalPositionType& value)
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

ParseTree& ParseTree::operator=(const ParseTree& tree)
{
    FreeNode(m_root);

    m_root = new Node(NullPosition);
    DeepCopyNode(tree.m_root, m_root);

    return *this;
}

void ParseTree::DeepCopyNode(const Node* from, Node* where)
{
    if (where == nullptr || from == nullptr) {
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
