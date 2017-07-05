#include "rotationeditor.h"
#include "ui_rotationeditor.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"


RotationEditor::RotationEditor(MainWindow * window, QWidget *parent) :
QDialog(parent),
window(window),
ui(new Ui::RotationEditor)
{
	ui->setupUi(this);

	memcpy(originalAngles, window->angles, sizeof(window->angles));

	sliders[0] = ui->horizontalSlider;
	sliders[1] = ui->horizontalSlider_2;
	sliders[2] = ui->horizontalSlider_3;

	for(size_t i = 0; i < sliders.size(); ++i)
	{
		sliders[i]->setValue(window->angles[i]);
		connect(sliders[i], &QSlider::valueChanged, this, &RotationEditor::updateAngleDisplay);
	}

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RotationEditor::accepted);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RotationEditor::rejected);

	updateAngleDisplay(0);
}

RotationEditor::~RotationEditor()
{
	delete ui;
}

void RotationEditor::accepted()
{
	accept();
}

void RotationEditor::rejected()
{
	memcpy(window->angles, originalAngles, sizeof(window->angles));
	window->applyAngles();
	window->ui->widget->repaint();
	reject();
}

void RotationEditor::updateAngleDisplay(int)
{
	for(size_t i = 0; i < sliders.size(); ++i)
	{
		window->angles[i] = sliders[i]->value();
	}

	window->applyAngles();
	window->ui->widget->repaint();
}
