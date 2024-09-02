## Simplify your Code LLM solutions using CodeLLM Dev Kit (CLDK)

<!-- TODOs: 

Example: Raju

Experience: Rangeet, before and after

Nice picture: Rahul -->

### Introduction
Introduction:

As our team at IBM Research started to build a solution for application modernization based on Large Language Models for code (CodeLLMs), we were faced with the choice of different static-analysis tooling to use coupled with core code LLM capabilities. These static-analysis tools, such as [Tree-sitter](https://tree-sitter.github.io/tree-sitter/), [WALA](https://github.com/wala/WALA), etc., each vary in their support for different programming languages, programming-language features, and types of program analyses. Developers require a deep understanding of such tools and what they offer for each programming language, to create meaningful context/prompt augmentations when developing CodeLLM solutions. This, in turn, becomes a tedious task that often requires considerable amount of code to be written to support various CodeLLM use cases, such as test generation, code summarization, code translation, etc.  To address this problem, we have developed CodeLLM Development Kit (CLDK), which abstracts the interaction with the static-analysis tools and enables seamless integration with CodeLLMs. 
![image](cldk-logo.png)

CLDK finds its use in different phases of the CodeLLM lifecycle. It can help augment and enable generation of instruct datasets to enhance existing models. It can be used to build new code models by creating finetuning datasets, build code-related LLM-assisted solutions, support evaluation of models, and last, but not least, simplify the development of enterprise use cases. 

At IBM Research, we use CLDK for various enterprise use cases, including code explanation and test generation, as part of our efforts with [watsonx Code Assistant for Enterprise Java Applications](https://www.ibm.com/products/watsonx-code-assistant-for-enterprise-java-applications). We observed a significant boost in both productivity and creativity through the use of this tool. Our experience showed that CLDK offers several key benefits: (1) it abstracts the complex details of various program analysis engines, such as WALA, which runs on Java and typically requires significant expertise to generate code-analysis results, and Tree-sitter, which relies on query-based language processing and requires proper post-processing for extracting relevant information; (2) it facilitates the creation of [Pydantic](https://docs.pydantic.dev/latest/concepts/models/) models for code analysis (example, [Java code analysis model](https://github.com/IBM/codellm-devkit/blob/main/cldk/models/java/models.py)); (3) it enables support for multiple program-analysis backends, including WALA and Tree-sitter; (4) it supports multiple programming languages (currently, Java and Python, with plans to support additional languages soon); and (5) it offers different levels of analysis, allowing for more efficient workflows by enabling users to bypass more intensive analyses, such as those based on call graphs, program dependency graphs, system dependency graphs, when a symbol-table-based analysis suffices, which is especially valuable for enterprise-grade projects.


How to use CLDK in practice:

Installing CLDK is very simple. All you have to do is run the below command.

```
pip install git+https://github.com/IBM/codellm-devkit.git
```

To understand the potential of CLDK, let’s walkthrough an example of summarizing a Java method using a CodeLLM with the help of program analysis.  As we will be interacting with a CodeLLM in this example, let's downnload and install `Ollama`, a tool that helps run LLMs locally, from here https://ollama.com/download. 

Although any CodeLLM of choice can be used with CLDK, for this example, we will use the IBM Granite code 8b-instruct model. More information about IBM Granite code models can be found at https://github.com/ibm-granite/granite-code-models. Once we have Ollama up and running, let's pull the Granite model.

```bash
ollama pull granite-code:8b-instruct
```

After the download completes, let's test it once with a basic prompt.

```bash
ollama run granite-code:8b-instruct \"Write a python function to print 'Hello, World!'\"
```
You should see a response from the Granite model, consisting of Python code and some instructions.  Let's also install Ollama's Python SDK, so we can interact with the model in our example.

```bash
pip install ollama
```

We will use [Apache Commons CLI](https://github.com/apache/commons-cli) Java application to exercise a few analysis operations.  Let's download it.  Make sure you have `wget` installed on your system.

```bash
wget https://github.com/apache/commons-cli/archive/refs/tags/rel/commons-cli-1.7.0.zip -O /tmp/commons-cli-1.7.0.zip && unzip -o /tmp/commons-cli-1.7.0.zip -d /tmp
```

Now that we are done with the pre-requisites, let's get back to our example and start coding it in a Python notebook. First, let's warm ourselves up with a few basic operations on this Java application. You can copy/paste the following code snippets into your Python notebook.

A few necessary imports:

```python
from pathlib import Path
import ollama
from cldk import CLDK
from cldk.analysis import AnalysisLevel
```

Create an instance of CLDK by providing the programming language of the code under analysis; in this case it is Java.

```python
# Create a new instance of the CLDK class
cldk = CLDK(language="java")
```

CLDK uses different analysis engines---[CodeAnalyzer](https://github.com/IBM/codenet-minerva-code-analyzer) (built using [WALA](https://github.com/wala/WALA) and [JavaParser](https://github.com/javaparser/javaparser)), [Tree-sitter](https://tree-sitter.github.io/tree-sitter/), and [CodeQL](https://codeql.github.com/) (future). CodeAnalyzer is selected as a default analysis engine for Java. 

CLDK supports different analysis levels as well---symbol table, call graph, program dependency graph, and system dependency graph. The analysis level can be selected using the `AnalysisLevel` enumerated type.
Let's select `symbol_table` as the analysis level and also provide the path to the downloaded Commons CLI application.

```python
# Create an analysis object over the java application
analysis = cldk.analysis(project_path="/tmp/commons-cli-rel-commons-cli-1.7.0", analysis_level=AnalysisLevel.symbol_table)
```
Using this analysis object we can get a wealth of information about this application.  For example, we can get all the classes, methods, their signatures, parameters, code bodies, callers, callees, and a lot more! 

Let's list all the classes in the application.

```python
print(list(analysis.get_classes().keys()))
```
You shoud see a list of Java classes.  To play with this further, let's choose one particular class, `org.apache.commons.cli.GnuParser` from the classes in this application. We can get the Java file for this class and the code as well.

```python
gp_class_file = analysis.get_java_file('org.apache.commons.cli.GnuParser')
gp_class_file_path = Path(gp_class_file).absolute().resolve()
gp_code_body = gp_class_file_path.read_text()
```
We will use this code body later in the example.  We can also list all the methods in this class.  Let's print the first method.

```python
some_method = next(iter(analysis.get_methods_in_class('org.apache.commons.cli.GnuParser')))
print(some_method)
```
This happens to be `flatten(Options, String[], boolean)`.  We can get an object for the `flatten` method and print its signature.

```python
flatten_method = analysis.get_method('org.apache.commons.cli.GnuParser','flatten(Options, String[], boolean)')
print(flatten_method.signature)
```
We will use this signature too later in our example.

We mentioned above that CLDK also supports Tree-sitter.  Let's see how we can use Tree-sitter utilities to do further analysis to build an LLM prompt for code summarization.  We can add any caller/callee information for the method of interest, if they exist. Further, we can also delete any unwanted imports/comments that are not relevant in this context. All of this can be achieved by a single call to ```sanitize_focal_class``` API.

```python
# Instantiate tree sitter utilities with GnuParser code body.
tree_sitter_utils = cldk.tree_sitter_utils(source_code=gp_code_body)
# Sanitize this class body to keep only relevant information for flatten method.
sanitized_class = tree_sitter_utils.sanitize_focal_class(flatten_method.declaration)
print(sanitized_class)
```

Now we are ready to prompt the LLM to get a summary for this method.  For this let's use a couple of helper methods. 

The following helper method builds a prompt to instruct the LLM to summarize a price of code.  

```python
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
```

Our second helper method prompts the local model hosted by Ollama and returns the response.

```python
def prompt_ollama(message: str, model_id: str = "granite-code:8b-instruct") -> str:
    """Prompt local model on Ollama"""
    response_object = ollama.generate(model=model_id, prompt=message)
    return response_object["response"]
```

Using the above two methods and the program analysis we have done so far, let's get the code summary. We build the code-summarization prompt with the sanitized class and the target method signature.

```python
# Format the instruction for the given focal method and class
instruction = format_inst(
    code=sanitized_class,
    focal_method=flatten_method.declaration,
    focal_class='org.apache.commons.cli.GnuParser',
    language="java"
)
print(instruction)
```

Finally, we invoke the LLM with the prompt and print the LLM's response.

```python
# Prompt the local model on Ollama
llm_output = prompt_ollama(
    message=instruction,
    model_id="granite-code:8b-instruct",
)
print(llm_output)
```

In a few seconds, you will see the generated summary for the `flatten` method.

You just saw how easy it was to analyze a Java application and use the analysis artifacts to query a CodeLLM. The complete Python notebook for this example is available [here](https://github.com/IBM/codellm-devkit/blob/main/docs/examples/java/notebook/code_summarization.ipynb).

Now, we will show how with small changes, a JUnit test case can be generated using CLDK and LLM for the same method. For test generation, we will change the prompt, which takes, the focal method and class names, focal method body, and signatures of constructors of the focal class.

```python
def format_inst(focal_method_body, focal_method, focal_class, constructor_signatures, language):
    """
    Format the LLM instruction for the given focal method and class.
    """
    inst = f"Question: Can you generate junit tests with @Test annotation for the method `{focal_method}` in the class `{focal_class}` below. Only generate the test and no description.\n"
    inst += 'Use the constructor signatures to form the object if the method is not static. Generate the code under ``` code block.'
    inst += "\n"
    inst += f"```{language}\n"
    inst += f"public class {focal_class} " + "{\n"
    inst += f"{constructor_signatures}\n"
    inst += f"{focal_method_body} \n" 
    inst += "}"
    inst += "```\n"
    inst += "Answer:\n"
    return inst
```
First, we will get the signatures of all constructors in `org.apache.commons.cli.GnuParser`.

```python
class_name = 'org.apache.commons.cli.GnuParser'
constructor_signatures = ''
for method in analysis.get_methods_in_class(qualified_class_name=class_name):
    method_details = analysis.get_method(qualified_class_name=class_name, qualified_method_name=method)
    if method_details.is_constructor:
        constructor_signatures += method_details.signature + '\n'
```

We go through each method in the class and check if the method is a signature by simply checking the `is_constructor` attribute in the Pydantic model.
Now, we will provide all the details needed for prompt generation.

```python
method_body = method_details.declaration + flatten_method.code
method_name = flatten_method.signature.split("(")[0]
focal_class_name = class_name.split('.')[-1]
prompt = format_inst(
    focal_method_body=method_body,
    focal_method=method_name,
    focal_class=focal_class_name,
    constructor_signatures=constructor_signatures,
    language="java"
)
                
# Print the instruction
print(f"Instruction:\n{prompt}\n")
print(f"Generating test case ...\n")

# Prompt the local model on Ollama
llm_output = prompt_ollama(message=prompt)

# Print the LLM output
print(f"LLM Output:\n{llm_output}")
```
In a few seconds, you will see the genreratd JUnit test case for the `flatten` method.

CLDK makes code analysis significantly easier and developers and researchers can use without worrying about a lot of intrinsic details. The complete Python notebook for this example is available [here](https://github.com/IBM/codellm-devkit/blob/main/docs/examples/java/notebook/generate_unit_tests.ipynb). 

For more examples, please refer to our [GitHub repo](https://github.com/IBM/codellm-devkit/tree/main/docs/examples).

Link to Granite code models: https://github.com/ibm-granite/granite-code-models

Crosslink to AI Alliance: 

Link to CLDK GitHub repositiory: https://github.com/IBM/codellm-devkit

We have just started this journey - join our [community](https://github.com/ibm-granite-community).

We have started with support for Java and Python languages at symbol-table and call-graph analysis levels. Our next steps include enhancing CLDK in several directions: (1) support more programming languages, such as C, C++, JavaScript, Go, Rust, etc., (2) support deeper analysis by enabling call-graph analysis for more languages, and eventually support other analysis levels for all the programming languages, (3) support prompt template generation languages, such as LMQL, Guidance, etc., and (4) enable more features, such as more APIs targeted toward post-processing of the LLM-generated code, enabling RAG, and other LLM-based use cases.
