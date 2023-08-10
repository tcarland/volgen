FROM ghcr.io/tcarland/tcalibcore:latest

WORKDIR /opt

RUN mkdir -p /opt/volgen
COPY . /opt/volgen

RUN  cd volgen && make