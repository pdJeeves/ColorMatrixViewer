#ifndef ROTATIONEDITOR_H
#define ROTATIONEDITOR_H

#include <QDialog>

namespace Ui {
class RotationEditor;
}

class RotationEditor : public QDialog
{
	Q_OBJECT

public:
	explicit RotationEditor(QWidget *parent = 0);
	~RotationEditor();

private:
	Ui::RotationEditor *ui;
};

#endif // ROTATIONEDITOR_H
