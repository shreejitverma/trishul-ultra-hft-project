import re
import os

def process_latex(content):
    # 1. Ensure \centering is in figure environments
    def fix_figure(match):
        fig_content = match.group(0)
        if '\\centering' not in fig_content:
            # Insert \centering after \begin{figure}[...]
            fig_content = re.sub(r'(\\begin\{figure\}(?:\[.*?\])?\s*)', r'\1    \\centering\n', fig_content)
        return fig_content

    content = re.sub(r'\\begin\{figure\}.*?\\end\{figure\}', fix_figure, content, flags=re.DOTALL)

    # 2. Wrap tikzpicture in adjustbox if not already wrapped
    # This regex looks for tikzpicture and its optional arguments
    def wrap_tikz(match):
        tikz_block = match.group(0)
        # Check if already wrapped (simple check for \begin{adjustbox} before it)
        # We look back in the content to see if it's already wrapped.
        # However, re.sub with a callback doesn't easily give us the context outside the match.
        # So we'll use a more complex regex or a multi-step approach.
        return tikz_block

    # Alternative approach for wrapping:
    # Use a regex that captures the tikzpicture but excludes those already inside adjustbox.
    # Actually, it's easier to find all adjustbox+tikzpicture and skip them,
    # and then find remaining tikzpictures.
    
    # We'll use a placeholder for already wrapped ones
    placeholder = "ALREADY_WRAPPED_TIKZ"
    wrapped_blocks = []
    
    def store_wrapped(match):
        wrapped_blocks.append(match.group(0))
        return f"%%{placeholder}_{len(wrapped_blocks)-1}%%"
    
    # Temporarily hide already wrapped ones
    content = re.sub(r'\\begin\{adjustbox\}\{.*?\}\s*\\begin\{tikzpicture\}.*?\\end\{tikzpicture\}\s*\\end\{adjustbox\}', store_wrapped, content, flags=re.DOTALL)
    
    # Wrap remaining tikzpictures
    def wrap_it(match):
        return f"\\begin{{adjustbox}}{{max width=\\textwidth, center}}\n{match.group(0)}\n\\end{{adjustbox}}"
    
    content = re.sub(r'\\begin\{tikzpicture\}.*?\\end\{tikzpicture\}', wrap_it, content, flags=re.DOTALL)
    
    # Restore already wrapped ones
    for i, block in enumerate(wrapped_blocks):
        content = content.replace(f"%%{placeholder}_{i}%%", block)
        
    return content

files_to_process = [
    "FE900_Thesis/chapter7.tex",
    "FE900_Thesis/chapter6.tex",
    "FE900_Thesis/chapter4.tex",
    "FE900_Thesis/chapter5.tex",
    "FE900_Thesis/notethesis.tex",
    "FE900_Thesis/chapter1.tex",
    "FE900_Thesis/SV_Introduction.tex",
    "FE900_Thesis/symbols.tex",
    "FE900_Thesis/chapter2.tex",
    "FE900_Thesis/chapter3.tex",
    "FE900_Thesis/diagrams/fig8_unified.tex",
    "FE900_Thesis/diagrams/fig2_orderbook.tex",
    "FE900_Thesis/diagrams/fig7_monitoring.tex",
    "FE900_Thesis/diagrams/fig6_execution.tex",
    "FE900_Thesis/diagrams/fig3_event_pipeline.tex",
    "FE900_Thesis/diagrams/fig5_fpga_internal.tex",
    "FE900_Thesis/diagrams/fig1_ingestion.tex",
    "FE900_Thesis/diagrams/fig4_fpga_t2t.tex",
    "FE900_Thesis/glossary.tex",
    "FE900_Thesis/SV_stevensThesis.tex",
    "FE900_Thesis/chapter8.tex",
    "FE900_Thesis/Appendix.tex"
]

for file_path in files_to_process:
    if os.path.exists(file_path):
        with open(file_path, 'r') as f:
            content = f.read()
        
        new_content = process_latex(content)
        
        if new_content != content:
            with open(file_path, 'w') as f:
                f.write(new_content)
            print(f"Processed {file_path}")
        else:
            print(f"No changes needed for {file_path}")
