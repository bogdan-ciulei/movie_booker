#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MovieBooker.hpp"

#include <thread>
#include <atomic>
#include <algorithm>

using ::testing::UnorderedElementsAre;

// Tests that adding a movie with a list of theaters succeeds and
// that the movie and its theaters can be retrieved afterward.
TEST(MovieBookerTest, AddMovieAndListMoviesAndTheaters) 
{
    MovieBooker mb;
    EXPECT_TRUE(mb.AddMovie("Matrix", {"T1", "T2"}));

    auto movies = mb.GetMovies();
    EXPECT_THAT(movies, UnorderedElementsAre("Matrix"));

    auto theaters = mb.GetTheatersForMovie("Matrix");
    EXPECT_THAT(theaters, UnorderedElementsAre("T1", "T2"));
}

// Tests that requesting theaters for a non-existent movie returns an empty list.
TEST(MovieBookerTest, GetTheatersForUnknownMovieReturnsEmpty) 
{
    MovieBooker mb;
    auto theaters = mb.GetTheatersForMovie("NoSuchMovie");
    EXPECT_TRUE(theaters.empty());
}

// Tests that when a movie is added, its theater starts with 20 free seats
// numbered from 1 to 20.
TEST(MovieBookerTest, FreeSeatsInitiallyAllAvailable) 
{
    MovieBooker mb;
    mb.AddMovie("M", {"Hollywood"});

    auto free = mb.GetFreeSeats("Hollywood", "M");
    EXPECT_EQ(free.size(), 20u);
    EXPECT_EQ(free.front(), 1u);
    EXPECT_EQ(free.back(), 20u);
}

// Tests that booking seats succeeds and that subsequent queries reflect
// the booked seats being removed from the free seats list.
TEST(MovieBookerTest, BookSeatsSucceedsAndSubsequentGetsReflectBooking) 
{
    MovieBooker mb;
    mb.AddMovie("M", {"Hollywood"});

    EXPECT_TRUE(mb.BookSeats("Hollywood", "M", {1, 2}));

    auto free = mb.GetFreeSeats("Hollywood", "M");
    // 1 and 2 should not be in free list
    EXPECT_EQ(std::count(free.begin(), free.end(), 1u), 0);
    EXPECT_EQ(std::count(free.begin(), free.end(), 2u), 0);
    EXPECT_EQ(free.size(), 18u);
}

// Tests that attempting to book a seat that is already booked fails.
TEST(MovieBookerTest, BookingAlreadyBookedSeatFails) 
{
    MovieBooker mb;
    mb.AddMovie("M", {"Central"});

    EXPECT_TRUE(mb.BookSeats("Central", "M", {3}));
    EXPECT_FALSE(mb.BookSeats("Central", "M", {3}));
}

// Tests that invalid seat IDs (0 and >20) are rejected by BookSeats.
TEST(MovieBookerTest, InvalidSeatIdsAreRejected) 
{
    MovieBooker mb;
    mb.AddMovie("M", {"Sala"});

    // 0 is invalid
    EXPECT_FALSE(mb.BookSeats("Sala", "M", {0}));
    // >20 is invalid
    EXPECT_FALSE(mb.BookSeats("Sala", "M", {21}));
}

// Tests that concurrent attempts to book the same seat result in only one
// successful booking across multiple threads.
TEST(MovieBookerTest, ConcurrentBookingOnlyOneSucceeds) 
{
    MovieBooker mb;
    mb.AddMovie("M", {"Sala"});

    const int threads = 8;
    std::atomic<int> successCount{0};
    std::vector<std::thread> ths;
    for (int i = 0; i < threads; ++i) {
        ths.emplace_back( [&mb, &successCount]() 
        {
            if (mb.BookSeats("Sala", "M", {10})) 
                successCount.fetch_add(1, std::memory_order_relaxed);
            
        });
    }
    for (auto &t : ths) t.join();

    EXPECT_EQ(successCount.load(), 1);
}

// Tests that when a single theater runs two different movies, booking seats for
// one movie does not affect the seat availability of the other movie. This test
// explicitly verifies independence by checking both showings in the same MovieBooker instance.
TEST(MovieBookerTest, BookingInOneMovieDoesNotAffectOtherMovieInSameTheater) 
{
    MovieBooker mb;
    mb.AddMovie("MovieA", {"DualTheater"});
    mb.AddMovie("MovieB", {"DualTheater"});

    // Book seat 10 for MovieA showing in DualTheater
    EXPECT_TRUE(mb.BookSeats("DualTheater", "MovieA", {10}));

    // Seat 10 should be booked for MovieA
    auto freeA = mb.GetFreeSeats("DualTheater", "MovieA");
    EXPECT_EQ(std::count(freeA.begin(), freeA.end(), 10u), 0);

    // Seat 10 should still be free for MovieB
    auto freeB = mb.GetFreeSeats("DualTheater", "MovieB");
    EXPECT_EQ(std::count(freeB.begin(), freeB.end(), 10u), 1);
}
