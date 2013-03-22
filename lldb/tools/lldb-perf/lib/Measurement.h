//===-- Measurement.h -------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef __PerfTestDriver__Measurement__
#define __PerfTestDriver__Measurement__

#include "Gauge.h"
#include "Timer.h"
#include "Metric.h"
#include "MemoryGauge.h"

namespace lldb_perf
{
template <typename GaugeType, typename Callable>
class Measurement : public WriteResults
{
public:
    Measurement () :
        m_gauge (),
        m_callable (),
        m_metric ()
    {
    }
    
    Measurement (Callable callable, const char* name, const char* desc)  :
        m_gauge (),
        m_callable (callable),
        m_metric (Metric<typename GaugeType::ValueType>(name, desc))
    {
    }

    Measurement (const char* name, const char* desc)  :
        m_gauge (),
        m_callable (),
        m_metric (Metric<typename GaugeType::ValueType>(name, desc))
    {
    }

    template <typename GaugeType_Rhs, typename Callable_Rhs>
    Measurement (const Measurement<GaugeType_Rhs, Callable_Rhs>& rhs) :
        m_gauge(rhs.GetGauge()),
        m_callable(rhs.GetCallable()),
        m_metric(rhs.GetMetric())
    {
    }

    template <typename... Args>
    void
    operator () (Args... args)
    {
        m_gauge.Start();
        m_callable(args...);
        m_metric.Append (m_gauge.Stop());
    }
    
    virtual const Callable&
    GetCallable () const
    {
        return m_callable;
    }
    
    virtual const GaugeType&
    GetGauge () const
    {
        return m_gauge;
    }
    
    virtual const Metric<typename GaugeType::ValueType>&
    GetMetric () const
    {
        return m_metric;
    }
    
    void
    Start ()
    {
        m_gauge.Start();
    }
    
    typename GaugeType::ValueType
    Stop ()
    {
        auto value = m_gauge.Stop();
        m_metric.Append(value);
        return value;
    }
        
    virtual void
    Write (CFCMutableDictionary& parent)
    {
        m_metric.Write(parent);
    }

    void
    WriteStartValue (Results &results)
    {
        auto metric = GetMetric ();
        results.GetDictionary().Add(metric.GetName(), metric.GetDescription(), lldb_perf::GetResult<typename GaugeType::ValueType> (NULL, metric.GetStartValue()));
    }
    
    void
    WriteStopValue (Results &results)
    {
        auto metric = GetMetric ();
        results.GetDictionary().Add(metric.GetName(), metric.GetDescription(), lldb_perf::GetResult<typename GaugeType::ValueType> (NULL, metric.GetStopValue()));
    }

    void
    WriteAverageValue (Results &results)
    {
        auto metric = GetMetric ();
        results.GetDictionary().Add(metric.GetName(), metric.GetDescription(), lldb_perf::GetResult<typename GaugeType::ValueType> (NULL, metric.GetAverage()));
    }

protected:
    GaugeType m_gauge;
    Callable m_callable;
    Metric<typename GaugeType::ValueType> m_metric;
};
    
template <typename Callable>
class TimeMeasurement : public Measurement<TimeGauge,Callable>
{
public:
    TimeMeasurement () :
        Measurement<TimeGauge,Callable> ()
    {
    }
    
    TimeMeasurement (Callable callable,
                     const char* name = NULL,
                     const char* descr = NULL) :
        Measurement<TimeGauge,Callable> (callable, name, descr)
    {
    }
    
    template <typename Callable_Rhs>
    TimeMeasurement (const TimeMeasurement<Callable_Rhs>& rhs) :
        Measurement<TimeGauge,Callable>(rhs)
    {
    }
    
    template <typename GaugeType_Rhs, typename Callable_Rhs>
    TimeMeasurement (const Measurement<GaugeType_Rhs, Callable_Rhs>& rhs) :
        Measurement<GaugeType_Rhs,Callable_Rhs>(rhs)
    {
    }
    
    template <typename... Args>
    void
    operator () (Args... args)
    {
        Measurement<TimeGauge,Callable>::operator()(args...);
    }
};

template <typename Callable>
class MemoryMeasurement : public Measurement<MemoryGauge,Callable>
{
public:
    MemoryMeasurement () : Measurement<MemoryGauge,Callable> ()
    {
    }
    
    MemoryMeasurement (Callable callable,
                       const char* name,
                       const char* descr) :
        Measurement<MemoryGauge,Callable> (callable, name, descr)
    {
    }

    MemoryMeasurement (const char* name, const char* descr) :
        Measurement<MemoryGauge,Callable> (name, descr)
    {
    }

    template <typename Callable_Rhs>
    MemoryMeasurement (const MemoryMeasurement<Callable_Rhs>& rhs) :
        Measurement<MemoryGauge,Callable>(rhs)
    {
    }
    
    template <typename GaugeType_Rhs, typename Callable_Rhs>
    MemoryMeasurement (const Measurement<GaugeType_Rhs, Callable_Rhs>& rhs) :
        Measurement<GaugeType_Rhs,Callable_Rhs>(rhs)
    {
    }
    
    template <typename... Args>
    void
    operator () (Args... args)
    {
        Measurement<MemoryGauge,Callable>::operator()(args...);
    }
};
    
}

#endif /* defined(__PerfTestDriver__Measurement__) */
