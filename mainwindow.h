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
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();


	void draw(QPainter & painter, QSize size);
	bool event(QEvent * event) Q_DECL_OVERRIDE;

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
	void applyMatrix();

	bool openFile(QImage *slot, QImage * other, const QString & filename);
	bool saveFile(const QString & filename);


	uint8_t matrix[MATRIX_SIZE];
	uint8_t angles[3];

	QString filename;

	QImage original;
	QImage modifier;
	QImage render;

	double zoom;
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
