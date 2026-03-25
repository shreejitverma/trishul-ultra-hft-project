import os
import re

tex_dir = "/Users/shreejitverma/Documents/GitHub/ultra-hft-project/FE900_Thesis"
chapter_files = [f for f in os.listdir(tex_dir) if (f.startswith('chapter') or f == 'SV_Introduction.tex') and f.endswith('.tex')]

def check_figures(filepath):
    with open(filepath, 'r') as f:
        content = f.read()
    
    begin_count = len(re.findall(r'\\begin\{figure\}', content))
    end_count = len(re.findall(r'\\end\{figure\}', content))
    
    if begin_count != end_count:
        print(f"FILE: {filepath} has mismatched figure environment! {begin_count} vs {end_count}")
    
    # Extract blocks
    # Note: this doesn't handle nested environments but figure shouldn't be nested.
    figures = re.findall(r'\\begin\{figure\}(?:\[.*?\])?(.*?)\\end\{figure\}', content, re.DOTALL)
    
    for i, fig_content in enumerate(figures):
        lines = [line.strip() for line in fig_content.splitlines() if line.strip()]
        for line in lines:
            allowed = [r'\\centering', r'\\caption', r'\\label', r'\\input\{diagrams/.*\}']
            is_allowed = False
            for pattern in allowed:
                if re.match(r'^\s*' + pattern, line):
                    is_allowed = True
                    break
            
            if not is_allowed:
                # Special check for multiple commands on one line or comments
                # If the line contains ONLY allowed things, it's fine.
                # But for now, let's just report.
                print(f"FILE: {os.path.basename(filepath)}, Fig {i+1} check: {line}")

if __name__ == "__main__":
    for f in chapter_files:
        check_figures(os.path.join(tex_dir, f))
