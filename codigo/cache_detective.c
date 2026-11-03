/*
 * =====================================================================
 *  CACHE DETECTIVE: Aula 12, Hierarquia de Memoria e Memoria Cache
 *  Infraestrutura de Hardware, Prof. Ronierison Maciel
 * =====================================================================
 *
 *  Um unico programa com tres modos:
 *
 *    ./cache_detective degraus [max_MB]
 *        MEDICAO REAL. Latencia por acesso (ns) em vetores de 4 KB ate
 *        128 MB, com o MESMO numero de acessos em todos os tamanhos
 *        (o experimento dos "degraus" da aula). Dois percursos:
 *          - sequencial: cada acesso vai para a linha de cache vizinha;
 *          - aleatorio : cada acesso vai para uma linha sorteada.
 *        Gera resultados/degraus.csv e resultados/degraus.svg
 *
 *    ./cache_detective padroes [repeticoes]
 *        MEDICAO REAL. Os "dois programas com os mesmos dados":
 *          - matriz percorrida por linhas  x por colunas  (Aula 01)
 *          - particulas AoS                x SoA          (slide 14)
 *        Cada medicao e repetida; o programa informa mediana, minimo e
 *        maximo. Gera resultados/padroes.csv
 *        O modo "gprof [repeticoes]" faz o mesmo sem gravar o CSV
 *        (usado pelo make gprof).
 *
 *    ./cache_detective perfil
 *        Mesmas funcoes do modo 'padroes', com dados menores e uma
 *        unica execucao. Feito para rodar sob o Valgrind/Cachegrind
 *        (make cachegrind), que deixa o programa 20-50x mais lento.
 *
 *  Compilacao (o Makefile faz isso):  gcc -O1 -g -o cache_detective cache_detective.c
 *
 *  Por que tantas funcoes pequenas? Para que o gprof e o Cachegrind
 *  mostrem o custo de CADA padrao separadamente (por nome de funcao).
 * =====================================================================
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>

/* ------------------------------------------------------------------ */
/* PARAMETROS: podem ser alterados                                    */
/* ------------------------------------------------------------------ */
#define LINHA_CACHE      64           /* bytes por linha de cache (aula)       */
#define ACESSOS_DEGRAUS  (8L << 20)   /* ~8,4 milhoes de acessos por medicao  */
#define REP_DEGRAUS      5            /* repeticoes por tamanho (vale mediana) */
#define MAX_MB_PADRAO    128          /* maior vetor do modo degraus           */

#define N_MATRIZ         4000         /* matriz 4000 x 4000 ints = 61 MB       */
#define N_PARTICULAS     2000000L     /* 2 milhoes x 64 B = 122 MB (AoS)       */
#define REP_PADROES      7            /* repeticoes de cada padrao (padrao)    */
#define REP_MAX          200          /* limite do argumento [repeticoes]      */

#define N_MATRIZ_PERFIL     1000      /* modo perfil (Cachegrind): 3,8 MB      */
#define N_PARTICULAS_PERFIL 200000L   /* modo perfil: 12 MB (AoS)              */

#define PASTA_RESULTADOS "resultados"

/* ------------------------------------------------------------------ */
/* Utilitarios: tempo, numeros aleatorios, mediana                     */
/* ------------------------------------------------------------------ */

/* Relogio monotonico de alta resolucao, em nanossegundos.
 * (clock() mede tempo de CPU e tem resolucao ruim em alguns sistemas.) */
static double agora_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

/* Gerador xorshift64: rapido e reproduzivel (mesma semente = mesma ordem). */
static unsigned long long estado_rng = 2026ULL;
static unsigned long long aleatorio(void) {
    estado_rng ^= estado_rng << 13;
    estado_rng ^= estado_rng >> 7;
    estado_rng ^= estado_rng << 17;
    return estado_rng;
}

static int compara_double(const void *a, const void *b) {
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

/* Ordena o vetor e devolve a mediana (valor central). */
static double mediana(double *v, int n) {
    qsort(v, n, sizeof(double), compara_double);
    return (n % 2) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) / 2.0;
}

static void *alocar(size_t bytes) {
    void *p = NULL;
    if (posix_memalign(&p, LINHA_CACHE, bytes) != 0 || p == NULL) {
        fprintf(stderr, "ERRO: memoria insuficiente para %zu MB.\n", bytes >> 20);
        exit(1);
    }
    return p;
}

/* NAO_INLINE impede o compilador de "colar" a funcao dentro de quem a chama.
 * Sem isso, o gprof e o Cachegrind nao conseguiriam mostrar o custo de cada
 * padrao separadamente (tudo apareceria dentro de modo_padroes). */
#define NAO_INLINE __attribute__((noinline))

/* Variavel 'volatile': o compilador e obrigado a gravar nela, entao nao
 * pode eliminar os lacos medidos por "achar que o resultado nao e usado". */
volatile long long sumidouro;

/* ------------------------------------------------------------------ */
/* Informacoes do computador (tamanhos de cache)                       */
/* ------------------------------------------------------------------ */
static long cache_bytes[4];   /* [1]=L1d, [2]=L2, [3]=L3; 0 = desconhecido */

static void ler_caches(void) {
    char caminho[128], tipo[32], tam[32];
    int nivel;
    for (int i = 0; i < 8; i++) {
        snprintf(caminho, sizeof caminho, "/sys/devices/system/cpu/cpu0/cache/index%d/level", i);
        FILE *f = fopen(caminho, "r");
        if (!f) break;
        if (fscanf(f, "%d", &nivel) != 1) nivel = 0;
        fclose(f);
        snprintf(caminho, sizeof caminho, "/sys/devices/system/cpu/cpu0/cache/index%d/type", i);
        f = fopen(caminho, "r");
        if (!f || fscanf(f, "%31s", tipo) != 1) { if (f) fclose(f); continue; }
        fclose(f);
        if (strcmp(tipo, "Instruction") == 0) continue;
        snprintf(caminho, sizeof caminho, "/sys/devices/system/cpu/cpu0/cache/index%d/size", i);
        f = fopen(caminho, "r");
        if (!f || fscanf(f, "%31s", tam) != 1) { if (f) fclose(f); continue; }
        fclose(f);
        long v = atol(tam);
        if (strchr(tam, 'K')) v *= 1024;
        if (strchr(tam, 'M')) v *= 1024 * 1024;
        if (nivel >= 1 && nivel <= 3) cache_bytes[nivel] = v;
    }
#ifdef _SC_LEVEL1_DCACHE_SIZE   /* alternativa (glibc) */
    if (!cache_bytes[1]) { long v = sysconf(_SC_LEVEL1_DCACHE_SIZE); if (v > 0) cache_bytes[1] = v; }
    if (!cache_bytes[2]) { long v = sysconf(_SC_LEVEL2_CACHE_SIZE);  if (v > 0) cache_bytes[2] = v; }
    if (!cache_bytes[3]) { long v = sysconf(_SC_LEVEL3_CACHE_SIZE);  if (v > 0) cache_bytes[3] = v; }
#endif
}

static void texto_tamanho(long bytes, char *saida, size_t n) {
    if (bytes >= 1024L * 1024) snprintf(saida, n, "%ld MB", bytes >> 20);
    else                       snprintf(saida, n, "%ld KB", bytes >> 10);
}

static void imprimir_caches(void) {
    char t[32];
    printf(" Caches informadas pelo sistema:");
    int achou = 0;
    for (int n = 1; n <= 3; n++) {
        if (cache_bytes[n]) {
            texto_tamanho(cache_bytes[n], t, sizeof t);
            printf("  L%d%s = %s", n, n == 1 ? "d" : "", t);
            achou = 1;
        }
    }
    if (!achou) printf("  nao disponiveis (rode 'lscpu' ou pesquise o modelo do processador)");
    printf("\n");
}

static FILE *abrir_resultado(const char *nome) {
    char caminho[256];
    mkdir(PASTA_RESULTADOS, 0755);
    snprintf(caminho, sizeof caminho, "%s/%s", PASTA_RESULTADOS, nome);
    FILE *f = fopen(caminho, "w");
    if (!f) { perror(caminho); exit(1); }
    return f;
}

/* =================================================================== */
/* MODO 1: DEGRAUS (pointer chasing)                                   */
/* =================================================================== */
/*
 * Cada elemento ocupa exatamente UMA linha de cache (64 bytes) e guarda
 * o endereco do proximo elemento a visitar. O laco faz p = p->prox:
 * cada leitura DEPENDE da anterior, entao o processador nao consegue
 * adiantar varias leituras ao mesmo tempo. Assim medimos a LATENCIA real
 * de cada nivel da hierarquia.
 */
typedef struct No {
    struct No *prox;
    char preenchimento[LINHA_CACHE - sizeof(struct No *)];
} No;

/* Percorre a cadeia 'passos' vezes. E esta funcao que o cronometro mede. */
NAO_INLINE static No *perseguir(No *p, long passos) {
    for (long k = 0; k < passos; k++)
        p = p->prox;
    return p;
}

/* Liga os n nos em ordem sequencial (0->1->2->...->n-1->0)
 * ou em uma ordem sorteada (ciclo unico que visita todos os nos). */
static No *montar_cadeia(No *nos, long n, int aleatoria) {
    long *ordem = malloc(n * sizeof(long));
    if (!ordem) { fprintf(stderr, "ERRO: memoria insuficiente.\n"); exit(1); }
    for (long i = 0; i < n; i++) ordem[i] = i;
    if (aleatoria) {                       /* embaralhamento de Fisher-Yates */
        for (long i = n - 1; i > 0; i--) {
            long j = (long)(aleatorio() % (unsigned long long)(i + 1));
            long tmp = ordem[i]; ordem[i] = ordem[j]; ordem[j] = tmp;
        }
    }
    for (long i = 0; i < n; i++)
        nos[ordem[i]].prox = &nos[ordem[(i + 1) % n]];
    No *inicio = &nos[ordem[0]];
    free(ordem);
    return inicio;
}

static double medir_cadeia(No *inicio, long passos) {
    double t[REP_DEGRAUS];
    sumidouro += (long long)(size_t)perseguir(inicio, passos / 4);   /* aquecimento */
    for (int r = 0; r < REP_DEGRAUS; r++) {
        double t0 = agora_ns();
        No *fim = perseguir(inicio, passos);
        t[r] = (agora_ns() - t0) / passos;
        sumidouro += (long long)(size_t)fim;
    }
    return mediana(t, REP_DEGRAUS);
}

/* Grafico SVG simples: eixo X dobra a cada ponto, eixo Y logaritmico. */
static void svg_degraus(int n, const long *tam, const double *seq, const double *ale) {
    FILE *f = abrir_resultado("degraus.svg");
    const int L = 900, A = 520, me = 80, md = 140, mt = 80, mb = 80;
    const double w = L - me - md, h = A - mt - mb;
    double ymin = 0.5, ymax = 1000.0;           /* 0,5 ns a 1000 ns (log) */
    #define PX(i) (me + w * (i) / (double)(n - 1))
    #define PY(v) (mt + h - h * (log10(v) - log10(ymin)) / (log10(ymax) - log10(ymin)))
    fprintf(f, "<svg xmlns='http://www.w3.org/2000/svg' width='%d' height='%d' "
               "font-family='Helvetica, Arial, sans-serif'>\n", L, A);
    fprintf(f, "<rect width='100%%' height='100%%' fill='#ffffff'/>\n");
    fprintf(f, "<text x='%d' y='34' font-size='20' font-weight='bold' fill='#0b0b0b'>"
               "Degraus da hierarquia de memoria (medicao real)</text>\n", me);
    fprintf(f, "<text x='%d' y='56' font-size='13' fill='#52514e'>Latencia por acesso "
               "(escala logaritmica) x tamanho do vetor. Linhas tracejadas: caches informadas pelo sistema.</text>\n", me);
    const double grade[] = {0.5, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000};
    for (int g = 0; g < 11; g++) {
        fprintf(f, "<line x1='%d' y1='%.1f' x2='%.1f' y2='%.1f' stroke='#e4e3df'/>\n",
                me, PY(grade[g]), me + w, PY(grade[g]));
        fprintf(f, "<text x='%d' y='%.1f' font-size='12' fill='#52514e' text-anchor='end'>%g</text>\n",
                me - 8, PY(grade[g]) + 4, grade[g]);
    }
    char t[32];
    for (int i = 0; i < n; i++) {
        texto_tamanho(tam[i], t, sizeof t);
        fprintf(f, "<text x='%.1f' y='%.1f' font-size='11' fill='#52514e' text-anchor='end' "
                   "transform='rotate(-40 %.1f %.1f)'>%s</text>\n",
                PX(i), mt + h + 18, PX(i), mt + h + 18, t);
    }
    /* linhas verticais nas capacidades das caches */
    for (int c = 1; c <= 3; c++) {
        if (!cache_bytes[c]) continue;
        double pos = log2((double)cache_bytes[c] / tam[0]);
        if (pos < 0 || pos > n - 1) continue;
        double x = PX(pos);
        fprintf(f, "<line x1='%.1f' y1='%d' x2='%.1f' y2='%.1f' stroke='#8a8984' "
                   "stroke-dasharray='5,4'/>\n", x, mt, x, mt + h);
        texto_tamanho(cache_bytes[c], t, sizeof t);
        fprintf(f, "<text x='%.1f' y='%d' font-size='12' fill='#52514e' text-anchor='middle'>"
                   "L%d (%s)</text>\n", x, mt - 6, c, t);
    }
    const char *nomes[2] = {"Aleatorio", "Sequencial"};
    const char *cores[2] = {"#FF6002", "#2B2B2B"};
    const double *series[2] = {ale, seq};
    for (int s = 0; s < 2; s++) {
        fprintf(f, "<polyline fill='none' stroke='%s' stroke-width='2.5' points='", cores[s]);
        for (int i = 0; i < n; i++)
            fprintf(f, "%.1f,%.1f ", PX(i), PY(fmax(series[s][i], ymin)));
        fprintf(f, "'/>\n");
        for (int i = 0; i < n; i++) {
            texto_tamanho(tam[i], t, sizeof t);
            fprintf(f, "<circle cx='%.1f' cy='%.1f' r='4.5' fill='%s' stroke='#fff' stroke-width='1.5'>"
                       "<title>%s, %s: %.2f ns</title></circle>\n",
                    PX(i), PY(fmax(series[s][i], ymin)), cores[s], nomes[s], t, series[s][i]);
        }
        fprintf(f, "<text x='%.1f' y='%.1f' font-size='13' font-weight='bold' fill='#0b0b0b'>%s</text>\n",
                PX(n - 1) + 12, PY(fmax(series[s][n - 1], ymin)) + 4, nomes[s]);
    }
    fprintf(f, "<text transform='translate(24 %.1f) rotate(-90)' font-size='13' fill='#52514e' "
               "text-anchor='middle'>Nanossegundos por acesso (log)</text>\n", mt + h / 2);
    fprintf(f, "<text x='%.1f' y='%d' font-size='13' fill='#52514e' text-anchor='middle'>"
               "Tamanho do vetor</text>\n", me + w / 2, A - 14);
    fprintf(f, "</svg>\n");
    fclose(f);
    #undef PX
    #undef PY
}

static void modo_degraus(int max_mb) {
    long max_bytes = (long)max_mb << 20;
    long tam[32];
    double seq[32], ale[32];
    int n = 0;

    printf("\n=== CACHE DETECTIVE: DEGRAUS (medicao real) ===\n");
    imprimir_caches();
    printf(" Acessos por medicao: %ld | repeticoes: %d (vale a mediana)\n\n",
           ACESSOS_DEGRAUS, REP_DEGRAUS);
    printf(" %9s | %15s | %14s | %8s\n", "Tamanho", "Sequencial (ns)", "Aleatorio (ns)", "Razao");
    printf(" ----------+-----------------+----------------+---------\n");

    FILE *csv = abrir_resultado("degraus.csv");
    fprintf(csv, "tamanho_bytes;tamanho;sequencial_ns;aleatorio_ns;razao\n");

    for (long bytes = 4096; bytes <= max_bytes && n < 32; bytes *= 2) {
        long nos_n = bytes / LINHA_CACHE;
        No *nos = alocar((size_t)bytes);
        memset(nos, 0, (size_t)bytes);                 /* toca todas as paginas */
        No *ini_seq = montar_cadeia(nos, nos_n, 0);
        double t_seq = medir_cadeia(ini_seq, ACESSOS_DEGRAUS);
        No *ini_ale = montar_cadeia(nos, nos_n, 1);
        double t_ale = medir_cadeia(ini_ale, ACESSOS_DEGRAUS);
        free(nos);

        char t[32];
        texto_tamanho(bytes, t, sizeof t);
        printf(" %9s | %15.2f | %14.2f | %7.1fx\n", t, t_seq, t_ale, t_ale / t_seq);
        fflush(stdout);
        fprintf(csv, "%ld;%s;%.3f;%.3f;%.2f\n", bytes, t, t_seq, t_ale, t_ale / t_seq);
        tam[n] = bytes; seq[n] = t_seq; ale[n] = t_ale; n++;
    }
    fclose(csv);
    if (n > 1) svg_degraus(n, tam, seq, ale);
    printf("\n Arquivos: %s/degraus.csv  e  %s/degraus.svg\n\n", PASTA_RESULTADOS, PASTA_RESULTADOS);
}

/* =================================================================== */
/* MODO 2: PADROES, matriz (linhas x colunas) e particulas (AoS x SoA) */
/* =================================================================== */

/* Matriz N x N guardada por LINHAS (como em C): m[i][j] = m[i*N + j] */
NAO_INLINE static void inicializar_matriz(int *m, long n) {
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            m[i * n + j] = (int)((i + j) % 10);
}

/* Percorre por linhas: enderecos consecutivos (16 ints por linha de cache). */
NAO_INLINE static long long soma_linhas(const int *m, long n) {
    long long s = 0;
    for (long i = 0; i < n; i++)
        for (long j = 0; j < n; j++)
            s += m[i * n + j];
    return s;
}

/* Percorre por colunas: cada acesso salta n*4 bytes (outra linha de cache). */
NAO_INLINE static long long soma_colunas(const int *m, long n) {
    long long s = 0;
    for (long j = 0; j < n; j++)
        for (long i = 0; i < n; i++)
            s += m[i * n + j];
    return s;
}

/* A struct do "Ache o bug" (slide 14): 8 doubles = 64 bytes = 1 linha. */
typedef struct {
    double x, y, z;
    double vx, vy, vz;
    double massa, carga;
} Particula;

/* Versao SoA: um vetor para cada campo. */
typedef struct {
    double *x, *y, *z, *vx, *vy, *vz, *massa, *carga;
} Particulas;

NAO_INLINE static void inicializar_particulas(Particula *aos, Particulas *soa, long n) {
    for (long i = 0; i < n; i++) {
        double v = (double)(i % 100);
        aos[i].x = v; aos[i].y = v + 1; aos[i].z = v + 2;
        aos[i].vx = 0; aos[i].vy = 0; aos[i].vz = 0;
        aos[i].massa = 1; aos[i].carga = -1;
        soa->x[i] = v; soa->y[i] = v + 1; soa->z[i] = v + 2;
        soa->vx[i] = 0; soa->vy[i] = 0; soa->vz[i] = 0;
        soa->massa[i] = 1; soa->carga[i] = -1;
    }
}

/*
 * As duas somas abaixo usam 4 acumuladores independentes (s0..s3), do MESMO
 * jeito nas duas versoes. Motivo (metodologia): com um unico acumulador, cada
 * soma em ponto flutuante precisa esperar a anterior terminar (~1 ns), e esse
 * custo de CALCULO esconderia o custo da MEMORIA que queremos medir.
 * n deve ser multiplo de 4 (2.000.000 e 200.000 sao).
 */

/* So usa 8 bytes (x) de cada 64 bytes trazidos. */
NAO_INLINE static double soma_aos(const Particula *p, long n) {
    double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    for (long i = 0; i < n; i += 4) {
        s0 += p[i].x;
        s1 += p[i + 1].x;
        s2 += p[i + 2].x;
        s3 += p[i + 3].x;
    }
    return (s0 + s1) + (s2 + s3);
}

/* Os x estao lado a lado: 8 valores uteis por linha de cache. */
NAO_INLINE static double soma_soa(const Particulas *p, long n) {
    const double *x = p->x;
    double s0 = 0, s1 = 0, s2 = 0, s3 = 0;
    for (long i = 0; i < n; i += 4) {
        s0 += x[i];
        s1 += x[i + 1];
        s2 += x[i + 2];
        s3 += x[i + 3];
    }
    return (s0 + s1) + (s2 + s3);
}

static void alocar_soa(Particulas *s, long n) {
    size_t b = (size_t)n * sizeof(double);
    s->x = alocar(b); s->y = alocar(b); s->z = alocar(b); s->vx = alocar(b);
    s->vy = alocar(b); s->vz = alocar(b); s->massa = alocar(b); s->carga = alocar(b);
}

static void liberar_soa(Particulas *s) {
    free(s->x); free(s->y); free(s->z); free(s->vx);
    free(s->vy); free(s->vz); free(s->massa); free(s->carga);
}

/* Estatisticas de uma serie de medicoes (em ms). */
typedef struct { double med, min, max; } Estat;

static Estat estat(double *v, int n) {
    Estat e;
    e.med = mediana(v, n);       /* mediana() ordena o vetor */
    e.min = v[0];
    e.max = v[n - 1];
    return e;
}

static void imprimir_linha(FILE *csv, const char *nome, Estat e, long long verif) {
    printf(" %-22s | %10.2f | %10.2f | %10.2f\n", nome, e.med, e.min, e.max);
    fprintf(csv, "%s;%.3f;%.3f;%.3f;%lld\n", nome, e.med, e.min, e.max, verif);
}

static void modo_padroes(int perfil, int rep, int gravar) {
    long nm = perfil ? N_MATRIZ_PERFIL : N_MATRIZ;
    long np = perfil ? N_PARTICULAS_PERFIL : N_PARTICULAS;
    if (perfil) rep = 1;

    printf("\n=== CACHE DETECTIVE: PADROES%s ===\n",
           perfil ? " (modo perfil, para o Cachegrind)" : " (medicao real)");
    printf(" Matriz: %ld x %ld ints (%.1f MB) | Particulas: %ld (AoS %.1f MB, campo x em SoA %.1f MB)\n",
           nm, nm, nm * nm * 4.0 / (1 << 20), np,
           np * (double)sizeof(Particula) / (1 << 20), np * 8.0 / (1 << 20));
    printf(" Repeticoes: %d\n\n", rep);

    int *m = alocar((size_t)(nm * nm) * sizeof(int));
    Particula *aos = alocar((size_t)np * sizeof(Particula));
    Particulas soa;
    alocar_soa(&soa, np);
    inicializar_matriz(m, nm);
    inicializar_particulas(aos, &soa, np);

    double tl[REP_MAX], tc[REP_MAX], ta[REP_MAX], ts[REP_MAX];
    long long sl = 0, sc = 0;
    double sa = 0, ss = 0;
    for (int r = 0; r < rep; r++) {           /* alterna os padroes a cada rodada */
        double t0 = agora_ns(); sl = soma_linhas(m, nm);    tl[r] = (agora_ns() - t0) / 1e6;
        t0 = agora_ns();        sc = soma_colunas(m, nm);   tc[r] = (agora_ns() - t0) / 1e6;
        t0 = agora_ns();        sa = soma_aos(aos, np);     ta[r] = (agora_ns() - t0) / 1e6;
        t0 = agora_ns();        ss = soma_soa(&soa, np);    ts[r] = (agora_ns() - t0) / 1e6;
        sumidouro += sl + sc + (long long)(sa + ss);
    }

    if (!perfil && gravar) {
        FILE *csv = abrir_resultado("padroes.csv");
        fprintf(csv, "padrao;mediana_ms;minimo_ms;maximo_ms;soma_verificacao\n");
        printf(" %-22s | %10s | %10s | %10s\n", "Padrao", "Mediana ms", "Minimo ms", "Maximo ms");
        printf(" -----------------------+------------+------------+-----------\n");
        Estat el = estat(tl, rep), ec = estat(tc, rep), ea = estat(ta, rep), es = estat(ts, rep);
        imprimir_linha(csv, "Matriz por linhas", el, sl);
        imprimir_linha(csv, "Matriz por colunas", ec, sc);
        imprimir_linha(csv, "Particulas AoS", ea, (long long)sa);
        imprimir_linha(csv, "Particulas SoA", es, (long long)ss);
        fclose(csv);
        printf("\n Razao colunas / linhas : %.2fx\n", ec.med / el.med);
        printf(" Razao AoS / SoA        : %.2fx\n", ea.med / es.med);
        printf("\n Arquivo: %s/padroes.csv\n", PASTA_RESULTADOS);
    }
    printf(" Verificacao: somas linhas/colunas %s, AoS/SoA %s\n\n",
           sl == sc ? "IGUAIS (ok)" : "DIFERENTES (erro!)",
           sa == ss ? "IGUAIS (ok)" : "DIFERENTES (erro!)");

    free(m); free(aos); liberar_soa(&soa);
}

/* =================================================================== */
int main(int argc, char **argv) {
    ler_caches();
    if (argc >= 2 && strcmp(argv[1], "degraus") == 0) {
        int max_mb = (argc >= 3) ? atoi(argv[2]) : MAX_MB_PADRAO;
        if (max_mb < 1) max_mb = MAX_MB_PADRAO;
        modo_degraus(max_mb);
    } else if (argc >= 2 && strcmp(argv[1], "padroes") == 0) {
        int rep = (argc >= 3) ? atoi(argv[2]) : REP_PADROES;
        if (rep < 1 || rep > REP_MAX) rep = REP_PADROES;
        modo_padroes(0, rep, 1);
    } else if (argc >= 2 && strcmp(argv[1], "gprof") == 0) {
        /* igual a "padroes", mas sem gravar CSV (para nao sobrescrever a medicao) */
        int rep = (argc >= 3) ? atoi(argv[2]) : REP_PADROES;
        if (rep < 1 || rep > REP_MAX) rep = REP_PADROES;
        modo_padroes(0, rep, 0);
    } else if (argc >= 2 && strcmp(argv[1], "perfil") == 0) {
        modo_padroes(1, 1, 0);
    } else {
        printf("Uso:\n"
               "  ./cache_detective degraus [max_MB]   (medicao: latencia x tamanho)\n"
               "  ./cache_detective padroes [reps]     (medicao: linhas x colunas, AoS x SoA)\n"
               "  ./cache_detective perfil             (dados menores, para o Cachegrind)\n");
        return 1;
    }
    return 0;
}
