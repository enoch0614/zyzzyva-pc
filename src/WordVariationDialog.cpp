//---------------------------------------------------------------------------
// WordVariationDialog.cpp
//
// A dialog for displaying variations of a word, such as hooks, anagrams, etc.
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

#include "WordVariationDialog.h"
#include "Defs.h"
#include "DefinitionLabel.h"
#include "WordEngine.h"
#include "WordListView.h"
#include "WordListViewItem.h"
#include <qapplication.h>
#include <qlayout.h>
#include <set>

using namespace Defs;
using std::set;

//---------------------------------------------------------------------------
//  WordVariationDialog
//
//! Constructor.
//
//! @param parent the parent widget
//! @param name the name of this widget
//! @param modal whether the dialog is modal
//! @param f widget flags
//---------------------------------------------------------------------------
WordVariationDialog::WordVariationDialog (WordEngine* we, const QString& word,
                                          WordVariationType variation,
                                          QWidget* parent, const char* name,
                                          bool modal, WFlags f)
    : QDialog (parent, name, modal, f), wordEngine (we)
{
    QVBoxLayout* mainVlay = new QVBoxLayout (this, MARGIN, SPACING,
                                             "mainVlay");
    Q_CHECK_PTR (mainVlay);

    QHBoxLayout* labelHlay = new QHBoxLayout (SPACING, "labelHlay");
    Q_CHECK_PTR (labelHlay);
    mainVlay->addLayout (labelHlay);

    labelHlay->addStretch (1);

    wordLabel = new DefinitionLabel (this, "wordLabel");
    Q_CHECK_PTR (wordLabel);
    labelHlay->addWidget (wordLabel);

    labelHlay->addStretch (1);

    QHBoxLayout* listHlay = new QHBoxLayout (SPACING, "listHlay");
    Q_CHECK_PTR (listHlay);
    mainVlay->addLayout (listHlay);

    leftList = new WordListView (wordEngine, this, "leftList");
    Q_CHECK_PTR (leftList);
    leftList->setResizeMode (QListView::LastColumn);
    leftList->setShowSortIndicator(true);
    listHlay->addWidget (leftList);

    rightList = new WordListView (wordEngine, this, "rightList");
    Q_CHECK_PTR (rightList);
    rightList->setResizeMode (QListView::LastColumn);
    rightList->setShowSortIndicator(true);
    rightList->hide();
    listHlay->addWidget (rightList);

    QHBoxLayout* buttonHlay = new QHBoxLayout (SPACING, "buttonHlay");
    Q_CHECK_PTR (buttonHlay);
    mainVlay->addLayout (buttonHlay);

    buttonHlay->addStretch (1);

    closeButton = new QPushButton ("&Close", this, "closeButton");
    Q_CHECK_PTR (closeButton);
    closeButton->setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);
    closeButton->setAutoDefault (false);
    closeButton->setFocus();
    connect (closeButton, SIGNAL (clicked()), SLOT (accept()));
    buttonHlay->addWidget (closeButton);

    setWordVariation (word, variation);
    resize (minimumSizeHint().width(), 500);
}

//---------------------------------------------------------------------------
//  ~WordVariationDialog
//
//! Destructor.
//---------------------------------------------------------------------------
WordVariationDialog::~WordVariationDialog()
{
}

//---------------------------------------------------------------------------
//  setWordVariation
//
//! Display the front and back hooks of a word.
//
//! @param word the word whose hooks are displayed
//! @param variation the variation of the word to display
//---------------------------------------------------------------------------
void
WordVariationDialog::setWordVariation (const QString& word, WordVariationType
                                       variation)
{
    QString title;
    SearchSpec spec;
    QValueList<SearchSpec> leftSpecs;
    QValueList<SearchSpec> rightSpecs;
    switch (variation) {

        case VariationAnagrams:
        title = "Anagrams for: " + word;
        spec.type = Anagram;
        spec.pattern = word;
        leftSpecs << spec;
        leftList->setTitle ("Anagrams");
        break;

        case VariationSubanagrams:
        title = "Subanagrams for: " + word;
        spec.type = Subanagram;
        spec.pattern = word;
        leftSpecs << spec;
        leftList->setTitle ("Subanagrams");
        break;

        case VariationHooks:
        title = "Hooks for: " + word;
        spec.type = Pattern;
        spec.pattern = "?" + word;
        leftSpecs << spec;
        spec.pattern = word + "?";
        rightSpecs << spec;
        leftList->setTitle ("Front Hooks");
        rightList->setTitle ("Back Hooks");
        break;

        case VariationAnagramHooks:
        title = "Anagram Hooks for: " + word;
        spec.type = Anagram;
        spec.pattern = "?" + word;
        leftSpecs << spec;
        leftList->setTitle ("Anagram Hooks");
        break;

        case VariationBlankAnagrams:
        title = "Blank Anagrams for: " + word;
        spec.type = Anagram;
        for (int i = 0; i < word.length(); ++i) {
            spec.pattern = word.left (i) + "?" +
                word.right (word.length() - i - 1);
            leftSpecs << spec;
        }
        leftList->setTitle ("Blank Anagrams");
        break;

        case VariationBlankMatches:
        title = "Blank Matches for: " + word;
        spec.type = Pattern;
        for (int i = 0; i < word.length(); ++i) {
            spec.pattern = word.left (i) + "?" +
                word.right (word.length() - i - 1);
            leftSpecs << spec;
        }
        leftList->setTitle ("Blank Matches");
        break;

        case VariationExtensions:
        title = "Extensions for: " + word;
        spec.type = Pattern;
        spec.pattern = "*?" + word;
        leftSpecs << spec;
        spec.pattern = word + "?*";
        rightSpecs << spec;
        leftList->setTitle ("Front Extensions");
        rightList->setTitle ("Back Extensions");
        break;

        case VariationTranspositions:
        title = "Transpositions for: " + word;
        spec.type = Pattern;
        for (int i = 0; i < word.length() - 1; ++i) {
            spec.pattern = word.left (i) + word.mid (i + 1, 1) +
                word.mid (i, 1) + word.right (word.length() - i - 2);
            leftSpecs << spec;
        }
        leftList->setTitle ("Transpositions");
        break;

        default: break;
    }

    setCaption (title);
    wordLabel->setText (title);

    QApplication::setOverrideCursor (Qt::waitCursor);

    set<QString> wordSet;
    QStringList wordList;
    QStringList::iterator wit;
    QValueList<SearchSpec>::iterator sit;
    for (sit = leftSpecs.begin(); sit != leftSpecs.end(); ++sit) {
        wordList = wordEngine->search (*sit, true);
        for (wit = wordList.begin(); wit != wordList.end(); ++wit) {
            if (wordSet.find (*wit) == wordSet.end())
                new WordListViewItem (leftList, *wit);
            wordSet.insert (*wit);
        }
    }

    if (!rightSpecs.empty()) {
        wordSet.clear();
        for (sit = rightSpecs.begin(); sit != rightSpecs.end(); ++sit) {
            wordList = wordEngine->search (*sit, true);
            for (wit = wordList.begin(); wit != wordList.end(); ++wit) {
                if (wordSet.find (*wit) == wordSet.end())
                    new WordListViewItem (rightList, *wit);
                wordSet.insert (*wit);
            }
        }
        rightList->show();
    }

    QApplication::restoreOverrideCursor();
}
