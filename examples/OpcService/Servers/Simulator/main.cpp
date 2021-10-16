#include <iostream>
#include "simulatorapp.h"
#include <boost/program_options.hpp>
#include "simulatoropc.h"
using namespace std;
namespace po = boost::program_options;

static Wt::WApplication* createApplication(const Wt::WEnvironment& env)
{
    return new SimulatorApp(env);
}

int main(int argc, char* argv[])
{
    // set the defaults
    int web_port = 8084;
    int opc_port = 4840;
    std::string name("Server");
    std::unique_ptr<Wt::WServer> webThread = std::make_unique<Wt::WServer>(argv[0]);  // the web server
    //
    cout << "Simulator" << endl;
    po::options_description desc("Simulator options");
    desc.add_options()("help", "produce help message")("web", po::value<int>(), "Web server port")(
        "opc",
        po::value<int>(),
        "OPC server port")("name", po::value<std::string>(), "Server name");
    // parse the command line
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    //
    if (vm.count("help")) {
        cout << desc << "\n";
        return 1;
    }
    //
    // Get the object name
    if (vm.count("name")) {
        name = vm["name"].as<std::string>();
    }
    // get the web port
    if (vm.count("web")) {
        web_port = vm["web"].as<int>();
    }
    // get the opc port
    if (vm.count("opc")) {
        opc_port = vm["opc"].as<int>();
    }
    // load configuration
    MRL::OpcServiceCommon::instance()->loadSettings();  // get the site (global) settings
    if (!MRL::OpcServiceCommon::instance()->loadConfiguration(name)) {
        // set up default configuration
        TRC("Failed to load configuration")
    }
    TRC("Loaded Config")
    MRL::OpcServiceCommon::data().dump();
    //
    // start web server
    {
        TRC("Starting Web")
        // start the web interface
        // Now start the web thread
        static char* av[4];
        static char portId[32];
        sprintf(portId, "--http-port=%d", web_port);
        av[0] = const_cast<char*>("Simulator");
        av[1] = const_cast<char*>("--docroot=.");
        av[2] = const_cast<char*>("--http-address=0.0.0.0");
        av[3] = portId;
        //
        webThread->setServerConfiguration(4, (char**)(av), WTHTTP_CONFIGURATION);
        webThread->addEntryPoint(Wt::EntryPointType::Application, createApplication);
        // initialise the model for the Wt model and link to the configuration and runtime
        webThread->start();  // start the server thread
    }
    //
    // start the opc server
    SimulatorOpc opc(opc_port);
    opc.start();
    //
    webThread->stop();
    sleep(1);
    //
    return 0;
}
