//---------------------------------------------------------------------------
// WordGraph.cpp
//
// A Directed Acyclic Word Graph class.
//
// Copyright 2004, 2005 Michael W Thelen <mike@pietdepsi.com>.
//
// This file is part of Zyzzyva.
//
// Zyzzyva is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Zyzzyva is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//---------------------------------------------------------------------------

#include "WordGraph.h"
#include "Defs.h"
#include <qregexp.h>
#include <iostream>
#include <stack>
#include <set>

using namespace std;
using namespace Defs;

//---------------------------------------------------------------------------
// WordGraph
//
//! Constructor.
//---------------------------------------------------------------------------
WordGraph::WordGraph()
    : top (0)
{
}

//---------------------------------------------------------------------------
// ~WordGraph
//
//! Destructor.
//---------------------------------------------------------------------------
WordGraph::~WordGraph()
{
}

//---------------------------------------------------------------------------
// addWord
//
//! Add a word to the graph.
//
//! @param w the word to add
//---------------------------------------------------------------------------
void
WordGraph::addWord (const QString& w)
{
    if (w.isEmpty())
        return;

    Node* node = top;
    Node* parentNode = 0;
    QChar c;
    int wordLength = w.length();
    for (int i = 0; i < wordLength; ++i) {
        c = w.at (i);

        // Empty node, so create a new node and link from its parent
        if (!node) {
            node = new Node (c);
            (parentNode ? parentNode->child : top) = node;
            ++numNodes;
        }

        // Nonempty node, so find the current letter in the chain
        else {
            while (node->letter != c) {
                if (!node->next) {
                    node->next = new Node (c);
                    ++numNodes;
                }
                node = node->next;
            }
        }

        parentNode = node;
        node = node->child;
    }

    parentNode->eow = true;
}

//---------------------------------------------------------------------------
// containsWord
//
//! Determine whether the graph contains a word.
//
//! @param w the word to search for
//---------------------------------------------------------------------------
bool
WordGraph::containsWord (const QString& w) const
{
    if (w.isEmpty() || !top)
        return false;

    Node* node = top;
    bool eow = false;
    QChar c;
    int wordLength = w.length();
    for (int i = 0; i < wordLength; ++i) {
        if (!node)
            return false;

        c = w.at (i);
        while (node->letter != c) {
            if (!node->next)
                return false;
            node = node->next;
        }

        eow = node->eow;
        node = node->child;
    }

    return eow;
}

//---------------------------------------------------------------------------
// search
//
//! Search for acceptable words matching a search specification.
//
//! @param spec the search specification
//! @return a list of acceptable words
//---------------------------------------------------------------------------
QStringList
WordGraph::search (const SearchSpec& spec) const
{
    QStringList wordList;
    if (!top)
        return wordList;

    // Use set to eliminate duplicates since patterns with wildcards may match
    // the same word in more than one way
    set <QString> wordSet;
    stack <TraversalState> states;
    QString word;
    QChar c;

    QString unmatched = spec.pattern;
    bool wildcard = false;
    Node* node = top;

    // If Pattern match is unspecified, change it to a single wildcard
    // character.  Also, remove any redundant wildcards.
    if (spec.type == Pattern) {
        if (unmatched.isEmpty())
            unmatched = "*";
        else
            unmatched.replace (QRegExp ("\\*+"), "*");
    }

    // If Anagram or Subanagram match contains a wildcard, note it and remove
    // the wildcard character from the match pattern.
    else if ((spec.type == Anagram) || (spec.type == Subanagram)) {
        wildcard = unmatched.contains ('*');
        if (wildcard)
            unmatched = unmatched.replace ('*', "");
    }

    // Traverse the tree looking for matches
    while (node) {

        // Stop if word is at max length
        if (int (word.length()) < spec.maxLength) {
            QString origWord = word;
            QString origUnmatched = unmatched;

            // Get the next character in the Pattern match.  Allow a wildcard
            // to match the empty string.
            if (spec.type == Pattern) {
                c = unmatched.at (0);
                if (c == "*")
                    states.push (TraversalState (node, word, unmatched.right (
                                unmatched.length() - 1)));
            }

            // Traverse next nodes, looking for matches
            for (; node; node = node->next) {
                if (spec.excludeLetters.contains (node->letter))
                    continue;

                unmatched = origUnmatched;
                word = origWord;

                // Special processing for Pattern match
                if (spec.type == Pattern) {

                    // A node matches wildcard characters or its own letter
                    if (c == node->letter)
                        word += node->letter;
                    else if ((c == "*") || (c == "?"))
                        word += node->letter.lower();
                    else
                        continue;

                    // If this node matches, push its child on the stack to be
                    // traversed later
                    if (node->child) {
                        if (c == "*")
                            states.push (TraversalState (node->child, word,
                                                         unmatched));

                        states.push (TraversalState (node->child, word,
                                                     unmatched.right (
                                                     unmatched.length() - 1)));
                    }

                    // If end of word and end of pattern, put the word in the
                    // list
                    if (node->eow &&
                        ((unmatched.length() == 1) ||
                        ((unmatched.length() == 2) &&
                         (QChar (unmatched.at (1)) == "*"))) &&
                        matchesSpec (word.upper(), spec))
                        wordSet.insert (word);
                }

                // Special processing for Anagram or Subanagram match
                else if ((spec.type == Anagram) || (spec.type == Subanagram)) {

                    // Try to match the current letter against the pattern.
                    // If the letter doesn't match exactly, match a ? char.
                    int index = unmatched.find (node->letter);
                    bool wildcardMatch = false;
                    if (index < 0) {
                        index = unmatched.find ("?");
                        wildcardMatch = true;
                    }
                    bool match = (index >= 0);

                    // If this letter matched or a wildcard was specified,
                    // keep traversing after possibly adding the current word.
                    if (match || wildcard) {
                        word += (match && !wildcardMatch) ? node->letter
                            : node->letter.lower();

                        if (match)
                            unmatched.replace (index, 1, "");

                        if (node->child && (wildcard || !unmatched.isEmpty()))
                            states.push (TraversalState (node->child, word,
                                                         unmatched));

                        if (node->eow &&
                            ((spec.type == Subanagram) ||
                              unmatched.isEmpty()) &&
                            matchesSpec (word.upper(), spec))
                            wordList << word;
                    }
                }
            }
        }

        // Done traversing next nodes, pop a child off the stack
        node = 0;
        if (states.size()) {
            TraversalState state = states.top();
            node = state.node;
            unmatched = state.unmatched;
            word = state.word;
            states.pop();
        }
    }

    // Build word list from word set for Pattern match
    if (spec.type == Pattern) {
        set<QString>::iterator it;
        for (it = wordSet.begin(); it != wordSet.end(); ++it)
            wordList << *it;
    }

    return wordList;
}

//---------------------------------------------------------------------------
// compress
//
//! Compress the graph by eliminating duplication of common suffixes.
//---------------------------------------------------------------------------
void
WordGraph::compress()
{
}

//---------------------------------------------------------------------------
// print
//
//! Print a rudimentary ASCII representation of the graph.
//---------------------------------------------------------------------------
void
WordGraph::print() const
{
    if (!top) {
        cout << "(empty)" << endl;
        return;
    }

    Node* node = top;
    stack< pair<Node*,int> > nodeStack;
    int depth = 0;

    while (node) {
        cout << node->letter;
        if (node->eow)
            cout << "*";
        else
            cout << " ";

        if (!node->child) {
            cout << endl;
            while (!node->next) {
                if (!nodeStack.size())
                    break;
                node = nodeStack.top().first;
                depth = nodeStack.top().second;
                nodeStack.pop();
            }
            node = node->next;
            for (int i = 0; i < depth; ++i)
                cout << "  ";
        }

        else {
            nodeStack.push (make_pair (node, depth));
            node = node->child;
            ++depth;
        }

    }

}

//---------------------------------------------------------------------------
// matchesSpec
//
//! Determine whether a word matches a search specification.  Only the
//! following attributes are checked: Include Letters, Consist Letters/Pct,
//! Min Length.  All other attributes are assumed to have been checked in the
//! course of finding the word to be checked.
//---------------------------------------------------------------------------
bool
WordGraph::matchesSpec (QString word, const SearchSpec& spec) const
{
    // Check Min Length
    if (int (word.length()) < spec.minLength)
        return false;

    // Check Include Letters
    QString tmpWord = word;
    int includeLen = spec.includeLetters.length();
    for (int i = 0; i < includeLen; ++i) {
        int index = tmpWord.find (spec.includeLetters.at (i));
        if (index < 0)
            return false;
        tmpWord.replace (index, 1, "");
    }

    // Check Consist Letters and Consist Percent
    if (spec.consistPercent > 0) {
        int wordLen = word.length();
        int consist = 0;
        for (int i = 0; i < wordLen; ++i) {
            if (spec.consistLetters.contains (word.at (i)))
                ++consist;
        }
        if (((consist * 100) / wordLen) < spec.consistPercent)
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------
// Node
//
//! Constructor.
//---------------------------------------------------------------------------
WordGraph::Node::Node(char c, bool e)
    : letter (c), eow (e), next (0), child (0)
{
}

//---------------------------------------------------------------------------
// ~Node
//
//! Destructor.  Destroy this node along with its next and child nodes.
//---------------------------------------------------------------------------
WordGraph::Node::~Node()
{
    delete next;
    delete child;
}

//---------------------------------------------------------------------------
// operator==
//
//! Compare two nodes to see if they are equivalent.  Two nodes are
//! equivalent if their internal data are equal, and if their CHILD and
//! NEXT pointers are exactly the same.
//
//! @param rhs the right-hand side of the comparison
//! @return true if equivalent, false otherwise
//---------------------------------------------------------------------------
bool
WordGraph::Node::operator== (const Node& rhs)
{
    return ((letter == rhs.letter) && (eow == rhs.eow)
            && (next == rhs.next) && (child == rhs.child));
}
