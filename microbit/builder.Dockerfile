FROM debian:stretch

RUN apt update
RUN apt -y upgrade
RUN apt -y install python-setuptools cmake build-essential ninja-build python-dev libffi-dev libssl-dev gcc-arm-none-eabi
RUN easy_install pip
RUN pip install --upgrade pip setuptools
RUN pip install yotta

RUN apt -y install srecord

WORKDIR /yotta_wd
CMD yotta build