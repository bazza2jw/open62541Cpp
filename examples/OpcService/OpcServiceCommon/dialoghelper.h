#ifndef DIALOGHELPER_H
#define DIALOGHELPER_H

#include "designwt.h"
#include <typeinfo>
#include <string>
#include <functional>
#include <memory>
namespace MRL {
/*!
    \brief The DialogHelper class
    wraps a modal dialog
*/
class DialogHelper
{
    Wt::WDialog* _d = nullptr;

public:
    DialogHelper() = default;
    void connect(Wt::WDialog* d)
    {
        _d = d;
        d->finished().connect(this, &DialogHelper::dialogclosed);
    }
    virtual ~DialogHelper() {}
    virtual void onClose(Wt::WDialog::DialogCode /*code*/) {}
    void dialogclosed(Wt::WDialog::DialogCode code)
    {
        onClose(code);
        if (_d)
            _d->hide();
    }
    virtual void show() {}
};

template <typename T, typename P>
/*!
    \brief The ModalDialog class
    wrap a modal dialog
*/
class ModalDialog : public DialogHelper
{
    std::unique_ptr<T> _dialog;

public:
    ModalDialog(P* parent)
    {
        _dialog = std::make_unique<T>(parent);
        connect(_dialog.get());  // connect to standard buttons
    }

    void show()
    {
        _dialog->show();  // this does not block
    }

    T* dialog() { return _dialog.get(); }
    // derive a close handler
};

}  // namespace MRL

#endif  // DIALOGHELPER_H
