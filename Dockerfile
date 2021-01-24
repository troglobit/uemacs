FROM gcc:10 as build

WORKDIR /build

# Grab dependencies
COPY UNLICENSE .
COPY README.md .
COPY autogen.sh .
COPY configure.ac .
COPY Makefile.am .
COPY ./src/ ./src

# Configure source
RUN ./autogen.sh
RUN yes '' | ./configure # Say yes to everything!

# Use as many cores as possible. Not that it's needed, but why the hell not?
RUN make -j$(lscpu | grep -E "^CPU\(s\):" | awk '{ print $2 }')
RUN strip /build/src/uemacs

# And finally the runtime image
FROM debian:buster-slim
WORKDIR /usr/local/bin

COPY --from=build /build/src/uemacs /usr/local/bin/uemacs

CMD /usr/local/bin/uemacs
