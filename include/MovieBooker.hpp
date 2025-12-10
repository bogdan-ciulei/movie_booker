#pragma once

#include "ImovieBooker.hpp"

#include <unordered_map>
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
class MovieBooker : public IMovieBooker {
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
     * @brief Return free seats for a theater.
     * @param theater Theater name.
     * @return Vector of 1-based free seat indices.
     */
    std::vector<unsigned int> GetFreeSeats(const std::string& theater) override;

    /**
     * @brief Return true if the theater is known.
     * @param theater Theater name.
     * @return true when theater exists in the catalog.
     */
    bool IsTheater(const std::string& theater) override;

    /**
     * @brief Attempt to book seats in a theater. (Legacy API books the first movie entry for the theater.)
     * @param theater Theater name.
     * @param seatIds Vector of 1-based seat indices.
     * @return true on successful booking, false otherwise.
     */
    bool BookSeats(const std::string& theater, const std::vector<unsigned int>& seatIds) override;

private:
    // master lists and indexes
    std::vector<std::string> movies_; // index -> movie name
    std::unordered_map<std::string, std::size_t> movie_index_; // movie name -> index

    std::vector<std::string> theaters_; // index -> theater name
    std::unordered_map<std::string, std::size_t> theater_index_; // theater name -> index

    // per-theater list of movie entries. Each entry represents a movie showing in that theater
    struct TheaterMovieEntry {
        std::size_t movie_id;
        std::mutex mutex; // protects seats for this movie showing
        std::vector<bool> seats; // true = booked
        TheaterMovieEntry(std::size_t mid, std::size_t seatCount)
            : movie_id(mid), seats(seatCount, false) {}
    };

    // theater name -> vector of movie entries (stored as unique_ptr because entries contain non-copyable mutex)
    std::unordered_map<std::string, std::vector<std::unique_ptr<TheaterMovieEntry>>> theater_movies_;

    // mutex protecting maps and vectors structure
    std::mutex map_mutex_;
};