#include "MovieBooker.hpp"

#include <algorithm>
#include <set>

static constexpr std::size_t kSeatsPerTheater = 20;

bool MovieBooker::AddMovie(const std::string& movie, const std::vector<std::string>& theatres) {
    if (movie.empty() || theatres.empty()) return false;

    std::lock_guard<std::mutex> lock(map_mutex_);

    // ensure movie exists
    std::size_t movie_id;
    auto mit = movie_index_.find(movie);
    if (mit == movie_index_.end()) 
    {
        movie_id = movies_.size();
        movies_.push_back(movie);
        movie_index_.emplace(movie, movie_id);
    } else 
    {
        movie_id = mit->second;
    }

    for (const auto &theater : theatres) 
    {
        // ensure theater exists
        if (theater_index_.find(theater) == theater_index_.end()) 
        {
            std::size_t tid = theaters_.size();
            theaters_.push_back(theater);
            theater_index_.emplace(theater, tid);
        }

        auto &entries = theater_movies_[theater];
        bool exists = false;
        for (const auto &e : entries) {
            if (e->movie_id == movie_id) 
            { 
                exists = true; 
                break; 
            }
        }
        if (!exists) {
            entries.emplace_back(std::make_unique<TheaterMovieEntry>(movie_id, kSeatsPerTheater));
        }
    }

    return true;
}

std::vector<std::string> MovieBooker::GetMovies() {
    std::lock_guard<std::mutex> lock(map_mutex_);
    return movies_;
}

std::vector<std::string> MovieBooker::GetTheatersForMovie(const std::string& movie) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto mit = movie_index_.find(movie);
    if (mit == movie_index_.end()) return {};
    std::size_t mid = mit->second;

    std::vector<std::string> result;
    result.reserve(theater_movies_.size());
    for (const auto &p : theater_movies_) {
        const auto &theater = p.first;
        const auto &entries = p.second;
        for (const auto &e : entries) {
            if (e->movie_id == mid) { result.push_back(theater); break; }
        }
    }
    return result;
}

std::vector<unsigned int> MovieBooker::GetFreeSeats(const std::string& theater) {
    std::unique_lock<std::mutex> mapLock(map_mutex_);
    auto it = theater_movies_.find(theater);
    if (it == theater_movies_.end()) return {};
    auto &entries = it->second;
    if (entries.empty()) return {};

    auto &entry = *entries[0];
    std::unique_lock<std::mutex> entryLock(entry.mutex);
    mapLock.unlock();

    std::vector<unsigned int> freeSeats;
    freeSeats.reserve(entry.seats.size());
    for (std::size_t i = 0; i < entry.seats.size(); ++i) {
        if (!entry.seats[i]) freeSeats.push_back(static_cast<unsigned int>(i + 1));
    }
    return freeSeats;
}

bool MovieBooker::IsTheater(const std::string& theater) {
    std::lock_guard<std::mutex> lock(map_mutex_);
    return theater_index_.find(theater) != theater_index_.end();
}

bool MovieBooker::BookSeats(const std::string& theater, const std::vector<unsigned int>& seatIds) {
    if (theater.empty() || seatIds.empty()) return false;

    std::set<unsigned int> uniqueIds;
    for (auto id : seatIds) {
        if (id == 0 || id > kSeatsPerTheater) return false;
        uniqueIds.insert(id);
    }
    if (uniqueIds.size() != seatIds.size()) return false;

    std::unique_lock<std::mutex> mapLock(map_mutex_);
    auto it = theater_movies_.find(theater);
    if (it == theater_movies_.end()) return false;
    auto &entries = it->second;
    if (entries.empty()) return false;

    auto &entry = *entries[0];
    std::unique_lock<std::mutex> entryLock(entry.mutex);
    mapLock.unlock();

    for (auto id : seatIds) {
        std::size_t idx = static_cast<std::size_t>(id - 1);
        if (entry.seats[idx]) return false;
    }
    for (auto id : seatIds) entry.seats[static_cast<std::size_t>(id - 1)] = true;
    return true;
}
