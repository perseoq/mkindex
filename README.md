# mkindex

## Descripción

`mkindex` es una herramienta de línea de comandos escrita en C++ que genera automáticamente un archivo `index.html` con un índice de todos los documentos HTML encontrados en un directorio especificado. Para cada documento, extrae el título (del primer `<h1>`) y una descripción (del primer `<p>` después del `<h1>`), creando una lista ordenada alfabéticamente con enlaces a los documentos.

## Características principales

- Extrae títulos de documentos HTML (etiquetas `<h1>`)
- Genera descripciones a partir del primer párrafo (`<p>`) después del título
- Ordena los resultados alfabéticamente por título
- Crea un archivo `index.html` con diseño responsive y estilo similar a motores de búsqueda
- Incluye metadatos como fecha de generación automática

## Requisitos

- Compilador C++ compatible con C++17 (g++ 9+ o clang 10+)
- Sistema operativo Linux/macOS (probado en Ubuntu 20.04+ y macOS 10.15+)
- Biblioteca estándar de filesystem (incluida en C++17)

## Instalación

1. Clona el repositorio o descarga el archivo `mkindex.cpp`
2. Compila con:

```bash
g++ -std=c++17 mkindex.cpp -o mkindex -lstdc++fs
```

3. Mueve el ejecutable a tu PATH (opcional):

```bash
sudo mv mkindex /usr/local/bin/
```

## Uso básico

```bash
./mkindex -p [directorio_con_html] -o [directorio_salida]
```

Ejemplo:

```bash
./mkindex --path ./documentos --output ./public
```

## Opciones

| Opción          | Descripción                              |
|-----------------|------------------------------------------|
| `-h, --help`    | Muestra la ayuda                         |
| `-p, --path`    | Directorio con archivos HTML a indexar    |
| `-o, --output`  | Directorio de salida para index.html      |

## Limitaciones conocidas

- Solo procesa archivos con extensión `.html`
- No indexa archivos llamados `index.html` (para evitar recursión)
- La extracción de texto asume una estructura HTML semántica básica
