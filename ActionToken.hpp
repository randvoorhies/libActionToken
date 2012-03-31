#ifndef LIBACTIONTOKEN_ACTIONTOKEN_HPP
#define LIBACTIONTOKEN_ACTIONTOKEN_HPP

#include <functional>
#include <thread>
#include <chrono>
#include <iostream>

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

    //! Action tokens cannot be copied
    ActionToken(ActionToken const& other) = delete;

    //! Move constructor - ActionTokens may only be moved
    ActionToken(ActionToken && other);

    //! Has the action completed, or been canceled?
    bool complete();

    //! Wait for the action to complete
    void wait();

    //! Cancel the action
    void cancel();

  private:
    bool itsComplete;
    std::chrono::nanoseconds itsPollRate;
    std::function<bool()> itsIsComplete;
    std::function<void()> itsOnCancel;
};

// ######################################################################
template<class DurationType>
ActionToken::ActionToken(
    std::function<bool()> isComplete,
    std::function<void()> onCancel,
    DurationType pollrate) :
  itsComplete(false),
  itsPollRate(pollrate),
  itsIsComplete(isComplete),
  itsOnCancel(onCancel)
{ }

// ######################################################################
ActionToken::ActionToken(ActionToken && other) :
  itsComplete(other.itsComplete),
  itsPollRate(other.itsPollRate),
  itsIsComplete(other.itsIsComplete),
  itsOnCancel(other.itsOnCancel)
{
  other.itsComplete = true;
  other.itsIsComplete = [](){return true;};
  other.itsOnCancel = [](){};
}

// ######################################################################
ActionToken::~ActionToken()
{
  wait(); 
}

// ######################################################################
void ActionToken::cancel()
{
  if(complete()) return;
  itsComplete = true;
  itsOnCancel();
}

// ######################################################################
bool ActionToken::complete()
{
  if(itsComplete) return true;
  if(itsIsComplete()) itsComplete = true;
  return itsComplete;
}

// ######################################################################
void ActionToken::wait()
{
  while(!complete()) std::this_thread::sleep_for(itsPollRate);
}

#endif // LIBACTIONTOKEN_ACTIONTOKEN_HPP

