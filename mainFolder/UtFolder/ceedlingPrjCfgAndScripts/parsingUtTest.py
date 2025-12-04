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

UNIT_TEST_COLLECTION = "../utCollection"
UNIT_TEST_PREFIX = "TEST_"
UNIT_EXECUTION_FOLDER = "utExecutionAndResults/utUnderTest"
UNIT_EXECUTION_FOLDER_BUILD = "utExecutionAndResults/utUnderTest/build"
UNIT_RESULT_FOLDER = "utExecutionAndResults/utResults"
PROJECT = "project.yml"

# Find the position of the script removing its name and the disk
SCRIPT_PATH = os.path.abspath(__file__)
SCRIPT_DIRECTORY_PATH = os.path.dirname(SCRIPT_PATH)
RELATIVE_PATH = SCRIPT_DIRECTORY_PATH.split(":", 1)[-1].lstrip("\\/")
NORMALIZED_PATH = RELATIVE_PATH.replace("\\", "/")

DOCKER_BASE = ["docker", "run", "-it", "--rm","-v", "/c/" + NORMALIZED_PATH + ":/home/dev/project","throwtheswitch/madsciencelab-plugins:latest"]
CEEDLING_GCOV_ALL = ["ceedling", "gcov:all"]
CEEDLING_CLEAN = ["ceedling", "clean"]

DOCKER_GCOV_ALL = DOCKER_BASE + CEEDLING_GCOV_ALL
DOCKER_CLEAN = DOCKER_BASE + CEEDLING_CLEAN
# ==============================
# CONFIGURATION & MODULE LIST
# ==============================
moduli = [
	{"nome_modulo": "monitoring.c", "nome_funzione": "monitoring", "percorso": "../../codeBase/src"},
    {"nome_modulo": "func_do_work.c", "nome_funzione": "func_do_work", "percorso": "../../codeBase/src"},
]


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
    print(f"Copy data from {src_folder} to {dest_folder}")
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

def clear_folder(folder_path: str):
    """
    Delete all files and subdirectories inside the given folder.
    The folder itself is preserved.
    """
    if not os.path.exists(folder_path):
        print(f"⚠️ Folder '{folder_path}' does not exist.")
        return

    for item in os.listdir(folder_path):
        item_path = os.path.join(folder_path, item)
        try:
            if os.path.isfile(item_path) or os.path.islink(item_path):
                os.remove(item_path)   # delete file or symbolic link
            elif os.path.isdir(item_path):
                shutil.rmtree(item_path)  # delete subdirectory recursively
        except Exception as e:
            print(f"❌ Error deleting {item_path}: {e}")

    print(f"✅ Folder '{folder_path}' cleared successfully.")

def updateUnitUnderTest(unitMetaData,unitName):
    if not unitMetaData:
        print(f"No entry found with nome_funzione = {unitName}")
        sys.exit(1)


    for modulo in unitMetaData:
        module_name   = modulo["nome_modulo"]
        function_name = modulo["nome_funzione"]
        percorso      = modulo["percorso"]

        extracted_body = find_and_extract_function(module_name, function_name, percorso)

        if extracted_body:
            modify_file_after_marker(
                os.path.join(UNIT_TEST_COLLECTION, f"TEST_{function_name}", "src", f"{function_name}.c"),
                extracted_body,
            )
    clear_folder(UNIT_EXECUTION_FOLDER)
    copy_folder_contents(UNIT_TEST_COLLECTION +"/"+ UNIT_TEST_PREFIX + unitName, UNIT_EXECUTION_FOLDER)



def run_bash_cmd(cmd):
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError as e:
        print("Ceedling has failed:", e)


def print_help():
    script_name = os.path.basename(sys.argv[0])
    help_text = f"""
Usage:
  python {script_name} <function_name|all>
  python {script_name} -h | --help | help

Description:
  This script:
    - Extracts the specified C function from the code base
    - Injects it into the corresponding TEST_<function_name> unit file
      after the marker: /* FUNCTION TO TEST */
    - Copies the unit test project into:
        {UNIT_EXECUTION_FOLDER}
    - Runs Ceedling inside the Docker container
    - Collects build and coverage results into:
        {UNIT_RESULT_FOLDER}

Arguments:
  function_name        Name of the function to test.
                       It must match 'nome_funzione' in the 'moduli' dictionary.
  all                  Run the process for all modules listed in 'moduli'.

Options:
  -h, --help, help     Show this help message and exit.

Examples:
  python {script_name} monitoring
  python {script_name} all
"""
    print(help_text.strip())


if __name__ == "__main__":
    # ---- handle missing args / help ----
    if len(sys.argv) < 2:
        print_help()
        sys.exit(1)

    if sys.argv[1] in ("-h", "--help", "help"):
        print_help()
        sys.exit(0)

    unitToTest = extract_function_name(sys.argv[1])
    print(f"You passed the argument: {unitToTest}")

    if unitToTest == "all":
        # ✅ Run for all modules in the dictionary
        for module in moduli:
            function_name = module["nome_funzione"]
            print(f"▶ Processing unit: {function_name}")
            updateUnitUnderTest([module], function_name)

            run_bash_cmd(DOCKER_CLEAN)
            run_bash_cmd(DOCKER_GCOV_ALL)

            copy_folder_contents(
                UNIT_EXECUTION_FOLDER_BUILD,
                os.path.join(UNIT_RESULT_FOLDER, function_name + "Results")
            )
    else:
        # ✅ Run only for the selected unit
        unitMetaData = [m for m in moduli if m["nome_funzione"] == unitToTest]
        if not unitMetaData:
            print(f"❌ No module found for function '{unitToTest}'")
            sys.exit(1)

        updateUnitUnderTest(unitMetaData, unitToTest)
        run_bash_cmd(DOCKER_CLEAN)
        run_bash_cmd(DOCKER_BASE + ["ceedling", "gcov:" + unitToTest])
        copy_folder_contents(
            UNIT_EXECUTION_FOLDER_BUILD,
            os.path.join(UNIT_RESULT_FOLDER, unitToTest + "Results")
        )
