#pragma once

//#include <vector>

#include "tView.hpp"
#include "../common/tSetting.hpp"
//#include "tListSetting.hpp"
#include "../data/tDataReference.hpp"
#include "../data/tDataSelectionPolicy.hpp"


class QCustomPlot; class QCPGraph;
class tMatrixDataSelection;

class tPlotView : public tView
{

    Q_OBJECT

public:

    T_TYPEDEFS(tPlotView)

    ~tPlotView();

    //bool init_impl(tData::List& data) override;
    bool isApplicable(const tData* data) const override;

public slots:

    void update() override;


protected:

    void updatePlotstyle(QCPGraph*, uint, uint);

    //tPlotView();
    tPlotView(tWindow* window = nullptr);

    QCustomPlot* const m_plot;

    tSetting<tMatrixDataSelection> m_xdata, m_ydata;
    tSetting<tMatrixDataSelection> m_cdata;

    tSetting<double> m_spread = 0;
    tSetting<bool> m_scale = false, m_center = false;

    QComboBox m_plotStyle;

    tSetting<bool> m_logx = false, m_logy = false;
    tSetting<bool> m_xLabels = true, m_yLabels = true;

    QLineEdit m_xAxisLabel, m_yAxisLabel;

private:

    struct Private;
    Private* m_private;

    friend class Review;

    //T_VIEW_COPYABLE(tPlotView)
    tView* copy(tWindow* window) const override;

    arma::mat m_buf;

    QRubberBand* m_rubberBand;

    // for rubberband-interaction of plot
private slots:

    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);
    void leaveEvent(QEvent *event);

    void axisLabelChanged();
};


// regard factor matrices and reduce points to plot
class tPlotSelectionPolicy : public tDataSelectionPolicy
{

public:

    tPlotSelectionPolicy(tMatrixDataSelection& sel, uint graphCount = 20);
    void setGraphCountHint(uint graphCount);

private:

    void onExtentChange() override;
    void onDataChange() override;
    void onListChange() override;

    tMatrixDataSelection& m_sel;
    uint m_graphCount;
};
