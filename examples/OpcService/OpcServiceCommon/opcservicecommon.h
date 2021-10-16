#ifndef OPCSERVICECOMMON_H
#define OPCSERVICECOMMON_H

#include "opcservicecommon_global.h"
#include "variantpropertytree.h"
#include <Wt/Json/Object>
#include <Wt/Json/Value>
#include <Wt/Json/Parser>
#include <Wt/Json/Serializer>

namespace MRL {

// the root directory
constexpr const char* RootDir = "/usr/local/MRL5/OpcService";
/*!
    \brief The OpcServiceCommon class
    singletons shared by objects
    mostly configuration
*/
class OpcServiceCommon
{
    //
    std::string _name;                   // server name
    VariantPropertyTree _data;           // shared data in a property tree
    static OpcServiceCommon* _instance;  // configuration singleton
    //
public:
    OpcServiceCommon();
    /*!
     * \brief instance
     * \return
     */
    static OpcServiceCommon* instance()
    {
        if (!_instance)
            _instance = new OpcServiceCommon();
        return _instance;
    }
    /*!
     * \brief data
     * \return
     */
    static VariantPropertyTree& data() { return _instance->_data; }

    /*!
     * \brief name
     * \return
     */
    const std::string& name() { return _name; }
    static bool saveConfiguration(const std::string& n);       // load the named configuration
    static bool saveSettings();                                // load site settings
    static bool loadConfiguration(const std::string& n = "");  // load the named configuration
    static bool loadSettings();                                // load site settings
    /*!
     * \brief settingFileName
     * \return
     */
    static std::string settingFileName(const std::string& n)
    {
        std::string f = std::string(MRL::RootDir) + "/data/" + n + ".setting";
        return f;
    }
    /*!
     * \brief globalFileName
     * \return
     */
    static std::string globalFileName()
    {
        std::string f = std::string(MRL::RootDir) + "/data/settings.global";
        return f;
    }
};

//
// Some helpers
//

template <typename T>
/*!
    \brief stringToNumber
    \param Text
    \return
*/
inline T stringToNumber(const std::string& Text)
{  // Text not by const reference so that the function can be used with a
    if (!Text.empty()) {
        // character array as argument
        std::stringstream ss(Text);
        T result;
        return ss >> result ? result : T(0);
    }
    return T(0);
}

/*!
    \brief stringToBool
    \param s
    \return
*/
bool stringToBool(const std::string& s);
/*!
    \brief boolToString
    \param f
    \return
*/
inline const char* boolToString(bool f)
{
    return f ? "True" : "False";
}

int stringTimeToInt(const std::string& s);

/*!
    \brief stringToJson
    \param s
    \param v
    \return
*/
inline bool stringToJson(const std::string& s, Wt::Json::Object& v)
{
    if (!s.empty()) {
        try {
            Wt::Json::parse(s, v);
            return true;
        }
        catch (const std::exception& e) {
            EXCEPT_TRC
        }
        catch (...) {
            EXCEPT_DEF
        }
    }
    return false;
}

/*!
    \brief jsonToString
    \param v
    \param s
    \return
*/
inline bool jsonToString(Wt::Json::Object& v, std::string& s)
{
    try {
        s = Wt::Json::serialize(v);
        return true;
    }
    catch (const std::exception& e) {
        EXCEPT_TRC
    }
    catch (...) {
        EXCEPT_DEF
    }
    return false;
}

}  // namespace MRL
#endif  // OPCSERVICECOMMON_H
