#ifndef ROTATIONEDITOR_H
#define ROTATIONEDITOR_H

#include <QDialog>
#include <array>

class MainWindow;
class QSlider;

namespace Ui {
class RotationEditor;
}

class RotationEditor : public QDialog
{
	Q_OBJECT

public:
	explicit RotationEditor(MainWindow * window, QWidget *parent = 0);
	~RotationEditor();

	void accepted();
	void rejected();
	void updateAngleDisplay(int);

private:
	uint8_t originalAngles[3];

	MainWindow * window;
	std::array<QSlider*, 3> sliders;
	Ui::RotationEditor *ui;
};

#endif // ROTATIONEDITOR_H
