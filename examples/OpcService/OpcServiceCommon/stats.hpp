#ifndef STATS_HPP
#define STATS_HPP
/*!
    \file stats.hpp

    \author B. J. Hill
    \date __DATE__
    License:  GNU LESSER GENERAL PUBLIC LICENSE 2.1
    (c)  Micro Research Limited 2010 -
    $Id: stats.hpp,v 1.1.1.1 2013/12/24 18:07:04 barry Exp $
*/
#include <vector>
#include <math.h>
#include <time.h>
namespace MRL {
/*!
    \brief The StatisticsBase class
    Use this for reducing large data sets for plotting
*/
class StatisticsBase
{
    double _lastValue       = 0.0;
    unsigned _numberSamples = 0;
    double _sum             = 0.0;
    double _sumSquares      = 0.0;
    double _minimum         = 0.0;
    double _maximum         = 0.0;

public:
    StatisticsBase() {}
    StatisticsBase(const StatisticsBase& s)
        : _lastValue(s._lastValue)
        , _numberSamples(s._numberSamples)
        , _sum(s._sum)
        , _sumSquares(s._sumSquares)
        , _minimum(s._minimum)
        , _maximum(s._maximum)
    {
    }

    virtual void clear()
    {
        _lastValue = _sum = _sumSquares = _minimum = _maximum = 0.0;
        _numberSamples                                        = 0;
    }

    void setValue(double v)
    {
        _lastValue = v;
        _sum += v;
        _sumSquares += v * v;
        if (!_numberSamples) {
            _minimum = _maximum = v;
        }
        else {
            if (v > _maximum) {
                _maximum = v;
            }
            else if (v < _minimum) {
                _minimum = v;
            };
        }
        _numberSamples++;
    }

    /*!
        \brief lastValue
        \return
    */
    double lastValue() const { return _lastValue; }
    /*!
        \brief minimum
        \return
    */
    double minimum() const { return _minimum; }
    /*!
        \brief maximum
        \return
    */
    double maximum() const { return _maximum; }
    /*!
        \brief numberSamples
        \return
    */
    int numberSamples() const { return _numberSamples; }
    /*!
        \brief mean
        \return
    */
    double mean() const
    {
        if (_numberSamples) {
            return _sum / double(_numberSamples);
        }
        return 0.0;
    }

    /**
        @brief  Returns the variance of the values

        @fn variance
        @return double
    */
    double variance() const
    {
        if (_numberSamples > 1) {
            return ((_sumSquares - ((_sum * _sum) / _numberSamples)) / (_numberSamples - 1));
        }
        else {
            return (0.0);
        }
    }
    // standard deviation
    /**
        @brief  Returns the standard deviation of the values

        @fn getStdDev
        @return double
    */
    double stdDev() const
    {
        if (_numberSamples <= 0 || variance() <= 0) {
            return (0);
        }
        else {
            return ((double)sqrt(variance()));
        }
    }
};

/**
    @brief  General statistics class for populations with convenience features for data acquisition systems
    This class calculates the statisics of values added to it. This class includes SPC features. Upper and lower control
   limits can be defined and are tested each time a new value is added to the object. The times of data addition and
   when control limits are exceeded are tracked.
    @class Statistics stats.hpp "stats.hpp"
*/
class Statistics
{
    double lastValue       = 0.0;
    unsigned numberSamples = 0;
    double sum             = 0.0;
    double sumSquares      = 0.0;
    double minimum         = 0.0;
    double oaximum         = 0.0;
    //
    // SPC metrics
    //
    bool trackSpc       = false;
    double upperControl = 100.0;
    double lowerControl = 0.0;
    //
    int trendCount   = 0;
    int meanCrowding = 0;
    int triggerCount = 0;

    bool dirTrendUp   = false;
    bool dirTrendDown = false;
    //
    time_t updateTime;
    bool upperControlEnabled = false;
    bool lowerControlEnabled = false;
    time_t lowerControlTriggerTime;
    time_t upperControlTriggerTime;
    //
    // SPC events
    bool _triggerCountExceeded = false;
    bool _meanCrowdingExceeded = false;
    bool _trendCountExceeded   = false;
    //
    bool _triggerCountEnabled = false;
    bool _meanCrowdingEnabled = false;
    bool _trendCountEnabled   = false;

    //
    int _triggerCountLimit = 4;
    int _meanCrowdingLimit = 10;
    int _trendCountLimit   = 5;
    //

public:
    enum { SpcAlarmNone = 0, SpcAlarmMeanCrowding = 1, SpcAlarmTriggerCount = 2, SpcAlarmTrendCount = 4 };
    /**
        @brief Constructs a Statistics object

        @fn Statistics
    */
    Statistics() {}

    /**
        @brief  Copy constructor

        @fn Statistics
        @param s Object to copy
    */
    Statistics(const Statistics& s)
        : lastValue(s.lastValue)
        , numberSamples(s.numberSamples)
        , sum(s.sum)
        , sumSquares(s.sumSquares)
        , minimum(s.minimum)
        , maximum(s.maximum)
        , trackSpc(s.trackSpc)
        , upperControl(s.upperControl)
        , lowerControl(s.lowerControl)
        , trendCount(s.trendCount)
        , meanCrowding(s.meanCrowding)
        , triggerCount(s.triggerCount)
        , dirTrendUp(false)
        , dirTrendDown(false)
        , updateTime(s.updateTime)
        , upperControlEnabled(s.upperControlEnabled)
        , lowerControlEnabled(s.lowerControlEnabled)
        , lowerControlTriggerTime(s.lowerControlTriggerTime)
        , upperControlTriggerTime(s.upperControlTriggerTime)
    {
    }

    virtual ~Statistics() {}
    /**
        @brief  reset the statistics tracks

        @fn clear
    */
    virtual void clear()
    {
        lastValue = sum = sumSquares = minimum = maximum = 0;
        numberSamples = trendCount = meanCrowding = triggerCount = 0;
        dirTrendUp = dirTrendDown = false;
    }
    /**
        @brief adds a new value and updates the statistics

        @fn setValue
        @param v value to add to statistics
    */
    virtual void setValue(double v);
    /**
        @brief Gets the last value added to the statisics object

        @fn getLastValue
        @return double
    */
    double getLastValue() const { return lastValue; }
    /**
        @brief Get the number of samples added to the set

        @fn getNumberSamples
        @return unsigned
    */
    unsigned getNumberSamples() const { return numberSamples; }
    /**
        @brief Returns the minimum of the values added to the statitics

        @fn getMinimum
        @return double
    */
    double getMinimum() const { return minimum; }
    /**
        @brief Returns the maximum of the values added to the statisics

        @fn getMaximum
        @return double
    */
    double getMaximum() const { return maximum; }
    /**
        @brief Returns maximum minus minimum

        @fn getRange
        @return double
    */
    double getRange() const { return maximum - minimum; }
    /**
        @brief Returns the T-Value for the statisics set

        @fn tval
        @param pLevel
        @param degreesOfFreedom
        @return double
    */
    static double tval(double pLevel, int degreesOfFreedom);
    /**
        @brief  Returns the sum of values added

        @fn getSum
        @return double
    */
    double getSum() const { return sum; }
    /**
        @brief Returns the upper control  limit

        @fn getUpperControl
        @return double
    */
    double getUpperControl() const { return upperControl; }

    /**
        @brief Returns the upper control  limit

        @fn getUpperControl
        @return double
    */
    void setUpperControl(double v) { upperControl = v; }

    /**
        @brief Returns the lower control limit

        @fn getLowerControl
        @return double
    */
    double getLowerControl() const { return lowerControl; }
    /**
        @brief Returns the lower control limit

        @fn getLowerControl
        @return double
    */
    void setLowerControl(double v) { lowerControl = v; }

    /**
        @brief Returns if the upper control limit is enabled

        @fn getUpperControlEnabled
        @return bool
    */
    bool getUpperControlEnabled() const { return upperControlEnabled; }
    /**
        @brief  Returns if the lower control limit is enabled

        @fn getLowerControlEnabled
        @return bool
    */
    bool getLowerControlEnabled() const { return lowerControlEnabled; }
    /**
        @brief Sets the enable state for the upper control limit

        @fn setUpperControlEnabled
        @param f  Set to true to enable control limit
    */
    void setUpperControlEnabled(bool f) { upperControlEnabled = f; }
    /**
        @brief Sets the enable state for the lower control limit

        @fn setLowerControlEnabled
        @param f Set to true to enable the control limit
    */
    void setLowerControlEnabled(bool f) { lowerControlEnabled = f; }
    /**
        @brief Gets the time of the last value added to the statisics

        @fn getUpdateTime
        @return QDateTime
    */
    time_t getUpdateTime() const { return updateTime; }
    /**
        @brief  Returns the last time the upper control limit was exceeded, if enabled

        @fn getUpperControlTriggerTime
        @return QDateTime
    */
    time_t getUpperControlTriggerTime() const { return upperControlTriggerTime; }
    /**
        @brief Returns the time the lower control limit was exceeded, if enabled

        @fn getLowerControlTriggerTime
        @return QDateTime
    */
    time_t getLowerControlTriggerTime() const { return lowerControlTriggerTime; }

    /**
        @brief  Returns the variance of the values

        @fn variance
        @return double
    */
    double variance() const
    {
        if (getNumberSamples() > 1) {
            return ((sumSquares - ((sum * sum) / getNumberSamples())) / (getNumberSamples() - 1));
        }
        else {
            return (0.0);
        }
    }
    // standard deviation
    /**
        @brief  Returns the standard deviation of the values

        @fn getStdDev
        @return double
    */
    double getStdDev() const
    {
        if (getNumberSamples() <= 0 || variance() <= 0) {
            return (0);
        }
        else {
            return ((double)sqrt(variance()));
        }
    }
    //
    /**
        @brief  The determines the confidence value such that the given percentage of values lie within mean +/- the
       confidence value

        @fn confidence
        @param interval  This is the percentage confidence to be evaluated
        @return double
    */
    double confidence(int interval) const
    {
        double p = ((double)interval) / 100.0;
        return confidence(p);
    }
    //
    /**
        @brief  The determines the confidence value such that the given fraction of values lie within mean +/- the
       confidence value

        @fn confidence
        @param p_value fraction of values to calculate confidence limit for
        @return double
    */
    double confidence(double p_value) const
    {
        int df = getNumberSamples() - 1;
        if (df <= 0)
            return HUGE_VAL;
        double t = tval((1.0 + p_value) * 0.5, df);
        if (t == HUGE_VAL)
            return t;
        else
            return (t * getStdDev()) / sqrt(double(getNumberSamples()));
    }
    /**
        @brief  Returns the mean of the values

        @fn getMean
        @return double
    */
    double getMean() const
    {
        if (numberSamples > 0) {
            return sum / (double)numberSamples;
        };
        return 0;
    }
    //
    /**
        @brief Returns the number of consecitively increasing or decreasing values that have been added.
        This values can indicate a drift in the population. This is an SPC metric.

        @fn getTrendCount
        @return unsigned
    */
    int getTrendCount() const { return trendCount; }
    /**
        @brief Returns the number of consectiuve values that have exceed the control limits.
        This is an SPC metric. Typically the control limits are set at one standard deviation. Therefore one in three
       values should exceed the control limits. Typically three consecutive values outside the control limits is an
       indicator of a fault condition

        @fn getTriggerCount
        @return unsigned
    */
    int getTriggerCount() const { return triggerCount; }
    /**
        @brief Returns the numbe rof consecutive values not outside the control limits
        This is an SPC metric. As one in three values should lie outside the control limits the absence of values
       outside the control limits can indicate a fault. Typically a mean crowding value of  > 10 is taken to mean either
       the control limits are incorrect or the data being measured is faulty (eg failed sensor))
        @fn getMeanCrowding
        @return unsigned
    */
    int getMeanCrowding() const { return meanCrowding; }
    /**
        @brief  Returrns true if SPC tracking is enabled

        @fn getTrackSpc
        @return bool
    */
    bool getTrackSpc() const { return trackSpc; }
    /**
        @brief  Sets or clears SPC tracking

        @fn setTrackSpc
        @param f  True enables SPC tracking
    */
    void setTrackSpc(bool f) { trackSpc = f; }

    int spcAlarmTriggered();  // returns set of flags

    /*!
        \brief triggerCountExceeded
        \return
    */
    bool triggerCountExceeded() const { return _triggerCountExceeded; }
    /*!
        \brief meanCrowdingExceeded
        \return
    */
    bool meanCrowdingExceeded() const { return _meanCrowdingExceeded; }
    /*!
        \brief trendCountExceeded
        \return
    */
    bool trendCountExceeded() const { return _trendCountExceeded; }
    //
    /*!
        \brief triggerCountLimit
        \return
    */
    int triggerCountLimit() const { return _triggerCountLimit; }
    /*!
        \brief setTriggerCountLimit
        \param v
        \return
    */
    void setTriggerCountLimit(int v) { _triggerCountLimit = v; }
    //
    /*!
        \brief meanCrowdingLimit
        \return
    */
    int meanCrowdingLimit() const { return _meanCrowdingLimit; }
    /*!
        \brief setMeanCrowdingLimit
        \param v
        \return
    */
    void setMeanCrowdingLimit(int v) { _meanCrowdingLimit = v; }
    /*!
        \brief trendCountLimit
        \return
    */
    int trendCountLimit() const { return _trendCountLimit; }
    /*!
        \brief setTrendCountLimit
        \param v
        \return
    */
    void setTrendCountLimit(int v) { _trendCountLimit = v; }

    /*!
        \brief triggerCountExceeded
        \return
    */
    bool triggerCountEnabled() const { return _triggerCountEnabled; }
    /*!
        \brief meanCrowdingExceeded
        \return
    */
    bool meanCrowdingEnabled() const { return _meanCrowdingEnabled; }
    /*!
        \brief trendCountExceeded
        \return
    */
    bool trendCountEnabled() const { return _trendCountEnabled; }

    /*!
        \brief triggerCountExceeded
        \return
    */
    void setTriggerCountEnabled(bool f) { _triggerCountEnabled = f; }
    /*!
        \brief meanCrowdingExceeded
        \return
    */
    void setMeanCrowdingEnabled(bool f) { _meanCrowdingEnabled = f; }
    /*!
        \brief trendCountExceeded
        \return
    */
    void setTrendCountEnabled(bool f) { _trendCountEnabled = f; }
};
//
// used for testing a value against
/**
    @brief This class handles SCADA threshold triggers.
    SCADA systems typically have four action / event limits; HiHi, HiLo,LoHi,LoLo.
    Some times called upper alarm, upper warning, lower warning aand lower alarm.
    This class  encapsulates one threshold.

    @class StatisticsThreshold stats.hpp "stats.hpp"
*/
class StatisticsThreshold
{
    double _threshold = 0;
    int _triggerCount = 0;
    bool _enabled     = false;
    bool _triggered   = false;
    time_t _triggerTime;

public:
    typedef enum { None = 0, HiHi, HiLo, LoHi, LoLo, NumberThresholds } ThresholdTypes;

    /**
        @brief This constructs a threshold

        @fn StatisticsThreshold
        @param t threshold limit
    */
    StatisticsThreshold(double t = 0)
        : _threshold(t)
        , _triggerCount(0)
        , _enabled(false)
        , _triggered(false)
    {
    }
    /**
        @brief Copy constructor

        @fn StatisticsThreshold
        @param s object to copy from
    */
    StatisticsThreshold(const StatisticsThreshold& s)
        : _threshold(s._threshold)
        , _triggerCount(s._triggerCount)
        , _enabled(s._enabled)
        , _triggered(s._triggered)
    {
    }
    //
    /**
        @brief returns the trigger threshold

        @fn threshold
        @return double
    */
    double threshold() const { return _threshold; }
    /**
        @brief sets the trigger threshold

        @fn setThreshold
        @param t  the threshold
        @param e threshold enable
    */
    void setThreshold(double t, bool e = true)
    {
        _threshold = t;
        _enabled   = e;
    }
    /**
        @brief returns the number of times the threshold has been triggered

        @fn triggerCount
        @return int
    */
    int triggerCount() const { return _triggerCount; }
    /**
        @brief resets the trigger count to zero and clears the triggered flag

        @fn clear
    */
    void clear()
    {
        _triggerCount = 0;
        _triggered    = false;
    }
    //
    /**
        @brief  compares the value with the treshold and triggers if it more than the threshold
        Returns true if triggered.
        @fn compareHi
        @param v
        @return bool
    */
    bool compareHi(double v)
    {
        _triggered = (v >= _threshold) && _enabled;
        if (_triggered)
            _triggerCount++;
        return _triggered;
    }
    //
    /**
        @brief  compares the value with the treshold and triggers if it less than the threshold
        Returns true if triggered
        @fn compareLo
        @param v value to compare
        @return bool
    */
    bool compareLo(double v)
    {
        _triggered = (v <= _threshold) && _enabled;
        if (_triggered)
            _triggerCount++;
        return _triggered;
    }
    //
    /**
        @brief  increments the triggere count

        @fn increment
    */
    void increment() { _triggerCount++; }
    /**
        @brief  returns true if the threshold is enabled

        @fn enabled
        @return bool
    */
    bool enabled() { return _enabled; }
    /**
        @brief  sets the enabled state of the trigger

        @fn setEnabled
        @param f if true the threshold is enabled
    */
    void setEnabled(bool f) { _enabled = f; }
    /**
        @brief returns the trigger state of the trheshold

        @fn triggered
        @return bool
    */
    bool triggered() const { return _triggered; }
    //
};

// statistics with alarm thresholds
/**
    @brief  This class is a Statistics class with the four standard SCADA thresholds, HiHi, HiLo, LoHi,LoLo
    As each value is
    @class StatisticsThresholdSet stats.hpp "stats.hpp"
*/
class StatisticsThresholdSet : public Statistics
{
    std::vector<StatisticsThreshold> _thresholds;
    //
    bool _triggered = false;  // any of thresholds triggered after setValue
    bool _hihilolo  = false;  // hihi or lolo triggered
    bool _hilolohi  = false;  // hilo or lohi triggered
    //
    // SPC events
    bool _triggerCountExceeded = false;
    bool _meanCrowdingExceeded = false;
    bool _trendCountExceeded   = false;
    //
    bool _triggerCountEnabled = false;
    bool _meanCrowdingEnabled = false;
    bool _trendCountEnabled   = false;

    //
    int _triggerCountLimit = 4;
    int _meanCrowdingLimit = 10;
    int _trendCountLimit   = 5;
    //
public:
    /**
        @brief Constructs

        @fn StatisticsThresholdSet
    */
    StatisticsThresholdSet()
        : _thresholds(StatisticsThreshold::NumberThresholds)
        , _triggered(false)
        , _hihilolo(false)
        , _hilolohi(false)
    {
    }
    //
    /**
        @brief  Copy constructor

        @fn StatisticsThresholdSet
        @param s object to copy
    */
    StatisticsThresholdSet(const StatisticsThresholdSet& s)
        : Statistics(s)
        , _thresholds(s._thresholds)
        , _triggered(s._triggered)
        , _hihilolo(s._hihilolo)
        , _hilolohi(s._hilolohi)
    {
    }
    //
    /**
        @brief returns the indexed threshold

        @fn thresholds
        @param i threshold index
        @return StatisticsThreshold
    */
    StatisticsThreshold& thresholds(int i) { return _thresholds[i]; }
    //
    /**
        @brief  sets all thresholds

        @fn setThresholds
        @param lolo LoLo threshold
        @param lohi  LoHi threshold
        @param hilo HiLo threshold
        @param hihi  HiHi threshold
        @param loloEnable LoLo enable
        @param lohiEnable LoHi enable
        @param hiloEnable HiLo enable
        @param hihiEnable HiHi enable
    */
    void setThresholds(double lolo,
                       double lohi,
                       double hilo,
                       double hihi,
                       bool loloEnable = true,
                       bool lohiEnable = true,
                       bool hiloEnable = true,
                       bool hihiEnable = true)
    {
        _thresholds[StatisticsThreshold::HiHi].setThreshold(hihi, hihiEnable);
        _thresholds[StatisticsThreshold::HiLo].setThreshold(hilo, hiloEnable);
        _thresholds[StatisticsThreshold::LoHi].setThreshold(lohi, lohiEnable);
        _thresholds[StatisticsThreshold::LoLo].setThreshold(lolo, loloEnable);
    }

    void setThreshold(StatisticsThreshold::ThresholdTypes i, double level, bool enable = true)
    {
        _thresholds[i].setThreshold(level, enable);
    }

    /*!
        \brief

        \fn setThreshold
        \param i
        \param t
    */
    void setThreshold(StatisticsThreshold::ThresholdTypes i, StatisticsThreshold& t) { _thresholds[i] = t; }

    //
    /**
        @brief  returns true if any threshold has been triggered by the last value added

        @fn triggered
        @return bool
    */
    bool triggered() const
    {
        return _triggered;  // any levels triggered
    }
    /**
        @brief  returns true if either HiHi or LoLo has been triggered

        @fn triggeredHiHiLoLo
        @return bool
    */
    bool triggeredHiHiLoLo() const { return _hihilolo; }
    /**
        @brief returns true if either HiLo or LoHi have been triggered.

        @fn triggeredHiLoLoHi
        @return bool
    */
    bool triggeredHiLoLoHi() const { return _hilolohi; }
    //
    // return the highest state

    /**
        @brief

        @return StatisticsThreshold::ThresholdTypes
    */
    StatisticsThreshold::ThresholdTypes maxState()
    {
        if (triggered()) {
            if (_thresholds[StatisticsThreshold::HiHi].triggered())
                return StatisticsThreshold::HiHi;
            if (_thresholds[StatisticsThreshold::LoLo].triggered())
                return StatisticsThreshold::LoLo;
            if (_thresholds[StatisticsThreshold::HiLo].triggered())
                return StatisticsThreshold::HiLo;
            if (_thresholds[StatisticsThreshold::LoHi].triggered())
                return StatisticsThreshold::LoHi;
        }
        return StatisticsThreshold::None;
    }

    //
    /**
        @brief Adds a new value to the statistics. The thresholds are tested and triggered.

        @fn setValue
        @param v
    */
    void setValue(double v)
    {
        _hihilolo =
            _thresholds[StatisticsThreshold::LoLo].compareLo(v) || _thresholds[StatisticsThreshold::HiHi].compareHi(v);
        _hilolohi =
            _thresholds[StatisticsThreshold::HiLo].compareHi(v) || _thresholds[StatisticsThreshold::LoHi].compareLo(v);
        _triggered = _hihilolo || _hilolohi;
        if (!triggered())
            _thresholds[StatisticsThreshold::None].increment();  // nothing changed
        Statistics::setValue(v);
    }
    //
    /**
        @brief Clears and resets the statistics object and trigger states

        @fn clear
    */
    void clear()
    {
        for (int i = 0; i < StatisticsThreshold::NumberThresholds; i++)
            _thresholds[i].clear();
        Statistics::clear();
        _triggered = _hihilolo = _hilolohi = false;
        _triggerCountExceeded              = false;
        _meanCrowdingExceeded              = false;
        _trendCountExceeded                = false;
    }
    //
    /*!
        \brief hiHiLoLoCount
        \return
    */
    int hiHiLoLoCount()
    {
        // Action / Alarm count
        return _thresholds[StatisticsThreshold::LoLo].triggerCount() +
               _thresholds[StatisticsThreshold::HiHi].triggerCount();
    }
    /*!
        \brief hiLoLoHiCount
        \return
    */
    int hiLoLoHiCount()
    {
        // Alert / Warning count
        return _thresholds[StatisticsThreshold::LoHi].triggerCount() +
               _thresholds[StatisticsThreshold::HiLo].triggerCount();
    }
};
}  // namespace MRL
#endif
