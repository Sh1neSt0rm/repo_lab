#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <tinyxml2.h>
#include <expat.h>

using namespace tinyxml2;

void process_xml(const std::string& xml_content) {
    XML_Parser expat_parser = XML_ParserCreate(NULL);
    if (expat_parser) {
        XML_Parse(expat_parser, xml_content.c_str(), static_cast<int>(xml_content.size()), XML_TRUE);
        XML_ParserFree(expat_parser);
    }

    XMLDocument doc;
    XMLError err = doc.Parse(xml_content.c_str());
    if (err != XML_SUCCESS) {
        std::cerr << "Error parsing XML: " << doc.ErrorStr() << std::endl;
        return;
    }

    XMLElement* root = doc.FirstChildElement("config");
    if (!root) {
        std::cerr << "No <config> root element found." << std::endl;
        return;
    }

    XMLElement* user = root->FirstChildElement("user");
    if (user) {
        const char* name = user->Attribute("name");
        const char* role = user->Attribute("role");

        if (name) {
            // ИСПРАВЛЕНО (CWE-120): Использование snprintf вместо strcpy с защитой от переполнения
            char username_buf[16];
            snprintf(username_buf, sizeof(username_buf), "%s", name);
            std::cout << "User loaded: " << username_buf << std::endl;
        }

        if (role) {
            // ИСПРАВЛЕНО (CWE-134): Безопасный вывод строки без форматной уязвимости
            std::cout << "Assigned role: " << role << std::endl;
        }
    }

    XMLElement* comment = root->FirstChildElement("comment");
    if (comment && comment->GetText()) {
        const char* text = comment->GetText();
        char* note_buf = (char*)malloc(64);
        if (note_buf) {
            snprintf(note_buf, 64, "%s", text);

            // ИСПРАВЛЕНО (CWE-416): Устранено Use-After-Free и двойное освобождение памяти
            if (strstr(note_buf, "drop") != nullptr) {
                std::cout << "Comment note (dropped): contains drop keyword." << std::endl;
                free(note_buf);
                note_buf = nullptr; // Обнуляем указатель после освобождения
            } else {
                std::cout << "Comment note: " << note_buf << std::endl;
                free(note_buf);
                note_buf = nullptr;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    std::stringstream buffer;
    if (argc > 1) {
        std::ifstream file(argv[1]);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
            return 1;
        }
        buffer << file.rdbuf();
    } else {
        buffer << std::cin.rdbuf();
    }

    process_xml(buffer.str());
    return 0;
}
