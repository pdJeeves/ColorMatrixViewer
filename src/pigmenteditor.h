#ifndef PIGMENTEDITOR_H
#define PIGMENTEDITOR_H
#include "mainwindow.h"
#include <QDialog>
#include <array>

class MainWindow;
class QSlider;

namespace Ui {
class PigmentEditor;
}

class PigmentEditor : public QDialog
{
	Q_OBJECT

public:
	explicit PigmentEditor(MainWindow * window, QWidget *parent = 0);
	~PigmentEditor();

	void accepted();
	void rejected();
	void updateDisplay(int);

private:
	uint8_t original[sizeof(((MainWindow*)0L)->pigments)];

	MainWindow * window;
	std::array<QSlider*, sizeof(((MainWindow*)0L)->pigments)> sliders;
	Ui::PigmentEditor *ui;
};

#endif // ROTATIONEDITOR_H
