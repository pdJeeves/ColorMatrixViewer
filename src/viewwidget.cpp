#include "viewwidget.h"
#include "mainwindow.h"
#include <QPainter>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPaintEvent>

ViewWidget::ViewWidget(QWidget *parent) : QWidget(parent)
{

}

void ViewWidget::wheelEvent(QWheelEvent * event)
{
	window->event(event);
}

void ViewWidget::keyPressEvent(QKeyEvent * event)
{
	window->event(event);
}

void ViewWidget::keyReleaseEvent(QKeyEvent * event)
{
	window->event(event);
}

void ViewWidget::paintEvent(QPaintEvent * event)
{
static int grid_size = 8;
static QBrush background(Qt::white);
static QBrush square_color(Qt::gray);

	QPainter painter;
	painter.begin(this);

	painter.fillRect(event->rect(), background);
	for(int x = 0, column = 0; x < event->rect().width(); x += grid_size, ++column)
	{
		for(int y = column & 0x01? grid_size : 0; y < event->rect().height(); y += grid_size*2)
		{
			painter.fillRect(QRect(x, y, grid_size, grid_size), square_color);
		}
	}

	window->draw(painter, size());
	painter.end();
}
