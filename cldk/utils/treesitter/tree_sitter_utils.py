from tree_sitter import Query, Node

from cldk.models.treesitter import Captures


class TreeSitterUtils:
    def __frame_query_and_capture_output(self, query: str, code_to_process: str) -> Captures:
        """Frame a query for the tree-sitter parser.

        Parameters
        ----------
        query : str
            The query to frame.
        code_to_process : str
            The code to process.
        """
        framed_query: Query = self.language.query(query)
        tree = self.parser.parse(bytes(code_to_process, "utf-8"))
        return Captures(framed_query.captures(tree.root_node))

    def __safe_ascend(self, node: Node, ascend_count: int) -> Node:
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
            return self.__safe_ascend(node.parent, ascend_count - 1)