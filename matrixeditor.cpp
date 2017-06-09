#include "matrixeditor.h"
#include "ui_matrixeditor.h"
#include "ui_mainwindow.h"

#include "mainwindow.h"

MatrixEditor::MatrixEditor(MainWindow * window, QWidget *parent) :
QDialog(parent),
window(window),
ui(new Ui::MatrixEditor)
{
	ui->setupUi(this);

	memcpy(originalMatrix, window->matrix, sizeof(window->matrix));

	spinBox[0] = ui->spinBox;
	spinBox[1] = ui->spinBox_2;
	spinBox[2] = ui->spinBox_3;
	spinBox[3] = ui->spinBox_4;
	spinBox[4] = ui->spinBox_5;
	spinBox[5] = ui->spinBox_6;
	spinBox[6] = ui->spinBox_7;
	spinBox[7] = ui->spinBox_8;
	spinBox[8] = ui->spinBox_9;
	spinBox[9] = ui->spinBox_10;
	spinBox[10] = ui->spinBox_11;
	spinBox[11] = ui->spinBox_12;
	spinBox[12] = ui->spinBox_13;
	spinBox[13] = ui->spinBox_14;
	spinBox[14] = ui->spinBox_15;

	for(size_t i = 0; i < MATRIX_SIZE; ++i)
	{
		spinBox[i]->setValue(window->matrix[i]);
		connect(spinBox[i], SIGNAL(valueChanged(int)), this, SLOT(updateMatrixDisplay(int)));
	}

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &MatrixEditor::accepted);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &MatrixEditor::rejected);

	updateMatrixDisplay(0);
}

MatrixEditor::~MatrixEditor()
{
	delete ui;
}

void MatrixEditor::accepted()
{
	accept();
}

void MatrixEditor::rejected()
{
	memcpy(window->matrix, originalMatrix, sizeof(window->matrix));
	window->applyMatrix();
	window->ui->widget->repaint();
	reject();
}

void MatrixEditor::updateMatrixDisplay(int)
{
	for(size_t i = 0; i < MATRIX_SIZE; ++i)
	{
		window->matrix[i] = spinBox[i]->value();
	}

	window->applyMatrix();
	window->ui->widget->repaint();
}
