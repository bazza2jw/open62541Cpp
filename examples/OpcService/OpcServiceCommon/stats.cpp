#include "stats.hpp"
#include <time.h>
//
// $Id: stats.cpp,v 1.1.1.1 2013/12/24 18:07:04 barry Exp $
// Statistics class
//
void MRL::Statistics::setValue(double v)
{
    updateTime = time(nullptr);
    sum += v;
    sumSquares += v * v;
    if (!numberSamples)
        minimum = maximum = v;
    numberSamples++;
    if (v > maximum) {
        maximum = v;
    }
    else if (v < minimum) {
        minimum = v;
    };
    if (trackSpc) {
        if (v > lastValue) {
            if (dirTrendUp) {
                trendCount++;
            }
            else {
                trendCount = 0;
            };
            dirTrendUp   = true;
            dirTrendDown = false;
        }
        else if (v < lastValue) {
            if (v < lastValue) {
                if (dirTrendDown) {
                    trendCount++;
                }
                else {
                    trendCount = 0;
                };
            };
            dirTrendUp   = false;
            dirTrendDown = true;
        }
        else {
            dirTrendUp = dirTrendDown = false;
            trendCount                = 0;
        };

        if (upperControlEnabled && (v >= upperControl)) {
            triggerCount++;
            meanCrowding = 0;
        }
        else if (lowerControlEnabled && (v <= lowerControl)) {
            triggerCount++;
            meanCrowding = 0;
        }
        else if (lowerControlEnabled && upperControlEnabled) {
            meanCrowding++;
            triggerCount = 0;
        }
        else {
            triggerCount = 0;
            meanCrowding = 0;
        }
    };
    lastValue = v;
}
/*!
 * \brief MRL::Statistics::tval
 * \param p
 * \param df
 * \return
 */
double MRL::Statistics::tval(double p, int df)
{
    double t;
    int positive = p >= 0.5;
    p            = (positive) ? 1.0 - p : p;
    if (p <= 0.0 || df <= 0)
        t = HUGE_VAL;
    else if (p == 0.5)
        t = 0.0;
    else if (df == 1)
        t = 1.0 / tan((p + p) * 1.57079633);
    else if (df == 2)
        t = sqrt(1.0 / ((p + p) * (1.0 - p)) - 2.0);
    else {
        double ddf = df;
        double a   = sqrt(log(1.0 / (p * p)));
        double aa  = a * a;
        a          = a - ((2.515517 + (0.802853 * a) + (0.010328 * aa)) /
                 (1.0 + (1.432788 * a) + (0.189269 * aa) + (0.001308 * aa * a)));
        t          = ddf - 0.666666667 + 1.0 / (10.0 * ddf);
        t          = sqrt(ddf * (exp(a * a * (ddf - 0.833333333) / (t * t)) - 1.0));
    }
    return (positive) ? t : -t;
}

/*!
 * \brief StatisticsThresholdSet::spcAlarmTriggered
 * \return
 */
int MRL::Statistics::spcAlarmTriggered()
{
    int ret = SpcAlarmNone;
    if (getTrackSpc()) {
        if (trendCountEnabled()) {
            _trendCountExceeded = (getTrendCount() > trendCountLimit());
            if (_trendCountExceeded)
                ret |= SpcAlarmTrendCount;
        }
        if (triggerCountEnabled()) {
            _triggerCountExceeded = (getTriggerCount() > triggerCountLimit());
            if (_triggerCountExceeded)
                ret |= SpcAlarmTriggerCount;
        }
        if (meanCrowdingEnabled()) {
            _meanCrowdingExceeded = (getMeanCrowding() > meanCrowdingLimit());
            if (_meanCrowdingExceeded)
                ret |= SpcAlarmMeanCrowding;
        }
    }
    return ret;
}
