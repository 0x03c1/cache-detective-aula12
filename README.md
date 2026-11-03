# CACHE DETECTIVE: Aula 12

**Infraestrutura de Hardware · Hierarquia de Memória e Memória Cache · Prof. Ronierison Maciel (CESAR School) · 1,0 ponto**

Atividade prática investigativa: descobrir por que dois programas que processam os mesmos dados têm tempos tão diferentes, usando três instrumentos.

| Etapa | Pergunta | Instrumento |
|---|---|---|
| 1 | **Quanto** custa? | Medição de tempo em C |
| 2 | **Onde** o tempo é gasto? | Profiling com `gprof` |
| 3 | **Por quê**? | Simulação de cache com Valgrind/Cachegrind |

## Abrir o ambiente (sem instalar nada)

[![Open in GitHub Codespaces](https://github.com/codespaces/badge.svg)](https://codespaces.new/0x03C1/cache-detective-aula12)

1. Clique no botão acima (é preciso estar logado no GitHub) e depois em **Create codespace**.
2. Aguarde de 3 a 5 minutos: o ambiente instala `gcc`, `gprof` e `valgrind` sozinho.
3. No terminal do VS Code (`` Ctrl+` ``), digite:

   ```bash
   cd codigo
   make verificar
   ```

Outra forma de abrir: nesta página, clique no botão verde **Code**, aba **Codespaces**, e depois em **Create codespace on main**.

Já usa Linux, WSL2 ou Docker? Veja os outros caminhos em [`como_rodar.md`](como_rodar.md).

## Por onde começar

1. **Ambiente:** [`como_rodar.pdf`](como_rodar.pdf) (Codespaces, Linux/WSL2/Docker ou dados de referência).
2. **Roteiro da atividade:** [`atividade_alunos.pdf`](atividade_alunos.pdf).
3. **Entrega:** preencha o [`modelo_entrega.md`](modelo_entrega.md).

## O que tem aqui

```
cache-detective-aula12/
├── README.md                  este arquivo
├── como_rodar.pdf / .md       como abrir o ambiente
├── atividade_alunos.pdf / .md roteiro da atividade
├── modelo_entrega.md          modelo para preencher e entregar
├── codigo/
│   ├── cache_detective.c      o programa investigado
│   └── Makefile               make verificar | medir | gprof | cachegrind
├── docker/                    ambiente alternativo com Docker
├── dados_referencia/          use SOMENTE se o ambiente não funcionar (declare na entrega)
└── .devcontainer/             configuração automática do Codespaces
```

## Comandos da atividade (dentro da pasta `codigo`)

```bash
make verificar                    # confere as ferramentas
make info                         # processador e caches
make medir                        # Etapa 1: medição de tempo
make padroes                      # Etapa 1: 2ª execução das somas
make gprof                        # Etapa 2: profiling
make cachegrind                   # Etapa 3: Rodada A
make cachegrind D1=65536,8,64     # Etapa 3: Rodada B
make cachegrind D1=32768,8,128    # Etapa 3: Rodada C
zip -r resultados.zip resultados  # para anexar na entrega
```

No Codespaces, para baixar o `resultados.zip`, clique nele com o botão direito no painel de arquivos e escolha **Download**.

## Ao terminar

Apague o seu codespace em <https://github.com/codespaces> (**⋯ → Delete**), para não gastar as horas gratuitas da sua conta.
