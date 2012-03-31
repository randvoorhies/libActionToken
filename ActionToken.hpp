#include <functional>
#include <thread>
#include <chrono>

class ActionToken
{
  public:
    //! Construct a new action token
    /*! \param isComplete A function that returns true only when the action has completed
        \param onCancel An function that will be called when the action has been canceled 
        \param pollrate The rate at which the isComplete function will be polled when wait()ing for completion. */
    template<class DurationType = std::chrono::milliseconds>
    ActionToken(
        std::function<bool()> isComplete,
        std::function<void()> onCancel = [](){},
        DurationType pollrate = std::chrono::milliseconds(10));

    //! Destructor
    /*! On destruction, the ActionToken will always call wait() */
    ~ActionToken();

    //! Has the action completed, or been canceled?
    bool complete();

    //! Wait for the action to complete
    void wait();

    //! Cancel the action
    void cancel();

  private:
    bool itsDone;
    std::chrono::nanoseconds itsPollRate;
    std::function<bool()> itsIsComplete;
    std::function<void()> itsOnCancel;
};



template<class DurationType>
ActionToken::ActionToken(
    std::function<bool()> isComplete,
    std::function<void()> onCancel,
    DurationType pollrate) :
  itsDone(false),
  itsPollRate(pollrate),
  itsIsComplete(isComplete),
  itsOnCancel(onCancel)
{ }

ActionToken::~ActionToken()
{ wait(); }

void ActionToken::cancel()
{
  if(complete()) return;
  itsDone = true;
  itsOnCancel();
}

bool ActionToken::complete()
{
  if(itsDone) return true;
  bool complete = itsIsComplete();
  if(complete) itsDone = true;
  return itsIsComplete();
}

void ActionToken::wait()
{
  while(!complete()) std::this_thread::sleep_for(itsPollRate);
}


