#pragma once

#include "tView.hpp"
#include "../common/tSetting.hpp"

#include "../data/tDataReference.hpp"
#include "../data/tDataSelectionPolicy.hpp"



class tMatrixDataSelection;

class tImageView : public tView
{

    Q_OBJECT

public:

    T_TYPEDEFS(tImageView)

    ~tImageView();

    bool isApplicable(const tData* data) const override;

public slots:

    void update() override;

protected:

    //void updatePlotstyle(QCPGraph*, uint, uint);

    //tImageView();
    tImageView(tWindow* window = nullptr);

    tSetting<tMatrixDataSelection> m_data;

    tSetting<int> m_folding = 0;


private:

    struct Private;
    Private* m_private;

    friend class Review;

    //T_VIEW_COPYABLE(tImageView)

    tView* copy(tWindow* window) const override;

    arma::mat m_buf;

private slots:

    void mousePress(QMouseEvent* mevent);
    void mouseMove(QMouseEvent *mevent);
    void mouseRelease(QMouseEvent *mevent);
    void leaveEvent(QEvent *event);

    void axisLabelChanged();
};

class tImageSelectionPolicy : public tDataSelectionPolicy
{

public:

    tImageSelectionPolicy(tMatrixDataSelection& sel, uint graphCount = 20);

    void setGraphCountHint(uint graphCount);

private:

    void onExtentChange() override;
    void onDataChange() override;
    void onListChange() override;

    tMatrixDataSelection& m_sel;
    uint m_graphCount;
};
