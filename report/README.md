# Relatório LaTeX

Esta pasta contém dois documentos do projeto 2D Plotter:

- `relatorio.tex` / `relatorio.pdf` — relatório técnico completo;
- `apresentacao_mobile.tex` / `apresentacao_mobile.pdf` — apresentação
  vertical 9:16 para leitura em celular.

## Estrutura

```text
report/
├── relatorio.tex
├── relatorio.pdf
├── apresentacao_mobile.tex
├── apresentacao_mobile.pdf
├── build.ps1
├── README.md
└── figures/
    └── README.md
```

## Compilação

Execute a partir de qualquer diretório:

```powershell
.\report\build.ps1
.\report\build.ps1 -Document apresentacao_mobile
```

O script usa `latexmk -lualatex` quando disponível e, se necessário, executa
duas passagens de `lualatex`. Após sucesso, remove somente arquivos auxiliares;
os fontes e PDFs são preservados.

Opções:

```powershell
.\report\build.ps1 -Clean
.\report\build.ps1 -Document apresentacao_mobile -Clean
.\report\build.ps1 -KeepTemp
.\report\build.ps1 -Open
```
