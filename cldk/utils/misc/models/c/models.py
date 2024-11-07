################################################################################
# Copyright IBM Corporation 2024
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
Models module
"""

from typing import List, Optional
from pydantic import BaseModel


class COutput(BaseModel):
    """
    Represents the output of a C function.

    Parameters
    ----------
    type : str
        The type of the output.
    qualifiers : List[str]
        The list of type qualifiers. E.g.: const, volatile, restrict, etc.
    is_reference : bool
        A flag indicating whether the output is a pointer.
    """

    type: str
    # Type qualifiers: const, volatile, restrict, etc.
    qualifiers: List[str]
    is_reference: bool


class CParameter(COutput):
    """
    Represents a parameter in a C function.

    Parameters
    ----------
    name : str
        The name of the parameter.
    specifiers : List[str]
        The list of storage class specifiers. E.g.: auto, register, static, extern, etc.
    """

    name: str
    # Storage-class specifiers: auto, register, static, extern, etc.
    specifiers: List[str]


class CFunction(BaseModel):
    """
    Represents a function in C.

    Parameters
    ----------
    name : str
        The name of the function.
    signature : str
        The signature of the function.
    comment : Optional[str]
        The comment associated with the function.
    parameters : List[CParameter]
        The parameters of the function.
    output : COutput
        The return of the function.
    code : str
        The code block of the callable.
    start_line : int
        The starting line number of the callable in the source file.
    end_line : int
        The ending line number of the callable in the source file.
    specifiers : List[str]
        The list of storage class specifiers. E.g.: auto, register, static, extern, etc.
    """

    name: str
    signature: str
    code: str
    start_line: int
    end_line: int
    parameters: List[CParameter]
    output: COutput
    comment: Optional[str]
    # Storage-class specifiers: auto, register, static, extern, etc.
    specifiers: List[str]


class CImport(BaseModel):
    """
    Represents a C import.

    Parameters
    ----------
    value : str
        The name or path of file being imported.
    is_system : bool
        A flag indicating whether the import is a system one.
    """

    value: str
    is_system: bool


class CTranslationUnit(BaseModel):
    """
    Represents the content of a C file.

    Parameters
    ----------
    imports : List[CImport]
        The list of imports present inside the file.
    functions : List[CFunction]
        The functions defined inside the file.
    """

    imports: List[CImport]
    functions: List[CFunction]

    # TODO: type definitions, structs
