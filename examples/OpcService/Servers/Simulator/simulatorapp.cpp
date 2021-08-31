#include "simulatorapp.h"
#include "../../OpcServiceCommon/opcservicecommon.h"
#include "simulatoropc.h"

// sync to current values
/*!
 * \brief SimulatorApp::updateValues
 * handle update notification from the simulator process
 */
void SimulatorApp::updateValues()
{
    MRL::PropertyPath p;
    p.push_back(STOCKDEFS::RuntimeSection);
    int v         = MRL::OpcServiceCommon::data().getValue<int>(p, "Value");
    std::string s = std::to_string(v);
    frame()->CurrentValue->setText(s);
    triggerUpdate();
}

/*!
 * \brief SimulatorApp::handleUpdate
 */
void SimulatorApp::handleUpdate()
{
    SimulatorApp* app = dynamic_cast<SimulatorApp*>(Wt::WApplication::instance());
    if (app) {
        app->updateValues();
    }
}

/*!
 * \brief SimulatorApp::settingsClicked
 */
void SimulatorApp::settingsClicked()
{
    if (!_settings)  // create on demand
    {
        _settings = std::make_unique<SimulatorSettingsDialog>(frame());
    }
    _settings->show();
}

/*!
 * \brief SimulatorApp::startClicked
 */
void SimulatorApp::startClicked()
{
    // make the server active - that is generating new values
    (SimulatorOpc::instance()->getProcess())->start();
}

/*!
 * \brief SimulatorApp::stopClicked
 */
void SimulatorApp::stopClicked()
{
    // make the server inactive that is stop generating values
    (SimulatorOpc::instance()->getProcess())->stop();
}

/*!
 * \brief SimulatorSettingsDialog::show
 */
void SimulatorSettingsDialog::show()
{
    MRL::PropertyPath path;
    path.push_back(STOCKDEFS::ConfigureSection);

    // load the fields up - JSON numbers are all doubles
    int i = int(MRL::OpcServiceCommon::data().getValue<double>(path, "Type"));
    dialog()->Type->setCurrentIndex(i);
    i = int(MRL::OpcServiceCommon::data().getValue<double>(path, "Range"));
    if (i < 10)
        i = 10;
    dialog()->Range->setValue(i);
    i = int(MRL::OpcServiceCommon::data().getValue<double>(path, "Interval"));
    if (i < 2)
        i = 2;
    dialog()->Interval->setValue(i);
    MRL::ModalDialog<Simulator::Dialog_SettingsDialog, Simulator>::show();
}

/*!
 * \brief SimulatorSettingsDialog::onClose
 * \param code
 */
void SimulatorSettingsDialog::onClose(Wt::WDialog::DialogCode code)
{
    if (code == Wt::WDialog::Accepted) {
        MRL::PropertyPath path;
        path.push_back(STOCKDEFS::ConfigureSection);
        MRL::OpcServiceCommon::data().setValue(path, "Type", (double)(dialog()->Type->currentIndex()));
        MRL::OpcServiceCommon::data().setValue(path, "Range", (double)(dialog()->Range->value()));
        MRL::OpcServiceCommon::data().setValue(path, "Interval", (double)(dialog()->Interval->value()));
        // write the configuration away
        MRL::OpcServiceCommon::saveConfiguration(MRL::OpcServiceCommon::instance()->name());
        // in this case the data collection / interface process reads settings every iteration.
        // generally it may be necessary to notify that the configuration has changed
    }
}
