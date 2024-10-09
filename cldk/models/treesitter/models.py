from dataclasses import dataclass
from typing import Dict, List, Tuple
from ipdb import set_trace
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
