#include "TaskManager.h"
#include "VulkanRenderer.h"

TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

int32_t TaskManager::Init(uint32_t threadPoolSize)
{
    std::function<void()> pTaskHandler = [this]() { this->TaskExecutor(); };

    for (uint32_t i = 0; i < threadPoolSize; ++i)
        m_ThreadPool.push_back(std::thread(pTaskHandler));

    return 0;
}

void TaskManager::Shutdown()
{
    // Flag all threads to shutdown
    {
        std::unique_lock<std::mutex> lock(m_CriticalSection);
        m_ShuttingDown = true;
        m_QueueCondition.notify_all();
    }

    // Wait for all threads to be done
    std::vector<std::thread>::iterator iter = m_ThreadPool.begin();
    while (iter != m_ThreadPool.end())
    {
        iter->join();
        iter = m_ThreadPool.erase(iter);
    }
}

void TaskManager::AddTask(Task& newTask)
{
    std::unique_lock<std::mutex> lock(m_CriticalSection);
    m_TaskQueue.push(std::move(newTask));

    // Wake a single thread to pick up the task
    m_QueueCondition.notify_one();
}

void TaskManager::AddTaskList(std::queue<Task>& newTaskList)
{
    std::unique_lock<std::mutex> lock(m_CriticalSection);
    while (newTaskList.size())
    {
        m_TaskQueue.push(std::move(newTaskList.front()));
        newTaskList.pop();
    }

    // Wake up all threads to pick up as many concurrent tasks as possible
    m_QueueCondition.notify_all();
}

void TaskManager::AddTaskListAndWait(std::queue<Task>& newTaskList)
{
    if (newTaskList.empty()) return;

    std::condition_variable cv;
    std::mutex waitMut;
    bool waitingForTasks = true;
    auto tasksDone = [&w = waitingForTasks, &mut = waitMut, &cond = cv](void*) {
        std::scoped_lock l(mut);
        w = false;
        cond.notify_all();
        };
    TaskCompletionCallback cb(Task(tasksDone), (uint32_t)newTaskList.size());
    std::queue<Task> tasks;
    while (newTaskList.size()) 
    {
        newTaskList.front().pTaskCompletionCallback = &cb;
        tasks.emplace(newTaskList.front());
        newTaskList.pop();
    }

    AddTaskList(tasks);
    {
        PROFILE_SCOPED("AddTaskListAndWait");
        // wait for task to complete
        std::unique_lock l(waitMut);
        cv.wait(l, [&wait = waitingForTasks]() { return wait == false; });
    }
}


void TaskManager::TaskExecutor()
{
    auto ThreadID = std::this_thread::get_id();
    auto mapping = VulkanRenderer::get()->RegisterThreadMapping();
    //printf("Thread_%llu initialized to mapping [%u]\n", ThreadID,mapping);
    std::string s("TaskThread_" + std::to_string(mapping));
    OPTICK_THREAD(s.c_str());

    while (true)
    {
        Task taskToExecute;
        {
            // Try to unload a task to execute
            std::unique_lock<std::mutex> lock(m_CriticalSection);
            m_QueueCondition.wait(lock, [this] { return !this->m_TaskQueue.empty() || m_ShuttingDown; });    // Sleep until a task is available to execute or we are shutting down

            if (m_ShuttingDown)
                break;

            // Get the task (if not shutting down
            taskToExecute = m_TaskQueue.front();
            m_TaskQueue.pop();
        }

        while (taskToExecute.pTaskFunction)
        {
            // Execute the task
            taskToExecute.pTaskFunction(taskToExecute.pTaskParam);

            // When we are done, if there was a completion callback, tick it down and execute if needed
            if (taskToExecute.pTaskCompletionCallback)
            {
                // If this was the last task on which we were waiting, execute the completion task now
                if (--taskToExecute.pTaskCompletionCallback->TaskCount == 0)
                {
                    taskToExecute = taskToExecute.pTaskCompletionCallback->CompletionTask;
                    delete taskToExecute.pTaskCompletionCallback;
                    continue;
                }
            }

            // No completion task to run, fetch another task or sleep
            break;
        }
    }
}
