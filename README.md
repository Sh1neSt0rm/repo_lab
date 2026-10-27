## 1. Сравнение результатов SAST (до и после)

* **Инструменты:** `cppcheck`, `flawfinder`, `clang-tidy`.
* **До исправления:** 
  * `cppcheck` успешно выявил Use-After-Free (`CWE-416`).
  * `flawfinder` обнаружил небезопасные функции `strcpy` (`CWE-120`) и `printf` (`CWE-134`).
  * `clang-tidy` комплексно подтвердил все три уязвимости через статический анализ AST и символьное выполнение.
* **После исправления:** 
  * Замена `strcpy` на `snprintf`, устранение форматной строки и корректное обнуление указателей после `free()` привели к **полному отсутствию предупреждений** от всех трёх анализаторов.

---

## 2. Сравнение результатов SCA (до и после)

* **Инструменты:** `syft` (генератор SBOM), `grype`, `trivy`.
* **До исправления:** 
  * В SBOM зафиксированы уязвимые версии `tinyxml2/9.0.0` и `expat/2.6.0` (с более ранними индексами CVE).
  * Сканеры `grype` и `trivy` зафиксировали критические уязвимости (включая CVE-2023-34194).
* **После исправления:** 
  * Зависимости обновлены до безопасных версий (`tinyxml2/10.0.0`, `expat/2.9.0`).
  * Повторный SCA-сканирование подтвердило статус `No vulnerabilities found`.

---

## 3. Инструкция по сборке и запуску проекта

### Требования
* CMake (версия 3.15+)
* C++17 компилятор (Clang / GCC)
* Пакетный менеджер Conan (`pip install conan`)

### Сборка и запуск уязвимой версии
```bash
cd vulnerable_app
conan profile detect --force
conan lock create conanfile.txt --lockfile-out=conan.lock
conan install conanfile.txt --output-folder=build --build=missing -s build_type=Debug
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/app valid.xml
Сборка и запуск исправленной версии
Bash
cd fixed_app
conan profile detect --force
conan lock create conanfile.txt --lockfile-out=conan.lock
conan install conanfile.txt --output-folder=build --build=missing -s build_type=Debug
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/app valid.xml