## Simplify your Code LLM solutions using CodeLLM Dev Kit (CLDK)

<!-- TODOs: 

Example: Raju

Experience: Rangeet, before and after

Nice picture: Rahul -->

### Introduction
Introduction:

As our team at IBM Research started to build a solution for application modernization based on Code Large Language Model (CodeLLM), we were faced with the abundance of multiple static analysis tooling to use coupled with core CodeLLM capabilities. These static analysis tools, such as [Tree-sitter](https://tree-sitter.github.io/tree-sitter/), [WALA](https://github.com/wala/WALA), etc., each range in their support for different programming language and its coverage of programming language features. Developers require a deep understanding of program analysis tooling and what they offer for each programming language, to create meaningful context/prompt augmentations when developing CodeLLM solutions. This in turn becomes a tedious task, and often requires multiple lines of code to be written to support various CodeLLM enabled tasks such as, generating unit tests, summarizing code, etc.  As a result, we have developed CodeLLM Development Kit (CLDK), which abstracts the interaction with the static analysis tools and enables seamless integration with CodeLLMs. 
![image](cldk-logo.png)

CLDK finds its use in different aspects of CodeLLM lifecycle. It can help augment and enable generation of instruct datasets to enhance the existing models, it can be used to build new code models by creating finetuning datasets, build more code-related LLM assisted solutions, support evaluation of the models, and last but not least simplify development of enterprise use cases. 

At IBM Research, we used CLDK for several use cases, such as generating code explanation, test generation for [enterprise use cases](https://www.ibm.com/products/watsonx-code-assistant-for-enterprise-java-applications) and we found that this tool has increased the productivity and creativity significantly. Based on experience, we found that CLDK helps (a) abstracting the intrinsic details of various program analysis engine, such as WALA runs on Java, which requires significant expertise in generating code analysis, Tree-sitter runs on query-based language and needs proper post-processing to get relevant information, etc., (b) creating a [Pydantic](https://docs.pydantic.dev/latest/concepts/models/) model for code analysis, (c) enabling support for multiple program analysis backends, such as WALA, Tree-sitter, and (d) support for multiple programming languages (currently supports Java and Python, and we will be releasing support for several more languages very soon), and (e) support for different levels of analysis, such as not all use cases requires call-graph analysis and can be handled through symbol table analysis--saving significant time, especially when performing analysis at enterprise-grade projects.

At IBM Research, we utilized CLDK for various enterprise use cases, including code explanation generation and test generation, as part of our efforts with [watsonx Code Assistant for Enterprise Java Applications](https://www.ibm.com/products/watsonx-code-assistant-for-enterprise-java-applications). We observed a significant boost in both productivity and creativity through the use of this tool. Our experience showed that CLDK offers several key benefits: (a) it abstracts the complex details of various program analysis engines, such as WALA, which runs on Java and typically requires significant expertise to generate code analysis, and Tree-sitter, which relies on query-based language processing and requires proper post-processing for extracting relevant information; (b) it facilitates the creation of [Pydantic](https://docs.pydantic.dev/latest/concepts/models/) models for code analysis (example, [Java code analysis model](https://github.com/IBM/codellm-devkit/blob/main/cldk/models/java/models.py)) ; (c) it enables support for multiple program analysis backends, including WALA and Tree-sitter; (d) it supports multiple programming languages—currently Java and Python, with plans to release support for additional languages soon; and (e) it offers different levels of analysis, allowing for more efficient workflows by enabling users to bypass more intensive analyses, such as call graph, program dependency graph, system dependency graph-based analysis, when a symbol table analysis suffices, which is especially valuable for enterprise-grade projects.



How to use CLDK in practice:

Installing CLDK is very simple. All you have to do is run the below command.

```
pip install git+https://github.com/IBM/codellm-devkit.git
```

To understand the potential of CLDK, let’s walkthrough an example of summarizing a java method using a Code LLM with the help of program analysis.  Since we will be interacting with a CodeLLM in this example, let's downnload and install ```Ollama```, a tool that helps run LLMs locally, from here https://ollama.com/download. 

While any CodeLLM of choice can be used with CLDK, for this example, we will use IBM Granite 8b-instruct CodeLLM. More information about IBM Granite code models can be found at https://github.com/ibm-granite/granite-code-models. Once we have Ollama up and running, let's pull granite model.

```bash
ollama pull granite-code:8b-instruct
```

After the download completes, let's test it once with a basic prompt.

```bash
ollama run granite-code:8b-instruct \"Write a python function to print 'Hello, World!'
```
You should see a response from the granite model with python code and some instructions.  Let's also install Ollama's python sdk, so we can interact with the model in our example.

```bash
pip install ollama
```

We will use commons-cli java application to exercise a few analysis operations.  Let's download it.  Make sure you have ```wget``` installed on your system.

```bash
wget https://github.com/apache/commons-cli/archive/refs/tags/rel/commons-cli-1.7.0.zip -O /tmp/commons-cli-1.7.0.zip && unzip -o /tmp/commons-cli-1.7.0.zip -d /tmp
```

Now that we are done with the pre-reqs, let's get back to our example and start coding it in a python notebook. First, let's warm ourselves up with a few basic operations on this Java application. You can copy/paste the following code snippets into your python notebook.

A few necessary imports.

```python
from pathlib import Path
import ollama
from cldk import CLDK
from cldk.analysis import AnalysisLevel
```

Create an object of CLDK by providing the programming language of the source code we want to analyze, in this case it is ```java```.

```python
# Create a new instance of the CLDK class
cldk = CLDK(language="java")
```

CLDK uses different analysis engines -- Codeanalyzer (built using WALA and Javaparser), Tree-sitter, and CodeQL (future). Codeanalyzer is selected as a default analysis engine for ```java``` programming language. 

CLDK supports different analysis levels as well --symbol table, call graph, program dependency graph, and system dependency graph. Analysis level can be selected using ```AnalysisLevel``` enum. Let's select ```symbol_table``` for the AnalysisLevel and also provide path for our example appliation we just downloaded.

```python
# Create an analysis object over the java application
analysis = cldk.analysis(project_path="/tmp/commons-cli-rel-commons-cli-1.7.0", analysis_level=AnalysisLevel.symbol_table)
```
Using this analysis object we can get a wealth of information about this application.  For example, we can get all the classes, methods, their signatures, parameters, code bodies, callers, callees and a lot more! 

Let's list all the classes in this application.

```python
print(list(analysis.get_classes().keys()))
```
You shoud see a list of java classes.  To play with this further, let's choose one particular class 'org.apache.commons.cli.GnuParser' out of many in this application. We can get the java file for this class, and get the code as well.

```python
gp_class_file = analysis.get_java_file('org.apache.commons.cli.GnuParser')
gp_class_file_path = Path(gp_class_file).absolute().resolve()
gp_code_body = gp_class_file_path.read_text()
```
We will use this code body later in the example.  We can also list all the methods in this class.  Let's print the first one.

```python
some_method = next(iter(analysis.get_methods_in_class('org.apache.commons.cli.GnuParser')))
print(some_method)
```
This happens to be `flatten(Options, String[], boolean)`.  We can get an object for this ```flatten``` method and print its signature as well.

```python
flatten_method = analysis.get_method('org.apache.commons.cli.GnuParser','flatten(Options, String[], boolean)')
print(flatten_method.signature)
```
We will use this signature too later in our example.

We mentioned above that CLDK also supports Tree-sitter.  Let's see how we can use Tree-sitter utilities to do further analysis to build an LLM prompt for our code summarization.  We can add any caller/callee information for the method of interest, if they exist. Further, we can also delete any unwanted imports/comments that are not relevant in this context. All of this can be achieved by a single call to ```sanitize_focal_class``` API.

```python
# Instantiate tree sitter utilities with GnuParser code body.
tree_sitter_utils = cldk.tree_sitter_utils(source_code=gp_code_body)
# Sanitize this class body to keep only relevant information for flatten method.
sanitized_class = tree_sitter_utils.sanitize_focal_class(flatten_method.declaration)
print(sanitized_class)
```

Now we are ready to prompt the LLM to get a summary for this method.  For this let's use a couple of helper methods. 

Following helper method builds a prompt to request LLM for code summary.  

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

Using the above two methods and the program analysis we have done so far, let's get the code summary.


Build our code summarization prompt and see how it looks like.  Use the sanitized class body, and the signature of flatten method.
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

Request a summary from LLM and print it out.

```python
# Prompt the local model on Ollama
llm_output = prompt_ollama(
    message=instruction,
    model_id="granite-code:8b-instruct",
)
print(llm_output)
```
In a few secs you will see the summary for flatten method printed.  
You just saw how easy it was to analyze a Java application and use the analysis artifacts to query a CodeLLM. A complete notebook for this example is also available at https://github.com/IBM/codellm-devkit/blob/main/docs/examples/java/code_summarization.ipynb.

For more examples, please refer to our [GitHub repo](https://github.com/IBM/codellm-devkit/tree/main/docs/examples).

Link to granite: https://github.com/ibm-granite/granite-code-models

Crosslink to AI Alliance: 

Link to CLDK GitHub: https://github.com/IBM/codellm-devkit

We have just started this journey - join our community. (granite adoption community)

We started with support for Java and Python with symbol table and call-graph support. Our immediate next step includes increasing the support in multiple directions — (a) more programming languages, such as C, C++, JavaScript, Go, Rust, etc., (b) deeper analysis by enabling call-graph analysis for more languages, and eventually support another analysis level for all the programming languages, (c) support prompt template generation languages, such as LMQL, Guidance, etc., and (d) enable more features like more APIs targeted towards post-processing of the LLM-based code generation, enabling RAG, and other LLM-based use cases.
