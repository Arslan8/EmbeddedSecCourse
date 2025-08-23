# Use Ubuntu as base
FROM ubuntu:24.04

# Install essential tools
RUN apt-get update && \
    apt-get install -y build-essential clang llvm afl++ git sudo curl vim && \
    rm -rf /var/lib/apt/lists/*

# Set a working directory inside the container
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]

