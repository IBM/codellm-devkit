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

from dataclasses import dataclass
from typing import Dict, List

from tree_sitter import Node


@dataclass
class Captures:
    """This class is a dataclass that represents the captures from a tree-sitter query.
    Attributes
    ----------
    captures : List[Capture]
        A list of captures from the tree-sitter query.
    """

    @dataclass
    class Capture:
        """This class is a dataclass that represents a single capture from a tree-sitter query.
        Attributes
        ----------
        node : Node
            The node that was captured.
        name : str
            The name of the capture.
        """

        node: Node
        name: str

    def __init__(self, captures: Dict[str, List[Node]]):
        self.captures = []
        for capture_name, captures in captures.items():
            self.captures = [self.Capture(node=node, name=capture_name) for node in captures]

    def __getitem__(self, index: int) -> Capture:
        """Get the capture at the specified index.
        Parameters:
        -----------
        index : int
            The index of the capture to get.
        Returns
        -------
        Capture
            The capture at the specified index.
        """
        return self.captures[index]

    def __iter__(self):
        """Return an iterator over the captures."""
        return iter(self.captures)

    def __len__(self) -> int:
        """Return the number of captures."""
        return len(self.captures)
