# CACHE DETECTIVE: Entrega

**Infraestrutura de Hardware · Aula 12 · Prof. Ronierison Maciel · Valor: 1,0 ponto**

> Preencham os campos entre colchetes e apaguem as instruções em itálico. Salvem como `.md` ou exportem para PDF.

## 1. Integrantes

- [Nome completo 1]
- [Nome completo 2, se for em dupla]

## 2. Hipóteses (registradas antes da execução)

**H1: formato do gráfico dos degraus**

| Percurso | Alternativa (a, b, c ou d) | Justificativa (1 frase) |
|---|---|---|
| Sequencial | [ ] | [ ] |
| Aleatório | [ ] | [ ] |

**H2: previsão de misses (linha de 64 B, cache começa vazia)**

| Função | Misses esperados | Raciocínio |
|---|---:|---|
| `soma_linhas` (1000 × 1000 ints) | [ ] | [ ] |
| `soma_colunas` (1000 × 1000 ints) | [ ] | [ ] |
| `soma_aos` (200.000 partículas) | [ ] | [ ] |
| `soma_soa` (200.000 partículas) | [ ] | [ ] |

**H3 (função que vai dominar o gprof):** [ ] · Justificativa: [ ]

## 3. O que foi executado e em qual ambiente

*De 2 a 4 linhas: comandos executados, se repetiram algo, se usaram alguma variação.*

[ ]

| Item | Valor |
|---|---|
| Ambiente | [ ] Codespaces · [ ] WSL2 · [ ] Linux nativo · [ ] Docker · [ ] Dados de referência do professor (motivo: …) |
| Processador (`make info`) | [ ] |
| Caches L1d / L2 / L3 (`make info`) | [ ] |
| Compilador (versão) | [ ] |

## 4. Resultados

**Tabela 1 (Etapa 1): patamares do percurso aleatório (`make medir`)**

| Patamar | Faixa de tamanhos | Latência aproximada (ns) | Nível da hierarquia (interpretação) |
|---|---|---:|---|
| 1 | [ ] | [ ] | [ ] |
| 2 | [ ] | [ ] | [ ] |
| 3 | [ ] | [ ] | [ ] |
| 4 (se houver) | [ ] | [ ] | [ ] |

Percurso sequencial em 128 MB: [ ] ns · Aleatório em 128 MB: [ ] ns

*Insiram aqui o gráfico `resultados/degraus.svg` (captura de tela).*

**Tabela 2 (Etapa 1): duas execuções de `padroes` (ms)**

| Função | 1ª mediana | 1ª mín. a máx. | 2ª mediana | 2ª mín. a máx. |
|---|---:|---:|---:|---:|
| `soma_linhas` | | | | |
| `soma_colunas` | | | | |
| `soma_aos` | | | | |
| `soma_soa` | | | | |

**Tabela 3 (Etapa 2): gprof (`make gprof`)**

| Função | % time | self seconds |
|---|---:|---:|
| `soma_colunas` | | |
| `soma_linhas` | | |
| `soma_aos` | | |
| `soma_soa` | | |

*Insiram uma captura de tela do perfil plano.*

**Tabela 4 (Etapa 3): Cachegrind (leituras e misses de L1)**

| Função | Dr | D1mr A (32 KB, 64 B) | Taxa A | D1mr B (64 KB, 64 B) | Taxa B | D1mr C (32 KB, 128 B) | Taxa C |
|---|---:|---:|---:|---:|---:|---:|---:|
| `soma_linhas` | | | | | | | |
| `soma_colunas` | | | | | | | |
| `soma_aos` | | | | | | | |
| `soma_soa` | | | | | | | |

DLmr da Rodada A: `soma_linhas` = [ ] · `soma_colunas` = [ ]

*Insiram uma captura de tela de uma rodada do Cachegrind.*

**Tabela 5 (correlação entre as etapas): razões**

| Par | Tempo (Etapa 1, mediana) | gprof (self seconds) | Misses L1 (D1mr, Rodada A) |
|---|---:|---:|---:|
| `soma_colunas` ÷ `soma_linhas` | | | |
| `soma_aos` ÷ `soma_soa` | | | |

## 5. Respostas às questões investigativas

**Q1: Lendo os degraus.**
[ ]

**Q2: O sequencial não sobe os degraus.**
[ ]

**Q3: Medir não é rodar uma vez.**
(a) [ ]
(b) [ ]
(c) [ ]

**Q4: Quanto × onde × por quê.**
(a) [ ]
(b) [ ]
(c) [ ]

**Q5: Mudando a cache simulada.**
(a) [ ]
(b) [ ]
(c) [ ]

**Q6: Parecer técnico e metodologia.**
(a) [ ]
(b) [ ]

## 6. Conclusão

*De 3 a 5 linhas: as hipóteses se confirmaram? O que as três etapas mostram sobre a hierarquia de memória e a localidade?*

[ ]
