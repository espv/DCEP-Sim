FROM ubuntu:20.04
LABEL Description="Build environment"

ENV HOME /root
ENV SRC /code
ENV PATH="${PATH}:/usr/bin/cmake/bin/"

WORKDIR "/code"
SHELL ["/bin/bash", "-c"]

RUN apt update && apt install tzdata -y
ENV TZ="Europe/Oslo"

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential \
    clang \
    gdb \
    wget \
    xauth

RUN apt-get update && \
    apt-get install -y software-properties-common && \
    rm -rf /var/lib/apt/lists/*

# Install OpenJDK-7
RUN add-apt-repository ppa:openjdk-r/ppa && \
    apt-get update && \
    apt-get install -y openjdk-11-jdk && \
    apt-get install -y ant && \
    apt-get clean


RUN apt-get update && apt-get install -y --no-install-recommends \
    python3.5 \
    python3-pip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN add-apt-repository ppa:mhier/libboost-latest && \
    apt-get update && \
    apt-get install -y libboost-all-dev

RUN apt-get update && apt-get install -y --no-install-recommends \
    libyaml-cpp-dev

RUN wget https://github.com/Kitware/CMake/releases/download/v3.20.0/cmake-3.20.0-linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/cmake-install.sh

COPY . /code
RUN /code/scripts/configure-optimized.sh

RUN apt-get update && \
    apt-get install -y software-properties-common && \
    rm -rf /var/lib/apt/lists/*
# Install OpenJDK-7
RUN add-apt-repository ppa:openjdk-r/ppa && \
    apt-get update && \
    apt-get install -y openjdk-11-jdk && \
    apt-get install -y ant && \
    apt-get clean
RUN ant -buildfile /code/expose/gui/viewer/build.xml


#CMD ["java","-cp","/code/expose/gui/viewer/dist/gui.jar", "borgui.client.ClientMain"]
CMD ["bash","/code/scripts/complete-start-gui.sh"]

