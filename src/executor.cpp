/**                           _            _
 *        /\  /\___ _ __ ___ | | ___   ___| | __
 *       / /_/ / _ \ '_ ` _ \| |/ _ \ / __| |/ /
 *      / __  /  __/ | | | | | | (_) | (__|   <
 *      \/ /_/ \___|_| |_| |_|_|\___/ \___|_|\_\
 *      Tyler Neely 2015 t@jujit.su
 */

#include <pthread.h>
#include <stdlib.h> // For random.

#include <cstdlib>
#include <iostream>

#include <mesos/executor.hpp>

#include <stout/duration.hpp>
#include <stout/lambda.hpp>
#include <stout/os.hpp>

using namespace mesos;

using std::cout;
using std::endl;
using std::string;


void run(ExecutorDriver* driver, const TaskInfo& task)
{
  os::sleep(Seconds(random() % 10));

  TaskStatus status;
  status.mutable_task_id()->MergeFrom(task.task_id());
  status.set_state(TASK_FINISHED);

  driver->sendStatusUpdate(status);
}


void* start(void* arg)
{
  lambda::function<void(void)>* thunk = (lambda::function<void(void)>*) arg;
  (*thunk)();
  delete thunk;
  return NULL;
}


class HemlockExecutor : public Executor
{
public:
  virtual ~HemlockExecutor() {}

  virtual void registered(ExecutorDriver* driver,
                          const ExecutorInfo& executorInfo,
                          const FrameworkInfo& frameworkInfo,
                          const SlaveInfo& slaveInfo)
  {
    cout << "Registered executor on " << slaveInfo.hostname() << endl;
  }

  virtual void reregistered(ExecutorDriver* driver,
                            const SlaveInfo& slaveInfo)
  {
    cout << "Re-registered executor on " << slaveInfo.hostname() << endl;
  }

  virtual void disconnected(ExecutorDriver* driver) {}

  virtual void launchTask(ExecutorDriver* driver, const TaskInfo& task)
  {
    cout << "Starting task " << task.task_id().value() << endl;

    lambda::function<void(void)>* thunk =
      new lambda::function<void(void)>(lambda::bind(&run, driver, task));

    pthread_t pthread;
    if (pthread_create(&pthread, NULL, &start, thunk) != 0) {
      TaskStatus status;
      status.mutable_task_id()->MergeFrom(task.task_id());
      status.set_state(TASK_FAILED);

      driver->sendStatusUpdate(status);
    } else {
      pthread_detach(pthread);

      TaskStatus status;
      status.mutable_task_id()->MergeFrom(task.task_id());
      status.set_state(TASK_RUNNING);

      driver->sendStatusUpdate(status);
    }
  }

  virtual void killTask(ExecutorDriver* driver, const TaskID& taskId) {}
  virtual void frameworkMessage(ExecutorDriver* driver, const string& data) {}
  virtual void shutdown(ExecutorDriver* driver) {}
  virtual void error(ExecutorDriver* driver, const string& message) {}
};


int main(int argc, char** argv)
{
  cout << "args:" << argv << endl;
  HemlockExecutor executor;
  MesosExecutorDriver driver(&executor);
  return driver.run() == DRIVER_STOPPED ? 0 : 1;
}
