#include "ActionToken.hpp"
#include <iostream>
#include <thread>
#include <mutex>

// ######################################################################
class CountingEngine
{
  public:
    CountingEngine();
    ~CountingEngine();
    void setTarget(int target);
    int getCount();

  private:
    void run();
    int itsCount;
    int itsTarget;
    std::mutex itsMtx;
    bool itsRunning;
    std::thread itsRunThread;
};
CountingEngine countingEngine;

// ######################################################################
ActionToken countTo(int val)
{
  countingEngine.setTarget(val);

  std::function<bool()> isComplete = [val]() { return countingEngine.getCount() == val; };
  std::function<void()> onCancel = []() { countingEngine.setTarget(countingEngine.getCount()); };

  return ActionToken(isComplete, onCancel);
}

// ######################################################################
int main()
{
  ActionToken countingAction = countTo(10);

  while(!countingAction.complete())
  {
    char q;
    std::cin >> q;
    if(q == 'x') countingAction.cancel();
  }

  std::this_thread::sleep_for(std::chrono::seconds(10));
  return 0;
}





















//////////////////////// Counting Engine Details ... Ignore This //////////////////////// 
CountingEngine::CountingEngine() :
  itsCount(0),
  itsRunning(true),
  itsRunThread(std::thread(std::bind(&CountingEngine::run, this)))
{ }

CountingEngine::~CountingEngine() { itsRunning = false; itsRunThread.join(); }

void CountingEngine::setTarget(int target) 
{
  std::lock_guard<std::mutex> _(itsMtx);
  itsTarget = target; 
}

int CountingEngine::getCount() 
{
  std::lock_guard<std::mutex> _(itsMtx);
  return itsCount;
}

void CountingEngine::run()
{
  while(itsRunning)
  {
    {
      std::lock_guard<std::mutex> _(itsMtx);
      if(itsCount < itsTarget)      itsCount++;
      else if(itsCount > itsTarget) itsCount--;
      std::cout << "Count " << itsCount << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

