# CodeLLM-Devkit: A Python library for seamless interaction with CodeLLMs

![image](./docs/assets/cldk.png)
[![Python 3.11](https://img.shields.io/badge/python-3.11-blue.svg)](https://www.python.org/downloads/release/python-3110/)

## Prerequisites

- Python 3.11+
- Poetry (see [doc](https://python-poetry.org/docs/))

## Installation

Obtain Codellm-DevKit from below:

```bash
git clone git@github.com:IBM/codellm-devkit.git /path/to/cloned/repo
```

Install CodeLLM-Devkit

```bash
pip install -U /path/to/cloned/repo
```

## Usage

### 1.  Obtain sample application to experiment with (we'll use Daytrader 8 as an example)

```bash
wget https://github.com/OpenLiberty/sample.daytrader8/archive/refs/tags/v1.2.tar.gz
```

Extract the archive and navigate to the `daytrader8` directory:

```bash
tar -xvf v1.2.tar.gz
tar -xvf v1.2.tar.gz
```

Save the location to where daytrader8 is extracted to, as we will need it later:

```bash
export DAYTRADER8_DIR=/path/to/sample.daytrader8-1.2
```

Then you can use the following command to run the codeanalyzer:

```python
import os
from rich import print  # Optional, for pretty printing.
from cldk import CLDK
from cldk.models.java.models import *

# Initialize the Codellm-DevKit object with the project directory, language, and backend.
cldk = CLDK(language="java")

analysis = cldk.analysis(
    project_path=os.getenv("DAYTRADER8_DIR"),
    analysis_backend="codeanalyzer",
    analysis_json_path="/tmp",
    eager=True,
    analysis_level='call-graph'
)
# Get the java application view for the project. The application view is a representation of the project as a graph with all the classes, methods, and fields.
app: JApplication = analysis.get_application_view()

# Get all the classes in the project.
classes_dict = analysis.get_classes()
print(classes_dict)
```