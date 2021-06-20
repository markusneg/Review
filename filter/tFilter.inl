#include "tFilter.hpp"


template <typename T>
void tFilter::registerInput(T& input)
{
    //std::cout << "[tFilter] invalid input!\n" << std::flush;

    m_inputs.push_back([&](){return static_cast<tData*>(input->getData());});

    if(std::is_base_of<tObject, T>::value)
    {
        input.addCallbackSafe([&]()
        {
            //tWindow* oldInputWindow =
            if(input->getData())
                input->getData()->getOwnerWindow()->removeOutputWindow(m_window);
            //oldInputWindow

        }, this, sigSettingChanged);

        input.addCallbackSafe([&]()
        {
            if(input->getData())
                input->getData()->getOwnerWindow()->addOutputWindow(m_window);

            run();

        }, this, sigSettingChanged);

    }
}

/*template <typename T>
void tFilter::registerInput(T** input)
{
}*/

template <typename T>
void tFilter::unregisterInput(T& input)
{

}

inline bool tFilter::isEnabled() const
{
    return m_enabled;
}
