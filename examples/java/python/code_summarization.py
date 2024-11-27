import os
from pathlib import Path
import ollama
from cldk import CLDK


def format_inst(code, focal_method, focal_class, language):
    """
    Format the instruction for the given focal method and class.
    """
    inst = f"Question: Can you write a brief summary for the method `{focal_method}` in the class `{focal_class}` below?\n"

    inst += "\n"
    inst += f"```{language}\n"
    inst += code
    inst += "```" if code.endswith("\n") else "\n```"
    inst += "\n"
    return inst


def prompt_ollama(message: str, model_id: str = "granite-code:8b-instruct") -> str:
    """Prompt local model on Ollama"""
    response_object = ollama.generate(model=model_id, prompt=message)
    return response_object["response"]


if __name__ == "__main__":
    # (1) Create a new instance of the CLDK class
    cldk = CLDK(language="java")

    # (2) Create an analysis object over the java application
    analysis = cldk.analysis(project_path="JAVA_APP_PATH")

    # (3) Iterate over all the files in the project
    for file_path, class_file in analysis.get_symbol_table().items():
        class_file_path = Path(file_path).absolute().resolve()
        # (4) Iterate over all the classes in the file
        for type_name, type_declaration in class_file.type_declarations.items():
            # (5) Iterate over all the methods in the class
            for method in type_declaration.callable_declarations.values():
                # (6) Get code body of the method
                code_body = class_file_path.read_text()

                # (7) Initialize the treesitter utils for the class file content
                tree_sitter_utils = cldk.tree_sitter_utils(source_code=code_body)

                # (8) Sanitize the class for analysis
                sanitized_class = tree_sitter_utils.sanitize_focal_class(method.declaration)

                # (9) Format the instruction for the given focal method and class
                instruction = format_inst(
                    code=sanitized_class,
                    focal_method=method.declaration,
                    focal_class=type_name,
                    language="java"
                )

                # (10) Prompt the local model on Ollama
                llm_output = prompt_ollama(
                    message=instruction,
                    model_id="granite-code:20b-instruct",
                )

                # (11) Print the instruction and LLM output
                print(f"Instruction:\n{instruction}")
                print(f"LLM Output:\n{llm_output}")