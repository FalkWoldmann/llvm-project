//===-- Unittests for clock_nanosleep ---------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "hdr/errno_macros.h"
#include "hdr/time_macros.h"
#include "hdr/types/struct_timespec.h"
#include "src/time/clock_gettime.h"
#include "src/time/clock_nanosleep.h"
#include "test/UnitTest/ErrnoCheckingTest.h"
#include "test/UnitTest/Test.h"

using LlvmLibcClockNanosleep = LIBC_NAMESPACE::testing::ErrnoCheckingTest;

TEST_F(LlvmLibcClockNanosleep, RelativeMonotonic) {
  timespec req = {0, 1000};
  timespec rem = {0, 0};
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &rem), 0);
}

TEST_F(LlvmLibcClockNanosleep, AbsoluteInThePast) {
  timespec deadline;
  ASSERT_EQ(LIBC_NAMESPACE::clock_gettime(CLOCK_MONOTONIC, &deadline), 0);
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                            &deadline, nullptr),
            0);
}

TEST_F(LlvmLibcClockNanosleep, AbsoluteInTheFuture) {
  timespec deadline;
  ASSERT_EQ(LIBC_NAMESPACE::clock_gettime(CLOCK_MONOTONIC, &deadline), 0);
  deadline.tv_nsec += 5000000; // 5 ms
  if (deadline.tv_nsec >= 1000000000) {
    deadline.tv_nsec -= 1000000000;
    deadline.tv_sec += 1;
  }
  // rem is ignored for absolute sleeps and must be left untouched.
  timespec rem = {-1, -1};
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                                            &deadline, &rem),
            0);
  EXPECT_EQ(rem.tv_sec, static_cast<decltype(rem.tv_sec)>(-1));
  EXPECT_EQ(rem.tv_nsec, static_cast<decltype(rem.tv_nsec)>(-1));
  timespec now;
  ASSERT_EQ(LIBC_NAMESPACE::clock_gettime(CLOCK_MONOTONIC, &now), 0);
  EXPECT_TRUE(
      now.tv_sec > deadline.tv_sec ||
      (now.tv_sec == deadline.tv_sec && now.tv_nsec >= deadline.tv_nsec));
}

TEST_F(LlvmLibcClockNanosleep, ThreadCpuTime) {
  timespec req = {0, 1000};
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(CLOCK_THREAD_CPUTIME_ID, 0, &req,
                                            nullptr),
            EINVAL);
}

TEST_F(LlvmLibcClockNanosleep, InvalidClock) {
  timespec req = {0, 1000};
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(-1, 0, &req, nullptr), EINVAL);
}

TEST_F(LlvmLibcClockNanosleep, InvalidNanoseconds) {
  timespec req = {0, 1000000000};
  EXPECT_EQ(LIBC_NAMESPACE::clock_nanosleep(CLOCK_MONOTONIC, 0, &req, nullptr),
            EINVAL);
}
