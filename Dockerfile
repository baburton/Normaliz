# Dockerfile for Normaliz

FROM ubuntu:xenial

RUN apt-get update -qq \
    && apt-get -qq install -y \
    build-essential m4 \
    autoconf autogen libtool \
    libboost-dev \
    libgmp-dev \
    git \
    wget \
    unzip \
    sudo \
    python3-pip

RUN adduser --quiet --shell /bin/bash --gecos "norm user,101,," --disabled-password norm \
    && adduser norm sudo \
    && chown -R norm:norm /home/norm \
    && echo 'norm ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER norm
ENV HOME /home/norm
ENV PATH ${HOME}/bin:${PATH}

WORKDIR /home/norm

# ENV NUMBER_CORES=$(sudo cat /proc/cpuinfo | grep processor | wc -l)
# ENV NUMBER_CORES = 4

COPY . /home/norm/Normaliz

#git clone https://github.com/Normaliz/Normaliz.git && \
#     git checkout master &&\

RUN   sudo chown -R norm:norm Normaliz && \
    cd Normaliz && \
    ./install_normaliz_with_opt.sh &&\
    sudo make install  &&\
    sudo ldconfig &&\
    cd PyNormaliz &&\
    sudo python3 setup.py install
    
CMD /bin/bash
