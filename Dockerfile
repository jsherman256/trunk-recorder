FROM debian:buster

# Install build and runtime dependencies for RTLSDR only
RUN apt-get update && apt-get install -y gnuradio gnuradio-dev libuhd-dev libgnuradio-uhd3.7.13 gr-osmosdr libosmosdr0 git cmake make build-essential libboost-all-dev libusb-1.0-0.dev ffmpeg libaacs0 libcppunit-dev libcppunit-1.14-0 libvo-aacenc0 libssl-dev openssl curl libcurl3-gnutls libcurl4 libcurl4-openssl-dev 

# If the source has changed, we need to rebuild it
COPY src /src
WORKDIR /src
RUN cmake . 
RUN make

# Only needed if uploading
RUN echo "deb http://deb.debian.org/debian buster contrib non-free" >> /etc/apt/sources.list
RUN echo "deb-src http://deb.debian.org/debian buster contrib non-free" >> /etc/apt/sources.list
RUN apt-get update && apt-get install -y libfdk-aac1 libfdk-aac-dev sox
RUN apt-get source fdkaac && cd fdkaac-0.6.3/ && autoreconf -i && ./configure && make && make install

CMD ["/src/recorder", "--config=/app/config.json"]
