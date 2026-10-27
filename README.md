## 1. Сравнение результатов SAST (до и после)

* **Инструменты:** `cppcheck`, `flawfinder`, `clang-tidy`.
* **До исправления:** 
  * `cppcheck` успешно выявил Use-After-Free (`CWE-416`).
  * `flawfinder` обнаружил небезопасные функции `strcpy` (`CWE-120`) и `printf` (`CWE-134`).
  * `clang-tidy` комплексно подтвердил все три уязвимости через статический анализ AST и символьное выполнение.
* **После исправления:** 
  * Замена `strcpy` на `snprintf`, устранение форматной строки и корректное обнуление указателей после `free()` привели к отсутствию предупреждений от всех трёх анализаторов.

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
* Пакетный менеджер Conan (`pip3 install conan`)

### Сборка и запуск уязвимой версии
```bash
cd vulnerable_app
conan profile detect --force
conan lock create conanfile.txt --lockfile-out=conan.lock
conan install conanfile.txt --output-folder=build --build=missing -s build_type=Debug
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/app valid.xml
```

### Сборка и запуск исправленной версии
```bash
cd fixed_app
conan profile detect --force
conan lock create conanfile.txt --lockfile-out=conan.lock
conan install conanfile.txt --output-folder=build --build=missing -s build_type=Debug
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/app valid.xml
```

---

## 4. Проверка уязвимостей

### A. Статический анализ (SAST)
```bash
mkdir -p reports/sast_before

# 1. Cppcheck (поиск дефектов памяти и потока управления)
cppcheck --enable=all --inconclusive --std=c++17 main.cpp 2> reports/sast_before/cppcheck_report.txt

# 2. Flawfinder (сигнатурный поиск небезопасных функций вроде strcpy и printf)
flawfinder --context --minlevel=1 main.cpp > reports/sast_before/flawfinder_report.txt

# 3. Clang-Tidy (комплексный AST-анализ)
/opt/homebrew/opt/llvm/bin/clang-tidy main.cpp -p build \
  --extra-arg="-isysroot$(xcrun --show-sdk-path)" \
  -checks='bugprone-*,cert-*,clang-analyzer-*,security-*' \
  > reports/sast_before/clang_tidy_report.txt 2>&1
```

### Просмотр отчётов
```bash
cat reports/sast_before/cppcheck_report.txt
cat reports/sast_before/flawfinder_report.txt
cat reports/sast_before/clang_tidy_report.txt
```

### Б. Композиционный анализ (SCA)
```bash
mkdir -p reports/sca_before

# 1. Генерация SBOM в формате CycloneDX JSON
syft dir:. -o cyclonedx-json=reports/sca_before/sbom.cdx.json

# 2. Сканирование зависимостей через Grype (с привязкой CPE)
grype sbom:reports/sca_before/sbom.cdx.json --add-cpes-if-none > reports/sca_before/grype_report.txt

# 3. Сканирование зависимостей через Trivy
trivy sbom reports/sca_before/sbom.cdx.json > reports/sca_before/trivy_report.txt
```

### Просмотр отчётов
```bash
cat reports/sca_before/grype_report.txt
cat reports/sca_before/trivy_report.txt
```