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
SymbolTable module
"""

from abc import ABC, abstractmethod


class SymbolTable(ABC):
    def __init__(self) -> None:
        super().__init__()

    """
    Language agnostic functions
    """

    @abstractmethod
    def get_methods(self, **kwargs):
        """
        Given an application or a source code, get all the methods
        """
        pass

    @abstractmethod
    def get_imports(self, **kwargs):
        """
        Given an application or a source code, get all the imports
        """
        pass

    @abstractmethod
    def get_variables(self, **kwargs):
        """
        Given an application or a source code, get all the variables
        """
        pass

    """
        OOP-specific functions
    """

    @abstractmethod
    def get_classes(self, **kwargs):
        """
        Given an application or a source code, get all the classes
        """
        pass

    @abstractmethod
    def get_classes_by_criteria(self, **kwargs):
        """
        Given an application or a source code, get all the classes given the inclusion and exclution criteria
        """
        pass

    @abstractmethod
    def get_sub_classes(self, **kwargs):
        """
        Given an application or a source code, get all the sub-classes
        """
        pass

    @abstractmethod
    def get_nested_classes(self, **kwargs):
        """
        Given an application or a source code, get all the nested classes
        """
        pass

    @abstractmethod
    def get_constructors(self, **kwargs):
        """
        Given an application or a source code, get all the constructors
        """
        pass

    @abstractmethod
    def get_methods_in_class(self, **kwargs):
        """
        Given an application or a source code, get all the methods within the given class
        """
        pass

    @abstractmethod
    def get_fields(self, **kwargs):
        """
        Given an application or a source code, get all the fields
        """
        pass
