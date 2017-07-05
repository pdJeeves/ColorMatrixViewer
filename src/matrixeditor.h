#ifndef MATRIXEDITOR_H
#define MATRIXEDITOR_H
#include "mainwindow.h"
#include <array>
#include <QDialog>

namespace Ui {
class MatrixEditor;
}

class QSpinBox;
class MainWindow;

class MatrixEditor : public QDialog
{
	Q_OBJECT

public:
	explicit MatrixEditor(MainWindow * window, QWidget *parent = 0);
	~MatrixEditor();

public slots:
	void accepted();
	void rejected();
	void updateMatrixDisplay(int);

private:
	uint8_t originalMatrix[MATRIX_SIZE];

	MainWindow * window;
	std::array<QSpinBox*, MATRIX_SIZE> spinBox;
	Ui::MatrixEditor *ui;
};

#endif // MATRIXEDITOR_H
