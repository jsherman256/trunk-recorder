FROM debian:buster

RUN apt-get update && apt-get install -y gnuradio gnuradio-dev libuhd-dev libgnuradio-uhd3.7.13 gr-osmosdr libosmosdr0 git cmake make build-essential libboost-all-dev libusb-1.0-0.dev ffmpeg libaacs0 libcppunit-dev libcppunit-1.14-0 libvo-aacenc0 libssl-dev openssl curl libcurl3-gnutls libcurl4 libcurl4-openssl-dev

WORKDIR /src
COPY . .

RUN cmake . 
RUN make && cp recorder /recorder
CMD ["/recorder", "--config=/app/config.json"]
