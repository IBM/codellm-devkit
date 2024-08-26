from enum import Enum


class AnalysisLevel(str, Enum):
    """Analysis levels"""
    symbol_table = "symbol-table"
    call_graph = "call-graph"
    program_dependency_graph = "program-dependency-graph"
    system_dependency_graph = "system-dependency-graph"
