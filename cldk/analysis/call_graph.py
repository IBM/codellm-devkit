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
CallGraph module
"""

from abc import ABC, abstractmethod


class CallGraph(ABC):
    def __init__(self) -> None:
        super().__init__()

    @abstractmethod
    def get_callees(self, **kwargs):
        """
        Given a source code, get all the callees
        """
        pass

    @abstractmethod
    def get_callers(self, **kwargs):
        """
        Given a source code, get all the callers
        """
        pass

    @abstractmethod
    def get_call_graph(self, **kwargs):
        """
        Given an application, get the call graph
        """
        pass

    @abstractmethod
    def get_call_graph_json(self, **kwargs):
        """
        Given an application, get call graph in JSON format
        """
        pass

    @abstractmethod
    def get_class_call_graph(self, **kwargs):
        """
        Given an application and a class, get call graph
        """
        pass

    @abstractmethod
    def get_entry_point_classes(self, **kwargs):
        """
        Given an application, get all the entry point classes
        """
        pass

    @abstractmethod
    def get_entry_point_methods(self, **kwargs):
        """
        Given an application, get all the entry point methods
        """
        pass

    @abstractmethod
    def get_service_entry_point_classes(self, **kwargs):
        """
        Given an application, get all the service entry point classes
        """
        pass

    @abstractmethod
    def get_service_entry_point_methods(self, **kwargs):
        """
        Given an application, get all the service entry point methods
        """
        pass
