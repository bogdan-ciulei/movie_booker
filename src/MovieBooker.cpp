#include <MovieBooker.hpp>

#include <algorithm>
#include <set>

static constexpr std::size_t kSeatsPerTheater = 20;

bool MovieBooker::AddMovie(const std::string& movie, const std::vector<std::string>& theatres) 
{
    if (movie.empty() || theatres.empty()) return false;

    std::lock_guard<std::mutex> lock(map_mutex_);

    // ensure movie entry map exists (creates if missing)
    auto &theater_map = movie_theaters_[movie];

    for (const auto &theater : theatres) 
    {
        // ensure theater has an id
        auto tit = theater_index_.find(theater);
        if (tit == theater_index_.end()) 
        {
            std::size_t tid = theater_index_.size();
            theater_index_.emplace(theater, tid);
            tit = theater_index_.find(theater);
        }
        std::size_t tid = tit->second;

        // ensure theater entry for this movie exists
        auto it = theater_map.find(theater);
        if (it == theater_map.end()) 
            theater_map.emplace(theater, std::make_unique<TheaterEntry>(tid, kSeatsPerTheater));
    }

    return true;
}

std::vector<std::string> MovieBooker::GetMovies() 
{
    std::lock_guard<std::mutex> lock(map_mutex_);
    std::vector<std::string> result;
    result.reserve(movie_theaters_.size());
    for (const auto &p : movie_theaters_)
        result.push_back(p.first);
   
    return result;
}

std::vector<std::string> MovieBooker::GetTheatersForMovie(const std::string& movie) 
{
    std::lock_guard<std::mutex> lock(map_mutex_);
    auto mit = movie_theaters_.find(movie);
    if (mit == movie_theaters_.end())
        return {};                                      // no such movie

    std::vector<std::string> result;
    result.reserve(mit->second.size());
    for (const auto &p : mit->second) 
        result.push_back(p.first);
    
    return result;
}


std::vector<unsigned int> MovieBooker::GetFreeSeats(const std::string& theater, const std::string& movie) 
{
    if (theater.empty() || movie.empty()) 
        return {};

    std::unique_lock<std::mutex> mapLock(map_mutex_);
    auto mit = movie_theaters_.find(movie);
    if (mit == movie_theaters_.end()) 
        return {};                                  // movie not found

    auto &theater_map = mit->second;
    auto it = theater_map.find(theater);
    if (it == theater_map.end())
        return {};                                  // theater not showing this movie

    auto &entry = *it->second;
    std::unique_lock<std::mutex> entryLock(entry.mutex);
    mapLock.unlock();

    std::vector<unsigned int> freeSeats;
    freeSeats.reserve(entry.seats.size());
    for (std::size_t i = 0; i < entry.seats.size(); ++i) 
        if (!entry.seats[i])
            freeSeats.push_back(static_cast<unsigned int>(i + 1));
    
    return freeSeats;
}

bool MovieBooker::IsTheater(const std::string& theater) 
{
    std::lock_guard<std::mutex> lock(map_mutex_);
    return theater_index_.find(theater) != theater_index_.end();
}

bool MovieBooker::BookSeats(const std::string& theater, const std::string& movie, const std::vector<unsigned int>& seatIds) 
{
    if (theater.empty() || seatIds.empty() || movie.empty()) 
        return false;

    std::set<unsigned int> uniqueIds;
    for (auto id : seatIds) 
    {
        if (id == 0 || id > kSeatsPerTheater)                   // seat number out of range
            return false;
        uniqueIds.insert(id);
    }
    if (uniqueIds.size() != seatIds.size())                     // seats are not unique
        return false;

    std::unique_lock<std::mutex> mapLock(map_mutex_);
    auto mit = movie_theaters_.find(movie);
    if (mit == movie_theaters_.end()) return false;

    auto &theater_map = mit->second;
    auto it = theater_map.find(theater);
    if (it == theater_map.end())                                // theater not found for this movie
        return false;

    auto &entry = *it->second;
    std::unique_lock<std::mutex> entryLock(entry.mutex);
    mapLock.unlock();

    for (auto id : seatIds) 
    {
        std::size_t idx = static_cast<std::size_t>(id - 1);
        if (entry.seats[idx]) 
            return false;                                       // already booked seat in list
    }
    for (auto id : seatIds) 
        entry.seats[static_cast<std::size_t>(id - 1)] = true;
    return true;
}
