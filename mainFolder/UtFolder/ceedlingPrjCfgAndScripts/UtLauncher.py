# my_script.py
import sys
import re

def extract_between_test_and_c(filename: str) -> str:
    """
    Extracts the text between 'TEST_' and '.c' from a given filename string.
    Example: 'test/TEST_func.c' -> 'func'
    """
    match = re.search(r'TEST_(.+)\.c$', filename)
    if match:
        return match.group(1)
    return ""

# Example usage
s = "test/TEST_func.c"
result = extract_between_test_and_c(s)
print(result)  # Output: func


def main():
    # Name of the file to create
    filename = "fileTest.txt"
    
    # Check if an argument was passed
    if len(sys.argv) > 1:
        arg = sys.argv[1]
    else:
        arg = "No argument provided"
    
    # Content to write into the file
    content = f"\n ========= UPDATE THE FUNCTION ========= {extract_between_test_and_c(arg)}\n"
    
    # Open the file in write mode and write content
    with open(filename, "w") as f:
        f.write(content)
    
    print(content)

if __name__ == "__main__":
    main()
