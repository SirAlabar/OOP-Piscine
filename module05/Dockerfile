FROM debian:bookworm-slim

RUN apt update && apt install -y \
    build-essential \
    cmake \
    libsfml-dev \
    libgtest-dev \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -S . -B build
RUN cmake --build build

# Criar usuário normal dentro do container
RUN useradd -ms /bin/bash railwayuser

# Criar output e dar permissão para esse usuário
RUN mkdir -p /app/output && chown -R railwayuser:railwayuser /app

# A partir daqui tudo roda como usuário normal
USER railwayuser

ENTRYPOINT ["./build/Railway_Simulator"]
