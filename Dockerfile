FROM debian:bookworm-slim AS build
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    cmake \
    ninja-build \
    curl \
    zip \
    unzip \
    pkg-config

WORKDIR /project
COPY . /project
RUN git submodule update --init --recursive
RUN ./external/vcpkg/bootstrap-vcpkg.sh

RUN cmake -E make_directory ./build
WORKDIR /project/build
RUN cmake .. --preset linux-x64-release && \
    cmake --build . 

FROM debian:bookworm-slim AS final
RUN apt-get update && apt-get install -y \
    libstdc++6 
RUN groupadd masterserver && useradd -m -g masterserver masterserver
USER masterserver
WORKDIR /app
COPY --chown=masterserver:masterserver --from=build /project/build/bin/Debug .
EXPOSE 27010/udp
ENTRYPOINT ["./xash-ms"]
CMD ["--ip", "0.0.0.0", "--ip6", "::", "--port", "27010"]
