import os
import re

def process_file(filepath):
    with open(filepath, 'r') as f:
        lines = f.readlines()
        
    out_lines = []
    brace_depth = 0
    in_comment = False
    
    # Regexes
    # Matches a typical function signature: return_type func_name(args)
    # Allows macros like ULTRA_ALWAYS_INLINE, etc.
    func_pattern = re.compile(r'^(\s*)(?:(?:inline|static|virtual|explicit|constexpr|ULTRA_ALWAYS_INLINE)\s+)*([\w:<>_]+(?:\s*[\*&]+)?\s+)?([\w_~]+)\s*\((.*?)\)\s*(?:const|noexcept|override|final|volatile)*\s*(?:\{|;)')
    
    # Matches a variable declaration: Type name = ...; or Type name;
    var_pattern = re.compile(r'^(\s*)(?:(?:static|constexpr|const|mutable|thread_local)\s+)*([\w:<>_]+(?:\s*[\*&]+)?)\s+([\w_]+)(?:\s*(?:=|\[|\{).*)?;$')
    
    i = 0
    while i < len(lines):
        line = lines[i]
        stripped = line.strip()
        
        # Track multiline comments
        if '/*' in stripped and not in_comment:
            if '*/' not in stripped:
                in_comment = True
        elif '*/' in stripped and in_comment:
            in_comment = False
            
        if not in_comment and not stripped.startswith('//') and not stripped.startswith('#') and stripped:
            # Check function
            func_match = func_pattern.match(line)
            if func_match and 'operator' not in line and not line.startswith('}'):
                indent, ret_type, func_name, args = func_match.groups()
                
                # Check if it already has a comment
                has_doc = False
                if i > 0 and ('///' in lines[i-1] or '*/' in lines[i-1] or '//' in lines[i-1] or '*' in lines[i-1]):
                    has_doc = True
                
                if not has_doc and func_name not in ['if', 'while', 'for', 'switch', 'catch', 'sizeof']:
                    doc_lines = []
                    doc_lines.append(f"{indent}/**\n")
                    doc_lines.append(f"{indent} * @brief Auto-generated description for {func_name}.\n")
                    
                    if args and args.strip() and args.strip() != 'void':
                        args_list = [a.strip().split()[-1].replace('*','').replace('&','') for a in args.split(',') if a.strip()]
                        for a in args_list:
                            param_name = a.split('=')[0].strip()
                            if param_name and param_name not in ['...', 'void']:
                                doc_lines.append(f"{indent} * @param {param_name} Parameter description.\n")
                                
                    if ret_type and ret_type.strip() != 'void' and func_name != ret_type.strip():
                        doc_lines.append(f"{indent} * @return Returns {ret_type.strip()} value.\n")
                        
                    doc_lines.append(f"{indent} */\n")
                    out_lines.extend(doc_lines)
            
            # Check variable
            var_match = var_pattern.match(line)
            if var_match and brace_depth in [0, 1] and 'return' not in line and 'typedef' not in line and 'using' not in line and 'friend' not in line and 'class' not in line and 'struct' not in line and 'namespace' not in line:
                indent, var_type, var_name = var_match.groups()
                if var_name not in ['return', 'else', 'default', 'break', 'continue']:
                    # Check if it already has an inline comment
                    if '//' not in line and '///<' not in line and '/*' not in line:
                        # Add inline comment
                        line = line.rstrip('\r\n') + f" ///< {var_type.strip()} variable representing {var_name}.\n"
                        
        # Update brace depth
        brace_depth += line.count('{')
        brace_depth -= line.count('}')
        
        out_lines.append(line)
        i += 1
        
    with open(filepath, 'w') as f:
        f.writelines(out_lines)

# Process all .cpp and .hpp files
for root, dirs, files in os.walk('.'):
    if 'third_party' in root or 'build' in root or '.git' in root or 'venv' in root or 'FE900_Thesis' in root:
        continue
    for file in files:
        if file.endswith('.cpp') or file.endswith('.hpp'):
            filepath = os.path.join(root, file)
            print(f"Processing {filepath}...")
            process_file(filepath)

print("Doxygen comments and inline variables updated via regex.")
