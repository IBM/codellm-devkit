from abc import ABC, abstractmethod


class ProgramDependenceGraph(ABC):
    def __init__(self) -> None:
        super().__init__()
