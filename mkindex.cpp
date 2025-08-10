#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

struct HtmlFile {
    std::string filename;
    std::string title;
    std::string description;
};

void printHelp() {
    std::cout << "Uso: mkindex [OPCIONES]\n"
              << "Genera un índice de documentos exacto con títulos y descripciones.\n\n"
              << "Opciones:\n"
              << "  -h, --help            Muestra esta ayuda\n"
              << "  -p, --path RUTA        Directorio con archivos HTML\n"
              << "  -o, --output RUTA      Directorio de salida para index.html\n\n"
              << "Ejemplos:\n"
              << "  mkindex --path ./docs --output ./indice\n"
              << "  mkindex -p ./html -o .\n";
}

std::string cleanContent(std::string content) {
    // Eliminar espacios iniciales/finales
    content.erase(0, content.find_first_not_of(" \t\n\r\f\v"));
    content.erase(content.find_last_not_of(" \t\n\r\f\v") + 1);
    
    // Eliminar etiquetas HTML internas
    size_t tag_start = content.find("<");
    while (tag_start != std::string::npos) {
        size_t tag_end = content.find(">", tag_start);
        if (tag_end == std::string::npos) break;
        content.erase(tag_start, tag_end - tag_start + 1);
        tag_start = content.find("<");
    }
    
    // Reemplazar &nbsp; y otras entidades
    size_t entity = content.find("&");
    while (entity != std::string::npos) {
        size_t end = content.find(";", entity);
        if (end != std::string::npos) {
            content.replace(entity, end - entity + 1, " ");
        }
        entity = content.find("&", entity + 1);
    }
    
    // Reemplazar múltiples espacios con uno solo
    std::string::iterator new_end = std::unique(content.begin(), content.end(),
        [](char lhs, char rhs) { return (lhs == rhs) && (lhs == ' '); });
    content.erase(new_end, content.end());
    
    return content;
}

std::pair<std::string, std::string> extractTitleAndDescription(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::string fallback = fs::path(filepath).stem().string();
        return {fallback, "Descripción no disponible"};
    }

    std::string content((std::istreambuf_iterator<char>(file)), 
                        std::istreambuf_iterator<char>());
    file.close();

    bool found_h1 = false;
    std::string title, description;

    size_t pos = 0;
    while (pos < content.length()) {
        if (!found_h1) {
            // Buscar el primer <h1>
            size_t h1_start = content.find("<h1", pos);
            if (h1_start != std::string::npos) {
                size_t h1_end = content.find("</h1>", h1_start);
                if (h1_end != std::string::npos) {
                    size_t content_start = content.find(">", h1_start) + 1;
                    title = content.substr(content_start, h1_end - content_start);
                    title = cleanContent(title);
                    found_h1 = true;
                    pos = h1_end + 5; // Avanzar más allá del </h1>
                    continue;
                }
            }
        } else {
            // Buscar el primer <p> después del <h1>
            size_t p_start = content.find("<p", pos);
            if (p_start != std::string::npos) {
                size_t p_end = content.find("</p>", p_start);
                if (p_end != std::string::npos) {
                    size_t content_start = content.find(">", p_start) + 1;
                    description = content.substr(content_start, p_end - content_start);
                    description = cleanContent(description);
                    
                    // Limitar descripción a 160 caracteres
                    if (description.length() > 160) {
                        description = description.substr(0, 157) + "...";
                    }
                    break;
                }
            }
        }
        pos++;
    }

    if (title.empty()) title = fs::path(filepath).stem().string();
    if (description.empty()) description = "Descripción no disponible";

    return {title, description};
}

void createDirectory(const std::string& path) {
    if (!fs::exists(path)) {
        fs::create_directories(path);
    }
}

std::string getCurrentDate() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%d/%m/%Y a las %H:%M", ltm);
    return std::string(buffer);
}

void generateIndex(const std::string& inputPath, const std::string& outputPath) {
    std::vector<HtmlFile> files;
    
    // Procesar archivos HTML
    for (const auto& entry : fs::directory_iterator(inputPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".html" && 
            entry.path().filename() != "index.html") {
            auto [title, description] = extractTitleAndDescription(entry.path().string());
            files.push_back({
                entry.path().filename().string(),
                title,
                description
            });
        }
    }
    
    // Ordenar alfabéticamente por título
    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return a.title < b.title;
    });
    
    // Crear directorio de salida si no existe
    createDirectory(outputPath);
    std::string outputFile = (fs::path(outputPath) / "index.html").string();
    
    // Generar archivo HTML
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Error: No se pudo crear " << outputFile << std::endl;
        return;
    }
    
    out << R"(<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Índice de Documentos</title>
    <style>
        :root {
            --primary-color: #1a0dab;
            --visited-color: #681da8;
            --url-color: #006621;
            --text-color: #202124;
            --description-color: #4d5156;
            --border-color: #dadce0;
            --bg-color: #ffffff;
        }
        
        body {
            font-family: 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', sans-serif;
            line-height: 1.6;
            color: var(--text-color);
            background-color: var(--bg-color);
            max-width: 900px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            margin-bottom: 30px;
            padding-bottom: 15px;
            border-bottom: 1px solid var(--border-color);
        }
        
        h1 {
            color: var(--primary-color);
            font-size: 24px;
            font-weight: 400;
            margin-bottom: 10px;
        }
        
        .result {
            margin-bottom: 28px;
        }
        
        .result-title {
            font-size: 20px;
            font-weight: 400;
            margin: 0;
            padding: 0;
        }
        
        .result-title a {
            color: var(--primary-color);
            text-decoration: none;
        }
        
        .result-title a:visited {
            color: var(--visited-color);
        }
        
        .result-title a:hover {
            text-decoration: underline;
        }
        
        .result-url {
            color: var(--url-color);
            font-size: 14px;
            margin: 3px 0;
            font-family: monospace;
        }
        
        .result-description {
            color: var(--description-color);
            font-size: 14px;
            margin: 5px 0 0 0;
        }
        
        footer {
            margin-top: 40px;
            padding-top: 15px;
            border-top: 1px solid var(--border-color);
            color: var(--description-color);
            font-size: 13px;
            text-align: center;
        }
        
        @media (max-width: 768px) {
            body {
                padding: 15px;
            }
            
            .result {
                margin-bottom: 22px;
            }
            
            .result-title {
                font-size: 18px;
            }
        }
    </style>
</head>
<body>
    <header>
        <h1>Índice de Documentos</h1>
    </header>
    
    <main>
)";
    
    for (const auto& file : files) {
        out << "        <div class=\"result\">\n"
            << "            <h2 class=\"result-title\"><a href=\"" << file.filename << "\">" << file.title << "</a></h2>\n"
            << "            <div class=\"result-url\">" << file.filename << "</div>\n"
            << "            <p class=\"result-description\">" << file.description << "</p>\n"
            << "        </div>\n";
    }
    
    out << R"(    </main>
    
    <footer>
        Índice generado automáticamente el )" << getCurrentDate() << R"(
    </footer>
</body>
</html>
)";
    
    std::cout << "Índice generado exitosamente en: " << outputFile << std::endl;
}

int main(int argc, char* argv[]) {
    std::string inputPath, outputPath = ".";
    
    if (argc == 1) {
        printHelp();
        return 1;
    }
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-p" || arg == "--path") {
            if (i + 1 < argc) {
                inputPath = argv[++i];
            } else {
                std::cerr << "Error: Falta la ruta después de " << arg << std::endl;
                return 1;
            }
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            } else {
                std::cerr << "Error: Falta la ruta después de " << arg << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Error: Argumento desconocido " << arg << std::endl;
            printHelp();
            return 1;
        }
    }
    
    if (inputPath.empty()) {
        std::cerr << "Error: Debes especificar una ruta con --path" << std::endl;
        printHelp();
        return 1;
    }
    
    if (!fs::exists(inputPath)) {
        std::cerr << "Error: El directorio no existe: " << inputPath << std::endl;
        return 1;
    }
    
    if (outputPath == "." || outputPath == "./") {
        outputPath = fs::current_path().string();
    }
    
    generateIndex(inputPath, outputPath);
    
    return 0;
}
