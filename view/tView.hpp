#pragma once

//#include <vector>

#include <QWidget>

class Review;
class tWindow;

#include "../common/tObject.hpp"
#include "../data/tData.hpp"


class tView : public QWidget, public tObject
{

    Q_OBJECT

public:

    T_TYPEDEFS(tView)

    ~tView();

    // get a copy of the prototyped view associated with a window
    virtual tView* copy(tWindow*) const = 0;

    QString getName() const override;
    void    setName(const QString& name) override;

    // check wether view can work with given data. View will initialize at the same time using the data provided
    //bool            init(tData::List* data = nullptr);
    //bool            init(tData::Ref data);

    // returns the input-select widget provided by derived class. May be nullptr!
    const QWidget*  getInputSelectWidget() const;
    QWidget*        getInputSelectWidget();

    // returns the settings widget provided by derived class. May be nullptr!
    const QWidget*  getSettingsWidget() const;
    QWidget*        getSettingsWidget();

    // check wether view is compatible with data
    virtual bool isApplicable(const tData*) const = 0;

public slots:

    virtual void update() = 0;

protected:

    //tView();
    tView(tWindow* window = nullptr);

    // register this view as dependant from data
    //void registerData(const tData::Ref& data);
    //void unregisterData(const tData::Ref& data);

    //template <typename T>
    //T* copy_internal(QWidget* window, tData::ConstRefList* refs);

    // this is the associated window which never changes throughout view lifetime
    tWindow *const m_window;

    // when provided, a dock intended for choosing input references is displayed. CAUTION: lifetime is then managed by tView Base
    QWidget* m_inputSelect = nullptr;

    // when provided, a settings dock is enabled for this view
    QWidget* m_settings = nullptr;

    //QWidget& m_window;                // the view can setup the window
    //const tData::List& m_refs;     // and look through the available data

private:

    //virtual bool init_impl(tData::List&) = 0;

    QString m_type;

    friend class Review;


signals:

    void newStatusInfo(QString);

};

// eqip derived views with this macro to enably copying
#define T_VIEW_COPYABLE(this_type) private: tView* copy(tWindow* window) override {return new this_type(window);}


