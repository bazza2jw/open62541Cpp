#include "variant.h"

// This is the object's type
#define JSON_OBJECT_TYPE "__TYPE__"

/*!
    \brief setJson
    \param v
    \param a
*/
void MRL::setJson(Wt::Json::Value& v, Variant& a)
{
    const char* tn = a.type().name();
    switch (tn[0]) {
        case 'i':
            v = boost::get<int>(a);
            break;
        case 'u':
            v = (double)(boost::get<unsigned>(a));
            break;
        case 'd':
            v = boost::get<double>(a);
            break;
        case 'S':
            v = boost::get<std::string>(a).c_str();
            break;
        case 'b':
            v = boost::get<bool>(a);
            break;
        case 't':
            v = (long long)(boost::get<time_t>(a));
        default:
            break;
    }
}

/*!
    \brief getJson
    \param v
    \param a
*/
void MRL::getJson(Wt::Json::Value& v, Variant& a)
{
    switch (v.type()) {
        case Wt::Json::NumberType:  ///< number - force to double
            a = v.orIfNull(0.0);
            break;
        case Wt::Json::StringType:  ///< UTF-8 string value
            a = v.orIfNull("");
            ;
            break;
        case Wt::Json::BoolType:  ///< bool value
            a = v.orIfNull(false);
            break;
        default:
            a = int(0);
            break;
    }
}

/*!
    \brief MRL::toString
    Could use visitors but .... this may be more efficent - exploits all type names start with different letter WARNING
   !!!!! \param v \return
*/
std::string MRL::toString(const Variant& v)
{
    std::string res = "";
    const char* tn  = v.type().name();
    switch (tn[0]) {
        case 'i':
            return std::to_string(boost::get<int>(v));
        case 'u':
            return std::to_string(boost::get<unsigned>(v));
        case 'd':
            return std::to_string(boost::get<double>(v));
        case 'S':
            return boost::get<std::string>(v);
        case 'b':
            return std::string(boost::get<bool>(v) ? "true" : "false");
        case 't':
            return std::to_string(boost::get<time_t>(v));

        default:
            break;
    }
    return res;
}

std::string MRL::toJsonString(const Variant& v)
{
    std::string res = "";
    const char* tn  = v.type().name();
    switch (tn[0]) {
        case 'i':
            return std::to_string(boost::get<int>(v));
        case 'u':
            return std::to_string(boost::get<unsigned>(v));
        case 'd':
            return std::to_string(boost::get<double>(v));
        case 'S': {
            std::string s = "\"" + boost::get<std::string>(v) + "\"";
            return s;
        }
        case 'b':
            return std::string(boost::get<bool>(v) ? "true" : "false");
        case 't':
            return std::to_string(boost::get<time_t>(v));

        default:
            break;
    }
    return res;
}
