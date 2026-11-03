# Ambiente Linux padronizado para o CACHE DETECTIVE (Aula 12).
# Mesmas ferramentas que os alunos instalam no WSL2/Ubuntu.
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive LANG=C.UTF-8
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
        build-essential binutils valgrind python3 make util-linux procps ca-certificates \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /atividade
CMD ["bash"]
