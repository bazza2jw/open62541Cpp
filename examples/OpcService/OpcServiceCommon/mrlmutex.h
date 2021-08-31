#ifndef MRLMUTEX_H
#define MRLMUTEX_H
#include <boost/optional/optional.hpp>
#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind.hpp>
namespace MRL {

// Mutexs - tree access needs to be thread safe
typedef boost::shared_mutex ReadWriteMutex;
typedef boost::shared_lock<boost::shared_mutex> ReadLock;
typedef boost::unique_lock<boost::shared_mutex> WriteLock;
}  // namespace MRL
#endif  // MRLMUTEX_H
