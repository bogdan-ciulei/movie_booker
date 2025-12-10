#pragma once
#include <string>
#include <vector>
#include <optional>

/**
 * @file ImovieBooker.hpp
 * @brief Public interface for a movie booking backend service.
 *
 * Implementations provide in-memory or external storage for movies, theaters
 * and seat booking operations. All methods are expected to be thread-safe when
 * used by concurrent callers (implementation dependent).
 */

/**
 * @brief Abstract interface for movie booking operations.
 *
 * Consumers (CLI, network server, tests) depend on this interface to query
 * available movies and theaters and to book seats.
 */
class IMovieBooker {
public:
    virtual ~IMovieBooker() = default;

    /**
     * @brief Add a movie with the list of theaters that show it.
     * @param movie The movie title (non-empty).
     * @param theatres Vector of theater names showing the movie.
     * @return true on success, false on invalid input or failure.
     */
    virtual bool AddMovie(const std::string& movie, const std::vector<std::string>& theatres) = 0;

    /**
     * @brief Get the list of movies currently known to the system.
     * @return Vector of movie titles.
     */
    virtual std::vector<std::string> GetMovies() = 0;

    /**
     * @brief Get theaters that show a given movie.
     * @param movie The movie title to query.
     * @return Vector of theater names showing the movie; empty if not found.
     */
    virtual std::vector<std::string> GetTheatersForMovie(const std::string& movie) = 0;

    /**
     * @brief Get a list of free seat indices for a theater.
     * @param theater The theater name to query.
     * @return Vector of 1-based seat indices that are free.
     */
    virtual std::vector<unsigned int> GetFreeSeats(const std::string& theater) = 0;

    /**
     * @brief Try to book the specified seats in a theater.
     * @param theater Theater name.
     * @param seatIds Vector of 1-based seat indices to book.
     * @return true if the seats were successfully booked, false otherwise.
     */
    virtual bool BookSeats(const std::string& theater, const std::vector<unsigned int>& seatIds) = 0;

    /**
     * @brief Return true if the theater exists in the system.
     * @param theater Theater name.
     * @return true when the theater is known.
     */
    virtual bool IsTheater(const std::string& theater) = 0;
};