FROM alpine

RUN apk update && apk add \
    alpine-sdk \
    cmake \
    git \
    xauth \
    python3 \
    boost-dev \
    openjdk11 \
    apache-ant \
    ttf-dejavu

RUN git clone https://github.com/jbeder/yaml-cpp.git && mkdir yaml-cpp/build && cmake -S yaml-cpp -B yaml-cpp/build && make -C yaml-cpp/build && make -C yaml-cpp/build install && rm -rf yaml-cpp

WORKDIR "/code"
COPY . /code
RUN python3 /code/ns3 clean && ash /code/scripts/configure-optimized.sh && ant -buildfile /code/expose/gui/viewer/build.xml jar

CMD ["ash","/code/scripts/complete-start-gui.sh"]

