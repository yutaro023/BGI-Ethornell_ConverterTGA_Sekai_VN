# 🖼️ BGI Converter TGA Usado para Extração de UI da engine BGI-Ethornell 

Conversor multi-formato para arquivos **BGI e BMP**, com exportação para **TGA (32-bit RGBA)**.

Suporta análise de arquivos, conversão individual e conversão em lote da pasta atual.

---

## ✨ Funcionalidades

- ✅ Detecta automaticamente o formato do arquivo  
- ✅ Converte para **TGA 32-bit (RGBA)**  
- ✅ Suporte a múltiplos formatos:
  - BMP 24-bit (BGR)
  - BMP 32-bit (BGRA)
  - BGI Custom RGBA (`0x00000020`)
  - BGI Custom RGB (`0x00000000`)
- ✅ Conversão em lote
- ✅ Filtro por padrão de nome
- ✅ Análise hexadecimal dos primeiros 64 bytes
- ✅ Menu interativo

---

## 📦 Formatos Suportados

| Formato | Descrição |
|----------|------------|
| BMP 24-bit | BGR padrão |
| BMP 32-bit | BGRA padrão |
| BGI 0x00000020 | RGBA direto |
| BGI 0x00000000 | RGB sem alpha |

---

## 🛠️ Compilação

### Linux / macOS (g++)

```bash
g++ -std=c++17 -O2 -o bgi_converter main.cpp
```

> ⚠️ Requer suporte a `std::filesystem` (C++17 ou superior)

Se necessário:

```bash
g++ -std=c++17 -O2 main.cpp -lstdc++fs -o bgi_converter
```

---

### Windows (MinGW)

```bash
g++ -std=c++17 -O2 -o bgi_converter.exe main.cpp
```

---

## 🚀 Modos de Uso

### ▶️ Modo Interativo

```bash
./bgi_converter
```

Abre o menu:

```
1. Converter arquivo específico
2. Converter todos da pasta
3. Converter com padrão
4. Analisar arquivo
0. Sair
```

---

### 🔍 Analisar Arquivo

Mostra:

- Tipo detectado
- Dimensões
- Bits por pixel
- Offset
- Hex dump (64 bytes)

```bash
./bgi_converter -a arquivo
```

Exemplo:

```bash
./bgi_converter -a SGTitle000300
```

---

### 🖼️ Converter Arquivo Individual

```bash
./bgi_converter -x arquivo [saida.tga]
```

Exemplos:

```bash
./bgi_converter -x SGTitle000300
```

Saída:
```
SGTitle000300.tga
```

Ou:

```bash
./bgi_converter -x SGTitle000300 titulo.tga
```

---

### 📁 Conversão em Lote

Converter todos os arquivos compatíveis da pasta:

```bash
./bgi_converter -b
```

Converter apenas arquivos com determinado padrão:

```bash
./bgi_converter -b SGTitle
```

---

## 🧠 Como Funciona

1. O programa detecta automaticamente se o arquivo é:
   - BMP padrão (verificando assinatura `BM`)
   - BGI custom (verificando header interno)

2. Converte todos os formatos para:
   - **RGBA 32-bit**
   - Estrutura TGA sem compressão
   - Orientação top-down

3. Para BMP:
   - Corrige alinhamento de linha (padding)
   - Inverte imagem (BMP é bottom-up)

---

## 📂 Estrutura do Projeto

```
bgi_converter/
│
├── main.cpp
└── README.md
```

---

## ⚙️ Requisitos

- C++17 ou superior
- `<filesystem>` disponível
- Sistema com suporte a arquivos binários padrão

---

## 📄 Licença

MIT License

---

## Densenvolvimento
    Durante o processo de Hacking de Subarashiki Hibi feito por mim e pelo Samns

## 👨‍💻 Autor
autor: [Samns](https://github.com/Samwns)

BGI Converter  
Ferramenta desenvolvida para manipulação e conversão de arquivos gráficos BGI.
