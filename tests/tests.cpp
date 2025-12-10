#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MovieBooker.hpp"

#include <thread>
#include <atomic>
#include <algorithm>

using ::testing::UnorderedElementsAre;

TEST(MovieBookerTest, AddMovieAndListMoviesAndTheaters) {
    MovieBooker mb;
    EXPECT_TRUE(mb.AddMovie("Matrix", {"T1", "T2"}));

    auto movies = mb.GetMovies();
    EXPECT_THAT(movies, UnorderedElementsAre("Matrix"));

    auto theaters = mb.GetTheatersForMovie("Matrix");
    EXPECT_THAT(theaters, UnorderedElementsAre("T1", "T2"));
}

TEST(MovieBookerTest, GetTheatersForUnknownMovieReturnsEmpty) {
    MovieBooker mb;
    auto theaters = mb.GetTheatersForMovie("NoSuchMovie");
    EXPECT_TRUE(theaters.empty());
}

TEST(MovieBookerTest, FreeSeatsInitiallyAllAvailable) {
    MovieBooker mb;
    mb.AddMovie("M", {"Hollywood"});

    auto free = mb.GetFreeSeats("Hollywood");
    EXPECT_EQ(free.size(), 20u);
    EXPECT_EQ(free.front(), 1u);
    EXPECT_EQ(free.back(), 20u);
}

TEST(MovieBookerTest, BookSeatsSucceedsAndSubsequentGetsReflectBooking) {
    MovieBooker mb;
    mb.AddMovie("M", {"Hollywood"});

    EXPECT_TRUE(mb.BookSeats("Hollywood", {1, 2}));

    auto free = mb.GetFreeSeats("Hollywood");
    // 1 and 2 should not be in free list
    EXPECT_EQ(std::count(free.begin(), free.end(), 1u), 0);
    EXPECT_EQ(std::count(free.begin(), free.end(), 2u), 0);
    EXPECT_EQ(free.size(), 18u);
}

TEST(MovieBookerTest, BookingAlreadyBookedSeatFails) {
    MovieBooker mb;
    mb.AddMovie("M", {"Central"});

    EXPECT_TRUE(mb.BookSeats("Central", {3}));
    EXPECT_FALSE(mb.BookSeats("Central", {3}));
}

TEST(MovieBookerTest, InvalidSeatIdsAreRejected) {
    MovieBooker mb;
    mb.AddMovie("M", {"Sala"});

    // 0 is invalid
    EXPECT_FALSE(mb.BookSeats("Sala", {0}));
    // >20 is invalid
    EXPECT_FALSE(mb.BookSeats("Sala", {21}));
}

TEST(MovieBookerTest, ConcurrentBookingOnlyOneSucceeds) {
    MovieBooker mb;
    mb.AddMovie("M", {"Sala"});

    const int threads = 8;
    std::atomic<int> successCount{0};
    std::vector<std::thread> ths;
    for (int i = 0; i < threads; ++i) {
        ths.emplace_back( [&mb, &successCount]() {
            if (mb.BookSeats("Sala", {10})) {
                successCount.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    for (auto &t : ths) t.join();

    EXPECT_EQ(successCount.load(), 1);
}
