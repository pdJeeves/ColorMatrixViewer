#include "rotationeditor.h"
#include "ui_rotationeditor.h"

RotationEditor::RotationEditor(QWidget *parent) :
QDialog(parent),
ui(new Ui::RotationEditor)
{
	ui->setupUi(this);
}

RotationEditor::~RotationEditor()
{
	delete ui;
}
