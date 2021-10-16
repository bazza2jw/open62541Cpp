#ifndef VARIANT_H
#define VARIANT_H
#include "propertytree.h"
#include <map>
#include <typeinfo>
#include <string>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <list>
//
// JSON support
#include <Wt/Json/Value>
//

namespace MRL {
typedef boost::variant<int, unsigned, double, std::string, bool, time_t, void*> Variant;
typedef std::list<Variant> VariantList;  //!< list of variants
typedef boost::shared_ptr<VariantList> VariantListPtr;
typedef std::vector<std::string> StringList;
typedef std::map<std::string, Variant> VariantMap;
typedef boost::shared_ptr<VariantMap> VariantMapPtr;
std::string toString(const Variant& v);
std::string toJsonString(const Variant& v);
/*!
    \brief PropertyPath
*/
typedef NodePath<std::string> PropertyPath;
//
// convert to/from JSON
void setJson(Wt::Json::Value&, Variant&);
void getJson(Wt::Json::Value&, Variant&);

template <typename T>
/*!
    \brief isType
    \param a
    \return
*/
inline bool isType(Variant& a)
{
    return a.type().hash_code() == typeid(T).hash_code();
}

inline const std::string valueToString(const MRL::Variant& v)
{
    return toString(v);
}

template <typename T>
const T& valueToType(const MRL::Variant& v)
{
    return boost::get<T>(v);
}

}  // namespace MRL

#endif  // VARIANT_H
