/*
 *      textedit.cpp
 *
 *      Copyright 2008 gpg4usb-team <gpg4usb@cpunk.de>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "QDebug"
#include "textedit.h"
#include "quitdialog.h"
class QFileDialog;
class QMessageBox;

TextEdit::TextEdit(QString iconPath)
{
    mIconPath = iconPath;
    countPage = 0;
    tabWidget = new QTabWidget(this);
    tabWidget->setMovable(true);
    tabWidget->setTabsClosable(true);
    tabWidget->setDocumentMode(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tabWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeTab(int)));
    newTab();
    setAcceptDrops(false);
}

void TextEdit::newTab()
{
    QString header = tr("untitled") +
                     QString::number(++countPage)+".txt";

    EditorPage *page = new EditorPage();
    tabWidget->addTab(page, header);
    tabWidget->setCurrentIndex(tabWidget->count() - 1);
    page->getTextPage()->setFocus();
    connect(page->getTextPage(), SIGNAL(modificationChanged(bool)), this, SLOT(showModified()));
//    setCursorPosition();
 }

void TextEdit::open()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open file"),
                                                          QDir::currentPath());
    foreach (QString fileName,fileNames){
        if (!fileName.isEmpty()) {
            QFile file(fileName);

            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                EditorPage *page = new EditorPage(fileName);

                QTextStream in(&file);
                QApplication::setOverrideCursor(Qt::WaitCursor);
                page->getTextPage()->setPlainText(in.readAll());
                page->setFilePath(fileName);
                QTextDocument *document = page->getTextPage()->document();
                document->setModified(false);

                tabWidget->addTab(page, strippedName(fileName));
                tabWidget->setCurrentIndex(tabWidget->count() - 1);
                QApplication::restoreOverrideCursor();
                page->getTextPage()->setFocus();
                connect(page->getTextPage(), SIGNAL(modificationChanged(bool)), this, SLOT(showModified()));
                //       setCursorPosition();
                //enableAction(true)
            } else {
                QMessageBox::warning(this, tr("Application"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(fileName)
                                     .arg(file.errorString()));
            }
        }
    }
}

void TextEdit::save()
{
    QString fileName = curPage()->getFilePath();

    if (fileName.isEmpty()) {
        //QString docname = tabWidget->tabText(tabWidget->currentIndex());
        //docname.remove(0,2);
        saveAs();
    } else {
        saveFile(fileName);
    }
}

bool TextEdit::saveFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream outputStream(&file);
        EditorPage *page = curPage();

        outputStream << page->getTextPage()->toPlainText();

        QTextDocument *document = page->getTextPage()->document();
        document->setModified(false);

        int curIndex = tabWidget->currentIndex();
        tabWidget->setTabText(curIndex, strippedName(fileName));
        page->setFilePath(fileName);
  //      statusBar()->showMessage(tr("File saved"), 2000);

        return true;
    } else {
        QMessageBox::warning(this, tr("File"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
}


bool TextEdit::saveAs()
{
    EditorPage *page = curPage();
    QString path;
    if(page->getFilePath() != "") {
        path = page->getFilePath();
    } else {
        path = tabWidget->tabText(tabWidget->currentIndex()).remove(0,2);
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file "),
                                                    path);
    return saveFile(fileName);
}


/**
 * close current tab
 */
void TextEdit::closeTab()
{
    removeTab(tabWidget->currentIndex());
    if (tabWidget->count() != 0) {
        curPage()->getTextPage()->setFocus();
    }
}

void TextEdit::removeTab(int index)
{
    // Do nothing, if no tab is opened
    if (tabWidget->count() == 0) {
        return;
    }

    // get the index of the actual current tab
    int lastIndex = tabWidget->currentIndex();

    // set the focus to argument index
    tabWidget->setCurrentIndex(index);

    if (maybeSaveCurrentTab(true)) {
        tabWidget->removeTab(index);

        if(index >= lastIndex) {
            tabWidget->setCurrentIndex(lastIndex);
        } else {
            tabWidget->setCurrentIndex(lastIndex-1);
        }
    }

    if (tabWidget->count() == 0) {
      //  enableAction(false);
    }
}


 /**
  * Check if current may need to be saved.
  * Call this function before closing the currently active tab-
  *
  * If it returns false, the close event should be aborted.
  */
bool TextEdit::maybeSaveCurrentTab(bool askToSave) {
    EditorPage *page = curPage();
    QTextDocument *document = page->getTextPage()->document();

    if (document->isModified()) {
        QMessageBox::StandardButton result;

        // write title of tab to docname and remove the leading *
        QString docname = tabWidget->tabText(tabWidget->currentIndex());
        docname.remove(0,2);

        QString filePath = page->getFilePath();
        if (askToSave) {
            result = QMessageBox::warning(this, tr("Unsaved document"),
                                   tr("<h3>The document \"%1\" has been modified.<br/>Do you want to save your changes?</h3>").arg(docname)+
                                   tr("<b>Note:</b> If you don't save these files, all changes are lost.<br/>"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        }
        if ((result == QMessageBox::Save) || (!askToSave)) {
            if (filePath == "") {
                //QString docname = tabWidget->tabText(tabWidget->currentIndex());
                //docname.remove(0,2);
                return saveAs();
            } else {
                return saveFile(filePath);
            }
        } else if (result == QMessageBox::Discard) {
            return true;
        } else {
            return false;
        }
    }
    return true;
}

/**
 *  Checks if there are unsaved documents in any tab,
 *  which may need to be saved. Call this function before
 *  closing the programme or all tabs.
 *
 *   If it returns false, the close event should be aborted.
 */
bool TextEdit::maybeSaveAnyTab()
{
    // get a list of all unsaved documents and their tabids
    QHash<int, QString> unsavedDocs = this->unsavedDocuments();

    /*
    * no unsaved documents, so app can be closed
    */
    if (unsavedDocs.size() == 0) {
        return true;
    }
    /*
     * only 1 unsaved document -> set modified tab as current
     * and show normal unsaved doc dialog
     */
    if(unsavedDocs.size() == 1) {
        int modifiedTab = unsavedDocs.keys().at(0);
        tabWidget->setCurrentIndex(modifiedTab);
        return maybeSaveCurrentTab(true);
    }

    /*
     * more than one unsaved documents
     */
    if(unsavedDocs.size() > 1) {
        QHashIterator<int, QString> i (unsavedDocs);

        QuitDialog *dialog;
        dialog=new QuitDialog(this, unsavedDocs, mIconPath);
        int result = dialog->exec();

        // if result is QDialog::Rejected, discard or cancel was clicked
        if (result == QDialog::Rejected){
            // return true, if discard is clicked, so app can be closed
            if (dialog->isDiscarded()){
                return true;
            } else {
                return false;
            }
        } else {
            bool allsaved=true;
            QList <int> tabIdsToSave = dialog->getTabIdsToSave();

            foreach (int tabId, tabIdsToSave) {
                tabWidget->setCurrentIndex(tabId);
                if (! maybeSaveCurrentTab(false)) {
                    allsaved=false;
                }
            }
            if (allsaved) {
                return true;
            } else {
                return false;
            }
        }
    }
    // code should never reach this statement
    return false;
}


QPlainTextEdit* TextEdit::curTextPage()
{
    EditorPage *curTextPage = qobject_cast<EditorPage *>(tabWidget->currentWidget());
    return curTextPage->getTextPage();
}


EditorPage* TextEdit::curPage()
{
    EditorPage *curPage = qobject_cast<EditorPage *>(tabWidget->currentWidget());
    return curPage;
}

void TextEdit::quote()
{
    QTextCursor cursor(curTextPage()->document());

    // beginEditBlock and endEditBlock() let operation look like single undo/redo operation
    cursor.beginEditBlock();
    cursor.setPosition(0);
    cursor.insertText("> ");
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.movePosition(QTextCursor::NextCharacter);
        if(!cursor.atEnd()) {
            cursor.insertText("> ");
        }
    }
    cursor.endEditBlock();
}

void TextEdit::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    curTextPage()->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();
    curPage()->setFilePath(fileName);
    tabWidget->setTabText(tabWidget->currentIndex(), strippedName(fileName));

   // statusBar()->showMessage(tr("File loaded"), 2000);
}

QString TextEdit::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void TextEdit::print()
{
#ifndef QT_NO_PRINTER
    QTextDocument *document = curTextPage()->document();
    QPrinter printer;

    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (dlg->exec() != QDialog::Accepted) {
        return;
    }
    document->print(&printer);

    //statusBar()->showMessage(tr("Ready"), 2000);
#endif
}

/** put a * in front of every modified document tab
  */
void TextEdit::showModified() {
    int index=tabWidget->currentIndex();
    QString title= tabWidget->tabText(index);
    // if doc is modified now, add leading * to title,
    // otherwise remove the leading * from the title
    if(curTextPage()->document()->isModified()) {
        tabWidget->setTabText(index, title.prepend("* "));
    } else {
        tabWidget->setTabText(index, title.remove(0,2));
    }
}

void TextEdit::switchTabUp() {
    if (tabWidget->count() > 1) {
        int newindex=(tabWidget->currentIndex()+1)%(tabWidget->count());
        tabWidget->setCurrentIndex(newindex);
    }
}

void TextEdit::switchTabDown() {
    if (tabWidget->count() > 1) {
        int newindex=(tabWidget->currentIndex()-1+tabWidget->count())%tabWidget->count();
        tabWidget->setCurrentIndex(newindex);
    }
}

/*
 *   return a hash of tabindexes and title of unsaved tabs
 */
QHash<int, QString> TextEdit::unsavedDocuments() {
    QHash<int, QString> unsavedDocs;  // this list could be used to implement gedit like "unsaved changed"-dialog

    for(int i=0; i < tabWidget->count(); i++) {
        EditorPage *ep = qobject_cast<EditorPage *> (tabWidget->widget(i));
        if(ep->getTextPage()->document()->isModified()) {
            QString docname = tabWidget->tabText(i);
            // remove * before name of modified doc
            docname.remove(0,2);
            unsavedDocs.insert(i, docname);
        }
    }
    return unsavedDocs;
}

void TextEdit::cut()
{
    curTextPage()->cut();
}

void TextEdit::copy()
{
    curTextPage()->copy();
}

void TextEdit::paste()
{
    curTextPage()->paste();
}

void TextEdit::undo()
{
    curTextPage()->undo();
}

void TextEdit::redo()
{
    curTextPage()->redo();
}

void TextEdit::selectAll()
{
    curTextPage()->selectAll();
}

/*void TextEdit::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/plain"))
        qDebug() << "enter textedit drag action";
        event->acceptProposedAction();
}

void TextEdit::dropEvent(QDropEvent* event)
{
    curTextPage()->setPlainText(event->mimeData()->text());

    foreach (QUrl tmp, event->mimeData()->urls())
    {
        qDebug() << "hallo" << tmp;
    }

    //event->acceptProposedAction();
}
*/
