FROM ubuntu:24.04

ENV XC32_VERSION=2.40
ENV MPLABX_VERSION=6.20

# Install dependencies
RUN apt-get update && apt-get install -y --no-install-recommends apt-utils

# Microchip tools require i386 compatability libs
RUN dpkg --add-architecture i386 \
    && apt-get update -yq \
    && apt-get install -yq --no-install-recommends curl libc6:i386 \
    libx11-6:i386 libxext6:i386 libstdc++6:i386 libexpat1:i386 \
    libxext6 libxrender1 libxtst6 libgtk2.0-0 libxslt1.1 procps \
    ca-certificates libusb-1.0-0

# Install MPLAB
# Use url: http://www.microchip.com/mplabx-ide-linux-installer to get the latest version
RUN curl -qgb "" -fLC - --retry 3 --retry-delay 3 -e "https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide" -o /tmp/mplabx-installer.tar "https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/MPLABX-v${MPLABX_VERSION}-linux-installer.tar" \
    && tar xf /tmp/mplabx-installer.tar && rm /tmp/mplabx-installer.tar \
    && USER=root ./MPLABX-v${MPLABX_VERSION}-linux-installer.sh --nox11 \
        -- --unattendedmodeui none --mode unattended \
    && rm ./MPLABX-v${MPLABX_VERSION}-linux-installer.sh

# Download and install XC32 compiler
RUN curl -fSL -A "Mozilla/4.0" -o /tmp/xc32.run "http://ww1.microchip.com/downloads/en/DeviceDoc/xc32-v${XC32_VERSION}-full-install-linux-installer.run" \
    && chmod a+x /tmp/xc32.run \
    && /tmp/xc32.run --mode unattended --unattendedmodeui none \
        --netservername localhost --LicenseType FreeMode \
    && rm /tmp/xc32.run
ENV PATH=/opt/microchip/xc32/v${XC32_VERSION}/bin:$PATH

VOLUME /tmp/.X11-unix

# Container Developer User Ident

RUN useradd user \
    && mkdir -p /home/user/MPLABXProjects \
    && touch /home/user/MPLABXProjects/.directory \
    && chown user:user /home/user/MPLABXProjects

VOLUME /home/user/MPLABXProjects