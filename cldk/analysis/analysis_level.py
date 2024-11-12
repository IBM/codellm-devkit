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
AnalysisLevel module
"""

from enum import Enum


class AnalysisLevel(str, Enum):
    """Analysis levels"""

    symbol_table = "symbol table"
    call_graph = "call graph"
    program_dependency_graph = "program dependency graph"
    system_dependency_graph = "system dependency graph"
