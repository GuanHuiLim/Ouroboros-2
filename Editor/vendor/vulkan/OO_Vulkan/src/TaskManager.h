#pragma once

#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

typedef std::function<void(void*)> TaskFunc;
struct TaskCompletionCallback;

struct Task
{
    TaskFunc                pTaskFunction{};              ///< The task to execute
    void*                   pTaskParam{};                 ///< Parameters (in the form of a void pointer) to pass to the task (NOTE** calling code is responsible for the memory backing parameter pointer).
    TaskCompletionCallback* pTaskCompletionCallback{};    ///< If this task is part of a larger group of tasks that require post-completion synchronization, they will be associated with a task sync primitive

    Task(TaskFunc pTaskFunction, void* pTaskParam = nullptr, TaskCompletionCallback* pCompletionCallback = nullptr) :
        pTaskFunction(pTaskFunction), pTaskParam(pTaskParam), pTaskCompletionCallback(pCompletionCallback) {}

private:
    friend class TaskManager;
    Task() {};
};

struct TaskCompletionCallback
{
    Task                    CompletionTask;     ///< The task to execute once the task count reaches 0
    std::atomic_uint        TaskCount{};          ///< Number of tasks this callback is paired with. Count will tick down upon completion of each dependent task.

    TaskCompletionCallback(Task completionTask, uint32_t taskCount = 1) :
        CompletionTask(completionTask), TaskCount(taskCount) {}

private:
    TaskCompletionCallback() = delete;
};


class TaskManager
{
public:

    /**
     * @brief   Constructor with default behavior.
     */
    TaskManager();

    /**
     * @brief   Destructor with default behavior.
     */
    virtual ~TaskManager();

    /**
     * @brief   Initialization function for the TaskManager. Dictates the size of our thread pool.
     */
    int32_t Init(uint32_t threadPoolSize);

    /**
     * @brief   Shuts down the task manager and joins all threads. Called from framework shut down procedures.
     */
    void Shutdown();

    /**
     * @brief   Enqueues a task for execution.
     */
    void AddTask(Task& newTask);

    /**
     * @brief   Enqueues multiple tasks for execution.
     */
    void AddTaskList(std::queue<Task>& newTaskList);

    void AddTaskListAndWait(std::queue<Task>& newTaskList);

private:
    TaskManager(const TaskManager&) = delete;
    TaskManager& operator=(const TaskManager&) = delete;
    TaskManager(const TaskManager&&) = delete;
    TaskManager& operator=(const TaskManager&&) = delete;

    void TaskExecutor();

    bool                        m_ShuttingDown = false;
    std::vector<std::thread>    m_ThreadPool = {};
    std::queue<Task>            m_TaskQueue = {};
    std::mutex                  m_CriticalSection;
    std::condition_variable     m_QueueCondition;
};
