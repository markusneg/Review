#pragma once

// auxilliary code for QCustomPlot

#include <QWidget>

class QCPColorGradient;
class tWindow;


// encapsulating a QCPColorMap which can be set up by this widget
class tColorGradient : public QWidget
{
    Q_OBJECT

public:

    tColorGradient();
    ~tColorGradient();

    // check wether coloring is enabled by user
    bool isEnabled() const;

    // provides direct access to colorGradient object
    const QCPColorGradient&  getColorGradient() const;
    QCPColorGradient&        getColorGradient();

    // to be called manually when gradient was changed directly
    void updateWidget();

public slots:

    // will ask user for color at position by means of a color selection window
    void setColor(double position);

signals:

    // emitted when color mode was switched on or off or the color changed
    void changed();

private:

    struct Private;
    Private* m_private;

    QCPColorGradient* m_colorGradient;

    uint m_announcedItemNumber = 0;

};
