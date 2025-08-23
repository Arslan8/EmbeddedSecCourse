docker build -t embeddedsec .
docker run -it --rm \
    -v "$(pwd)":/workspace \
    -w /workspace \
    embeddedsec
