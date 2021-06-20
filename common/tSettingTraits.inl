#include "tSetting.hpp"

#include <assert.h>

#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>

#include "../util/tQComboBox.hpp"
#include "../util/util.hpp"


// use these for using base class constructors
#define T_SETTING_DEFS(this_class, type) \
    typedef tSettingBase<type> Base; \
    this_class() : Base::tSettingBase() {init();} \
    this_class(const type& copyFrom) : Base::tSettingBase(copyFrom) {init(); this->valueToWidget();} \
    this_class(type& existingValue) : Base::tSettingBase(existingValue) {init(); this->valueToWidget();} \
    this_class(const this_class<type>& rhs) : Base::tSettingBase(rhs) {init(); this->valueToWidget();} \
    this_class<type>& operator=(const this_class<type>& rhs) {Base::operator=(rhs); return *this;} \
    ~this_class() {this->doCallbacks(sigDecaying); /*tSettingDetails::forwardSignals(this->m_value, this, false);*/ this->destroy();}


template <typename T>
class tSetting<T, typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value>::type> : public tSettingBase<T>
{

public:

    T_SETTING_DEFS(tSetting, T)

    void init()
    {
        this->setName("Setting (Integral)");

        /*QObject::connect(m_lineEdit, &QWidget::destroyed, []()
        {
           T_COUT("HEY!");
        });*/

        QObject::connect(m_lineEdit, &QLineEdit::editingFinished, [&]()
        {
            QString input = m_lineEdit->text();
            this->premangleText(input);
            setFromString(input);

            valueToWidget();    // test: on invalid input or when variable was entered, show old number
        });

        Base::m_widget = m_lineEdit;
    }

    void valueToWidget() override
    {
        m_lineEdit->setText(QString::number(Base::m_value));
    }

    QString valueAsText() const override
    {
        return QString::number(Base::m_value);
    }

    void setFromString(const QString& str)
    {
        bool isNumber;
        const T newValue = str.toDouble(&isNumber);
        if(isNumber) this->set(newValue);
    }



protected:

    QLineEdit* m_lineEdit = new QLineEdit();
};

template <>
class tSetting<bool> : public tSettingBase<bool>
{

public:

    T_SETTING_DEFS(tSetting, bool)

    void init()
    {
        setName("Setting (bool)");

        QObject::connect(m_checkBox, &QCheckBox::toggled, [&](bool on)
        {
            set(on);
        });

        m_widget = m_checkBox;
    }

    void valueToWidget() override
    {
        m_checkBox->setChecked(m_value);
    }

protected:

    QCheckBox* m_checkBox = new QCheckBox();
};

namespace tListSettingDetails
{
    template <typename T>
    void invalidate(T& value) {value.reset();}

    template <typename T>
    void invalidate(T*& value) {value = nullptr;}

    template <typename T, typename U>
    T value(U& v) {return v;}

    template <typename T, typename U>
    T value(U* v) {return static_cast<T>(v);}

    template <typename T, typename U>
    T value(tOwnership<U>& v) {return static_cast<T>(v.get());}

    template <typename T, typename U>
    T value(const tOwnership<U>& v) {return static_cast<T>(v.get());}

    template <typename T, typename U>
    void reg(T*&, const U&) {}

    template <typename T, typename U>
    void reg(T*& value, const U* at) {value = nullptr;}

    //template <typename T>
    //tList<T>* getList() {return nullptr;}

    //template <typename T, typename U>
    //T value(tReference<U>& v) {return static_cast<T>(PTR(v));}
}

template <typename T>
struct tListProxy
{
    virtual ~tListProxy() {}

    virtual const T next() = 0;
    virtual void reset() = 0;
    virtual void lock() = 0;
    virtual void unlock() = 0;

    // get the tObject base of list
    virtual const tObject& getList() = 0;
};

// default proxy good for tLists
template <typename T, typename T_LIST>
struct tListProxyImpl : public tListProxy<T>
{
    tListProxyImpl(const T_LIST& list) : m_listProxy(list), m_it(m_listProxy.begin())
    {
        //m_listProxy.addCallback();
    }

    const T next() override
    {
        if(m_it != m_listProxy.end())
            return tListSettingDetails::value<T>(*(m_it++));

        else return nullptr;
    }

    void reset() override {m_it = m_listProxy.begin();}
    void lock() override {m_listProxy.lock();}
    void unlock() override {m_listProxy.unlock();}
    const tObject& getList() override {return m_listProxy;}

    const T_LIST& m_listProxy;
    typename T_LIST::const_iterator m_it;
};


// make object references choosable out of a list of variable type. Lookup of candidates
// is realized using a functor which is expected to return nullptr when list end is reached

// unset fAutoSet to prevent selecting first element from newly set list

template <typename T>
class tSetting<T*> : public tSettingBase<T*>
{

public:

    T_SETTING_DEFS(tSetting, T*)

    // a setter may be given using setSetter(). Will be used instead of Base::set()
    typedef std::function<void(T*)> Setter;

    void init()
    {
        this->setName("Setting (List)");
        this->setFlags(fReseatable);

        repopulateItems();

        // update when box is opened by user
        QObject::connect(m_box, &tQComboBox::boxOpened, [=]()
        {
            repopulateItems();
        });

        // apply user choice to internal value
        QObject::connect(m_box, T_SIGNAL(tQComboBox, activated, int), [&](int index) {newUserSelection(index);});

        Base::m_widget = m_box;

        this->setFlags(fAutoSet);
    }

    void setSetter(const Setter& setter)
    {
        m_setter = setter;
    }

    void setEmptyTag(const QString& emptyTag)
    {
        m_emptyTag = emptyTag;
        repopulateItems();
    }

    void valueToWidget() override
    {
        // get the currenSt refs
        repopulateItems();
    }

    // repopulate combobox from list given by aux type
    void repopulateItems()
    {
        //if(!m_listProxy) return;

        m_box->clear();

        m_box->addItem(m_emptyTag); uint itemNumber = 1; bool itemRestored = false;

        //if(!m_listProxy) {m_box->setCurrentIndex(0); return;}

        if(m_listProxy)
        {
            m_listProxy->lock(); ON_LEAVE(m_listProxy->unlock(););

            T* obj; m_listProxy->reset();
            while( (obj = m_listProxy->next()) )
            {
                m_box->addItem(obj->getExtendedName());

                if(Base::m_value == obj)
                    {m_box->setCurrentIndex(itemNumber); itemRestored = true;}

                itemNumber++;
            }
        }

        if(!itemRestored)
            {m_box->setCurrentIndex(0); /*T_COUT("[tSetting<T*>] Did not find current value in list..");*/}

        if(this->checkFlags(fReseatable))
            m_box->addItem(QObject::tr("Other"));
    }

    template <typename U>
    void setList(const U* list)
    {
        // remove callbacks from previously set list and delete
        if(m_listProxy)
            unsetList();

        if(list)
        {
            list->addCallbackSafe([&]()
            {
                repopulateItems();
            }, this, sigChanged);

            list->addCallbackSafe([&]()
            {
                unsetList();
            }, this, sigDecaying);

            list->addCallbackSafe([&]()
            {
                onListContentChanged();
            }, this, sigDataChanged);
        }

        m_listProxy = list ? new tListProxyImpl<T*,U>(*list) : nullptr;

        repopulateItems();

        onListContentChanged();
    }

    // if given and fAutoSelect on, we choose specific data from list on list-set
    void setDefaultSelector(tFlags selector)
    {
        m_defaultSelector = selector;
    }

protected:

    // user has selected from combobox
    void newUserSelection(int index)
    {
        if(index == -1) index = 0;

        // first entry is "None"
        if(index == 0)
        {
            //tListSettingDetails::invalidate(this->m_value);
            //Base::valueChanged();
            this->set(nullptr);
            return;
        }

        if(!m_listProxy)
        {
            if(this->checkFlags(fReseatable) && index != 0)
                getListFromUser();

            else return;
        }

        m_listProxy->lock(); ON_LEAVE(m_listProxy->unlock();)

        // advance "iterator" to position wanted
        m_listProxy->reset();
        T* obj = m_listProxy->next();
        for(int n = 1; obj && n < index; ++n)
            obj = m_listProxy->next();

        // index was greater than list length
        if(!obj)
        {
            // assume, "other" has been chosen
            if(this->checkFlags(fReseatable))
                getListFromUser();

            return;
        }

        // nothing changed
        if(Base::m_value == obj) return;

        this->set(obj);
    }

    // register tObject to forward signals to Setting
    // Base: virtual void set(const T& value);
    void set(T* const& value) override
    {
        /// [TODO] we should also let value call back on decay, so that we can invalidate ptr.
        /// However, this is only neccessary when no list is available, as list forwards events of decaying content as sigChanged

        // cease forwarding previous values signals
        if(this->m_value)
        {
            tSettingDetails::forwardSignals(*this->m_value, this, false);
            this->m_value->removeCallback(this);
            this->removeCallback(this->m_value);
        }

        if(value)
        {
            tSettingDetails::forwardSignals(*value, this);

            value->addCallbackSafe([&]() {this->set(nullptr);}, this, sigDecaying);
            value->addCallbackSafe([&]() {this->repopulateItems();}, this, sigNameChanged);
        }

        if(m_setter)
        {
            this->doCallbacks(sigBeforeChange | sigSettingChanged);
            m_setter(value);
            //this->valueChanged(); // causes double callbacks when setter causes callbacks
        }
        else
            Base::set(value);
    }

    tQComboBox* m_box = new tQComboBox();

    tListProxy<T*>* m_listProxy = nullptr;

private:

    //enum UnsetMode {RemoveCallback, KeepCallback};
    void unsetList(/*UnsetMode mode = RemoveCallback*/)
    {
        if(!m_listProxy) return;

        /*if(mode == RemoveCallback)*/ m_listProxy->getList().removeCallback(this);
        delete m_listProxy;
        m_listProxy = nullptr;
    }

    void destroy()
    {
        if(m_listProxy) delete m_listProxy;
    }

    void onListContentChanged()
    {
        // default-select the first entry
        if(m_listProxy && this->checkFlags(fAutoSet))
        {
            if(m_defaultSelector)
            {
                m_listProxy->lock(); ON_LEAVE(m_listProxy->unlock();)
                m_listProxy->reset();

                T* obj = m_listProxy->next();
                for(int n = 1; obj; ++n)
                {
                    if(obj && (obj->getFlags() & m_defaultSelector) )
                        {newUserSelection(n); break;}

                    else obj = m_listProxy->next();
                }

                // no object with given flags found
                if(!obj) newUserSelection(1);
            }

            else newUserSelection(1);
        }
    }

    void getListFromUser() {} /// ??

    Setter m_setter;

    QString m_emptyTag = QObject::tr("None");

    tFlags m_defaultSelector = 0;
};


/// this needs some refactoring!
// for these types, enable selection by user
class tData; class tMatrixData;
template <> void tSetting<tData*>::getListFromUser();
template <> void tSetting<tMatrixData*>::getListFromUser();

// window is always directly chosen from workspace
class tWindow; class QPushButton;
//template <> class tSetting<tWindow*>;


template <>
class tSetting<tWindow*> : public tSettingBase<tWindow*>
{

public:

    typedef tWindow* tWindowPtr;
    T_SETTING_DEFS(tSetting, tWindowPtr)

    void init();
    void valueToWidget() override;

protected:

    void set(tWindow* const& value) override;
    QPushButton* m_checkBox;

private:

    void destroy();
};


/*template <typename T, typename U>
void setList(tSetting<T>& set, const U& list)
{
    set.setList(new tListProxyImpl<T,U>(list));
}*/
