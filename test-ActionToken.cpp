#include "ActionToken.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <ncurses.h>

// ######################################################################
class CountingEngine
{
  public:
    CountingEngine();
    ~CountingEngine();
    void setTarget(int target);
    int getCount();
    void start();

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
  initscr();
  nodelay(stdscr, TRUE);

  countingEngine.start();

  ActionToken countingAction = countTo(10);

  while(!countingAction.complete())
  {
    mvprintw(0,0,"Press any key to cancel");
    mvprintw(1,0,"Count: %d", countingEngine.getCount());

    if(getch() != ERR) countingAction.cancel();

    refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
  }

  mvprintw(2,0,"Counting Action Stopped");
  refresh();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  endwin();
  return 0;
}





















//////////////////////// Counting Engine Details ... Ignore This //////////////////////// 
CountingEngine::CountingEngine() :
  itsCount(0),
  itsRunning(false)
{ }

void CountingEngine::start()
{
  itsRunning = true;
  itsRunThread = std::thread(std::bind(&CountingEngine::run, this));
}

CountingEngine::~CountingEngine() {
  itsRunning = false; itsRunThread.join(); 
}

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
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}

