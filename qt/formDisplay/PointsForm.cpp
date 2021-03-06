/*Copyright 2002-2014 e-foto team (UERJ)
  This file is part of e-foto.

    e-foto is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    e-foto is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with e-foto.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EDom.h"

#include "PointsForm.h"
#include <QFileDialog>
#include <QFile>
#include <QString>

#include <sstream>

namespace br {
namespace uerj {
namespace eng {
namespace efoto {

PointsForm :: PointsForm(QWidget *parent) : AbstractForm(parent)
{
    setupUi(this);
    connect( pointsTable, SIGNAL( cellClicked(int,int)), this , SLOT( emitSignal(int) ) );
    pointsTable->setColumnHidden(0,true);
    pointsTable->setHorizontalHeaderLabels(QString("key;Point Id;Type;E;N;H").split(";"));

    //connect(importButton, SIGNAL(clicked()), this ,SLOT(importFromTxt()));
}

void PointsForm::fillvalues(std::string values)
{
    EDomElement ede(values);
    int rows=ede.children().size();
    pointsTable->setRowCount(rows);
    pointsTable->setColumnCount(6);

    std::deque<EDomElement> pntChildrens = ede.children();

    for (int i=0;i<rows;i++){

        std::deque<double> values;

        QTableWidgetItem *keyItem = new QTableWidgetItem( QString::fromUtf8 (pntChildrens.at(i).attribute("key").c_str()) );
        QTableWidgetItem *gcpItem = new QTableWidgetItem(pntChildrens.at(i).elementByTagName("pointId").toString().c_str()) ;
        QTableWidgetItem *typeItem = new QTableWidgetItem(pntChildrens.at(i).attribute("type").c_str()) ;
        values = pntChildrens.at(i).elementByTagName("gml:pos").toGmlPos();
        if(values.size() == 3)
        {
            QTableWidgetItem *EItem = new QTableWidgetItem(QString::number(values.at(0),'f',3));
            QTableWidgetItem *NItem = new QTableWidgetItem(QString::number(values.at(1),'f',3));
            QTableWidgetItem *HItem = new QTableWidgetItem(QString::number(values.at(2),'f',3));
            pointsTable->setItem(i,0,keyItem);
            pointsTable->setItem(i,1,gcpItem);
            pointsTable->setItem(i,2,typeItem);
            pointsTable->setItem(i,3,EItem);
            pointsTable->setItem(i,4,NItem);
            pointsTable->setItem(i,5,HItem);
        }
    }

    pntChildrens.clear();

}
std::string PointsForm::getvalues()
{
    std::string xmlString;
    std::stringstream auxStream;

    auxStream << "\nNot Available\n";

    xmlString = auxStream.str();
    return xmlString;
}

void PointsForm ::setReadOnly(bool state)
{
    pointsTable->setEnabled(state);
    // PointsForm is only to read in all times.
}

void PointsForm :: emitSignal(int i)
{
    emit clicked(i);
}

bool PointsForm::isForm(std::string formName)
{
    return !formName.compare("PointsForm");
}


} // namespace efoto
} // namespace eng
} // namespace uerj
} // namespace br
