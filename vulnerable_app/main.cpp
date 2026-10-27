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
            // УЯЗВИМОСТЬ 1: CWE-120 / CWE-787 (Buffer Overflow)
            // Копирование строки без проверки длины в фиксированный буфер на стеке (16 байт)
            char username_buf[16];
            strcpy(username_buf, name);
            std::cout << "User loaded: " << username_buf << std::endl;
        }

        if (role) {
            // УЯЗВИМОСТЬ 2: CWE-134 (Uncontrolled Format String)
            // Передача пользовательских данных напрямую первым аргументом в printf
            std::cout << "Assigned role: ";
            printf(role);
            printf("\n");
        }
    }

    XMLElement* comment = root->FirstChildElement("comment");
    if (comment && comment->GetText()) {
        const char* text = comment->GetText();
        char* note_buf = (char*)malloc(64);
        if (note_buf) {
            strncpy(note_buf, text, 63);
            note_buf[63] = '\0';

            // Если в комментарии есть слово "drop", буфер освобождается досрочно
            if (strstr(note_buf, "drop") != nullptr) {
                free(note_buf);
            }

            // УЯЗВИМОСТЬ 3: CWE-416 (Use-After-Free)
            // Чтение из уже освобожденного указателя note_buf (и двойное освобождение)
            std::cout << "Comment note: " << note_buf << std::endl;

            if (strstr(text, "drop") == nullptr) {
                free(note_buf);
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