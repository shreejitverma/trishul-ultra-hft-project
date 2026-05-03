import os
import re

dir_path = 'FE900_Thesis'
files_changed = 0

for root, _, files in os.walk(dir_path):
    for file in files:
        if file.endswith('.tex') or file.endswith('.sty'):
            filepath = os.path.join(root, file)
            with open(filepath, 'r', encoding='utf-8') as f:
                lines = f.readlines()
            
            new_lines = []
            in_listing = False
            changed = False
            for line in lines:
                if r'\begin{lstlisting}' in line:
                    in_listing = True
                
                new_line = line
                if not in_listing:
                    new_line = re.sub(r'"([^"]+)"', r"``\1''", line)
                    if new_line != line:
                        changed = True
                        
                new_lines.append(new_line)
                
                if r'\end{lstlisting}' in line:
                    in_listing = False
                    
            if changed:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.writelines(new_lines)
                files_changed += 1
                print(f"Fixed quotes in {filepath}")

print(f"Total files modified: {files_changed}")
