#include "keycustomize.h"
#include "ui_keycustomize.h"

#include "../pok3rtool/keymap.h"

#include "zlog.h"
using namespace LibChaos;

inline QString toQStr(ZString str){
    return QString::fromUtf8(str.raw(), str.size());
}

KeyCustomize::KeyCustomize(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyCustomize)
{
    ui->setupUi(this);

    model = new QStandardItemModel;
    model->setColumnCount(2);
    proxy = new QSortFilterProxyModel;
    proxy->setSourceModel(model);

    ui->tableView->setModel(proxy);

    for(auto it = Keymap::getAllKeycodes().cbegin(); it.more(); ++it){
        QList<QStandardItem *> row;
        row.push_back(new QStandardItem(toQStr(it.get().name)));
        row.push_back(new QStandardItem(toQStr(it.get().desc)));
        model->appendRow(row);
    }
}

KeyCustomize::~KeyCustomize(){
    delete ui;
}

void KeyCustomize::accept(){
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(select->hasSelection()){
        QModelIndexList list = select->selectedRows();
        if(list.size() == 1){
            QModelIndex idx = list.at(0);
            LOG("Selected: " << model->item(idx.row())->text().toStdString());
            QDialog::accept();
            ui->searchKey->clear();
            ui->tableView->selectionModel()->clearSelection();
            emit acceptedKey(currentKey, model->item(idx.row())->text());
        }
    }
}

void KeyCustomize::reject(){
    QDialog::reject();
    ui->searchKey->clear();
    ui->tableView->selectionModel()->clearSelection();
}

void KeyCustomize::on_searchKey_textChanged(const QString &str){
    LOG("Filter " << str.toStdString());

    QString regExp = QString(".*%1.*").arg(str);
    proxy->setFilterRegExp(QRegExp(regExp, Qt::CaseInsensitive));
    proxy->setFilterKeyColumn(1);
}
