################################################################################
# Copyright IBM Corporation 2025
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

"""
Python Tests
"""
import os
from unittest.mock import patch
from typing import List
from tree_sitter import Tree

from cldk.analysis.commons.treesitter import TreesitterPython
from cldk.models.python.models import PyClass, PyImport, PyMethod, PyModule

PYTHON_CODE = """
import os
from typing import List
from math import *

def env(env_var: str): -> str
  return os.getenv(env_var)

class Calculator():
  '''Calculator Class'''

  def __init__(self):
    self._total = 0

  @property
  def total(self):
    return self._total

  @total.setter
  def total(self, value):
    self._total = value

  def add(self, a, b):
    total += a + b
    return a + b

  def subtract(self, a, b):
    total += a - b
    return a - b

  def multiply(self, a, b):
    total += (a * b)
    return a * b

  def divide(self, a, b):
    total += (a / b)
    return a / b
"""


def test_is_parsable():
    """Should be able to parse the code"""
    python_sitter = TreesitterPython()

    code = "def is_parsable(self, code: str) -> bool: return True"
    is_parsable = python_sitter.is_parsable(code)
    assert is_parsable is True

    code = "def is_not_parsable(self, code: str) -> bool: return True if True else"
    is_parsable = python_sitter.is_parsable(code)
    assert is_parsable is False

    # Test when parse returns None
    with patch("cldk.analysis.commons.treesitter.treesitter_python.Parser.parse") as parse_mock:
        parse_mock.return_value = None
        code = "def is_parsable(self, code: str) -> bool: return True"
        is_parsable = python_sitter.is_parsable(code)
        assert is_parsable is False

    # Test exception conditions <- Not sure why this doesn't work
    # with patch("cldk.analysis.commons.treesitter.python_sitter.Node.children") as recursion_mock:
    #     recursion_mock.side_effect = RecursionError()
    #     code = "def is_parsable(self, code: str) -> bool: return True"
    #     is_parsable = python_sitter.is_parsable(code)
    #     assert is_parsable is False


def test_get_raw_ast():
    """Should return the raw AST"""
    python_sitter = TreesitterPython()

    raw_ast = python_sitter.get_raw_ast(PYTHON_CODE)
    assert raw_ast is not None
    assert isinstance(raw_ast, Tree)
    assert raw_ast.root_node is not None


def test_get_all_methods():
    """Should return all of the methods"""
    python_sitter = TreesitterPython()

    all_methods = python_sitter.get_all_methods(PYTHON_CODE)
    assert all_methods is not None
    assert isinstance(all_methods, List)
    assert len(all_methods) == 7
    for method in all_methods:
        assert isinstance(method, PyMethod)


def test_get_all_functions():
    """Should return all of the functions"""
    python_sitter = TreesitterPython()

    all_functions = python_sitter.get_all_functions(PYTHON_CODE)
    assert all_functions is not None
    assert isinstance(all_functions, List)
    assert len(all_functions) == 1
    for method in all_functions:
        assert isinstance(method, PyMethod)


def test_get_method_details():
    """Should return the method details"""
    python_sitter = TreesitterPython()

    method_details = python_sitter.get_method_details(PYTHON_CODE, "add(self, a, b)")
    assert method_details is not None
    assert isinstance(method_details, PyMethod)
    assert method_details.full_signature == "add(self, a, b)"

    # Test when get_all_methods returns empty list
    with patch("cldk.analysis.commons.treesitter.treesitter_python.TreesitterPython.get_all_methods") as method_mock:
        method_mock.return_value = []
        method_details = python_sitter.get_method_details(PYTHON_CODE, "add(self, a, b)")
        assert method_details is None


def test_get_all_imports():
    """Should return all of the imports"""
    python_sitter = TreesitterPython()

    all_imports = python_sitter.get_all_imports(PYTHON_CODE)
    assert all_imports is not None
    assert isinstance(all_imports, List)
    assert len(all_imports) == 3
    assert "import os" in all_imports
    assert "from typing import List" in all_imports
    assert "from math import *" in all_imports


def test_get_module_details():
    """Should return the module details"""
    python_sitter = TreesitterPython()

    module_details = python_sitter.get_module_details(PYTHON_CODE)
    assert module_details is not None
    assert isinstance(module_details, PyModule)
    assert len(module_details.functions) == 1
    assert len(module_details.classes) == 1
    assert len(module_details.imports) == 3


def test_get_all_import_details():
    """Should return all of the import details"""
    python_sitter = TreesitterPython()

    all_import_details = python_sitter.get_all_imports_details(PYTHON_CODE)
    assert all_import_details is not None
    assert isinstance(all_import_details, List)
    assert len(all_import_details) == 3
    for import_details in all_import_details:
        assert isinstance(import_details, PyImport)


def test_get_all_classes():
    """Should return all of the classes"""
    python_sitter = TreesitterPython()

    all_classes = python_sitter.get_all_classes(PYTHON_CODE)
    assert all_classes is not None
    assert isinstance(all_classes, List)
    assert len(all_classes) == 1
    assert isinstance(all_classes[0], PyClass)
    assert all_classes[0].class_name == "Calculator"
    assert len(all_classes[0].methods) == 7


def test_get_all_modules(tmp_path):
    """Should return all of the modules"""
    python_sitter = TreesitterPython()

    # set up some temporary modules
    temp_file_path = os.path.join(tmp_path, "hello.py")
    with open(temp_file_path, "w", encoding="utf-8") as hello_module:
        hello_module.write('print("Hello, world!")')
    temp_file_path = os.path.join(tmp_path, "bye.py")
    with open(temp_file_path, "w", encoding="utf-8") as bye_module:
        bye_module.write('print("Goodbye, world!")')

    all_modules = python_sitter.get_all_modules(tmp_path)
    assert all_modules is not None
    assert isinstance(all_modules, List)
    assert len(all_modules) == 2
    for module in all_modules:
        assert isinstance(module, PyModule)
