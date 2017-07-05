#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QWidget>

class MainWindow;

class ViewWidget : public QWidget
{
typedef QWidget super;
	Q_OBJECT
public:
	explicit ViewWidget(QWidget *parent = 0);

public:
	void keyPressEvent			(QKeyEvent * event)		Q_DECL_OVERRIDE;
	void keyReleaseEvent		(QKeyEvent * event)		Q_DECL_OVERRIDE;

	void wheelEvent				(QWheelEvent * event)   Q_DECL_OVERRIDE;
	void paintEvent				(QPaintEvent * event)	Q_DECL_OVERRIDE;

private:
friend class MainWindow;
	MainWindow * window;
};

#endif // VIEWWIDGET_H
