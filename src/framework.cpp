/**                           _            _
 *        /\  /\___ _ __ ___ | | ___   ___| | __
 *       / /_/ / _ \ '_ ` _ \| |/ _ \ / __| |/ /
 *      / __  /  __/ | | | | | | (_) | (__|   <
 *      \/ /_/ \___|_| |_| |_|_|\___/ \___|_|\_\
 *      Tyler Neely 2015 t@jujit.su
 */

#include <libgen.h>

#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

#include <mesos/resources.hpp>
#include <mesos/scheduler.hpp>

#include <stout/check.hpp>
#include <stout/exit.hpp>
#include <stout/numify.hpp>
#include <stout/os.hpp>
#include <stout/stringify.hpp>

using namespace mesos;

using boost::lexical_cast;

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;
using std::vector;

using mesos::Resources;

const int32_t CPUS_PER_TASK = 1;
const int32_t MEM_PER_TASK = 128;

class HemlockScheduler : public Scheduler
{
public:
  explicit HemlockScheduler(const ExecutorInfo& _executor)
    : tasksLaunched(0), tasksFinished(0), totalTasks(5), executor(_executor) {}

  virtual ~HemlockScheduler() {
    cout << "destroying scheduler" << endl;
  }

  virtual void registered(SchedulerDriver*,
                          const FrameworkID&,
                          const MasterInfo&)
  {
    cout << "Registered!" << endl;
  }

  virtual void reregistered(SchedulerDriver*, const MasterInfo& masterInfo) {
    cout << "Re-registered!" << endl;
  }

  virtual void disconnected(SchedulerDriver* driver) {
    cout << "Disconnected!" << endl;
  }


  virtual void resourceOffers(SchedulerDriver* driver,
                              const vector<Offer>& offers)
  {
    foreach (const Offer& offer, offers) {
      cout << "Received offer " /*<< offer.id()*/ << " with " << offer.resources() << endl;

      static const Resources TASK_RESOURCES = Resources::parse(
          "cpus:" + stringify(CPUS_PER_TASK) +
          ";mem:" + stringify(MEM_PER_TASK)).get();

      Resources remaining = offer.resources();

      // Launch tasks.
      vector<TaskInfo> tasks;
      while (tasksLaunched < totalTasks &&
             TASK_RESOURCES <= remaining.flatten()) {
        int taskId = tasksLaunched++;

        cout << "Launching task " << taskId << endl; // << " using offer " << offer.id() << endl;

        TaskInfo task;
        task.set_name("Task " + lexical_cast<string>(taskId));
        task.mutable_task_id()->set_value(lexical_cast<string>(taskId));
        task.mutable_slave_id()->MergeFrom(offer.slave_id());
        task.mutable_executor()->MergeFrom(executor);

        Option<Resources> resources = remaining.find(TASK_RESOURCES);
        CHECK_SOME(resources);
        task.mutable_resources()->MergeFrom(resources.get());
        remaining -= resources.get();

        tasks.push_back(task);
      }

      driver->launchTasks(offer.id(), tasks);
    }
  }

  virtual void offerRescinded(SchedulerDriver* driver,
                              const OfferID& offerId) {
    cout << "Offer Rescinded!" << endl;
  }

  virtual void statusUpdate(SchedulerDriver* driver, const TaskStatus& status)
  {
    /*
    int taskId = lexical_cast<int>(status.task_id().value());
    cout << "Task " << taskId << " is in state " << status.state() << endl;
    */
    int taskId = lexical_cast<int>(status.task_id().value());

    cout << "Task " << taskId << " is in state " << status.state() << endl;

    if (status.state() == TASK_FINISHED)
      tasksFinished++;

    if (status.state() == TASK_LOST ||
        status.state() == TASK_KILLED ||
        status.state() == TASK_FAILED) {
      cout << "Aborting because task " << taskId
           << " is in unexpected state " << status.state()
           << " with reason " << status.reason()
           << " from source " << status.source()
           << " with message '" << status.message() << "'"
           << endl;
      driver->abort();
    }

    if (tasksFinished == totalTasks) {
      cout << "tasksFinished == totalTasks, so stopping driver now" << endl;
      driver->stop();
    }
  }

  virtual void frameworkMessage(SchedulerDriver* driver,
                                const ExecutorID& executorId,
                                const SlaveID& slaveId,
                                const string& data) {
    cout << "in frameworkMessage" << endl;
  }

  virtual void slaveLost(SchedulerDriver* driver, const SlaveID& slaveId) {
    cout << "in slaveLost" << endl;
  }

  virtual void executorLost(SchedulerDriver* driver,
                            const ExecutorID& executorId,
                            const SlaveID& slaveId,
                            int status) {
    cout << "in executorLost" << endl;
  }

  virtual void error(SchedulerDriver* driver, const string& message) {
    cout << "in error" << endl;
  }

private:
  int tasksLaunched;
  int tasksFinished;
  int totalTasks;
  const ExecutorInfo executor;
};


int main(int argc, char** argv)
{
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <master>" << endl;
    return -1;
  }

  string path = os::realpath(dirname(argv[0])).get();
  string uri = path + "/hemlock_executor";

  ExecutorInfo executor;
  executor.mutable_executor_id()->set_value("default");
  executor.mutable_command()->set_value(uri);
  executor.set_name("Hemlock Executor");
  executor.set_source("hemlock_framework");

  HemlockScheduler scheduler(executor);

  FrameworkInfo framework;
  framework.set_user(""); // Have Mesos fill in the current user.
  framework.set_name("Hemlock Framework");

  if (os::hasenv("MESOS_CHECKPOINT")) {
    framework.set_checkpoint(
        numify<bool>(os::getenv("MESOS_CHECKPOINT")).get());
  }

  MesosSchedulerDriver* driver;
  if (os::hasenv("MESOS_AUTHENTICATE")) {
    cout << "Enabling authentication for the framework" << endl;

    if (!os::hasenv("DEFAULT_PRINCIPAL")) {
      EXIT(1) << "Expecting authentication principal in the environment";
    }

    if (!os::hasenv("DEFAULT_SECRET")) {
      EXIT(1) << "Expecting authentication secret in the environment";
    }

    Credential credential;
    credential.set_principal(getenv("DEFAULT_PRINCIPAL"));
    credential.set_secret(getenv("DEFAULT_SECRET"));

    framework.set_principal(getenv("DEFAULT_PRINCIPAL"));

    driver = new MesosSchedulerDriver(
        &scheduler, framework, argv[1], credential);
  } else {
    framework.set_principal("hemlock");

    driver = new MesosSchedulerDriver(
        &scheduler, framework, argv[1]);
  }

  int status = driver->run() == DRIVER_STOPPED ? 0 : 1;

  // Ensure that the driver process terminates.
  driver->stop();

  delete driver;
  return status;
}
