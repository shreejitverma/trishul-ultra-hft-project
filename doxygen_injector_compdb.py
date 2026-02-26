import clang.cindex
import os

def find_declarations(node, filepath, functions, variables):
    if node.location.file and os.path.abspath(node.location.file.name) == os.path.abspath(filepath):
        if node.kind in [clang.cindex.CursorKind.FUNCTION_DECL, clang.cindex.CursorKind.CXX_METHOD, clang.cindex.CursorKind.CONSTRUCTOR, clang.cindex.CursorKind.DESTRUCTOR, clang.cindex.CursorKind.FUNCTION_TEMPLATE]:
            functions.append(node)
        elif node.kind in [clang.cindex.CursorKind.VAR_DECL, clang.cindex.CursorKind.FIELD_DECL]:
            variables.append(node)

    for child in node.get_children():
        find_declarations(child, filepath, functions, variables)

def process_file(filepath, compdb):
    try:
        index = clang.cindex.Index.create()
    except Exception as e:
        print(f"Error creating clang index: {e}")
        return

    # Try to find compile command
    args = ['-x', 'c++', '-std=c++20', '-Iinclude', '-Isrc', '-Wno-everything']
    if compdb:
        cmds_iter = compdb.getCompileCommands(os.path.abspath(filepath))
        if cmds_iter:
            cmds = list(cmds_iter)
            if len(cmds) > 0:
                args = []
                cmd_args = list(cmds[0].arguments)
                for i in range(1, len(cmd_args)):
                    args.append(cmd_args[i])
    
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

    insertions = []
    func_lines = set()
    var_lines = set()

    for f_cursor in functions:
        line_num = f_cursor.location.line
        if line_num in func_lines:
            continue
            
        has_comment = False
        for offset in range(1, min(4, line_num)):
            prev_line = lines[line_num - offset - 1].strip()
            if prev_line.startswith('///') or prev_line.endswith('*/') or prev_line.startswith('*') or prev_line.startswith('//'):
                has_comment = True
                break
                
        if has_comment:
            continue
            
        func_lines.add(line_num)
        
        col = f_cursor.location.column
        indent = " " * (col - 1) if col > 1 else ""
        doc = []
        name = f_cursor.spelling or "Function"
        doc.append(indent + "/**\n")
        doc.append(indent + " * @brief Auto-generated description for " + name + ".\n")
        
        for arg in f_cursor.get_arguments():
            arg_name = arg.spelling if arg.spelling else "param"
            doc.append(indent + " * @param " + arg_name + " Parameter description.\n")
                
        try:
            ret_type = f_cursor.result_type.spelling
            if ret_type and ret_type != 'void':
                doc.append(indent + " * @return Returns " + ret_type + " value.\n")
        except Exception:
            pass 
            
        doc.append(indent + " */\n")
        insertions.append((line_num, True, doc))
        
    for v_cursor in variables:
        line_num = v_cursor.location.line
        if line_num in var_lines or line_num > len(lines):
            continue
            
        line = lines[line_num - 1]
        if '///<' not in line and '//' not in line and '/*' not in line:
            var_lines.add(line_num)
            type_str = v_cursor.type.spelling
            if not type_str or type_str == 'int': # fallback if unresolved
                type_str = "auto"
            comment = " ///< " + type_str + " variable representing " + v_cursor.spelling + "."
            insertions.append((line_num, False, comment))

    insertions.sort(key=lambda x: x[0], reverse=True)
    
    for line_num, is_function, payload in insertions:
        idx = line_num - 1
        if is_function:
            lines = lines[:idx] + payload + lines[idx:]
        else:
            lines[idx] = lines[idx].rstrip('\r\n') + payload + '\n'
            
    with open(filepath, 'w') as f:
        f.writelines(lines)

compdb = None
try:
    compdb = clang.cindex.CompilationDatabase.fromDirectory('build')
except clang.cindex.CompilationDatabaseError:
    print("Compilation database not found.")

for root, dirs, files in os.walk('.'):
    if 'third_party' in root or 'build' in root or '.git' in root or 'venv' in root or 'FE900_Thesis' in root:
        continue
    for file in files:
        if file.endswith('.cpp') or file.endswith('.hpp'):
            filepath = os.path.join(root, file)
            print("Processing " + filepath + "...")
            process_file(filepath, compdb)

print("Doxygen documentation generated using libclang and CompilationDatabase.")
