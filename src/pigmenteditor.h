#ifndef PIGMENTEDITOR_H
#define PIGMENTEDITOR_H
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
	uint8_t original[4];

	MainWindow * window;
	std::array<QSlider*, 4> sliders;
	Ui::PigmentEditor *ui;
};

#endif // ROTATIONEDITOR_H
