#ifndef VARIANTPROPERTYTREE_H
#define VARIANTPROPERTYTREE_H

#include "trace.hpp"
#include "propertytree.h"
#include <Wt/Json/Object>
#include <Wt/Json/Value>
#include <OpcServiceCommon/variant.h>
#include <OpcServiceCommon/vauletree.h>

namespace MRL {
/*!
 * \brief The VariantPropertyTree class
 * Wt::JSON does not store type information for numbers, so all numbers in Json are doubles
 */
class VariantPropertyTree : public ValueTree<Variant>
{
public:
    VariantPropertyTree() = default;

    template <typename P>
    void setNumber(const P& path, int v)
    {
        double a = v;
        setValue(path, a);
    }
    template <typename P>
    void setNumber(const P& path, unsigned int v)
    {
        double a = v;
        setValue(path, a);
    }
    template <typename P>
    void setNumber(const P& path, long long v)
    {
        double a = v;
        setValue(path, a);
    }
    template <typename P>
    void setNumber(const P& path, unsigned long long v)
    {
        double a = v;
        setValue(path, a);
    }

    template <typename P>
    void setNumber(const P& path, const std::string& v)
    {
        double a = std::stod(v);
        setValue(path, a);
    }

    template <typename P, typename T>
    /*!
     * \brief getNumber
     * \param path
     * \return variant as a number of requested type or zero
     */
    T getNumber(const P& path)
    {
        auto n   = node(path);
        double a = 0.0;
        if (n) {
            try {
                if (isType<std::string>(n->data())) {
                    std::string s = boost::get<std::string>(n->data());
                    a             = std::stod(s);
                }
                else if (isType<bool>(n->data())) {
                    bool f = boost::get<bool>(n->data());
                    a      = f ? 1.0 : 0.0;
                }
                else {
                    a = getValue<double>(path);
                }
            }
            catch (...) {
            }
        }
        return T(a);
    }
};
}  // namespace MRL

#endif  // VARIANTPROPERTYTREE_H
