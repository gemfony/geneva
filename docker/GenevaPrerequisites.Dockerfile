FROM centos:7

LABEL org.opencontainers.image.title="geneva-prerequisites" \
    org.opencontainers.image.description="Prerequisites for building and running the Geneva Optimization Library" \
    maintainer="Jonas Weßner"

RUN set -ex \
    && yum makecache fast \
    && yum -y update \
    && yum -y install epel-release \
    && yum -y install \
    wget \
    tar \
    which \
    python3 \
    gcc \
    gcc-c++\
    make \
    cmake3 \
    ninja-build \
    bzip2 \
    bzip2-devel \
    bzip2-libs \
    python-devel.x86_64 \
    clang \
    doxygen \
    graphviz \
    texlive \
    texlive-utils \
    # openssh is required by the mpi runtime
    openssh-clients \
    openssh-server \
    && yum clean all \
    && rm -rf /var/cache/yum

# install gcc 9 from source (with yum we can only install gcc 4)
RUN mkdir -p gcc-build && \
    cd gcc-build && \
    GCC_VERSION=9.2.0 && \
    wget https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.gz && \
    tar xzvf gcc-${GCC_VERSION}.tar.gz && \
    mkdir obj.gcc-${GCC_VERSION} && \
    cd gcc-${GCC_VERSION} && \
    ./contrib/download_prerequisites && \
    cd ../obj.gcc-${GCC_VERSION} && \
    ../gcc-${GCC_VERSION}/configure --disable-multilib --enable-languages=c,c++ && \
    make -j $(nproc) && \
    make install

# uninstall old versions of gcc. These were used to build the new version from source.
# Now after building and installing the more recent gcc version we remove the old one in order
# to prevent confusion 
RUN yum -y remove \
    gcc \
    gcc-c++

# install boost from source, because yum does not have the required boost 1.70 version
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz && \
    tar -xzf boost_1*
RUN cd boost_1* && ./bootstrap.sh
RUN CPLUS_INCLUDE_PATH="$CPLUS_INCLUDE_PATH:/opt/rh/python33/root/usr/include/python3.3m" && \
    export CPLUS_INCLUDE_PATH && \
    cd boost_1* && \
    ./b2 install --with=all

# install openmpi
RUN wget https://download.open-mpi.org/release/open-mpi/v4.1/openmpi-4.1.3.tar.gz
RUN tar -xzf openmpi-*.tar.gz && \
    cd openmpi-* && \
    ./configure
RUN cd openmpi-* && \
    make -j $(nproc)
RUN cd openmpi-* && \
    make install

# set default library path
ENV LD_LIBRARY_PATH "/usr/local/lib:/usr/local/lib64"