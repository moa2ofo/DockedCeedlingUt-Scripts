# -*- coding: utf-8 -*-
"""
Created on Thu Feb 20 18:37:13 2025

@author: moa2ofo
"""

import os
import re
import sys
import shutil
import subprocess


UNIT_EXECUTION_FOLDER = "utExecutionAndResults/utUnderTest"


# Lista dei file da escludere dalla copia
EXCLUDED_FILES = {"unity.h", "unity.c", "cmock.c", "cmock.h", "unity_internals.h", "cmock_internals.h"}
# ==============================
# CONFIGURATION & MODULE LIST
# ==============================
moduli = [
	{"nome_modulo": "func11_do_work.c", "nome_funzione": "func11_do_work", "percorso": "../../codeBase/folder/src"},
    {"nome_modulo": "func_do_work.c", "nome_funzione": "func_do_work", "percorso": "../../codeBase/src"},
]

# ==============================
# PROJECT STRUCTURE SETUP
# ==============================
def setup_project_structure(base_folder, name):
    """
    Creates a 'TEST_<name>' directory with 'src' and 'test' subdirectories.
    """
    test_folder_path = os.path.join(base_folder, f"TEST_{name}")
    src_folder = os.path.join(test_folder_path, "src")
    test_folder = os.path.join(test_folder_path, "test")
    
    os.makedirs(src_folder, exist_ok=True)
    os.makedirs(test_folder, exist_ok=True)
    
    c_file_path = os.path.join(src_folder, f"{name}.c")
    h_file_path = os.path.join(src_folder, f"{name}.h")
    test_c_file_path = os.path.join(test_folder, f"test_{name}.c")

    if not os.listdir(src_folder):
        with open(c_file_path, "w", encoding="utf-8") as c_file:
            c_file.write(f'#include "{name}.h"\n\n/* FUNCTION TO TEST */')
        with open(h_file_path, "w", encoding="utf-8") as h_file:
            h_file.write(f"#ifndef {name.upper()}_H\n#define {name.upper()}_H\n\n#endif\n")
        print(f"✅ Created {c_file_path} and {h_file_path}")
    
    if not os.listdir(test_folder):
        with open(test_c_file_path, "w", encoding="utf-8") as test_file:
            test_file.write(f'#include "{name}.h"\n\n// Test function\n\n')
        print(f"✅ Created {test_c_file_path}")


# ==============================
# FUNCTION EXTRACTION
# ==============================
def find_and_extract_function(file_name, function_name, unitPath):
    """
    Cerca un file in una directory e sottodirectory, estrae una funzione C e
    restituisce una versione "normalizzata" della funzione, contenente solo:

        <return_type> <function_name>(<params>) { ... }

    Rimuove qualificatori/attributi come:
    static, inline, extern, volatile, register, constexpr, __inline__,
    __forceinline, __attribute__((...)), ecc.
    """
    file_path = None
    print(f"unitPath : {unitPath}");
    print(f"file_name : {file_name}");
    print(f"function_name : {function_name}");
    # Cerca il file nei sottodirectory
    for root, _, files in os.walk(unitPath):
        if file_name in files:
            file_path = os.path.join(root, file_name)
            break

    if not file_path:
        return f"❌ Error: File '{file_name}' not found in directory '{unitPath}'."

    try:
        with open(file_path, "r", encoding="utf-8") as file:
            content = file.read()

        # Regex:
        # - ancora all'inizio riga (^)
        # - cattura tutto ciò che sta prima del nome funzione in "before"
        #   (tipo di ritorno + qualificatori + attributi)
        # - poi il nome funzione
        # - parametri
        # - eventuali __attribute__ dopo i parametri
        function_pattern = re.compile(
            rf"""
            (?P<header>
                ^[ \t]*
                (?P<before>[^\n]*?)          # return type + qualifiers + attributes (tutto sulla riga)
                \b{function_name}\b          # nome funzione
                \s*
                (?P<params>\([^)]*\))        # lista parametri
                (?P<post_attr>               # eventuali __attribute__ dopo i parametri
                    (?:\s*__attribute__\s*\(\([^)]*\)\))*
                )
            )
            \s*\{{                           # graffa di apertura del corpo
            """,
            re.MULTILINE | re.VERBOSE
        )

        match = function_pattern.search(content)

        if not match:
            return f"⚠️ Function '{function_name}' not found in '{file_name}'."

        header_start = match.start("header")

        # posizione della prima '{' dopo l'header
        brace_index = content.find("{", match.end("header"))
        if brace_index == -1:
            return f"⚠️ Opening brace for function '{function_name}' not found in '{file_name}'."

        # Troviamo la fine della funzione contando le graffe
        open_braces = 0
        end_index = None
        for i in range(brace_index, len(content)):
            char = content[i]
            if char == "{":
                open_braces += 1
            elif char == "}":
                open_braces -= 1
                if open_braces == 0:
                    end_index = i
                    break

        if end_index is None:
            return f"⚠️ Closing brace for function '{function_name}' not found in '{file_name}'."

        # ----- Normalizzazione dell'header -----
        before = match.group("before") or ""
        params = match.group("params")

        # 1) Rimuovi tutti gli __attribute__((...))
        before_clean = re.sub(r'__attribute__\s*\(\([^)]*\)\)\s*', ' ', before)

        # 2) Rimuovi i qualificatori tipo static, inline, extern, ecc.
        before_clean = re.sub(
            r'\b(static|inline|INLINE|extern|constexpr|volatile|register|__inline__|__forceinline)\b',
            ' ',
            before_clean
        )

        # 3) Normalizza whitespace
        before_clean = before_clean.replace('\t', ' ')
        return_type = ' '.join(before_clean.split())

        # Se per qualche motivo è vuoto, almeno non generiamo una cosa strana
        if not return_type:
            return_type = "void"  # fallback conservativo

        # Header "pulito"
        clean_header = f"{return_type} {function_name}{params}"

        # Corpo originale da '{' fino alla '}' corrispondente (inclusa)
        body_part = content[brace_index:end_index + 1]

        # Risultato finale: header pulito + corpo originale
        normalized_function = clean_header + " " + body_part
        print(f"FUNCTION BODY  \n\n{normalized_function}")
        return f"\n\n{normalized_function}"

    except Exception as e:
        return f"❌ Error reading file: {e}"



# ==============================
# MODIFY FILE AFTER MARKER
# ==============================
def modify_file_after_marker(file_path, new_content):
    """
    Replaces content in the file after a specific marker.
    """
    marker = "/* FUNCTION TO TEST */"
    try:
        with open(file_path, "r", encoding="utf-8") as file:
            content = file.read()
        
        marker_index = content.find(marker)
        if marker_index == -1:
            print("⚠️ Marker not found in file.")
            return
        
        modified_content = content[:marker_index + len(marker)] + "\n" + new_content + "\n"
        
        with open(file_path, "w", encoding="utf-8") as file:
            file.write(modified_content)
        print(f"✅ File '{file_path}' successfully updated.")
    except FileNotFoundError:
        print(f"❌ Error: File '{file_path}' not found.")
    except Exception as e:
        print(f"❌ Error modifying file: {e}")

def extract_function_name(path: str) -> str:
    # Get the filename only (e.g. TEST_func.c)
    filename = os.path.basename(path)
    # Remove extension (.c)
    name_no_ext = os.path.splitext(filename)[0]
    # Strip the TEST_ prefix
    if name_no_ext.startswith("TEST_"):
        return name_no_ext[len("TEST_"):]
    return name_no_ext           

def copy_folder_contents(src_folder: str, dest_folder: str):
    """
    Copy all files and subdirectories from src_folder into dest_folder.
    Creates dest_folder if it does not exist.
    """
    print(f"src_folder:{src_folder},dest_folder:{dest_folder}")
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)

    for item in os.listdir(src_folder):
        src_path = os.path.join(src_folder, item)
        dest_path = os.path.join(dest_folder, item)

        if os.path.isdir(src_path):
            # Copy subdirectory recursively
            shutil.copytree(src_path, dest_path, dirs_exist_ok=True)
        else:
            # Copy single file
            shutil.copy2(src_path, dest_path)
# ==============================
# MAIN EXECUTION
# ==============================
def updateUnitUnderTest(selected,argument):
    if not selected:
        print(f"No entry found with nome_funzione = {argument}")
        sys.exit(1)

    base_directory = "../"
    for modulo in selected:
        module_name   = modulo["nome_modulo"]
        function_name = modulo["nome_funzione"]
        percorso      = modulo["percorso"]

        extracted_body = find_and_extract_function(module_name, function_name, percorso)

        if extracted_body:
            # ✅ Creiamo la struttura di progetto se non esiste
            setup_project_structure(base_directory, function_name)
            modify_file_after_marker(
                os.path.join(base_directory, f"TEST_{function_name}", "src", f"{function_name}.c"),
                extracted_body,
            )
    copy_folder_contents("../TEST_"+argument,UNIT_EXECUTION_FOLDER)



def run_ceedling_tests():
    docker_cmd = [
        "docker", "run", "-it", "--rm",
        "-v", "/c/Ceedling/DockedCeedlingUt-Scripts/mainFolder/UtFolder/ceedlingPrjCfgAndScripts:/home/dev/project",
        "throwtheswitch/madsciencelab-plugins:latest",
        "ceedling", "test:all"
    ]

    try:
        subprocess.run(docker_cmd, check=True)
    except subprocess.CalledProcessError as e:
        print("❌ Error running Ceedling tests in Docker:", e)
    
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python script.py <function_name>")
        sys.exit(1)

    argument = extract_function_name(sys.argv[1])
    print(f"You passed the argument: {argument}")
    # Filter moduli to only those with matching nome_funzione
    selected = [m for m in moduli if m["nome_funzione"] == argument]
    updateUnitUnderTest(selected,argument)
    run_ceedling_tests()

            
   
