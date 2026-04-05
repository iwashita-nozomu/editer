FROM nvidia/cuda:12.2.0-devel-ubuntu22.04
ENV DEBIAN_FRONTEND=noninteractive
ARG NODE_VERSION=22.14.0

# GCCやビルド系 + Python
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates curl git \
    build-essential pkg-config cmake \
    python3 python3-pip python3-venv \
    openssh-client graphviz xz-utils \
 && rm -rf /var/lib/apt/lists/*

# Codex CLI 用の Node.js は apt repo に依存させず、公式 tarball から入れる
RUN curl -fsSL "https://nodejs.org/dist/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-x64.tar.xz" \
    | tar -xJ --strip-components=1 -C /usr/local \
 && node --version \
 && npm --version \
 && npm install -g @openai/codex \
 && codex --version \
 && npm cache clean --force

RUN python3 -m pip install --upgrade pip

# JAXは「コンテナ内同梱CUDA/cuDNN」を使う版（localじゃない）
RUN pip install --no-cache-dir "jax[cuda12]"

# その他依存パッケージ（requirements.txt で一元管理）
COPY docker/requirements.txt /tmp/requirements.txt
RUN pip install --no-cache-dir -r /tmp/requirements.txt

WORKDIR /work
ENV PYTHONPATH=/workspace/python
RUN git config --global --add safe.directory /mnt/git/template.git
RUN git config --global --add safe.directory /mnt/git/jax_util.git

CMD ["/bin/bash"]
