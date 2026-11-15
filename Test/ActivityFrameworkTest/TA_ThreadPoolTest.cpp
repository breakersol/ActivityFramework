/*
 * Copyright [2025] [Shuang Zhu / Sol]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "TA_ThreadPoolTest.h"
#include "Components/TA_Activity.h"

#include <atomic>
#include <barrier>
#include <vector>
#include <chrono>
#include <exception>
#include <new>
#include <semaphore>
#include <system_error>
#include <thread>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

TA_ThreadPoolTest::TA_ThreadPoolTest() {
    activities.fill(nullptr);
}


TA_ThreadPoolTest::~TA_ThreadPoolTest() {}

void TA_ThreadPoolTest::SetUp() {
    for (int i = 0; i < activities.size(); ++i) {
        auto activity = CoreAsync::TA_ActivityCreator::create(
            [](int a) {
                //        std::printf("%d\n", a);
                //        std::printf("%d has completed an activity!\n", std::this_thread::get_id());
                return a;
            },
            std::move(i));
        activities[i] = new CoreAsync::TA_ActivityProxy(activity);
    }
}

void TA_ThreadPoolTest::TearDown() {
    for (auto &activity : activities) {
        if (activity) {
            delete activity;
            activity = nullptr;
        }
    }
}

TEST_F(TA_ThreadPoolTest, postActivityTest) {
    auto ft = CoreAsync::TA_ThreadHolder::get().postActivity(activities[0]);
    EXPECT_EQ(0, ft().get<int>());
}

TEST_F(TA_ThreadPoolTest, notifyResultTest) {
    std::vector<CoreAsync::TA_ActivityResultFetcher> testVec;
    std::vector<int> validVec(1024);
    for (int i = 0; i < activities.size(); ++i) {
        testVec.emplace_back(CoreAsync::TA_ThreadHolder::get().postActivity(activities[i]));
        validVec[i] = i;
    }
    EXPECT_EQ(testVec.size(), validVec.size());
    for (int i = 0; i < testVec.size(); ++i) {
        EXPECT_EQ(testVec[i]().get<int>(), validVec[i]);
    }
}

TEST_F(TA_ThreadPoolTest, threadSizeTest) {
    EXPECT_EQ(std::thread::hardware_concurrency(), CoreAsync::TA_ThreadHolder::get().size());
}

TEST_F(TA_ThreadPoolTest, handlesIdentifyWorkersAndExcludeDependencyThread) {
    CoreAsync::TA_ThreadPool pool{2};
    const auto first = pool.threadHandle(0);
    EXPECT_TRUE(pool.contains(first));
    EXPECT_EQ(first.threadId(), pool.threadId(0));
    EXPECT_EQ(first.index(), 0u);
    EXPECT_EQ(pool.topPriorityThread(first.threadId()), pool.threadHandle(1));
    EXPECT_FALSE(pool.contains({}));
    EXPECT_FALSE(pool.contains({first.threadId(), 1}));
    EXPECT_THROW(pool.threadHandle(2), std::invalid_argument);
}

TEST_F(TA_ThreadPoolTest, proxyPreservesAffinityAndRejectsForeignPool) {
    CoreAsync::TA_ThreadPool pool{2};
    CoreAsync::TA_ThreadPool otherPool{1};
    auto activity = CoreAsync::TA_ActivityCreator::create([]() { return std::this_thread::get_id(); });
    auto proxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity, true);
    EXPECT_EQ(proxy->affinityThread(), CoreAsync::TA_ThreadPool::LocalThreadHandle{});
    EXPECT_FALSE(proxy->moveToThread({}));
    ASSERT_TRUE(proxy->moveToThread(otherPool.threadHandle(0)));
    EXPECT_THROW(static_cast<void>(pool.postActivity(proxy)), std::invalid_argument);
    EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Configuring);
    const auto target = pool.threadHandle(1);
    ASSERT_TRUE(proxy->moveToThread(target));
    ASSERT_TRUE(activity->setStolenEnabled(false));
    EXPECT_EQ(proxy->affinityThread(), target);
    auto result = pool.postActivity(proxy);
    EXPECT_EQ(result().get<std::thread::id>(), target.threadId());
    EXPECT_FALSE(proxy->moveToThread(pool.threadHandle(0)));
}

TEST_F(TA_ThreadPoolTest, nonStealableActivityRemainsOnItsAffinityThread) {
    std::binary_semaphore ownerStarted{0};
    std::binary_semaphore releaseOwner{0};
    std::binary_semaphore stealableActivityRan{0};
    std::atomic_bool stealableRanOnAffinityThread{true};
    std::atomic_bool protectedActivityRan{false};
    std::atomic_bool ranOnWrongThread{false};
    CoreAsync::TA_ThreadPool pool{2};

    auto blocker = CoreAsync::TA_ActivityCreator::create([&]() {
        ownerStarted.release();
        releaseOwner.acquire();
    });
    blocker->moveToThread(pool.threadHandle(1));
    blocker->setStolenEnabled(false);
    auto blockerResult = pool.postActivity(blocker, true);
    if (!ownerStarted.try_acquire_for(std::chrono::seconds{1})) {
        releaseOwner.release();
        FAIL() << "Affinity worker did not start the blocking activity";
    }

    const std::thread::id affinityThread = pool.threadId(1);
    auto stealableActivity = CoreAsync::TA_ActivityCreator::create([&]() {
        stealableRanOnAffinityThread.store(std::this_thread::get_id() == affinityThread,
                                           std::memory_order_relaxed);
        stealableActivityRan.release();
    });
    stealableActivity->moveToThread(pool.threadHandle(1));
    stealableActivity->setStolenEnabled(true);
    auto stealableResult = pool.postActivity(stealableActivity, true);

    auto protectedActivity = CoreAsync::TA_ActivityCreator::create([&]() {
        ranOnWrongThread.store(std::this_thread::get_id() != affinityThread, std::memory_order_relaxed);
        protectedActivityRan.store(true, std::memory_order_release);
    });
    protectedActivity->moveToThread(pool.threadHandle(1));
    protectedActivity->setStolenEnabled(false);
    auto protectedResult = pool.postActivity(protectedActivity, true);

    auto thiefTrigger = CoreAsync::TA_ActivityCreator::create([]() { return true; });
    thiefTrigger->moveToThread(pool.threadHandle(0));
    auto triggerResult = pool.postActivity(thiefTrigger, true);
    EXPECT_TRUE(triggerResult().get<bool>());

    if (!stealableActivityRan.try_acquire_for(std::chrono::seconds{1})) {
        releaseOwner.release();
        FAIL() << "Idle worker did not steal the eligible activity";
    }

    stealableResult();
    EXPECT_FALSE(stealableRanOnAffinityThread.load(std::memory_order_relaxed));
    EXPECT_FALSE(protectedActivityRan.load(std::memory_order_acquire));

    releaseOwner.release();
    blockerResult();
    protectedResult();

    EXPECT_TRUE(protectedActivityRan.load(std::memory_order_acquire));
    EXPECT_FALSE(ranOnWrongThread.load(std::memory_order_relaxed));
}

TEST_F(TA_ThreadPoolTest, sameActivityCannotBeSubmittedThroughDifferentProxies) {
    CoreAsync::TA_ThreadPool pool{1};
    std::binary_semaphore blockerStarted{0};
    std::binary_semaphore releaseBlocker{0};

    auto blocker = CoreAsync::TA_ActivityCreator::create([&]() {
        blockerStarted.release();
        releaseBlocker.acquire();
    });
    auto blockerResult = pool.postActivity(blocker, true);
    if (!blockerStarted.try_acquire_for(std::chrono::seconds{1})) {
        releaseBlocker.release();
        blockerResult();
        FAIL() << "Worker did not start the blocking activity";
        return;
    }

    auto activity = CoreAsync::TA_ActivityCreator::create([]() { return 42; });
    {
        auto firstProxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity, false);
        auto secondProxy = std::make_shared<CoreAsync::TA_ActivityProxy>(activity, false);
        auto result = pool.postActivity(firstProxy);

        EXPECT_THROW(static_cast<void>(pool.postActivity(secondProxy)), std::runtime_error);
        EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Queued);

        releaseBlocker.release();
        blockerResult();
        EXPECT_EQ(result().get<int>(), 42);
        EXPECT_EQ(activity->state(), CoreAsync::TA_ActivityState::Completed);
    }
    delete activity;
}

TEST_F(TA_ThreadPoolTest, concurrentGrowthPreservesHandlesAndSubmissions) {
    CoreAsync::TA_ThreadPool pool{2};
    const auto original = pool.threadHandle(0);
    std::barrier start{5};
    std::atomic_int completed{0};
    std::atomic_bool failed{false};
    std::vector<std::jthread> callers;
    for (int caller = 0; caller < 4; ++caller) {
        callers.emplace_back([&, caller]() {
            start.arrive_and_wait();
            try {
                if (caller < 2) {
                    for (int i = 0; i < 8; ++i) {
                        const auto added = pool.addThread();
                        if (!pool.contains(added) || pool.threadHandle(added.index()) != added) {
                            failed.store(true);
                        }
                    }
                } else {
                    for (int i = 0; i < 200; ++i) {
                        auto activity = CoreAsync::TA_ActivityCreator::create([&]() {
                            completed.fetch_add(1);
                            return 42;
                        });
                        // Exercise both affinity lookup and automatic selection.
                        if (i % 2 == 0) {
                            activity->moveToThread(original);
                        }
                        auto result = pool.postActivity(activity, true);
                        if (result().get<int>() != 42 || !pool.contains(original)) {
                            failed.store(true);
                        }
                        static_cast<void>(pool.size());
                        static_cast<void>(pool.topPriorityThread());
                    }
                }
            } catch (...) {
                failed.store(true);
            }
        });
    }
    start.arrive_and_wait();
    callers.clear();
    EXPECT_FALSE(failed.load());
    EXPECT_EQ(completed.load(), 400);
    ASSERT_EQ(pool.size(), 18u);
    EXPECT_EQ(pool.threadHandle(0), original);
    for (std::size_t i = 0; i < pool.size(); ++i) {
        const auto target = pool.threadHandle(i);
        auto activity = CoreAsync::TA_ActivityCreator::create([]() { return std::this_thread::get_id(); });
        activity->moveToThread(target);
        activity->setStolenEnabled(false);
        auto result = pool.postActivity(activity, true);
        EXPECT_EQ(result().get<std::thread::id>(), target.threadId());
    }
}

TEST_F(TA_ThreadPoolTest, activityCanAddWorker) {
    CoreAsync::TA_ThreadPool pool{1};
    auto activity = CoreAsync::TA_ActivityCreator::create([&]() { return pool.addThread(); });
    auto result = pool.postActivity(activity, true);
    const auto added = result().get<CoreAsync::TA_ThreadPool::LocalThreadHandle>();
    EXPECT_EQ(added.index(), 1u);
    EXPECT_TRUE(pool.contains(added));
    EXPECT_EQ(pool.size(), 2u);
}

TEST_F(TA_ThreadPoolTest, concurrentAdditionAndShutdownCoordinatePublication) {
    CoreAsync::TA_ThreadPool pool{2};
    std::barrier start{2};
    std::jthread grower([&]() {
        start.arrive_and_wait();
        for (int i = 0; i < 16; ++i) {
            try {
                pool.addThread();
            } catch (const std::runtime_error &) {
                return; // Shutdown won publication.
            }
        }
    });
    start.arrive_and_wait();
    pool.shutDown();
    grower.join();
    EXPECT_EQ(pool.size(), 0u);
    EXPECT_THROW(pool.addThread(), std::runtime_error);
    EXPECT_NO_THROW(pool.shutDown());
}

TEST_F(TA_ThreadPoolTest, concurrentShutdownCallersWaitForWorkers) {
    CoreAsync::TA_ThreadPool pool{1};
    std::binary_semaphore started{0};
    std::binary_semaphore release{0};
    auto activity = CoreAsync::TA_ActivityCreator::create([&]() {
        started.release();
        release.acquire();
        return 42;
    });
    auto result = pool.postActivity(activity, true);
    if (!started.try_acquire_for(std::chrono::seconds{1})) {
        release.release();
        FAIL() << "Worker did not start";
    }
    std::barrier start{3};
    std::atomic_int shutdowns{0};
    auto shutdown = [&]() {
        start.arrive_and_wait();
        pool.shutDown();
        shutdowns.fetch_add(1);
    };
    std::jthread first(shutdown);
    std::jthread second(shutdown);
    start.arrive_and_wait();
    release.release();
    first.join();
    second.join();
    EXPECT_EQ(shutdowns.load(), 2);
    EXPECT_EQ(result().get<int>(), 42);
    EXPECT_EQ(pool.size(), 0u);
}

TEST_F(TA_ThreadPoolTest, detachedWorkerFinishesBeforeShutdownReturns) {
    CoreAsync::TA_ThreadPool pool{1};
    const auto handle = pool.threadHandle(0);
    auto activity = CoreAsync::TA_ActivityCreator::create([]() { return 42; });
    auto result = pool.postActivity(activity, true);
    pool.setThreadDetached(handle);
    EXPECT_FALSE(pool.contains(handle));
    EXPECT_EQ(pool.threadId(0), std::thread::id{});
    EXPECT_THROW(pool.setThreadDetached(handle), std::invalid_argument);
    pool.shutDown();
    EXPECT_EQ(result().get<int>(), 42);
}

TEST_F(TA_ThreadPoolTest, shutdownCompletesEveryAcceptedSubmission) {
    CoreAsync::TA_ThreadPool pool{0};
    const auto worker = pool.addThread();
    EXPECT_EQ(worker.index(), 0u);
    std::vector<CoreAsync::TA_ActivityResultFetcher> results;
    for (int i = 0; i < 100; ++i) {
        auto activity = CoreAsync::TA_ActivityCreator::create([]() { return 42; });
        results.push_back(pool.postActivity(activity, true));
    }
    pool.shutDown();
    for (auto &result : results) {
        EXPECT_EQ(result().get<int>(), 42);
    }
    EXPECT_FALSE(pool.contains(worker));
    EXPECT_THROW(pool.threadId(0), std::invalid_argument);
}

#if defined(_MSC_VER) && defined(_DEBUG)
namespace {
// The shared debug CRT invokes this on the allocating thread, including DLL
// allocations. Fail only the test caller, never a running worker's allocation.
thread_local int allocationsBeforeFailure = -1;
thread_local std::binary_semaphore *pauseAllocation = nullptr;
thread_local std::binary_semaphore *resumeAllocation = nullptr;
int failThreadPoolAllocation(int kind, void *, size_t, int, long, const unsigned char *, int) {
    if (kind == _HOOK_ALLOC && pauseAllocation) {
        auto *pause = pauseAllocation;
        pauseAllocation = nullptr;
        pause->release();
        resumeAllocation->acquire();
    }
    if ((kind == _HOOK_ALLOC || kind == _HOOK_REALLOC) && allocationsBeforeFailure >= 0) {
        if (allocationsBeforeFailure-- == 0) {
            return false;
        }
    }
    return true;
}
}

TEST_F(TA_ThreadPoolTest, allocationFailuresRollBackBeforeRetry) {
    int failures = 0;
    int threadCreationFailures = 0;
    bool reachedSuccess = false;
    for (int allocation = 0; allocation < 64; ++allocation) {
        CoreAsync::TA_ThreadPool pool{2};
        const auto original = pool.threadHandle(0);
        const auto previousHook = _CrtSetAllocHook(failThreadPoolAllocation);
        allocationsBeforeFailure = allocation;
        bool threw = false;
        std::exception_ptr unexpected;
        try {
            pool.addThread();
        } catch (const std::bad_alloc &) {
            threw = true;
        } catch (const std::system_error &) {
            // The CRT thread-start allocation reports resource exhaustion
            // through std::thread rather than throwing std::bad_alloc.
            threw = true;
            ++threadCreationFailures;
        } catch (...) {
            unexpected = std::current_exception();
        }
        allocationsBeforeFailure = -1;
        _CrtSetAllocHook(previousHook);
        ASSERT_FALSE(unexpected);
        if (!threw) {
            reachedSuccess = true;
            EXPECT_EQ(pool.size(), 3u);
            break;
        }
        ++failures;
        ASSERT_EQ(pool.size(), 2u);
        ASSERT_EQ(pool.threadHandle(0), original);
        const auto added = pool.addThread();
        ASSERT_EQ(added.index(), 2u);
        ASSERT_TRUE(pool.contains(added));
        auto activity = CoreAsync::TA_ActivityCreator::create([]() { return std::this_thread::get_id(); });
        activity->moveToThread(added);
        activity->setStolenEnabled(false);
        auto result = pool.postActivity(activity, true);
        EXPECT_EQ(result().get<std::thread::id>(), added.threadId());
        pool.shutDown();
        EXPECT_EQ(pool.size(), 0u);
    }
    EXPECT_GT(failures, 0);
    EXPECT_TRUE(reachedSuccess);
    RecordProperty("injected_failures", failures);
    RecordProperty("thread_creation_failures", threadCreationFailures);
}
TEST_F(TA_ThreadPoolTest, pausedAdditionDoesNotBlockScheduling) {
    CoreAsync::TA_ThreadPool pool{1};
    const auto original = pool.threadHandle(0);
    std::binary_semaphore paused{0};
    std::binary_semaphore resume{0};
    std::binary_semaphore completed{0};
    std::atomic_bool failed{false};
    const auto previousHook = _CrtSetAllocHook(failThreadPoolAllocation);
    std::jthread grower([&]() {
        pauseAllocation = &paused;
        resumeAllocation = &resume;
        try {
            pool.addThread();
        } catch (...) {
            failed.store(true);
        }
        pauseAllocation = nullptr;
        resumeAllocation = nullptr;
    });
    const bool didPause = paused.try_acquire_for(std::chrono::seconds{1});
    std::jthread submitter([&]() {
        try {
            if (pool.size() != 1 || !pool.contains(original)
                || pool.topPriorityThread() != original) {
                failed.store(true);
            }
            auto activity = CoreAsync::TA_ActivityCreator::create([]() { return 42; });
            auto result = pool.postActivity(activity, true);
            if (result().get<int>() != 42) {
                failed.store(true);
            }
        } catch (...) {
            failed.store(true);
        }
        completed.release();
    });
    const bool madeProgress = completed.try_acquire_for(std::chrono::seconds{1});
    resume.release();
    grower.join();
    submitter.join();
    _CrtSetAllocHook(previousHook);
    EXPECT_TRUE(didPause);
    EXPECT_TRUE(madeProgress);
    EXPECT_FALSE(failed.load());
    EXPECT_EQ(pool.size(), 2u);
}

#endif
