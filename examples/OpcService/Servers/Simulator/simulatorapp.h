#ifndef SIMULATORAPP_H
#define SIMULATORAPP_H
#include <OpcServiceCommon/serverbase.h>
#include "Simulator.h"
#include <Wt/WServer>
#include <OpcServiceCommon/stockdefs.h>

/*!
    \brief The SimulatorSettingsDialog class
*/
class SimulatorSettingsDialog : public MRL::ModalDialog<Simulator::Dialog_SettingsDialog, Simulator>
{
public:
    SimulatorSettingsDialog(Simulator* p)
        : ModalDialog(p)
    {
        // Set the spin box ranges
        dialog()->Range->setMaximum(100);
        dialog()->Range->setMinimum(0);
        dialog()->Interval->setMaximum(100);
        dialog()->Interval->setMinimum(2);
    }
    void show();                                 // set up current values before edit
    void onClose(Wt::WDialog::DialogCode code);  // save editied values
};

/*!
    \brief The SimulatorApp class
*/
class SimulatorApp : public MRL::ServerBase<Simulator>
{
    // Define dialogs
    // Settings dialog
    std::unique_ptr<SimulatorSettingsDialog> _settings;

public:
    SimulatorApp(const Wt::WEnvironment& env)
        : ServerBase(env)
    {
        // connect buttons
        frame()->SettingsButton->clicked().connect(this, &SimulatorApp::settingsClicked);
        frame()->StartButton->clicked().connect(this, &SimulatorApp::startClicked);
        frame()->StopButton->clicked().connect(this, &SimulatorApp::startClicked);
        enableUpdates(true);
    }

    // sync to current values
    void updateValues();
    //
    static void handleUpdate();
    //
    // Add button handlers
    // Settings
    void settingsClicked();
    // Start
    void startClicked();
    // Stop
    void stopClicked();
    // periodic updates
};

#endif  // SIMULATORAPP_H
