# CACHE DETECTIVE: Como rodar
### Escolha o seu caminho: o Caminho A não exige instalar nada

**Infraestrutura de Hardware · Aula 12 · Prof. Ronierison Maciel**

A atividade precisa de um terminal **Linux** com `gcc`, `make`, `gprof` e `valgrind`. Escolha **um** caminho:

| Caminho | Para quem | Precisa instalar? | Tempo para ficar pronto |
|---|---|---|---|
| **A: GitHub Codespaces** | **Todos (recomendado em sala)** | Não. Só navegador e conta no GitHub | 3 a 5 min |
| **B: Linux, WSL2 ou Docker que você já tem** | Quem já usa um desses | Só os pacotes, se faltar algum | 1 a 2 min |
| **C: Dados de referência** | Quem não conseguiu A nem B | Não | Imediato |

> **Dica:** abram o Codespaces **logo no início**. Ele termina de preparar o ambiente sozinho enquanto vocês leem o caso.

---

## Caminho A: GitHub Codespaces (Linux no navegador)

O Codespaces é um computador Linux na nuvem que abre no navegador, com o VS Code e um terminal. A conta gratuita do GitHub inclui horas mensais de uso, mais do que suficientes para a atividade.

1. Entre em <https://github.com>. Quem não tem conta pode criar uma de graça em 2 minutos (*Sign up*).
2. Abra o repositório da atividade:

   > **<https://github.com/0x03C1/cache-detective-aula12>**

3. Clique no botão **"Open in GitHub Codespaces"** do README e depois em **"Create codespace"**. Outra forma: botão verde **Code**, aba **Codespaces**, **Create codespace on main**. Link direto: <https://codespaces.new/0x03C1/cache-detective-aula12>. Aguarde de 3 a 5 minutos: o ambiente instala `gcc`, `gprof` e `valgrind` automaticamente.
4. Quando o VS Code abrir no navegador, abra o terminal: menu principal (ícone de três linhas, no canto superior esquerdo) **→ Terminal → New Terminal** (ou `` Ctrl+` ``).
5. No terminal, digite:

   ```bash
   cd codigo
   make verificar
   ```

   Se todas as linhas mostrarem `[ok]`, está pronto. Se o `valgrind` aparecer como `[FALTA]`, espere mais um minuto (a instalação ainda está terminando) e rode de novo.

**Dicas no Codespaces:**

- **Ver o gráfico:** no painel de arquivos à esquerda, clique com o botão direito em `codigo/resultados/degraus.svg` → **Open With… → Image Preview**. Para salvá-lo no seu computador, use **Download…**.
- **Baixar os resultados para a entrega:** `zip -r resultados.zip resultados` e depois, no painel de arquivos, clique com o botão direito em `resultados.zip` → **Download…**.
- **Ao terminar:** em <https://github.com/codespaces>, clique em **⋯ → Delete** para não gastar suas horas gratuitas.
- O Codespaces roda em uma **máquina virtual na nuvem**. Declare isso na entrega: os tempos da Etapa 1 serão diferentes dos de um computador pessoal, e isso é esperado.

---

## Caminho B: Linux, WSL2 ou Docker que você já tem

1. Baixe o `pacote_alunos.zip` (anexo da atividade) e descompacte. Outra opção: clone o repositório, com `git clone https://github.com/0x03C1/cache-detective-aula12.git`.
2. No terminal Linux, entre na pasta `cache-detective-aula12/codigo` e rode `make verificar`.
3. Se faltar alguma ferramenta:

   ```bash
   sudo apt update && sudo apt install -y build-essential valgrind python3 zip   # Ubuntu / Debian / WSL2
   sudo dnf install -y gcc make binutils valgrind python3 zip                 # Fedora
   ```

**No WSL2:** para copiar o zip da pasta Downloads do Windows, use `cp /mnt/c/Users/SEU_USUARIO/Downloads/pacote_alunos.zip ~`. Para abrir arquivos no Windows, use `explorer.exe resultados/degraus.svg`.

**Com Docker:** na pasta `cache-detective-aula12`, rode `bash docker/entrar.sh` (macOS/Linux) ou `powershell -ExecutionPolicy Bypass -File docker\entrar.ps1` (Windows). Na primeira vez, o script constrói a imagem.

---

## Caminho C: Dados de referência

Se nenhum ambiente funcionou, **não pare**: a maior parte da nota está na interpretação.

1. Abra a pasta `dados_referencia/linux_docker_M1` do pacote ou do repositório <https://github.com/0x03C1/cache-detective-aula12>. Ela traz a saída de todas as etapas, executadas pelo professor.
2. Preencha as tabelas com esses dados e responda às questões normalmente.
3. Escreva na entrega: *"Dados de referência fornecidos pelo professor (Apple M1, Docker), pois [motivo]"*.

---

## Problemas comuns

| Sintoma | Solução |
|---|---|
| Link do Codespaces pede login | Entre na sua conta do GitHub (ou crie uma) e abra o link de novo |
| "You've used 100% of included services" | Sua cota gratuita acabou: use o Caminho B ou C |
| `valgrind: command not found` logo após abrir | A instalação automática ainda está rodando: espere 1 minuto |
| `make: *** No targets specified` | Você não está na pasta `codigo`: rode `cd codigo` (no Codespaces) |
| `Makefile:...: *** missing separator` | O Makefile foi editado e perdeu as tabulações. Baixe-o de novo |
| Tempos muito irregulares | Normal em máquinas virtuais compartilhadas: rode de novo e registre as duas execuções |

---

## Apêndice: WSL2 no seu Windows (para depois da aula)

Para o **artigo final** da disciplina, vale ter um Linux no próprio computador, porque medições numa máquina conhecida são mais fáceis de descrever. Não faça isso durante a aula: leva de 20 a 40 minutos e pede reinicialização.

1. Abra o **Terminal (Administrador)** e rode `wsl --install`. Reinicie quando for pedido.
2. Na janela do Ubuntu, crie um usuário e uma senha.
3. Rode `sudo apt update && sudo apt install -y build-essential valgrind python3 zip unzip`.

Se aparecer erro de virtualização (`0x80370102`), habilite *Intel VT-x* ou *AMD SVM* na BIOS do notebook.
