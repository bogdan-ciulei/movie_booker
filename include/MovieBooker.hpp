#pragma once

#include "ImovieBooker.hpp"

#include <unordered_map>
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <cstddef>
#include <memory>

/**
 * @file MovieBooker.hpp
 * @brief In-memory implementation of `IMovieBooker`.
 *
 * This class provides a thread-safe, in-memory store of movies, theaters and
 * seat availability. Each theater supports a fixed number of seats (20).
 */
class MovieBooker : public IMovieBooker 
{
public:
    MovieBooker() = default;
    ~MovieBooker() override = default;

    /**
     * @brief Add a movie and associated theaters to the catalog.
     * @param movie Title of the movie (must not be empty).
     * @param theatres Vector of theater names showing the movie.
     * @return true on success, false for invalid input.
     */
    bool AddMovie(const std::string& movie, const std::vector<std::string>& theatres) override; 

    /**
     * @brief Return all known movie titles.
     * @return Vector of movie titles.
     */
    std::vector<std::string> GetMovies() override;

    /**
     * @brief Return theaters showing the specified movie.
     * @param movie Movie title to query.
     * @return Vector of theater names, or empty vector if movie not found.
     */
    std::vector<std::string> GetTheatersForMovie(const std::string& movie) override;

    /**
     * @brief Return free seats for a theater showing a specific movie.
     * @param theater Theater name.
     * @param movie Movie title for the showing to query.
     * @return Vector of 1-based free seat indices.
     */
    std::vector<unsigned int> GetFreeSeats(const std::string& theater, const std::string& movie) override;

    /**
     * @brief Return true if the theater is known.
     * @param theater Theater name.
     * @return true when theater exists in the catalog.
     */
    bool IsTheater(const std::string& theater) override;

    /**
     * @brief Attempt to book seats in a theater for a specific movie showing.
     * @param theater Theater name.
     * @param movie Movie title for the showing to book seats in.
     * @param seatIds Vector of 1-based seat indices.
     * @return true on successful booking, false otherwise.
     */
    bool BookSeats(const std::string& theater, const std::string& movie, const std::vector<unsigned int>& seatIds) override;

private:
    
	// map to cover unique theater names
    std::unordered_map<std::string, std::size_t> theater_index_; // theater name -> index

    // per-movie map of theater entries. Each entry represents a theater showing for that movie
    struct TheaterEntry 
    {
        std::size_t theater_id;             // index into theaters_
        std::mutex mutex;                   // protects seats for this movie showing in this theater
        std::vector<bool> seats;            // true = booked
        TheaterEntry(std::size_t tid, std::size_t seatCount)
            : theater_id(tid), seats(seatCount, false) {}
    };

    // movie name -> map of theater name -> TheaterEntry
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<TheaterEntry>>> movie_theaters_;

    // mutex protecting maps and vectors structure
    std::mutex map_mutex_;
};