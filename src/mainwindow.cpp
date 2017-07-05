#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QFileDialog>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QPainter>
#include <QDir>
#include <cmath>
#include "quaternion.h"

#include "matrixeditor.h"
#include "rotationeditor.h"
#include "pigmenteditor.h"

const static double zoomFactor = .8;

MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	reset();

	connect(ui->actionEdit_Matrix, &QAction::triggered, this, &MainWindow::editMatrix);
	connect(ui->actionEdit_Angles, &QAction::triggered, this, &MainWindow::editAngles);
	connect(ui->actionEdit_Pigments, &QAction::triggered, this, &MainWindow::editPigments);

	connect(ui->actionClose, &QAction::triggered, this, &MainWindow::documentClose);
	connect(ui->actionNew, &QAction::triggered, this, &MainWindow::documentNew);
	connect(ui->actionLoad_Base, &QAction::triggered, this, &MainWindow::documentOpenOriginal);
	connect(ui->actionLoad_Modifier, &QAction::triggered, this, &MainWindow::documentOpenModifier);
	connect(ui->actionSave, &QAction::triggered, this, &MainWindow::documentSave);
	connect(ui->actionSave_As, &QAction::triggered, this, &MainWindow::documentSaveAs);

	connect(ui->actionReload, &QAction::triggered, this, &MainWindow::reset);
	connect(ui->actionNegate, &QAction::triggered, this, &MainWindow::onNegate);

	connect(ui->actionZoom_Out, &QAction::triggered, this, [this]() { zoom *= zoomFactor; ui->widget->repaint(); });
	connect(ui->actionZoom_In, &QAction::triggered, this, [this]() { zoom *= 1 / zoomFactor; ui->widget->repaint(); });
	connect(ui->actionZoom_100, &QAction::triggered, this, [this]() { zoom = 1.0; ui->widget->repaint(); });

	connect(ui->horizontalScrollBar, &QScrollBar::valueChanged, [this](int) { ui->widget->repaint(); });
	connect(ui->verticalScrollBar, &QScrollBar::valueChanged, [this](int) { ui->widget->repaint(); });

	ui->widget->window = this;
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::reset()
{
	memset(matrix, 0, MATRIX_SIZE);
	memset(angles, 0, sizeof(angles));
	memset(pigments, 128, sizeof(pigments));

	for(size_t y = 0; y < MATRIX_ROWS; ++y)
	{
		matrix[y + y*MATRIX_COLS] = 255;
	}

	zoom = 1.0;

	render = original;
	ui->widget->repaint();
}

uint8_t multiplyRow(const float * row, const uint8_t * colors)
{
	float r = 0;
	for(int i = 0; i < MATRIX_COLS; ++i)
	{
		r += colors[i] * row[i];
	}

	return r < 0? 0 : r < 255? (uint8_t) r : 255;
}


void MainWindow::onNegate()
{
	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb pixel = original.pixel(x, y);

			if(qAlpha(pixel) == 0)
			{
				continue;
			}

			pixel = qRgba(-qRed(pixel) & 0xFF, -qGreen(pixel) & 0xFF, -qBlue(pixel) & 0xFF, qAlpha(pixel));
			render.setPixel(x, y, pixel);
		}
	}

	ui->widget->repaint();
}

void MainWindow::applyMatrix()
{
	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	float mat[MATRIX_SIZE];

	for(size_t y = 0; y < MATRIX_ROWS; ++y)
	{
		float sum = 0;
		for(size_t x = 0; x < MATRIX_COLS; ++x)
		{
			int i = y*MATRIX_COLS + x;
			mat[i] = matrix[i] / 255.0;
			sum += mat[i];
		}

		if(sum > 1.0)
		{
			for(size_t x = 0; x < MATRIX_COLS; ++x)
			{
				int i = y*MATRIX_COLS + x;
				mat[i] = mat[i] / sum;
			}
		}
	}

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb pixel = original.pixel(x, y);

			if(qAlpha(pixel) == 0)
			{
				continue;
			}

			uint8_t colors[MATRIX_COLS];

			colors[0] = qRed(pixel);
			colors[1] = qGreen(pixel);
			colors[2] = qBlue(pixel);

			if(modifier.isNull())
			{
				colors[3] = 0;
				colors[4] = 0;
			}
			else
			{
				QRgb pixel = modifier.pixel(x, y);

				colors[3] = qRed(pixel);
				colors[4] = qGreen(pixel);
			}

			uint8_t red   = multiplyRow(mat + 0*MATRIX_COLS, colors);
			uint8_t green = multiplyRow(mat + 1*MATRIX_COLS, colors);
			uint8_t blue  = multiplyRow(mat + 2*MATRIX_COLS, colors);
			uint8_t alpha = qAlpha(pixel);

			pixel = qRgba(red, green, blue, alpha);
			render.setPixel(x, y, pixel);
		}
	}
}

void MainWindow::applyAngles()
{
	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);
#ifndef M_PI
#define M_PI 3.14159265358
#endif
	Quaternion q(angles[0] * M_PI / 128, angles[1] * M_PI / 128, angles[2] * M_PI / 128);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb pixel = original.pixel(x, y);

			if(qAlpha(pixel) == 0)
			{
				continue;
			}

			Vector3 c = Vector3::fromColor(qRed(pixel), qGreen(pixel), qBlue(pixel));
			c = q.rotate(c);
			pixel = qRgba(c.red(), c.green(), c.blue(), qAlpha(pixel));
			render.setPixel(x, y, pixel);
		}
	}
}

float MainWindow::applyPigment(float color, float pigment)
{
	return std::max(0.f, color + (pigment-128)/255.f);

	/*
	color   /= 255;
	pigment /= 255;

	float c = color;

	if(pigment < .5)
	{
		c = c*c*(pigment+.5);
		c = sqrt(c);
	}
	else
	{
		pigment = 1-pigment;
		c = 1 - c*c;
		c = c*(pigment+.5);
		c = sqrt(1-c);
	}

	return 255*c;
	*/
}


void MainWindow::applyPigments()
{/*
	if(pigments[0] == 128 && pigments[1] == 128 && pigments[2] == 128)
	{
		render = original;
		return;
	}*/

	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb pixel = original.pixel(x, y);

			if(qAlpha(pixel) == 0)
			{
				continue;
			}

//get rgb as floats
			float r = qRed(pixel)/255.f;
			float g = qGreen(pixel)/255.f;
			float b = qBlue(pixel)/255.f;

//get luminance
			float Y = sqrt(r*r + g*g + b*b);

			if(!Y)
			{
				render.setPixel(x, y, pixel);
				continue;
			}

			r = applyPigment(r/Y, pigments[0]);
			g = applyPigment(g/Y, pigments[1]);
			b = applyPigment(b/Y, pigments[2]);

//correct luminance
			float length = sqrt(r*r + g*g + b*b);
			if(length)
			{
				Y = Y/length;
//correct color
				r = r*Y;
				g = g*Y;
				b = b*Y;
			}

			r = std::max(0, std::min(255, (int) (r*255)));
			g = std::max(0, std::min(255, (int) (g*255)));
			b = std::max(0, std::min(255, (int) (b*255)));

			pixel = qRgba(r, g, b, qAlpha(pixel));
			render.setPixel(x, y, pixel);
		}
	}
}



static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void MainWindow::documentOpenOriginal()
{
	QFileDialog dialog(this, tr("Open File"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	while (dialog.exec() == QDialog::Accepted && !openFile(&original, &modifier, dialog.selectedFiles().first())) {}
}

void MainWindow::documentOpenModifier()
{
	QFileDialog dialog(this, tr("Open File"));
	initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

	while (dialog.exec() == QDialog::Accepted && !openFile(&modifier, &original, dialog.selectedFiles().first())) {}
}

void MainWindow::documentSave()
{
	if(filename.isNull())
	{
		documentSaveAs();
	}
	else
	{
		saveFile(filename);
	}
}

void MainWindow::documentSaveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

bool MainWindow::openFile(QImage * image, QImage *other, const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

	if(!other->isNull())
	{
		if(newImage.size() != other->size())
		{
			QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
											tr("Cannot load %1: dimensions of base and modifier do not match")
											.arg(QDir::toNativeSeparators(fileName)));
			return false;
		}
	}

	*image = std::move(newImage);
	if(image == &original) reset();

	return true;
}

void MainWindow::documentNew()
{
	documentClose();
}

void MainWindow::documentClose()
{
	filename = QString();
	original = QImage();
	modifier = QImage();
	render = QImage();
	reset();
}

bool MainWindow::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(render)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

void MainWindow::editMatrix()
{
	MatrixEditor dialog(this);
	dialog.show();
	dialog.exec();
}

void MainWindow::editAngles()
{
	RotationEditor dialog(this);
	dialog.show();
	dialog.exec();
}

void MainWindow::editPigments()
{
	PigmentEditor dialog(this);
	dialog.show();
	dialog.exec();
}


void MainWindow::editCopy()
{
	#ifndef QT_NO_CLIPBOARD
	QGuiApplication::clipboard()->setImage(render);
	#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void MainWindow::editPaste()
{
#ifndef QT_NO_CLIPBOARD
    QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
		original = std::move(newImage);
		render = original;
		filename = QString();
		reset();

        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void MainWindow::draw(QPainter & painter, QSize size)
{
	if(render.isNull()) return;

	painter.scale(zoom, zoom);
	size /= zoom;

	QSize s0 = render.size() - size;
	QPoint offset(ui->horizontalScrollBar->value() * s0.width () / 255,
				  ui->verticalScrollBar  ->value() * s0.height() / 255);

	painter.drawImage(0, 0, render, offset.x(), offset.y(), size.width(), size.height());
}

bool MainWindow::event(QEvent * event)
{
	switch(event->type())
	{
	case QEvent::KeyPress:
	case QEvent::KeyRelease:
	{
		QKeyEvent * key = static_cast<QKeyEvent *>(event);

		switch(key->key())
		{
		case Qt::Key_Up:
		case Qt::Key_Down:
			return super::event(event) && ui->verticalScrollBar->event(event);
		case Qt::Key_Left:
		case Qt::Key_Right:
			return super::event(event) && ui->horizontalScrollBar->event(event);
		default:
			return super::event(event);
		}
	}
	case QEvent::Wheel:
	{
		QWheelEvent * wheel = static_cast<QWheelEvent *>(event);

		if(wheel->modifiers() & Qt::ControlModifier)
		{
			if(wheel->orientation() == Qt::Vertical)
			{
				double angle = wheel->angleDelta().y();
				double factor = std::pow(1.0015, angle);
				zoom *= factor;
				ui->widget->repaint();
			}
		}
		else if(wheel->buttons() != Qt::MidButton)
		{
			if(wheel->orientation() == Qt::Horizontal)
			{
				return super::event(event) && ui->horizontalScrollBar->event(event);
			}
			else
			{
				return super::event(event) && ui->verticalScrollBar->event(event);
			}
		}
	}
	default:
		return super::event(event);
	}
}

