############################
# Stage 1: Builder + Tests
############################
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# ---- system dependencies ----
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# ---- install conan ----
RUN pip3 install --no-cache-dir conan

# ---- conan config ----
RUN conan profile detect --force

WORKDIR /app

# ---- copy project ----
COPY . .

# ---- install dependencies ----
RUN conan install . \
    --output-folder=build \
    --build=missing \
    -s build_type=Release

# ---- build ----
RUN cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build
 
RUN ctest --test-dir build --output-on-failure

############################
# Stage 2: Runtime image
############################
FROM ubuntu:22.04

WORKDIR /app

# ---- Copy application binary ----
COPY --from=builder /app/build/movie_booker /app/movie_booker

# ---- Copy runtime data file ----
COPY --from=builder /app/movies.json /app/movies.json

EXPOSE 8080

# ---- Run application ----
CMD ["./movie_booker"]