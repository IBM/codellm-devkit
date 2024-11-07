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


class GoParameter(BaseModel):
    """
    Represents a function/method parameter in Go.

    Parameters
    ----------
    name : Optional[str]
        The name of the parameter. If the parameter is not referenced, the name can be omitted.
    type : str
        The type of the parameter.
    is_reference : bool
        A flag indicating whether the type is a pointer.
    is_variadic : bool
        A flag indicating whether the parameter is variadic.
    """

    # Go allows parameters without name, they can't be used
    name: Optional[str] = None
    type: str
    is_reference: bool
    is_variadic: bool


class GoImport(BaseModel):
    """
    Represents an import in Go.

    Parameters
    ----------
    path : str
        The path to the imported package.
    name : Optional[str]
        The alias of the imported package. Useful when the implicit name is
        too long or to avoid package name clashes.
    """

    path: str
    name: Optional[str] = None


class GoCallable(BaseModel):
    """
    Represents a callable entity such as a method or function in Go.

    Parameters
    ----------
    name : str
        The name of the callable.
    signature : str
        The signature of the callable.
    comment : str
        The comment associated with the callable.
    modifiers : List[str]
        The modifiers applied to the callable (e.g., public, static).
    parameters : List[GoParameter]
        The parameters of the callable.
    return_types : List[str]
        The list of return type of the callable. Empty list, if the callable does not return a value.
    receiver : Optional[GoParameter]
        The callable's associated type. Only applicable for methods.
    code : str
        The code block of the callable.
    start_line : int
        The starting line number of the callable in the source file.
    end_line : int
        The ending line number of the callable in the source file.
    """

    name: str
    signature: str
    comment: str
    modifiers: List[str]
    parameters: List[GoParameter]
    # only methods have a receiver
    receiver: Optional[GoParameter] = None
    return_types: List[str]
    code: str
    start_line: int
    end_line: int


class GoSourceFile(BaseModel):
    """
    Represents a source file in Go.

    Parameters
    ----------
    imports : List[GoImport]
        The list of imports present in the source file.
    callables : List[GoCallable]
        The list of callable entities present in the source file.
    """

    imports: List[GoImport]
    callables: List[GoCallable]

    # TODO: types
