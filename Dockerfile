from ubuntu:12.04.5
MAINTAINER Wei Dong
RUN apt-get update && apt-get install -y gcc-arm-linux-gnueabihf make
RUN apt-get install -y g++-4.6-arm-linux-gnueabihf make
RUN ln -s /usr/arm-linux-gnueabihf/lib /lib/arm-linux-gnueabihf && ln -s /usr/arm-linux-gnueabihf/lib /usr/lib/arm-linux-gnueabihf
