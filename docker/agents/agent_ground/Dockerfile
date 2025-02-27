# Tag: hsfl2/demo_agent_ground:latest
# Tag: hsfl2/demo_agent_ground:0.1.0

############################################
# BUILD STAGE
FROM ubuntu:20.04 AS builder

# Set TZ to avoid tzdata prompt
ENV TZ=UTC \
    DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && apt-get -y autoremove \
    && apt-get autoclean \
    && rm -rf /var/lib/apt/lists/*

# Set up basic COSMOS directory structure
WORKDIR /cosmos
RUN mkdir -p \
    ./source \
    ./nodes \
    ./bin

# Clone source code
WORKDIR /cosmos/source
ADD https://api.github.com/repos/hsfl/cosmos-core/git/refs/heads/master version.json
RUN git clone --depth 1 https://github.com/hsfl/cosmos-core.git core
WORKDIR /cosmos/source/projects
ADD https://api.github.com/repos/hsfl/cosmos-project-template/git/refs/heads/main version.json
RUN git clone --depth 1 https://github.com/hsfl/cosmos-project-template.git

# Configure project
WORKDIR unibap-demo/build
RUN ./do_cmake_linux
# Build project
WORKDIR linux
RUN make -j8 agent agent_ground
# Manually move the compiled targets to /cosmos/bin
WORKDIR programs/agents
RUN cp agent /cosmos/bin/ && \
    cp ground/agent_ground /cosmos/bin/

############################################
# FINAL IMAGE
FROM ubuntu:20.04

# cosmosuser user and group IDs (26767 = COSMOS)
ARG UID=26767
ARG GID=26767

# Copy the binaries from the builder stage
WORKDIR /home/cosmosuser/cosmos/bin
COPY --from=builder /cosmos/bin/* .

# Copy over a demo realm
WORKDIR /home/cosmosuser/cosmos/realms
COPY --from=hsfl2/realms:demo /demo ./demo

# Set up basic COSMOS directory structure
WORKDIR /home/cosmosuser/cosmos
RUN mkdir -p \
    ./source \
    ./nodes \
    ./bin

# Following best practices for security, create a non-root user and group
RUN groupadd --gid "${GID}" cosmosuser && \
    useradd -s /bin/bash --uid "${UID}" --gid "${GID}"  -m cosmosuser && \
    chown -R cosmosuser:cosmosuser /home/cosmosuser/cosmos

# Switch to the non-root user
USER cosmosuser

# Add cosmos binary directory to path
ENV PATH="$PATH:/home/cosmosuser/cosmos/bin"

COPY ./docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
