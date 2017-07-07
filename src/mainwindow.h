#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MatrixEditor;
class RotationEditor;

#define MATRIX_ROWS 3
#define MATRIX_COLS 5
#define MATRIX_SIZE (MATRIX_ROWS*MATRIX_COLS)

class MainWindow : public QMainWindow
{
typedef QMainWindow super;
friend class MatrixEditor;
friend class RotationEditor;
friend class PigmentEditor;
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


	void draw(QPainter & painter, QSize size);
	bool event(QEvent * event) Q_DECL_OVERRIDE;

	uint8_t matrix[MATRIX_SIZE];
	uint8_t angles[3];
	uint8_t pigments[6];

	static float applyPigment(float color, float pigment);

private:
	void reset();

	void documentNew();
	void documentClose();

	void documentOpenOriginal();
	void documentOpenModifier();
	void documentSave();
	void documentSaveAs();

	void editCopy();
	void editPaste();

	void editMatrix();
	void editAngles();
	void editPigments();

	void applyMatrix();
	void applyAngles();
	void applyPigments();

	void onNegate();

	bool openFile(QImage *slot, QImage * other, const QString & filename);
	bool saveFile(const QString & filename);



	QString filename;

	QImage original;
	QImage modifier;
	QImage render;

	double zoom;
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
