import clang.cindex
import os

# We need to explicitly set library path if not found, but it usually works on pip install.
# If it fails, we will catch it.

def find_declarations(node, filepath, functions, variables):
    if node.location.file and node.location.file.name == filepath:
        if node.kind in [clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.CXX_METHOD, clang.cindex.CursorKind.CONSTRUCTOR, clang.cindex.CursorKind.DESTRUCTOR, clang.cindex.CursorKind.FUNCTION_TEMPLATE]:
            functions.append(node)
        elif node.kind in [clang.cindex.CursorKind.VAR_DECL, clang.cindex.CursorKind.FIELD_DECL]:
            variables.append(node)

    for child in node.get_children():
        find_declarations(child, filepath, functions, variables)

def process_file(filepath):
    try:
        index = clang.cindex.Index.create()
    except Exception as e:
        print(f"Error creating clang index: {e}")
        return

    args = ['-x', 'c++', '-std=c++20', '-Iinclude', '-Isrc', '-Wno-everything']
    try:
        tu = index.parse(filepath, args=args, options=clang.cindex.TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)
    except clang.cindex.TranslationUnitLoadError:
        print(f"Failed to parse {filepath}")
        return

    functions = []
    variables = []
    find_declarations(tu.cursor, filepath, functions, variables)
    
    with open(filepath, 'r') as f:
        lines = f.readlines()

    # Create a list of insertions: (line_num, is_function, text_or_lines)
    # Ensure no duplicates at the exact same line
    insertions = []
    
    # Keep track of modified lines to avoid double-insertion
    func_lines = set()
    var_lines = set()

    for f_cursor in functions:
        line_num = f_cursor.location.line
        if line_num in func_lines:
            continue
            
        # Check if already commented
        has_comment = False
        # Look up to 3 lines above for a comment block
        for offset in range(1, min(4, line_num)):
            prev_line = lines[line_num - offset - 1].strip()
            if prev_line.startswith('///') or prev_line.endswith('*/') or prev_line.startswith('*') or prev_line.startswith('//'):
                has_comment = True
                break
                
        if has_comment:
            continue
            
        func_lines.add(line_num)
        
        # Build comment
        col = f_cursor.location.column
        indent = " " * (col - 1) if col > 1 else ""
        doc = []
        name = f_cursor.spelling or "Function"
        doc.append(f"{indent}/**\n")
        doc.append(f"{indent} * @brief Auto-generated description for {name}.\n")
        
        # Use get_arguments() instead of get_args() if it's available
        for arg in f_cursor.get_arguments():
            arg_name = arg.spelling if arg.spelling else "param"
            doc.append(f"{indent} * @param {arg_name} Parameter description.\n")
                
        try:
            ret_type = f_cursor.result_type.spelling
            if ret_type and ret_type != 'void':
                doc.append(f"{indent} * @return {ret_type} value.\n")
        except Exception:
            pass # Constructors/Destructors might not have result_type
            
        doc.append(f"{indent} */\n")
        insertions.append((line_num, True, doc))
        
    for v_cursor in variables:
        line_num = v_cursor.location.line
        if line_num in var_lines or line_num > len(lines):
            continue
            
        # Check if already has inline comment
        line = lines[line_num - 1]
        if '///<' not in line and '//' not in line and '/*' not in line:
            var_lines.add(line_num)
            comment = f" ///< {v_cursor.type.spelling} variable representing {v_cursor.spelling}."
            insertions.append((line_num, False, comment))

    # Sort insertions by line number descending
    insertions.sort(key=lambda x: x[0], reverse=True)
    
    for line_num, is_function, payload in insertions:
        idx = line_num - 1
        if is_function:
            lines = lines[:idx] + payload + lines[idx:]
        else:
            lines[idx] = lines[idx].rstrip('\r\n') + payload + '\n'
            
    with open(filepath, 'w') as f:
        f.writelines(lines)

for root, dirs, files in os.walk('.'):
    if 'third_party' in root or 'build' in root or '.git' in root or 'venv' in root or 'FE900_Thesis' in root:
        continue
    for file in files:
        if file.endswith('.cpp') or file.endswith('.hpp'):
            filepath = os.path.join(root, file)
            print(f"Processing {filepath}...")
            process_file(filepath)

print("Doxygen documentation generated using libclang AST.")
