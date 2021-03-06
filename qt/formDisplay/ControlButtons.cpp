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
#include "ControlButtons.h"
#include <QPushButton>
#include <QLayout>

ControlButtons::ControlButtons()
{
	mainLayout = new QHBoxLayout;
	editButton = new QPushButton(tr("Edit"));
	deleteButton = new QPushButton(tr("Delete"));
	saveButton = new QPushButton(tr("Ok"));;
	cancelButton = new QPushButton(tr("Cancel"));
	newButton = new QPushButton(tr("New"));

	mainLayout->addStretch();
	mainLayout->addWidget(newButton);
	mainLayout->addWidget(editButton);
	mainLayout->addWidget(deleteButton);
	mainLayout->addWidget(saveButton);
	mainLayout->addWidget(cancelButton);
	mainLayout->addStretch();

	newButton->setVisible(false);
	editButton->setVisible(false);
	deleteButton->setVisible(false);
	saveButton->setVisible(false);
	cancelButton->setVisible(false);

	deleteButton->setEnabled(false);

	this->setLayout(mainLayout);
}

void ControlButtons::toSimpleMode()
{
	newButton->setVisible(false);
	editButton->setVisible(true);
	deleteButton->setVisible(true);
	saveButton->setVisible(false);
	cancelButton->setVisible(false);
}

void ControlButtons::toSingleMode()
{
	newButton->setVisible(false);
	editButton->setVisible(true);
	deleteButton->setVisible(false);
	saveButton->setVisible(false);
	cancelButton->setVisible(false);
}

void ControlButtons::toMultiMode()
{
	newButton->setVisible(true);
	editButton->setVisible(false);
	deleteButton->setVisible(true);
	saveButton->setVisible(false);
	cancelButton->setVisible(false);
}

void ControlButtons::toEditMode()
{
	newButton->setVisible(false);
	editButton->setVisible(false);
	deleteButton->setVisible(false);
	saveButton->setVisible(true);
	cancelButton->setVisible(true);
}

void ControlButtons::toInvisibleMode()
{
	newButton->setVisible(false);
	editButton->setVisible(false);
	deleteButton->setVisible(false);
	saveButton->setVisible(false);
	cancelButton->setVisible(false);
}

void ControlButtons::disconnectAll()
{
	newButton->disconnect();
	editButton->disconnect();
	deleteButton->disconnect();
	saveButton->disconnect();
	cancelButton->disconnect();
}

void ControlButtons::simpleConnect()
{
	toSimpleMode();
	connect(newButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(editButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(toInvisibleMode()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(toSimpleMode()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(toSimpleMode()));
}

void ControlButtons::singleConnect()
{
	toSingleMode();
	connect(newButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(editButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(toInvisibleMode()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(toSingleMode()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(toSingleMode()));
}

void ControlButtons::multiConnect()
{
	toMultiMode();
	connect(newButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(editButton, SIGNAL(clicked()), this, SLOT(toEditMode()));
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(toInvisibleMode()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(toMultiMode()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(toMultiMode()));
}
