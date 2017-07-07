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
#include <iostream>

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
			float length = c.length();
			c = q.rotate(c);
			c.normalize();
			c = c * length;
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

Vector3 TransformHSV(
        const Vector3 &in,  // color to transform
        float H,          // hue shift (in degrees)
        float S,          // saturation multiplier (scalar)
        float V           // value multiplier (scalar)
    )
{


    float VSU = V*S*cos(H);
    float VSW = V*S*sin(H);

    Vector3 ret;
    ret.x = (.299*V+.701*VSU+.168*VSW)*in.x
        + (.587*V-.587*VSU+.330*VSW)*in.y
        + (.114*V-.114*VSU-.497*VSW)*in.z;
    ret.y = (.299*V-.299*VSU-.328*VSW)*in.x
        + (.587*V+.413*VSU+.035*VSW)*in.y
        + (.114*V-.114*VSU+.292*VSW)*in.z;
    ret.z = (.299*V-.3*VSU+1.25*VSW)*in.x
        + (.587*V-.588*VSU-1.05*VSW)*in.y
        + (.114*V+.886*VSU-.203*VSW)*in.z;
    return ret;
}

Vector3 TransformByExample(
        const Vector3 &in,  // color to transform
        const Vector3 &r,   // pre-transformed red
        const Vector3 &g,   // pre-transformed green
        const Vector3 &b,   // pre-transformed blue
        float m  // Maximum value for a channel
    )
{
    Vector3 ret;
    ret.x = in.dot(r)/m;
    ret.y = in.dot(g)/m;
    ret.z = in.dot(b)/m;
    return ret;
}

float GetHue(float r, float g, float b, float & saturation)
{
	saturation = 0;

	if(r == g && g == b)
	{
		return 0;
	}
	else if(r >= g && r >= b)
	{
		float min = std::min(g, b);
		saturation = (r - min)/r;

		float h = (g - b)/(r - min);
		if(h < 0)
			h += 6;
		return h / 6;
	}
	else if(g >= b && g >= r)
	{
		float min = std::min(b, r);
		saturation = (g - min)/g;

		return ((b - r)/(g - min) + 2)/6;
	}
	else if(b >= g && b >= r)
	{
		float min = std::min(r, g);
		saturation = (b - min)/b;

		return ((r - g)/(b - min) + 4)/6;
	}

	return 0;
}

Vector3 GetFromQrgb(QRgb col)
{
	return Vector3::fromColor(qRed(col), qGreen(col), qBlue(col));
}

QRgb rawColor(int r, int g, int b, int a = 255)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

void MainWindow::applyPigments()
{
	float Pr = (pigments[0]/128.f);
	float Pg = (pigments[1]/128.f);
	float Pb = (pigments[2]/128.f);

	float saturation = 0;
	float value = std::max(Pr, std::max(Pg, Pb));
	float hue = -GetHue(Pr, Pg, Pb, saturation);

	const float swap_rg = (pigments[3] > 128? pigments[3] - 128 : 128 - pigments[3])/128.f;
	const float swap_gb = (pigments[4] > 128? pigments[4] - 128 : 128 - pigments[4])/128.f;
	const float swap_rb = (pigments[5] > 128? pigments[5] - 128 : 128 - pigments[5])/128.f;

	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb px = original.pixel(x, y);

			if(qAlpha(px) == 0)
				continue;

			Vector3 color(qRed(px)/256.f, qGreen(px)/256.f, qBlue(px)/256.f);
			float brightness = (color.x + color.y+color.z)/3;// sqrt(color.x*color.x*.241 + color.y*color.y*.691+ color.z*color.z*.068);

			float t = (brightness)*(1-brightness);

			float chroma = std::max(color.x, std::max(color.y, color.z)) -  std::min(color.x, std::min(color.y, color.z));
			float db = (Pr + Pg + Pb)/6 -.5;

			color = Vector3(color.x*(1-swap_rg)*(1-swap_rb) + color.y*(swap_rg) + color.z*(swap_rb)
			,
							color.y*(1-swap_rg)*(1-swap_gb) + color.x*(swap_rg) + color.z*(swap_gb)
			,
							color.z*(1-swap_gb)*(1-swap_rb) + color.x*(swap_rb) + color.y*(swap_gb)
			);

			color.x = color.x*(1-chroma) + (Pr <= 1.f? Pr*color.x : color.x + (1-color.x)*(Pr-1))*chroma + db;
			color.y = color.y*(1-chroma) + (Pg <= 1.f? Pg*color.y : color.y + (1-color.y)*(Pg-1))*chroma + db;
			color.z = color.z*(1-chroma) + (Pb <= 1.f? Pb*color.z : color.z + (1-color.z)*(Pb-1))*chroma + db;

			color.x = std::max(0.f, std::min(1.f, color.x));
			color.y = std::max(0.f, std::min(1.f, color.y));
			color.z = std::max(0.f, std::min(1.f, color.z));

			float luma = (color.x + color.y+color.z)/3;
			//sqrt(color.x*color.x*.241 + color.y*color.y*.691+ color.z*color.z*.068);

			luma  = (brightness*(1-t) + luma*t) - luma;

			color.x += luma;
			color.y += luma;
			color.z += luma;

			color.x = std::max(0.f, std::min(1.f, color.x));
			color.y = std::max(0.f, std::min(1.f, color.y));
			color.z = std::max(0.f, std::min(1.f, color.z));

			px = qRgba(color.x*255, color.y*255, color.z*255, qAlpha(px));
			render.setPixel(x, y, px);
		}
	}//*/

#if 0
	float acid     = std::cos(pigments[0]*M_PI/256);
	float electric = std::sin(pigments[1]*M_PI/128);

	const float acid = (pigments[0] > 128? pigments[0] - 128 : 128 - pigments[0])/128.f;

	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb px = original.pixel(x, y);

			if(qAlpha(px) == 0)
				continue;

			if(x == 0 || x == original.width()-1
			|| y == 0 || y == original.height()-1)
			{
				render.setPixel(x, y, px);
				continue;
			}

			Vector3 above = GetFromQrgb(original.pixel(x, y-1));
			Vector3 left = GetFromQrgb(original.pixel(x-1, y));
			Vector3 self = GetFromQrgb(original.pixel(x, y));

			Vector3 elecV = self.cross(above);
			Vector3 elecH = self.cross(left);

			Vector3 elec(sqrt(elecV.x*elecH.y), sqrt(elecV.y*elecH.z), sqrt(elecV.z*elecH.x);
/*
			float dt = (elecV.length() + elecH.length())*acid*255;

			QColor pixel((uint8_t)(qRed(px) + dt),
						 (uint8_t)(qGreen(px) + dt),
					     (uint8_t)(qBlue(px) + dt),
						  qAlpha(px));
*/
			px = qRgb(elec.red(), elec.green(), elec.blue());
			render.setPixel(x, y, px);
		}
	}
#endif
#if 0

	float Pr = (pigments[0]/255.f);
	float Pg = (pigments[1]/255.f);
	float Pb = (pigments[2]/255.f);

	float saturation = 0;
	float value = std::max(Pr, std::max(Pg, Pb));
	float hue = -GetHue(Pr, Pg, Pb, saturation);

	const float absRot = (pigments[3] > 128? pigments[3] - 128 : 128 - pigments[3])/128.f;

	render = QImage(original.size(), QImage::Format_ARGB32);
	render.fill(0);

	for(int y = 0; y < original.height(); ++y)
	{
		for(int x = 0; x < original.width(); ++x)
		{
			QRgb px = original.pixel(x, y);

			if(qAlpha(px) == 0)
				continue;

			QColor pixel(qRed(px)*(1-absRot) + qBlue(px)*absRot, qGreen(px), qRed(px)*absRot + qBlue(px)*(1-absRot), qAlpha(px));

			qreal H, S, V;
			pixel.getHsvF(&H, &S, &V);
			float t = V*(1-V);
			if(H >= 0)
			{
				H += hue;
				if(H > 1.0)
					H -= 1.0;
				if(H < 0)
					H += 1.0;
				pixel.setHsvF(H, S*saturation,  V*(1-t) + value*t);
			}
			else
			{
				pixel.setHsvF(0, S*saturation, V*(1-t) + value*t);
			}


			render.setPixel(x, y, pixel.rgba());
		}
	}//*/
#endif
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

