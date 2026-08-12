# Relatório LaTeX

Esta pasta contém o relatório técnico do projeto 2D Plotter:

- `relatorio.tex` — fonte autoritativa em LaTeX;
- `relatorio.pdf` — versão compilada para distribuição.

## Estrutura

```text
report/
├── relatorio.tex
├── relatorio.pdf
├── build.ps1
├── README.md
└── figures/
    ├── front-cover.png
    └── README.md
```

## Compilação

Execute a partir de qualquer diretório:

```powershell
.\report\build.ps1
```

O script usa `latexmk -lualatex` quando disponível e, se necessário, executa
duas passagens de `lualatex`. Após sucesso, remove somente arquivos auxiliares;
os fontes e PDFs são preservados.

Opções:

```powershell
.\report\build.ps1 -Clean
.\report\build.ps1 -KeepTemp
.\report\build.ps1 -Open
```
