//
// Created by Darwin Yuan on 2020/6/15.
//

#include <catch.hpp>
#include <trans-dsl/sched/helper/AsyncActionHelper.h>
#include "StupidTransactionContext.h"
#include "SimpleActionsDefs.h"
#include <trans-dsl/sched/helper/SwitchCaseHelper.h>
#include <trans-dsl/sched/helper/OptionalHelper.h>

#define __EVENT(n) Event{eventInfo##n}

namespace {
   using namespace TSL_NS;

   const Msg1 msg1{ 10, 20 };
   const EV_NS::ConsecutiveEventInfo eventInfo1{EV_MSG_1, msg1};

   const Msg2 msg2{ 30 };
   const EV_NS::ConsecutiveEventInfo eventInfo2{EV_MSG_2, msg2};

   const Msg4 msg4{ 30 };
   const EV_NS::ConsecutiveEventInfo eventInfo4{EV_MSG_4, msg4};


   SCENARIO("__switch test", "[switch]") {
      StupidTransactionContext context{};

      __switch(
         __case(__is_status(Result::OUT_OF_SCOPE), __asyn(AsyncAction1)),
         __case(__is_status(Result::INVALID_DATA), __asyn(AsyncAction2)),
         __otherwise(__asyn(AsyncAction4))) action;

      GIVEN("the runtime env has an OUT_OF_SCOPE error") {
         context.getRuntimeContext().reportFailure(Result::OUT_OF_SCOPE);
         REQUIRE(Result::CONTINUE == action.exec(context));

         THEN("should choose path1") {
            REQUIRE(Result::SUCCESS  == action.handleEvent(context, __EVENT(1)));
         }

         WHEN("event2 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(2)));
            }
         }

         WHEN("event 1 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(4)));
            }
         }
      }

      GIVEN("the runtime env has an INVALID_DATA error") {
         context.getRuntimeContext().reportFailure(Result::INVALID_DATA);
         REQUIRE(Result::CONTINUE == action.exec(context));

         THEN("should choose path 2") {
            REQUIRE(Result::SUCCESS == action.handleEvent(context, __EVENT(2)));
         }

         WHEN("event 1 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(1)));
            }
         }

         WHEN("event 4 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(4)));
            }
         }
      }

      GIVEN("the runtime env has an TIMEDOUT error") {
         context.getRuntimeContext().reportFailure(Result::TIMEDOUT);
         REQUIRE(Result::CONTINUE == action.exec(context));

         WHEN("event 4 received") {
            THEN("should return SUCCESS") {
               REQUIRE(Result::SUCCESS == action.handleEvent(context, __EVENT(4)));
            }
         }

         WHEN("event 1 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(1)));
               AND_WHEN("event 4 received") {
                  THEN("should return SUCCESS") {
                     REQUIRE(Result::SUCCESS == action.handleEvent(context, __EVENT(4)));
                  }
               }
               AND_WHEN("event 2 received") {
                  THEN("should return UNKNOWN_EVENT") {
                     REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(2)));
                     AND_WHEN("event 4 received") {
                        THEN("should return SUCCESS") {
                           REQUIRE(Result::SUCCESS == action.handleEvent(context, __EVENT(4)));
                        }
                     }
                  }
               }
            }
         }

         WHEN("event 2 received") {
            THEN("should return UNKNOWN_EVENT") {
               REQUIRE(Result::UNKNOWN_EVENT == action.handleEvent(context, __EVENT(2)));

               AND_WHEN("event 4 received") {
                  THEN("should return SUCCESS") {
                     REQUIRE(Result::SUCCESS == action.handleEvent(context, __EVENT(4)));
                  }
               }
            }
         }
      }
   };
}