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
Three Sitter Utils module
"""

from tree_sitter import Query, Node

from cldk.models.treesitter import Captures


class TreeSitterUtils:
    def frame_query_and_capture_output(self, parser, language, query: str, code_to_process: str) -> Captures:
        """Frame a query for the tree-sitter parser.

        Parameters
        ----------
        query : str
            The query to frame.
        code_to_process : str
            The code to process.
        """
        framed_query: Query = language.query(query)
        tree = parser.parse(bytes(code_to_process, "utf-8"))
        return Captures(framed_query.captures(tree.root_node))

    def safe_ascend(self, node: Node, ascend_count: int) -> Node:
        """Safely ascend the tree. If the node does not exist or if it has no parent, raise an error.

        Parameters
        ----------
        node : Node
            The node to ascend from.
        ascend_count : int
            The number of times to ascend the tree.

        Returns
        -------
        Node
            The node at the specified level of the tree.

        Raises
        ------
        ValueError
            If the node has no parent.
        """
        if node is None:
            raise ValueError("Node does not exist.")
        if node.parent is None:
            raise ValueError("Node has no parent.")
        if ascend_count == 0:
            return node
        else:
            return self.safe_ascend(node.parent, ascend_count - 1)
