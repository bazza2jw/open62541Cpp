#ifndef ROLLINGBUFFER_HPP
#define ROLLINGBUFFER_HPP
/*!
    \file rollingbuffer.hpp

    \author B. J. Hill
    \date __DATE__
    License:  GNU LESSER GENERAL PUBLIC LICENSE 2.1
    (c)  Micro Research Limited 2010 -
*/
//

#include "stats.hpp"
#include <deque>
#include <time.h>
//
namespace MRL {
/**
    @brief  Rolling buffers store the last n values deteremined by count or time frame
*/
template <typename T>
class RollingBuffer
{
public:
    typedef enum {
        TimeWindow = 0,  // buffer is time window controlled
        CountWindow      // buffer is item count controlled
    } WindowType;

    struct rItem {
        time_t _time;
        T _value;
        rItem(time_t t, T v)
            : _time(t)
            , _value(v)
        {
        }
        rItem(const rItem& i) = default;
    };

private:
    int _width = 60;  //!< number of samples to hold in the rolling buffer
    //
    std::deque<rItem> _buffer;  //!< buffer of time stamped values
    //
    bool _changed = false;          // if true stats need to be recalculated
    StatisticsThresholdSet _stats;  // the current statistic of the buffer
    WindowType _windowType = CountWindow;

public:
    /**
        @brief  Constructs a rolling buffer  of given size.

        @fn RollingBuffer
        @param width Buffer Size
    */
    RollingBuffer(int width = 60, WindowType w = CountWindow)
        : _width(width)
        , _windowType(w)
    {
    }

    /*!
        \brief changed
        \return
    */
    bool changed() const { return _changed; }
    /*!
        \brief setChanged
        \param f
    */
    void setChanged(bool f = true) { _changed = f; }
    /*!
        \brief size
        \return
    */
    int size() const { return int(_buffer.size()); }
    /**
        @brief  Copy constructor

        @fn RollingBuffer
        @param r  Object to copy
    */
    RollingBuffer(const RollingBuffer& r)
        : _width(r._width)
        , _buffer(r._buffer)
        , _changed(r._changed)
        , _windowType(r._windowType)
    {
    }

    /**
        @brief  Clears buffer and resets the statisics

        @fn clearBuffer
    */
    void clearBuffer()
    {
        _stats.clear();
        _buffer.clear();
    }
    //
    /**
        @brief Gets the buffer width

        @fn getWidth
        @return int
    */
    int width() const { return _width; }
    //
    /**
        @brief  Sets the buffer width

        @fn setWidth
        @param width
    */
    void setWidth(int w)
    {
        if (w > 0) {
            _width   = w;
            _changed = true;
        }
    }

    /**
        @brief  Adds a value to the rolling buffer. If more than width values are in the buffer then the oldest value is
       dropped.

        @fn addValue
        @param v  Value to add to buffer
    */
    void addValue(T v)
    {
        _changed = true;
        rItem d(::time(nullptr), v);
        _buffer.push_back(d);
        //
        if (_windowType == CountWindow) {
            while ((int)_buffer.size() > _width)
                _buffer.pop_front();
        }
        else {
            while ((std::difftime(_buffer.back()._time, _buffer.front()._time) > double(_width)) &&
                   (_buffer.size() > 0)) {
                _buffer.pop_front();
            }
        }
    }
    /*!
        \brief last
        \return
    */
    rItem& last() { return _buffer.back(); }

    std::deque<rItem>& buffer() { return _buffer; }
};

/*!
    \brief The StatisticsBuffer class
*/
class StatisticsBuffer : public RollingBuffer<double>
{
    StatisticsThresholdSet _stats;  // the current statistic of the buffer
public:
    StatisticsBuffer(int width = 60, WindowType w = CountWindow)
        : RollingBuffer(width, w)
    {
    }
    /**
        @brief Gets the current statistics without re-evaluating the statistics of the buffered values
        @fn readStatistics
        @return StatisticsThresholdSet
    */
    StatisticsThresholdSet& statistics() { return _stats; }
    /**
        @brief Evaluates the statistics of the buffer and returns the result.
        @return StatisticsThresholdSet
    */
    StatisticsThresholdSet& evaluate()
    {
        if (changed()) {
            // recalculate if necessary
            _stats.clear();
            for (size_t i = 0; i < buffer().size(); i++) {
                _stats.setValue(buffer()[i]._value);
            }
        };
        setChanged(false);
        return _stats;
    }
    /*!
        \brief clear
    */
    void clear()
    {
        clearBuffer();
        _stats.clear();
    }
};

/*!
    \brief The BooleanBuffer class
*/
class BooleanBuffer : public RollingBuffer<bool>
{
    int _hi = 0;
    int _lo = 0;

public:
    /*!
        \brief BooleanBuffer
        \param width
        \param w
    */
    BooleanBuffer(int width = 60, WindowType w = CountWindow)
        : RollingBuffer(width, w)
    {
    }

    int hi() const { return _hi; }
    int lo() const { return _lo; }

    /*!
        \brief evaluate
        \return
    */
    int evaluate()
    {
        if (changed()) {
            // recalculate if necessary
            _hi = 0;
            _lo = 0;
            for (size_t i = 0; i < buffer().size(); i++) {
                if (buffer()[i]._value) {
                    _hi++;
                }
                else {
                    _lo++;
                }
            }
        };
        setChanged(false);
        return _hi;
    }

    /*!
        \brief clear
    */
    void clear()
    {
        clearBuffer();
        _hi = _lo = 0;
    }
};
}  // namespace MRL
#endif
