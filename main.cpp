#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <algorithm>
#include <locale>
#include <windows.h>

namespace fs = std::filesystem;

// ============================================================================
// ESTRUTURAS DE HEADERS
// ============================================================================

#pragma pack(push, 1)
struct BGIHeader {
    uint16_t width;
    uint16_t height;
    uint32_t format;      // 0x00000020 = RGBA, outros valores = diferentes formatos
    uint32_t reserved1;
    uint32_t reserved2;
};

struct BMPHeader {
    uint16_t signature;   // 'BM'
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
};

struct BMPInfoHeader {
    uint32_t header_size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pixels_per_m;
    int32_t  y_pixels_per_m;
    uint32_t colors_used;
    uint32_t important_colors;
};
#pragma pack(pop)

// ============================================================================
// DETECÇÃO DE FORMATO
// ============================================================================

enum class BGIFormat {
    UNKNOWN,
    BMP_FORMAT,      // Formato BMP padrão (24/32-bit)
    RGBA_0x20,       // Formato 0x00000020 (RGBA direto)
    RGB_0x00,        // Formato 0x00000000 (RGB sem alpha)
};

struct FileInfo {
    BGIFormat format;
    int width;
    int height;
    int bpp;
    size_t data_offset;
};

FileInfo detect_format(const std::string& filename) {
    FileInfo info;
    info.format = BGIFormat::UNKNOWN;
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) return info;
    
    // Tentar ler como BMP primeiro
    BMPHeader bmp_header;
    file.read((char*)&bmp_header, sizeof(BMPHeader));
    
    if (bmp_header.signature == 0x4D42) {  // 'BM'
        BMPInfoHeader bmp_info;
        file.read((char*)&bmp_info, sizeof(BMPInfoHeader));
        
        info.format = BGIFormat::BMP_FORMAT;
        info.width = bmp_info.width;
        info.height = abs(bmp_info.height);
        info.bpp = bmp_info.bpp;
        info.data_offset = bmp_header.data_offset;
        
        file.close();
        return info;
    }
    
    // Tentar ler como formato BGI customizado
    file.seekg(0);
    BGIHeader bgi_header;
    file.read((char*)&bgi_header, sizeof(BGIHeader));
    
    info.width = bgi_header.width;
    info.height = bgi_header.height;
    info.data_offset = sizeof(BGIHeader);
    
    // Determinar formato baseado no campo format
    if (bgi_header.format == 0x00000020) {
        info.format = BGIFormat::RGBA_0x20;
        info.bpp = 32;
    } else if (bgi_header.format == 0x00000000) {
        info.format = BGIFormat::RGB_0x00;
        info.bpp = 24;
    }
    
    file.close();
    return info;
}

// ============================================================================
// ANÁLISE DE ARQUIVO
// ============================================================================

void analyze_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "❌ Erro ao abrir: " << filename << std::endl;
        return;
    }
    
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║     ANÁLISE DE ARQUIVO BGI             ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "Arquivo: " << filename << std::endl;
    std::cout << "Tamanho: " << file_size << " bytes" << std::endl;
    
    FileInfo info = detect_format(filename);
    
    std::cout << "\n┌─ Formato Detectado ─────────────────" << std::endl;
    
    switch (info.format) {
        case BGIFormat::BMP_FORMAT:
            std::cout << "│ Tipo: BMP Padrão" << std::endl;
            std::cout << "│ Formato: " << (info.bpp == 24 ? "BGR (24-bit)" : "BGRA (32-bit)") << std::endl;
            break;
            
        case BGIFormat::RGBA_0x20:
            std::cout << "│ Tipo: BGI Customizado" << std::endl;
            std::cout << "│ Formato: RGBA (32-bit) - 0x00000020" << std::endl;
            break;
            
        case BGIFormat::RGB_0x00:
            std::cout << "│ Tipo: BGI Customizado" << std::endl;
            std::cout << "│ Formato: RGB (24-bit) - 0x00000000" << std::endl;
            break;
            
        default:
            std::cout << "│ Tipo: DESCONHECIDO" << std::endl;
    }
    
    if (info.format != BGIFormat::UNKNOWN) {
        std::cout << "│ Dimensões: " << info.width << " x " << info.height << std::endl;
        std::cout << "│ BPP: " << info.bpp << std::endl;
        std::cout << "│ Offset de dados: " << info.data_offset << std::endl;
    }
    std::cout << "└─────────────────────────────────────" << std::endl;
    
    // Mostrar hex dump dos primeiros bytes
    file.seekg(0);
    uint8_t header[64];
    file.read((char*)header, 64);
    
    std::cout << "\n┌─ Primeiros 64 bytes (HEX) ──────────" << std::endl;
    for (int i = 0; i < 64; i += 16) {
        std::cout << "│ ";
        printf("%04X: ", i);
        for (int j = 0; j < 16 && (i + j) < 64; j++) {
            printf("%02X ", header[i + j]);
        }
        std::cout << std::endl;
    }
    std::cout << "└─────────────────────────────────────" << std::endl;
    
    file.close();
}

// ============================================================================
// CONVERSÃO BGI -> TGA
// ============================================================================

bool bgi_to_tga(const std::string& input_file, const std::string& output_file) {
    FileInfo info = detect_format(input_file);
    
    if (info.format == BGIFormat::UNKNOWN) {
        std::cerr << "❌ Formato desconhecido: " << input_file << std::endl;
        return false;
    }
    
    std::ifstream inFile(input_file, std::ios::binary);
    if (!inFile) {
        std::cerr << "❌ Erro ao abrir: " << input_file << std::endl;
        return false;
    }
    
    std::cout << "┌─ Extraindo para TGA ────────────────" << std::endl;
    std::cout << "│ Entrada: " << input_file << std::endl;
    std::cout << "│ Dimensões: " << info.width << "x" << info.height << std::endl;
    std::cout << "│ Formato: ";
    
    int width = info.width;
    int height = info.height;
    std::vector<uint8_t> pixels_rgba(width * height * 4);
    
    // Ler dados baseado no formato
    inFile.seekg(info.data_offset, std::ios::beg);
    
    switch (info.format) {
        case BGIFormat::BMP_FORMAT: {
            std::cout << "BMP " << info.bpp << "-bit" << std::endl;
            int bytes_per_pixel = info.bpp / 8;
            int row_size = ((width * bytes_per_pixel + 3) / 4) * 4;
            
            for (int y = 0; y < height; y++) {
                std::vector<uint8_t> row(row_size);
                inFile.read((char*)row.data(), row_size);
                
                for (int x = 0; x < width; x++) {
                    int src = x * bytes_per_pixel;
                    int dst = (y * width + x) * 4;
                    
                    pixels_rgba[dst + 0] = row[src + 2];  // R
                    pixels_rgba[dst + 1] = row[src + 1];  // G
                    pixels_rgba[dst + 2] = row[src + 0];  // B
                    pixels_rgba[dst + 3] = (info.bpp == 32) ? row[src + 3] : 255;
                }
            }
            
            // BMP é bottom-up, inverter
            std::vector<uint8_t> flipped(width * height * 4);
            for (int y = 0; y < height; y++) {
                memcpy(&flipped[y * width * 4], 
                       &pixels_rgba[(height - 1 - y) * width * 4], 
                       width * 4);
            }
            pixels_rgba = flipped;
            break;
        }
        
        case BGIFormat::RGBA_0x20: {
            std::cout << "RGBA 32-bit (0x20)" << std::endl;
            inFile.read((char*)pixels_rgba.data(), width * height * 4);
            break;
        }
        
        case BGIFormat::RGB_0x00: {
            std::cout << "RGB 24-bit (0x00)" << std::endl;
            for (int i = 0; i < width * height; i++) {
                uint8_t rgb[3];
                inFile.read((char*)rgb, 3);
                pixels_rgba[i * 4 + 0] = rgb[0];  // R
                pixels_rgba[i * 4 + 1] = rgb[1];  // G
                pixels_rgba[i * 4 + 2] = rgb[2];  // B
                pixels_rgba[i * 4 + 3] = 255;     // A
            }
            break;
        }
        
        default:
            inFile.close();
            return false;
    }
    
    inFile.close();
    
    // Escrever TGA
    std::ofstream outFile(output_file, std::ios::binary);
    if (!outFile) {
        std::cerr << "❌ Erro ao criar: " << output_file << std::endl;
        return false;
    }
    
    uint8_t tga_header[18] = {0};
    tga_header[2] = 2;  // Uncompressed RGB
    tga_header[12] = width & 0xFF;
    tga_header[13] = (width >> 8) & 0xFF;
    tga_header[14] = height & 0xFF;
    tga_header[15] = (height >> 8) & 0xFF;
    tga_header[16] = 32;
    tga_header[17] = 0x20; // Top-down, 8-bit alpha
    
    outFile.write((char*)tga_header, 18);
    outFile.write((char*)pixels_rgba.data(), pixels_rgba.size());
    outFile.close();
    
    std::cout << "│ ✓ TGA criado: " << output_file << std::endl;
    std::cout << "└─────────────────────────────────────" << std::endl;
    
    return true;
}

// ============================================================================
// CONVERSÃO EM LOTE (PASTA ATUAL)
// ============================================================================

void batch_convert_folder(const std::string& pattern = "") {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║   CONVERSÃO EM LOTE - PASTA ATUAL      ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    
    std::vector<std::string> files;
    
    // Listar arquivos da pasta atual
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            
            // Se há padrão, filtrar
            if (!pattern.empty()) {
                if (filename.find(pattern) == std::string::npos) {
                    continue;
                }
            }
            
            // Detectar se é um arquivo BGI válido
            FileInfo info = detect_format(filename);
            if (info.format != BGIFormat::UNKNOWN) {
                files.push_back(filename);
            }
        }
    }
    
    if (files.empty()) {
        std::cout << "⚠ Nenhum arquivo BGI encontrado" << std::endl;
        if (!pattern.empty()) {
            std::cout << "  Padrão usado: " << pattern << std::endl;
        }
        return;
    }
    
    std::sort(files.begin(), files.end());
    
    std::cout << "\nArquivos encontrados: " << files.size() << std::endl;
    if (!pattern.empty()) {
        std::cout << "Padrão: " << pattern << std::endl;
    }
    std::cout << std::endl;
    
    int success = 0;
    for (size_t i = 0; i < files.size(); i++) {
        std::cout << "\n[" << (i + 1) << "/" << files.size() << "] " << std::endl;
        
        std::string output = files[i] + ".tga";
        if (bgi_to_tga(files[i], output)) {
            success++;
        }
    }
    
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║           CONVERSÃO CONCLUÍDA          ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "Convertidos: " << success << "/" << files.size() << std::endl;
}

// ============================================================================
// MENU INTERATIVO
// ============================================================================

void print_menu() {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║     BGI CONVERTER v3.0                 ║" << std::endl;
    std::cout << "║     Suporte Multi-Formato              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "\n┌─ OPÇÕES ─────────────────────────────" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ 1. Converter arquivo específico para TGA" << std::endl;
    std::cout << "│ 2. Converter todos arquivos da pasta para TGA" << std::endl;
    std::cout << "│ 3. Converter arquivos com padrão para TGA" << std::endl;
    std::cout << "│ 4. Analisar arquivo" << std::endl;
    std::cout << "│ 0. Sair" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "└─────────────────────────────────────" << std::endl;
    std::cout << "\nEscolha uma opção: ";
}

void interactive_mode() {
    while (true) {
        print_menu();
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        if (choice == 0) {
            std::cout << "\n👋 Até logo!" << std::endl;
            break;
        }
        
        switch (choice) {
            case 1: {
                std::cout << "\nDigite o nome do arquivo: ";
                std::string filename;
                std::getline(std::cin, filename);
                
                std::string output = filename + ".tga";
                std::cout << "\nDigite o nome de saída (Enter para '" << output << "'): ";
                std::string custom_output;
                std::getline(std::cin, custom_output);
                
                if (!custom_output.empty()) {
                    output = custom_output;
                }
                
                std::cout << std::endl;
                bgi_to_tga(filename, output);
                break;
            }
            
            case 2: {
                batch_convert_folder();
                break;
            }
            
            case 3: {
                std::cout << "\nDigite o padrão (ex: SGTitle): ";
                std::string pattern;
                std::getline(std::cin, pattern);
                
                batch_convert_folder(pattern);
                break;
            }
            
            case 4: {
                std::cout << "\nDigite o nome do arquivo: ";
                std::string filename;
                std::getline(std::cin, filename);
                
                analyze_file(filename);
                break;
            }
            
            default:
                std::cout << "\n❌ Opção inválida!" << std::endl;
        }
        
        std::cout << "\nPressione Enter para continuar...";
        std::cin.get();
    }
}

// ============================================================================
// MAIN
// ============================================================================

void print_usage() {
    std::cout << "╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║     BGI CONVERTER v3.0                 ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "\nFormatos suportados:" << std::endl;
    std::cout << "  • BMP 24-bit (BGR)" << std::endl;
    std::cout << "  • BMP 32-bit (BGRA)" << std::endl;
    std::cout << "  • BGI RGBA (0x00000020)" << std::endl;
    std::cout << "  • BGI RGB (0x00000000)" << std::endl;
    std::cout << "\nModos de uso:" << std::endl;
    std::cout << "  ./bgi_converter                    - Modo interativo" << std::endl;
    std::cout << "  ./bgi_converter -a arquivo         - Analisar arquivo" << std::endl;
    std::cout << "  ./bgi_converter -x arquivo [saida] - Converter para TGA" << std::endl;
    std::cout << "  ./bgi_converter -b [padrão]        - Converter pasta (opcional: filtro)" << std::endl;
    std::cout << "\nExemplos:" << std::endl;
    std::cout << "  ./bgi_converter -a SGTitle000300" << std::endl;
    std::cout << "  ./bgi_converter -x SGTitle000300 titulo.tga" << std::endl;
    std::cout << "  ./bgi_converter -x SGTitle000300               (saída: SGTitle000300.tga)" << std::endl;
    std::cout << "  ./bgi_converter -b                             (todos arquivos)" << std::endl;
    std::cout << "  ./bgi_converter -b SGTitle                     (apenas SGTitle*)" << std::endl;
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    if (argc == 1) {
        interactive_mode();
        return 0;
    }
    
    std::string mode = argv[1];
    
    if (mode == "-a" || mode == "--analyze") {
        if (argc != 3) {
            std::cerr << "Uso: bgi_converter -a arquivo" << std::endl;
            return 1;
        }
        analyze_file(argv[2]);
        return 0;
    }
    else if (mode == "-x" || mode == "--extract") {
        if (argc < 3) {
            std::cerr << "Uso: bgi_converter -x arquivo [saida.tga]" << std::endl;
            return 1;
        }
        
        std::string output;
        if (argc >= 4) {
            output = argv[3];
        } else {
            output = std::string(argv[2]) + ".tga";
        }
        
        return bgi_to_tga(argv[2], output) ? 0 : 1;
    }
    else if (mode == "-b" || mode == "--batch") {
        std::string pattern = (argc >= 3) ? argv[2] : "";
        batch_convert_folder(pattern);
        return 0;
    }
    else if (mode == "-h" || mode == "--help") {
        print_usage();
        return 0;
    }
    else {
        std::cerr << "❌ Modo inválido!" << std::endl;
        print_usage();
        return 1;
    }
}
