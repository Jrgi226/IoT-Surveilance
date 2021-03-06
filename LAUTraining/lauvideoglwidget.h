#ifndef LAUVIDEOGLWIDGET_H
#define LAUVIDEOGLWIDGET_H

#include <QTime>
#include <QImage>
#include <QScreen>
#include <QObject>
#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPixelTransferOptions>

#include "laumemoryobject.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUVideoGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit LAUVideoGLWidget(QWidget *parent = NULL) : QOpenGLWidget(parent), videoTexture(NULL), counter(0) { ; }
    ~LAUVideoGLWidget();

    virtual bool isValid() const
    {
        return (wasInitialized());
    }

    bool wasInitialized() const
    {
        return (vertexArrayObject.isCreated());
    }

    void setFrame(LAUMemoryObject object);

    virtual void initialize();
    virtual void resize(int w, int h);
    virtual void paint();

protected:
    void initializeGL()
    {
        initialize();
    }

    void resizeGL(int w, int h)
    {
        resize(w, h);
    }

    void paintGL()
    {
        paint();
    }

    QOpenGLVertexArrayObject vertexArrayObject;
    QOpenGLBuffer quadVertexBuffer, quadIndexBuffer;
    QOpenGLShaderProgram program;
    QOpenGLTexture *videoTexture;

    int localWidth, localHeight;
    qreal devicePixelRatio;

private:
    int counter;
    QTime time;
};

#endif // LAUVIDEOGLWIDGET_H
