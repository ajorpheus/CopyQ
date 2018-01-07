/*
    Copyright (c) 2018, Lukas Holecek <hluk@email.cz>

    This file is part of CopyQ.

    CopyQ is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CopyQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CopyQ.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "iconselectdialog.h"

#include "common/config.h"
#include "gui/iconfont.h"
#include "gui/icons.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QKeyEvent>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

class IconListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit IconListWidget(QWidget *parent)
        : QListWidget(parent)
    {
        const int gridSize = iconFontSizePixels() + 8;
        const QSize size(gridSize, gridSize);

        setViewMode(QListView::IconMode);

        setFont( iconFont() );
        setGridSize(size);
        setResizeMode(QListView::Adjust);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setDragDropMode(QAbstractItemView::NoDragDrop);

        addItem( QString("") );
        item(0)->setSizeHint(size);
    }

    QString addIcon(ushort unicode, const QStringList &searchTerms)
    {
        const QString icon(unicode);
        auto item = new QListWidgetItem(icon, this);
        item->setSizeHint( gridSize() );
        item->setToolTip( searchTerms.join(", ") );
        return icon;
    }

    QString icon(const QModelIndex &index) const
    {
        return item(index.row())->text();
    }

    bool isIconSelected() const
    {
        const QModelIndex index = currentIndex();
        return index.isValid() && item(index.row())->isSelected();
    }

    void keyboardSearch(const QString &search) override
    {
        if (!m_search) {
            m_search = new QLineEdit(this);
            connect( m_search, SIGNAL(textChanged(QString)),
                     this, SLOT(onSearchTextChanged(QString)) );
            m_search->show();
            updateSearchPosition();
        }

        m_search->setText( m_search->text() + search );
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        if (m_search && event->key() == Qt::Key_Escape) {
            event->accept();
            stopSearch();
            return;
        }

        QListView::keyPressEvent(event);
    }

    void resizeEvent(QResizeEvent *event) override
    {
        QListWidget::resizeEvent(event);
        if (m_search)
            updateSearchPosition();
    }

private slots:
    void onSearchTextChanged(const QString &text)
    {
        if ( text.isEmpty() )
            stopSearch();
        else
            search( text.toLower() );
    }

private:
    void updateSearchPosition()
    {
        if (!m_search)
            return;

        const auto sizeDiff = this->size() - m_search->size();
        m_search->move( sizeDiff.width(), sizeDiff.height() );
    }

    void search(const QString &text)
    {
        setCurrentItem(nullptr);
        for (int row = 0; row < count(); ++row) {
            auto item = this->item(row);
            const bool matches = item->toolTip().contains(text);
            item->setHidden(!matches);
            if (matches && currentItem() == nullptr)
                setCurrentItem(item);
        }
    }

    void stopSearch()
    {
        if (!m_search)
            return;

        m_search->deleteLater();
        m_search = nullptr;
        search(QString());
        setFocus();
    }

    QLineEdit *m_search = nullptr;
};

IconSelectDialog::IconSelectDialog(const QString &defaultIcon, QWidget *parent)
    : QDialog(parent)
    , m_iconList(new IconListWidget(this))
    , m_selectedIcon(defaultIcon)
{
    setWindowTitle( tr("CopyQ Select Icon") );

    connect( m_iconList, SIGNAL(activated(QModelIndex)),
             this, SLOT(onIconListItemActivated(QModelIndex)) );

    addIcons();

    QPushButton *browseButton = new QPushButton(tr("Browse..."), this);
    if ( m_selectedIcon.size() > 2 )
        browseButton->setIcon(QIcon(m_selectedIcon));
    connect( browseButton, SIGNAL(clicked()),
             this, SLOT(onBrowse()) );

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    connect( buttonBox, SIGNAL(rejected()),
             this, SLOT(reject()) );
    connect( buttonBox, SIGNAL(accepted()),
             this, SLOT(onAcceptCurrent()) );

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_iconList);

    auto buttonLayout = new QHBoxLayout;
    layout->addLayout(buttonLayout);
    buttonLayout->addWidget(browseButton);
    buttonLayout->addWidget(buttonBox);

    m_iconList->setFocus();

    // Restore previous geometry.
    restoreWindowGeometry(this, false);
}

void IconSelectDialog::done(int result)
{
    saveWindowGeometry(this, false);

    if (result == QDialog::Accepted)
        emit iconSelected(m_selectedIcon);

    QDialog::done(result);
}

void IconSelectDialog::onIconListItemActivated(const QModelIndex &index)
{
    m_selectedIcon = m_iconList->icon(index);
    accept();
}

void IconSelectDialog::onBrowse()
{
    const QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Icon file"), m_selectedIcon,
                tr("Image Files (*.png *.jpg *.jpeg *.bmp *.ico *.svg)"));
    if ( !fileName.isNull() ) {
        m_selectedIcon = fileName;
        accept();
    }
}

void IconSelectDialog::onAcceptCurrent()
{
    if ( m_iconList->isIconSelected() )
        onIconListItemActivated( m_iconList->currentIndex() );
    else
        reject();
}

void IconSelectDialog::addIcons()
{
#include "add_icons.h"
}

void IconSelectDialog::addIcon(ushort unicode, const QStringList &searchTerms)
{
    const auto icon = m_iconList->addIcon(unicode, searchTerms);
    if (m_selectedIcon == icon)
        m_iconList->setCurrentRow(m_iconList->count() - 1);
}

#include "iconselectdialog.moc"
