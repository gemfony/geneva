#
# This file is taken from https://github.com/giovtorres/slurm-docker-cluster.
# It might have been adjusted to suit the geneva project
#

FROM geneva-prerequisites

ARG SLURM_TAG=slurm-19-05-1-2
ARG GOSU_VERSION=1.11

RUN set -ex \
    && yum makecache fast \
    && yum -y update \
    && yum -y install epel-release \
    && yum -y install \
    perl \
    git \
    gnupg \
    munge \
    munge-devel \
    python-devel \
    python-pip \
    python3 \
    python3-devel \
    python3-pip \
    mariadb-server \
    mariadb-devel \
    psmisc \
    bash-completion \
    vim-enhanced \
    && yum clean all \
    && rm -rf /var/cache/yum

# RUN ln -s /usr/bin/python3.4 /usr/bin/python3

RUN pip install Cython nose && pip3 install Cython nose

RUN set -ex \
    && wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-amd64" \
    && wget -O /usr/local/bin/gosu.asc "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-amd64.asc" \
    && export GNUPGHOME="$(mktemp -d)" \
    && gpg --batch --keyserver hkps://keys.openpgp.org --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 \
    && gpg --batch --verify /usr/local/bin/gosu.asc /usr/local/bin/gosu \
    && rm -rf "${GNUPGHOME}" /usr/local/bin/gosu.asc \
    && chmod +x /usr/local/bin/gosu \
    && gosu nobody true

RUN set -x \
    && git clone https://github.com/SchedMD/slurm.git \
    && pushd slurm \
    && git checkout tags/$SLURM_TAG \
    && ./configure --enable-debug --prefix=/usr --sysconfdir=/etc/slurm \
    --with-mysql_config=/usr/bin  --libdir=/usr/lib64 \
    && make install \
    && install -D -m644 etc/cgroup.conf.example /etc/slurm/cgroup.conf.example \
    && install -D -m644 etc/slurm.conf.example /etc/slurm/slurm.conf.example \
    && install -D -m644 etc/slurmdbd.conf.example /etc/slurm/slurmdbd.conf.example \
    && install -D -m644 contribs/slurm_completion_help/slurm_completion.sh /etc/profile.d/slurm_completion.sh \
    && popd \
    && rm -rf slurm \
    && groupadd -r --gid=995 slurm \
    && useradd -r -g slurm --uid=995 slurm \
    && mkdir /etc/sysconfig/slurm \
    /var/spool/slurmd \
    /var/run/slurmd \
    /var/run/slurmdbd \
    /var/lib/slurmd \
    /var/log/slurm \
    /data \
    && touch /var/lib/slurmd/node_state \
    /var/lib/slurmd/front_end_state \
    /var/lib/slurmd/job_state \
    /var/lib/slurmd/resv_state \
    /var/lib/slurmd/trigger_state \
    /var/lib/slurmd/assoc_mgr_state \
    /var/lib/slurmd/assoc_usage \
    /var/lib/slurmd/qos_usage \
    /var/lib/slurmd/fed_mgr_state \
    && chown -R slurm:slurm /var/*/slurm* \
    && /sbin/create-munge-key

# NOTE: We expect the build context to be set to geneva/docker/ directory.
#       This makes it possible to include files that are located outside of the
#       Directory where this dockerfile is located.
#       This means the paths for COPY instructions must be relative to geneva/docker/
# 

COPY ./docker/runner/slurm.conf /etc/slurm/slurm.conf
COPY ./docker/runner/slurmdbd.conf /etc/slurm/slurmdbd.conf


WORKDIR /home/app/geneva

COPY ./docker/runner/docker-entrypoint.sh /home/app

# Copy the entire library including source code and prebuilt binaries.
# This is necessary for installation
COPY . .

# install the geneva library
RUN cmake3 \
    --build /home/app/geneva/docker/docker-geneva-build \
    --target install \
    --parallel $(nproc)

# set working directory to the app directory to not confuse people who use this as base image
WORKDIR /home/app/

ENTRYPOINT ["/home/app/docker-entrypoint.sh"]

CMD ["slurmdbd"]
