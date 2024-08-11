# CodeLLM-Devkit: A Python library for seamless interaction with CodeLLMs

![image](./docs/assets/cldk.png)
[![Python 3.11](https://img.shields.io/badge/python-3.11-blue.svg)](https://www.python.org/downloads/release/python-3110/)

Codellm-devkit (CLDK) is a multilingual program analysis framework that bridges the gap between traditional static analysis tools and Large Language Models (LLMs) specialized for code (CodeLLMs). Codellm-devkit allows developers to streamline the process of transforming raw code into actionable insights by providing a unified interface for integrating outputs from various analysis tools and preparing them for effective use by CodeLLMs.

Codellm-devkit simplifies the complex process of analyzing codebases that span multiple programming languages, making it easier to extract meaningful insights and drive LLM-based code analysis. `CLDK` achieves this through an open-source Python library that abstracts the intricacies of program analysis and LLM interactions. With this library, developer can streamline the process of transforming raw code into actionable insights by providing a unified interface for integrating outputs from various analysis tools and preparing them for effective use by CodeLLMs.

**The purpose of Codellm-devkit is to enable the development and experimentation of robust analysis pipelines that harness the power of both traditional program analysis tools and CodeLLMs.**
By providing a consistent and extensible framework, Codellm-devkit aims to reduce the friction associated with multi-language code analysis and ensure compatibility across different analysis tools and LLM platforms.

Codellm-devkit is designed to integrate seamlessly with a variety of popular analysis tools, such as WALA, Tree-sitter, LLVM, and CodeQL, each implemented in different languages. Codellm-devkit acts as a crucial intermediary layer, enabling efficient and consistent communication between these tools and the CodeLLMs.

Codellm-devkit is constantly evolving to include new tools and frameworks, ensuring it remains a versatile solution for code analysis and LLM integration.

Codellm-devkit is:

- **Unified**: Provides a single framework for integrating multiple analysis tools and CodeLLMs, regardless of the programming languages involved.
- **Extensible**: Designed to support new analysis tools and LLM platforms, making it adaptable to the evolving landscape of code analysis.
- **Streamlined**: Simplifies the process of transforming raw code into structured, LLM-ready inputs, reducing the overhead typically associated with multi-language analysis.

Codellm-devkit is an ongoing project, developed at IBM Research.

## Table of Contents

- [CodeLLM-Devkit: A Python library for seamless interaction with CodeLLMs](#codellm-devkit-a-python-library-for-seamless-interaction-with-codellms)
  - [Table of Contents](#table-of-contents)
  - [Architectural and Design Overview](#architectural-and-design-overview)
  - [Quick Start: Example Walkthrough](#quick-start-example-walkthrough)
    - [Prerequisites](#prerequisites)
    - [Step 1:  Install CLDK](#step-1--install-cldk)
    - [Step 2:  Set up an Ollama server](#step-2--set-up-an-ollama-server)
    - [Step 3: Pull the latest version of Granite 8b instruct model from ollama](#step-3-pull-the-latest-version-of-granite-8b-instruct-model-from-ollama)

## Architectural and Design Overview

Below is a very high-level overview of the architectural of CLDK:


```mermaid
graph TD
User <--> A[CLDK]
    A[CLDK] <--> B[Languages]
        B --> C[Java, Python, Go, C++, JavaScript, TypeScript, Rust]
            C --> D[Data Models]
                D --> 13{Pydantic}
            13 --> 7            
            C --> 7{backends}
                7 <--> 8[CodeQL]
                    8 <--> 14[Analysis]
                7 <--> 9[WALA]
                    9 <--> 14[Analysis]
                7 <--> 10[Tree-sitter] 
                    10 <--> 14[Analysis]
                7 <--> 11[LLVM]
                    11 <--> 14[Analysis]
                7 <--> 12[CodeQL]
                    12 <--> 14[Analysis]

    A --> 17[Retrieval ‡]
    A --> 16[Prompting ‡]

X[‡ Yet to be implemented]
```

The user interacts by invoking the CLDK API. The CLDK API is responsible for handling the user requests and delegating them to the appropriate language-specific modules. 

Each language comprises of two key components: data models and backends.

1. **Data Models:** These are high level abstractions that represent the various language constructs and componentes in a structured format using pydantic. This confers a high degree of flexibility and extensibility to the models as well as allowing for easy accees of various data components via a simple dot notation. In addition, the data models are designed to be easily serializable and deserializable, making it easy to store and retrieve data from various sources.


2. **Analysis Backends:** These are the components that are responsible for interfacing with the various program analysis tools. The core backends are Treesitter, Javaparse, WALA, LLVM, and CodeQL. The backends are responsible for handling the user requests and delegating them to the appropriate analysis tools. The analysis tools perfrom the requisite analysis and return the results to the user. The user merely calls one of several high-level API functions such as `get_method_body`, `get_method_signature`, `get_call_graph`, etc. and the backend takes care of the rest. 

    Some langugages may have multiple backends. For example, Java has WALA, Javaparser, Treesitter, and CodeQL backends. The user has freedom to choose the backend that best suits their needs. 

We are currently working on implementing the retrieval and prompting components. The retrieval component will be responsible for retrieving the relevant code snippets from the codebase for RAG usecases. The prompting component will be responsible for generating the prompts for the CodeLLMs using popular prompting frameworks such as `PDL`, `Guidance`, or `LMQL`.  


## Quick Start: Example Walkthrough

In this section, we will walk through a simple example to demonstrate how to use CLDK. We will:

* Set up a local ollama server to interact with CodeLLMs
* Build a simple code summarization pipeline for a Java and a Python application.

### Prerequisites

Before we begin, make sure you have the following prerequisites installed:

  * Python 3.11 or later
  * Ollama v0.3.4 or later


### Step 1:  Install CLDK

You may install the latest version of CLDK from our GitHub repository:

```python
pip install git+https://github.com/IBM/codellm-devkit.git
```

Once CLDK is installed, you can import it into your Python code:

```python
from cldk import CLDK
```


### Step 2:  Set up an Ollama server

If don't already have ollama, please download and install it from here: [Ollama](https://ollama.com/download). 

Once you have ollama, start the server and make sure it is running.

If you're on MacOS, Linux, or WSL, you can check to make sure the server is running by running the following command:

```bash
sudo systemctl status ollama
```

You should see an output similar to the following:

```bash
➜ sudo systemctl status ollama
● ollama.service - Ollama Service
     Loaded: loaded (/etc/systemd/system/ollama.service; enabled; preset: enabled)
     Active: active (running) since Sat 2024-08-10 20:39:56 EDT; 17s ago
   Main PID: 23069 (ollama)
      Tasks: 19 (limit: 76802)
     Memory: 1.2G (peak: 1.2G)
        CPU: 6.745s
     CGroup: /system.slice/ollama.service
             └─23069 /usr/local/bin/ollama serve
```

If not, you may have to start the server manually. You can do this by running the following command:

```bash
sudo systemctl start ollama
```

### Step 3: Pull the latest version of Granite 8b instruct model from ollama

To pull the latest version of the Granite 8b instruct model from ollama, run the following command:

```bash
ollama pull granite-code:8b-instruct
```

Check to make sure the model was successfully pulled by running the following command:

```bash
ollama run granite-code:8b-instruct 'Write a function to print hello world in python'
```

The output should be similar to the following:

```
➜ ollama run granite-code:8b-instruct 'Write a function to print hello world in python'

def say_hello():
    print("Hello World!")
```