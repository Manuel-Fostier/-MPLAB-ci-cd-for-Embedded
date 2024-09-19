FROM ubuntu:24.04.1

ENV XC32VER v2.40
ENV MPLABXVER v6.20

MAINTAINER Vysakh P Pillai 

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends apt-utils

RUN dpkg --add-architecture i386 \
    && apt-get update -yq \
    && apt-get install -yq --no-install-recommends ca-certificates wget unzip libc6:i386 git \
        libx11-6:i386 libxext6:i386 libstdc++6:i386 libexpat1:i386 \
        libxext6 libxrender1 libxtst6 libgtk2.0-0 make \
    && rm -rf /var/lib/apt/lists/*

# Install MPLAB
RUN wget https://ww1.microchip.com/downloads/en/DeviceDoc/MPLABX-${MPLABXVER}-linux-installer.tar -q --show-progress --progress=bar:force:noscroll -O MPLABX-${MPLABXVER}-linux-installer.tar \
    && tar xf MPLABX-${MPLABXVER}-linux-installer.tar && rm -f MPLABX-${MPLABXVER}-linux-installer.tar \
    && USER=root ./*-installer.sh --nox11 \
    -- --unattendedmodeui none --mode unattended \
    && rm -f MPLABX-${MPLABXVER}-linux-installer.sh

# Install XC32
RUN wget https://ww1.microchip.com/downloads/en/DeviceDoc/xc32-${XC32VER}-full-install-linux-installer.run -q --show-progress --progress=bar:force:noscroll -O xc32-${XC32VER}-full-install-linux-installer.run\
    && chmod a+x xc32-${XC32VER}-full-install-linux-installer.run \
    && ./xc32-${XC32VER}-full-install-linux-installer.run \
    --mode unattended --unattendedmodeui none \
    --netservername localhost --LicenseType FreeMode \
    && rm -f xc32-${XC32VER}-full-install-linux-installer.run

ENV PATH $PATH:/opt/microchip/xc32/${XC32VER}/bin
ENV PATH $PATH:/opt/microchip/mplabx/${MPLABXVER}/mplab_platform/bin