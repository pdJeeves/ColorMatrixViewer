#include "pigmenteditor.h"
#include "ui_pigmenteditor.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"


PigmentEditor::PigmentEditor(MainWindow * window, QWidget *parent) :
QDialog(parent),
window(window),
ui(new Ui::PigmentEditor)
{
	ui->setupUi(this);

	memcpy(original, window->pigments, sizeof(window->pigments));

	sliders[0] = ui->horizontalSlider;
	sliders[1] = ui->horizontalSlider_2;
	sliders[2] = ui->horizontalSlider_3;
	sliders[3] = ui->horizontalSlider_4;
	sliders[4] = ui->horizontalSlider_5;
	sliders[5] = ui->horizontalSlider_6;


	for(size_t i = 0; i < sliders.size(); ++i)
	{
		sliders[i]->setValue(window->pigments[i]);
		connect(sliders[i], &QSlider::valueChanged, this, &PigmentEditor::updateDisplay);
	}

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &PigmentEditor::accepted);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &PigmentEditor::rejected);

	updateDisplay(0);
}

PigmentEditor::~PigmentEditor()
{
	delete ui;
}

void PigmentEditor::accepted()
{
	accept();
}

void PigmentEditor::rejected()
{
	memcpy(window->pigments, original, sizeof(window->pigments));
	window->applyPigments();
	window->ui->widget->repaint();
	reject();
}

void PigmentEditor::updateDisplay(int)
{
	for(size_t i = 0; i < sliders.size(); ++i)
	{
		window->pigments[i] = sliders[i]->value();
	}

	window->applyPigments();
	window->ui->widget->repaint();
}
