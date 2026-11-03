# =====================================================================
#  CACHE DETECTIVE: Aula 12 (Infraestrutura de Hardware)
#  Uso no Linux / WSL2 (Ubuntu). Rode "make ajuda" para ver os alvos.
# =====================================================================

CC      ?= gcc
# -O1: mesmo nivel de otimizacao usado na aula (slide 11).
# -g : guarda o nome das funcoes e linhas (usado pelo Cachegrind).
CFLAGS  = -O1 -g -Wall -std=c11
LDLIBS  = -lm

# Versao para o gprof:
#  -pg          instrumenta o programa para gerar o arquivo gmon.out
#  -fno-inline  impede o compilador de "colar" funcoes pequenas dentro de outras
#               (senao elas somem do relatorio do gprof)
#  -no-pie      evita relatorios vazios do gprof em distribuicoes recentes
PGFLAGS = -O1 -g -pg -fno-inline -no-pie -std=c11

# Cache simulada pelo Cachegrind: tamanho_bytes,associatividade,linha_bytes
# I1 = cache L1 de instrucoes (fixa; nao e o foco da atividade)
# D1 = cache L1 de dados | LL = ultimo nivel de cache.
# Valores fixos para que todos obtenham os MESMOS numeros (e uma simulacao).
I1 ?= 32768,8,64
D1 ?= 32768,8,64
LL ?= 1048576,16,64

MAX_MB ?= 128
# Repeticoes no gprof: ele amostra a cada 10 ms, entao precisa de alguns segundos.
REP_GPROF ?= 50
RES = resultados

.PHONY: ajuda verificar info medir degraus padroes gprof cachegrind perf tudo limpar

ajuda:
	@echo ""
	@echo "  make verificar   -> confere se gcc, gprof e valgrind estao instalados"
	@echo "  make info        -> mostra processador e caches (lscpu)"
	@echo "  make medir       -> ETAPA 1: medicao de tempo (degraus + padroes)"
	@echo "  make gprof       -> ETAPA 2: profiling (onde o tempo e gasto)"
	@echo "  make cachegrind  -> ETAPA 3: simulacao de cache (por que)"
	@echo "  make perf        -> opcional, so em Linux nativo: contadores de hardware"
	@echo "  make limpar      -> apaga executaveis e resultados"
	@echo ""
	@echo "  make tudo        -> roda todas as etapas em sequencia (demonstracao)"
	@echo ""
	@echo "  Variacoes: make degraus MAX_MB=32   |   make cachegrind D1=65536,8,64"
	@echo ""

cache_detective: cache_detective.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

cache_detective_pg: cache_detective.c
	$(CC) $(PGFLAGS) -o $@ $< $(LDLIBS)

verificar:
	@echo "Verificando ferramentas..."
	@command -v gcc      >/dev/null && echo "  [ok] gcc      $$(gcc -dumpfullversion)"           || echo "  [FALTA] gcc      -> sudo apt install build-essential"
	@command -v make     >/dev/null && echo "  [ok] make"                                          || echo "  [FALTA] make     -> sudo apt install build-essential"
	@command -v gprof    >/dev/null && echo "  [ok] gprof"                                         || echo "  [FALTA] gprof    -> sudo apt install binutils"
	@command -v valgrind >/dev/null && echo "  [ok] valgrind $$(valgrind --version)"               || echo "  [FALTA] valgrind -> sudo apt install valgrind"
	@command -v python3  >/dev/null && echo "  [ok] python3  (usado pelo cg_annotate)"         || echo "  [FALTA] python3  -> sudo apt install python3"
	@command -v lscpu    >/dev/null && echo "  [ok] lscpu"                                         || echo "  [aviso] lscpu nao encontrado (opcional)"
	@if [ "$$CODESPACES" = "true" ]; then echo "  Ambiente: GitHub Codespaces, maquina virtual na nuvem (declare isso no relatorio)"; \
	 elif grep -qi microsoft /proc/version 2>/dev/null; then echo "  Ambiente: WSL2 (declare isso no relatorio)"; \
	 elif [ -f /.dockerenv ]; then echo "  Ambiente: container Docker (declare isso no relatorio)"; \
	 else echo "  Ambiente: Linux"; fi

info:
	@echo "--- Processador e caches ---"
	@lscpu 2>/dev/null | grep -iE "model name|^cpu\(s\)|cache|architecture" || true
	@echo "--- Linha de cache (bytes) ---"
	@getconf LEVEL1_DCACHE_LINESIZE 2>/dev/null || true
	@echo "--- Sistema / compilador ---"
	@uname -sr
	@gcc --version | head -1

$(RES):
	@mkdir -p $(RES)

# ---------------- ETAPA 1: QUANTO custa? (medicao) ----------------
medir: degraus padroes

degraus: cache_detective | $(RES)
	./cache_detective degraus $(MAX_MB) | tee $(RES)/degraus.txt

padroes: cache_detective | $(RES)
	./cache_detective padroes | tee $(RES)/padroes.txt

# ---------------- ETAPA 2: ONDE o tempo e gasto? (gprof) ----------------
gprof: cache_detective_pg | $(RES)
	./cache_detective_pg gprof $(REP_GPROF) > /dev/null
	gprof -b -p ./cache_detective_pg gmon.out > $(RES)/gprof.txt
	@rm -f gmon.out
	@echo ""
	@echo "=== Perfil plano do gprof (resultados/gprof.txt) ==="
	@cat $(RES)/gprof.txt

# ---------------- ETAPA 3: POR QUE? (Cachegrind) ----------------
# Os arquivos levam a configuracao da D1 no nome, para nao sobrescrever rodadas:
#   D1=32768,8,64  ->  resultados/cachegrind_D1-32768-8-64_funcoes.txt
virgula := ,
CG = $(RES)/cachegrind_D1-$(subst $(virgula),-,$(D1))

cachegrind: cache_detective | $(RES)
	valgrind --tool=cachegrind --cache-sim=yes --I1=$(I1) --D1=$(D1) --LL=$(LL) \
	    --cachegrind-out-file=$(CG).out ./cache_detective perfil 2>&1 \
	    | tee $(CG)_resumo.txt
	cg_annotate --show=Dr,D1mr,DLmr --sort=D1mr --auto=no $(CG).out > $(CG)_funcoes.txt
	cg_annotate --show=Dr,D1mr,DLmr --auto=yes --context=1 $(CG).out > $(CG)_linhas.txt
	@echo ""
	@echo "=== Cache simulada: D1=$(D1)  LL=$(LL)  (tamanho,vias,linha) ==="
	@echo "=== Leituras (Dr) e misses de leitura (D1mr, DLmr) por funcao ==="
	@if grep -q "Function:file summary" $(CG)_funcoes.txt; then \
	    sed -n '/Function:file summary/,$$p' $(CG)_funcoes.txt | grep -v "^--" | grep -v "^$$"; \
	 else cat $(CG)_funcoes.txt; fi
	@echo ""
	@echo "Arquivos: $(CG)_funcoes.txt  e  $(CG)_linhas.txt (detalhe por linha do codigo)"

# ---------------- OPCIONAL: contadores de hardware (Linux nativo) ----------------
perf: cache_detective | $(RES)
	perf stat -e cache-references,cache-misses,L1-dcache-load-misses \
	    ./cache_detective padroes 2>&1 | tee $(RES)/perf.txt

limpar:
	rm -f cache_detective cache_detective_pg gmon.out
	rm -rf $(RES)

# Todas as etapas obrigatorias em sequencia (util para o professor demonstrar).
tudo: verificar info medir gprof
	$(MAKE) cachegrind D1=32768,8,64
	$(MAKE) cachegrind D1=65536,8,64
	$(MAKE) cachegrind D1=32768,8,128
