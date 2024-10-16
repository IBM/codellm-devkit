from abc import ABC
from pathlib import Path
from typing import Dict, List
from pandas import DataFrame

from cldk.analysis import SymbolTable
from cldk.analysis.python.treesitter import PythonSitter
from cldk.models.python.models import PyMethod, PyImport, PyModule, PyClass


class PythonAnalysis(SymbolTable):
    def __init__(
            self,
            analysis_backend: str,
            eager_analysis: bool,
            project_dir: str | Path | None,
            source_code: str | None,
            analysis_backend_path: str | None,
            analysis_json_path: str | Path | None,
            use_graalvm_binary: bool = None,
    ) -> None:
        self.project_dir = project_dir
        self.source_code = source_code
        self.analysis_json_path = analysis_json_path
        self.analysis_backend_path = analysis_backend_path
        self.eager_analysis = eager_analysis
        self.use_graalvm_binary = use_graalvm_binary

        # Initialize the analysis analysis_backend
        if analysis_backend.lower() == "codeql":
            raise NotImplementedError(f"Support for {analysis_backend} has not been implemented yet.")
        elif analysis_backend.lower() == "codeanalyzer":
            raise NotImplementedError(f"Support for {analysis_backend} has not been implemented yet.")
        elif analysis_backend.lower() == "treesitter":
            self.analysis_backend: PythonSitter = PythonSitter()
        else:
            raise NotImplementedError(f"Support for {analysis_backend} has not been implemented yet.")

    def get_methods(self) -> List[PyMethod]:
        """
        Given an application or a source code, get all the methods
        """
        return self.analysis_backend.get_all_methods(self.source_code)

    def get_functions(self) -> List[PyMethod]:
        """
        Given an application or a source code, get all the methods
        """
        return self.analysis_backend.get_all_functions(self.source_code)

    def get_modules(self) -> List[PyModule]:
        """
        Given the project directory, get all the modules
        """
        return self.analysis_backend.get_all_modules(self.project_dir)

    def get_method_details(self, method_signature: str) -> PyMethod:
        """
        Given the code body and the method signature, returns the method details related to that method
        Parameters
        ----------
        method_signature: method signature

        Returns
        -------
            PyMethod: Returns the method details related to that method
        """
        return self.analysis_backend.get_method_details(self.source_code, method_signature)

    def is_parsable(self, source_code: str) -> bool:
        """
                Check if the code is parsable
                Args:
                    source_code: source code

                Returns:
                    True if the code is parsable, False otherwise
        """
        return PythonSitter.is_parsable(self, source_code)

    def get_raw_ast(self, source_code: str) -> str:
        """
        Get the raw AST
        Args:
            code: source code

        Returns:
            Tree: the raw AST
        """
        return PythonSitter.get_raw_ast(self, source_code)

    def get_imports(self) ->  List[PyImport]:
        """
        Given an application or a source code, get all the imports
        """
        return self.analysis_backend.get_all_imports_details(self.source_code)

    def get_variables(self, **kwargs):
        """
        Given an application or a source code, get all the variables
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_classes(self) -> List[PyClass]:
        """
        Given an application or a source code, get all the classes
        """
        return self.analysis_backend.get_all_classes(self.source_code)

    def get_classes_by_criteria(self, **kwargs):
        """
        Given an application or a source code, get all the classes given the inclusion and exclution criteria
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_sub_classes(self, **kwargs):
        """
        Given an application or a source code, get all the sub-classes
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_nested_classes(self, **kwargs):
        """
        Given an application or a source code, get all the nested classes
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_constructors(self, **kwargs):
        """
        Given an application or a source code, get all the constructors
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_methods_in_class(self, **kwargs):
        """
        Given an application or a source code, get all the methods within the given class
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_fields(self, **kwargs):
        """
        Given an application or a source code, get all the fields
        """
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
