#pragma once

#include <vector>
#include <list>

#include <QMainWindow>

#include "../common/tWindow.hpp"
#include "../common/tSetting.hpp"
#include "../view/tView.hpp"
#include "../filter/tFilter.hpp"


namespace Ui {class Review;}
class QMdiArea;

class Review: public QMainWindow
{
    Q_OBJECT

public:

    explicit Review(QWidget *parent = 0);
    ~Review();

    tWindow*        getActiveWindow();
    const tWindow*  getActiveWindow() const;

    tWindow*        letUserSelectWindow();

public slots:

    bool newDataWindow();                       // query user for path
    bool newDataWindow(const QString& path);    // load from file
    bool newDataWindow(const tWindow& window);  // copy existing

    void newReferenceWindow();

    void newFilterWindow();

    // add data to existing window
    bool loadFromFile(const QString& path);
    bool loadFromFile();

    bool saveToFile(const QString& path);
    bool saveToFile();

    void copyWindow();
    void copyWindow(tWindow*);

    void changeWindowName();
    void changeWindowName(const QString& name);

    void newDatabaseInput();

private slots:

    void subWindowActivated(QMdiSubWindow* window);

private:

    struct Private;
    Private* m_private;

    tWindow* createWindow();
    tWindow* newDataWindow(tData::Ownership data);

    tWindow* m_activeWindow = nullptr;

    tSetting<tView*> m_selectedViewPrototype;
    tSetting<tFilter*> m_selectedFilterPrototype;

    // the windows currently open
    //std::vector<tWindow*> m_windows;

    tView::OwnershipList m_viewPrototypes;
    tFilter::OwnershipList m_filterPrototypes;

    void resetViewPrototypes();
    void resetFilterPrototypes();

    Ui::Review* m_ui;

    QMdiArea* m_mdiArea;

    //tSetting<bool> m_testSetting;

private slots:

    void newViewStatusInfo(QString);


};

#include <QMdiSubWindow>


// [TEST] here we have a chance to fully customize subwindows
class tQMdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT

public:

    tQMdiSubWindow(QWidget* parent = nullptr) : QMdiSubWindow(parent) {}

protected:

    void paintEvent(QPaintEvent* paintEvent) override;

private:

};
