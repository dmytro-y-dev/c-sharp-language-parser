#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "../LexicalAnalyzer.h"
#include "../SyntaxAnalyzer.h"

#include <wx/wx.h>
#include <wx/treectrl.h>

using namespace std;

#define ID_BTN_PARSE 10000

class MyMainWindow : public wxFrame
{
public:
    wxTextCtrl *txtCode;
    wxTextCtrl *txtResult;
    wxTreeCtrl *treeResult;

public:
    MyMainWindow();

    void OnBtnParse(wxCommandEvent& evt);

    DECLARE_EVENT_TABLE();
};

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    //if (!wxApp::OnInit()) return false;

    MyMainWindow* myWindow = new MyMainWindow;
    myWindow->Show();

    return true;
}

BEGIN_EVENT_TABLE(MyMainWindow, wxFrame)
    EVT_BUTTON(ID_BTN_PARSE, MyMainWindow::OnBtnParse)
END_EVENT_TABLE()

MyMainWindow::MyMainWindow() : 
wxFrame(nullptr, wxID_ANY, "C# Syntax Analyzer GUI", wxDefaultPosition, wxSize(790, 540), wxMINIMIZE_BOX | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)
{
    wxPanel *mainPanel = new wxPanel(this, wxID_ANY, wxPoint(0, 0), wxDefaultSize);

    new wxStaticText(mainPanel, wxID_ANY, "Parse tree:", wxPoint(5, 5));
    new wxStaticText(mainPanel, wxID_ANY, "Source code:", wxPoint(330, 5));

    treeResult = new wxTreeCtrl(mainPanel, wxID_ANY, wxPoint(5, 25), wxSize(320, 400), wxTR_SINGLE | wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
    txtCode = new wxTextCtrl(mainPanel, wxID_ANY, "", wxPoint(330, 25), wxSize(440, 400), wxTE_MULTILINE | wxHSCROLL | wxVSCROLL | wxTE_NOHIDESEL);

    new wxStaticText(mainPanel, wxID_ANY, "Result:", wxPoint(5, 435));
    txtResult = new wxTextCtrl(mainPanel, wxID_ANY, "", wxPoint(5, 455), wxSize(650, 25), wxTE_READONLY | wxTE_NOHIDESEL);
    new wxButton(mainPanel, ID_BTN_PARSE, "Parse", wxPoint(660, 455), wxSize(100, 25));
}

void DisplayNodeInWxTreeCtrl(wxTreeCtrl* outTree, const SyntaxParseTree::Node* node, wxTreeItemId outNode)
{
    if (node == nullptr) {
        outTree->Delete(outNode);

        return;
    }

    outTree->SetItemText(outNode, node->value.name);

    wxTreeItemId leftChild = outTree->InsertItem(outNode, 0, "");
    wxTreeItemId rightChild = outTree->InsertItem(outNode, 1, "");

    DisplayNodeInWxTreeCtrl(outTree, node->left, leftChild);
    DisplayNodeInWxTreeCtrl(outTree, node->right, rightChild);
}

void DisplayParseTreeInWxTreeCtrl(wxTreeCtrl* outTree, const SyntaxParseTree& tree)
{
    wxTreeItemId rootItem = outTree->AddRoot("");

    DisplayNodeInWxTreeCtrl(outTree, tree.GetRoot(), rootItem);
}

void MyMainWindow::OnBtnParse(wxCommandEvent& evt)
{
    treeResult->DeleteAllItems();
    treeResult->Refresh();
    txtResult->SetValue("");

    if (txtCode->GetValue().empty()) {
        txtResult->SetValue("File has been parsed successfully.");

        return;
    }

    // Lets analyze C# source code file
    // We start with loading source code from the file which user specified in command line

    string originalContentOfCSharpFile = txtCode->GetValue().c_str();

    // Do lexical analysis

    vector<Lexem> lexems;
    if (!DoLexicalAnalysis(originalContentOfCSharpFile, lexems)) {
        wxMessageBox("Error during lexical analysis!", "Error", wxOK | wxICON_ERROR | wxOK_DEFAULT, this);

        return;
    }

    vector<Lexem> tokens;
    ExtractTokensFromLexemsList(lexems, tokens);

    // Do syntax analysis

    vector<SyntaxRule> syntaxRules;
    if (!LoadSyntaxRules("syntax-grammar.txt", syntaxRules) || syntaxRules.empty()) {
        wxMessageBox("Error during loading syntax rules!", "Error", wxOK | wxICON_ERROR | wxOK_DEFAULT, this);

        return;
    }

    SyntaxParseTree syntaxTree;

    try {
        syntaxTree = DoSyntaxAnalysis(tokens, syntaxRules, syntaxRules[0].GetName());
    } catch (SyntaxError& errorText) {
        wxMessageBox(errorText, "Error", wxOK | wxICON_ERROR | wxOK_DEFAULT, this);

        return;
    }

    // Write syntax analysis results to text file

    if (syntaxTree.Empty()) {
        txtResult->SetValue("Unable to spawn program's text.");
    } else {
        txtResult->SetValue("File has been parsed successfully.");
    }

    DisplayParseTreeInWxTreeCtrl(treeResult, syntaxTree);
}
