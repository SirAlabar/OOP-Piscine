FROM debian:bookworm-slim

RUN apt update && apt install -y \
    build-essential \
    cmake \
    libsfml-dev \
    libgtest-dev \
    valgrind \
    libx11-6 \
    libxrandr2 \
    libxinerama1 \
    libxcursor1 \
    libxi6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -S . -B build
RUN cmake --build build

ENTRYPOINT ["./build/Railway_Simulator"]