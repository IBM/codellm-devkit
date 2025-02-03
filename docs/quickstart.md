---
icon: cldk/flame-20
hide:
  - toc
---
# :cldk-flame-20: Quickstart

Build code analysis pipelines with LLMs in minutes.

In this quickstart guide, we will use the [Apache Commons CLI](https://commons.apache.org/proper/commons-cli/) example 
codebase to demonstrate code analysis pipeline creation using CLDK, with both local LLM inference and automated code processing.

??? note "Installing CLDK and Ollama"

    This quickstart guide requires CLDK and Ollama. Follow these instructions to set up your environment:

    First, install CLDK and Ollama Python SDK:

    === "`pip`"

        ```shell
        pip install cldk ollama
        ```

    === "`poetry`"
        ```shell
        poetry add cldk ollama
        ```

    === "`uv`"
        ```shell
        poetry add cldk ollama
        ```

    Then, install Ollama:  
    
    === "Linux/WSL"

        Run the following command:

        ```shell
        curl -fsSL https://ollama.com/install.sh | sh
        ```
    
    === "macOS"
        
        Run the following command:

        ```shell
        curl -fsSL https://ollama.com/install.sh | sh
        ```

        Or, download the installer from [here](https://ollama.com/download/Ollama-darwin.zip).


## Step 1: Set Up Ollama Server

Model inference with CLDK starts with a local LLM server. We'll use Ollama to host and run the models.

=== "Linux/WSL"
    * Check if the Ollama server is running:
        ```shell
        sudo systemctl status ollama
        ```

    * If not running, start it:
        ```shell
        sudo systemctl start ollama
        ```

=== "macOS"
    On macOS, Ollama runs automatically after installation. You can verify it's running by opening Activity Monitor and searching for "ollama".

## Step 2: Pull the code LLM. 

* Let's use the Granite 8b-instruct model for this tutorial:
    ```shell
    ollama pull granite-code:8b-instruct
    ```

* Verify the installation:
    ```shell
    ollama run granite-code:8b-instruct 'Write a function to print hello world in python'
    ```

## Step 3: Download Sample Codebase

We'll use Apache Commons CLI as our example Java project:

```shell
wget https://github.com/apache/commons-cli/archive/refs/tags/rel/commons-cli-1.7.0.zip -O commons-cli-1.7.0.zip && unzip commons-cli-1.7.0.zip
```

Let's set the project path for future reference:
```shell
export JAVA_APP_PATH=/path/to/commons-cli-1.7.0
```

??? note "About the Sample Project"
    Apache Commons CLI provides an API for parsing command line options. It's a well-structured Java project that demonstrates various object-oriented patterns, making it ideal for code analysis experiments.

## Step 3: Create Analysis Pipeline

??? tip "What should I expect?"
    In about 40 lines of code, we will use CLDK to build a code summarization pipeline that leverages LLMs to generate summaries for a real world Java project! Without CLDK, this would require multiple tools and a much more complex setup.

Let's build a pipeline that analyzes Java methods using LLMs. Create a new file `code_summarization.py`:

```python title="code_summarization.py" linenums="1"  hl_lines="7 10 12-17 21-22 24-25 34-37" 
import ollama
from cldk import CLDK
from pathlib import Path
import os

# Create CLDK object, specify language as Java.
cldk = CLDK(language="java")  #  (1)!

# Create analysis object
analysis = cldk.analysis(project_path=os.getenv("JAVA_APP_PATH"))  #  (2)!

# Iterate over files
for file_path, class_file in analysis.get_symbol_table().items():
    # Iterate over classes
    for type_name, type_declaration in class_file.type_declarations.items():
        # Iterate over methods
        for method in type_declaration.callable_declarations.values():  #  (3)!
            # Get code body
            code_body = Path(file_path).absolute().resolve().read_text()

            # Initialize treesitter
            tree_sitter_utils = cldk.tree_sitter_utils(source_code=code_body)  # (4)!

            # Sanitize class 
            sanitized_class = tree_sitter_utils.sanitize_focal_class(method.declaration)  # (5)!

            # Format instruction
            instruction = (
                f"Question: Can you write a brief summary for the method " 
                f"`{method.declaration}` in the class `{type_name}` below?\n\n" 
                f"```java\n{sanitized_class}```\n"
            )

            # Prompt Ollama
            summary = ollama.generate(
                model="granite-code:8b-instruct", # (6)!
                prompt=instruction).get("response") # (7)!

            # Print output
            print(f"\nMethod: {method.declaration}")
            print(f"Summary: {summary}")
```


1.  Create a new instance of the CLDK class
2.  Create an `analysis` instance for the Java project. This object will be used to obtain all the analysis artifacts from the java project.
3.  In a nested loop, we can quickly iterate over the methods in the project and extract the code body.
4.  CLDK comes with a number of treesitter based utilities that can be used to extract and manipulate code snippets. 
5.  We use the `sanitize_focal_class()` method to extract the focal class for the method and sanitize any unwanted code in just one line of code.
6.  Try your favorite model for code summarization. We use the `granite-code:8b-instruct` model in this example.
7.  We prompt Ollama with the sanitized class and method declaration to generate a summary for the method.
---

### Running `code_summarization.py`

Save the file as `code_summarization.py` and run it:
```shell
python code_summarization.py
```

You'll see output like:
```
Method: parse
Summary: This method parses command line arguments using the specified Options object...

Method: validateOption
Summary: Validates if an option meets the required criteria including checking...

...
```

## Step 5: Customize Analysis

The pipeline can be customized in several ways:

=== "Change Model"
    Try different Granite model sizes:
    ```python
    summary = ollama.generate(
        model="granite-code:34b-instruct",  # Larger model! 
        prompt=instruction).get("response") 
    ```

=== "Modify Prompt"
    Adjust the task to generate a unit test:
    ```python
    def format_inst(code, focal_method, focal_class):
        return (f"Generate a complete unit test case using junit4 for the method `{focal_method}`...\n\n"
                f"```java\n{code}```\n")
    ```


## Next Steps

- Explore different analysis tasks like code repair, translation, test generation, and more...
- Create richer prompts with more analysis artifacts that CLDK provides.
- Implement batch processing for larger projects, or use the CLDK SDK to build custom analysis pipelines.
