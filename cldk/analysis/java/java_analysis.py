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
Java module
"""

from pathlib import Path
from typing import Dict, List, Tuple, Set, Union
import networkx as nx

from tree_sitter import Tree

from cldk.analysis.commons.treesitter import TreesitterJava
from cldk.models.java import JCallable
from cldk.models.java import JApplication
from cldk.models.java.models import JCRUDOperation, JComment, JCompilationUnit, JMethodDetail, JType, JField
from cldk.analysis.java.codeanalyzer import JCodeanalyzer


class JavaAnalysis:

    def __init__(
        self,
        project_dir: str | Path | None,
        source_code: str | None,
        analysis_backend_path: str | None,
        analysis_json_path: str | Path | None,
        analysis_level: str,
        target_files: List[str] | None,
        eager_analysis: bool,
    ) -> None:
        """Initialization method for Java Analysis backend.

        Args:
            project_dir (str | Path | None): The directory path of the project.
            source_code (str | None): Java file for single source file analysis.
            analysis_backend_path (str | None): The path to the analysis_backend, defaults to None and in the case of codeql, it is assumed that the cli is installed and available in the PATH. In the case of codeanalyzer the codeanalyzer.jar is downloaded from the lastest release.
            analysis_json_path (str | Path | None): The path save the to the analysis database (analysis.json), defaults to None. If None, the analysis database is not persisted.
            analysis_level (str): Analysis level (symbol-table, call-graph)
            eager_analysis (bool): A flag indicating whether to perform eager analysis, defaults to False. If True, the analysis is performed eagerly. That is, the analysis.json file is created during analysis every time even if it already exists.

        Raises:
            NotImplementedError: Raised when anaysis backend is not supported.

        Attributes:
            application (JApplication): The application view of the Java code.

        """

        self.project_dir = project_dir
        self.source_code = source_code
        self.analysis_level = analysis_level
        self.analysis_json_path = analysis_json_path
        self.analysis_backend_path = analysis_backend_path
        self.eager_analysis = eager_analysis
        self.target_files = target_files
        self.treesitter_java: TreesitterJava = TreesitterJava()
        # Initialize the analysis analysis_backend
        self.backend: JCodeanalyzer = JCodeanalyzer(
            project_dir=self.project_dir,
            source_code=self.source_code,
            eager_analysis=self.eager_analysis,
            analysis_level=self.analysis_level,
            analysis_json_path=self.analysis_json_path,
            analysis_backend_path=self.analysis_backend_path,
            target_files=self.target_files,
        )

    def get_imports(self) -> List[str]:
        """Should return  all the imports in the source code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            List[str]: List of all the imports.
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_variables(self, **kwargs):
        """_Returns all the variables.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_service_entry_point_classes(self, **kwargs):
        """Should return  all service entry point classes.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_service_entry_point_methods(self, **kwargs):
        """Should return  all the service entry point methods.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_application_view(self) -> JApplication:
        """Should return  application view of the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            JApplication: Application view of the java code.
        """
        if self.source_code:
            raise NotImplementedError("Support for this functionality has not been implemented yet.")
        return self.backend.get_application_view()

    def get_symbol_table(self) -> Dict[str, JCompilationUnit]:
        """Should return  symbol table.

        Returns:
            Dict[str, JCompilationUnit]: Symbol table
        """
        return self.backend.get_symbol_table()

    def get_compilation_units(self) -> List[JCompilationUnit]:
        """Should return  a list of all compilation units in the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not supported.

        Returns:
            List[JCompilationUnit]: Compilation units of the Java code.
        """
        return self.backend.get_compilation_units()

    def get_class_hierarchy(self) -> nx.DiGraph:
        """Should return  class hierarchy of the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            nx.DiGraph: The class hierarchy of the Java code.
        """

        raise NotImplementedError("Class hierarchy is not implemented yet.")

    def is_parsable(self, source_code: str) -> bool:
        """
        Check if the code is parsable
        Args:
            source_code: source code

        Returns:
            True if the code is parsable, False otherwise
        """
        return self.treesitter_java.is_parsable(source_code)

    def get_raw_ast(self, source_code: str) -> Tree:
        """
        Get the raw AST
        Args:
            code: source code

        Returns:
            Tree: the raw AST
        """
        return self.treesitter_java.get_raw_ast(source_code)

    def get_call_graph(self) -> nx.DiGraph:
        """Should return  the call graph of the Java code.

        Returns:
            nx.DiGraph: The call graph of the Java code.
        """
        return self.backend.get_call_graph()

    def get_call_graph_json(self) -> str:
        """Should return  a serialized call graph in json.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            str: Call graph in json.
        """
        if self.source_code:
            raise NotImplementedError("Producing a call graph over a single file is not implemented yet.")
        return self.backend.get_call_graph_json()

    def get_callers(self, target_class_name: str, target_method_declaration: str, using_symbol_table: bool = False) -> Dict:
        """Should return  a dictionary of callers of the target method.

        Args:
            target_class_name (str): Qualified target class name.
            target_method_declaration (str): Target method names
            using_symbol_table (bool, optional): Whether to use symbol table. Defaults to False.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            Dict: A dictionary of callers of target method.
        """

        if self.source_code:
            raise NotImplementedError("Generating all callers over a single file is not implemented yet.")
        return self.backend.get_all_callers(target_class_name, target_method_declaration, using_symbol_table)

    def get_callees(self, source_class_name: str, source_method_declaration: str, using_symbol_table: bool = False) -> Dict:
        """Should return  a dictionary of callees by the given method in the given class.

        Args:
            source_class_name (str): Qualified class name where the given method is.
            source_method_declaration (str): Given method
            using_symbol_table (bool): Whether to use symbol table.  Defaults to false.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            Dict: Dictionary with callee details.
        """
        if self.source_code:
            raise NotImplementedError("Generating all callees over a single file is not implemented yet.")
        return self.backend.get_all_callees(source_class_name, source_method_declaration, using_symbol_table)

    def get_methods(self) -> Dict[str, Dict[str, JCallable]]:
        """Should return  all methods in the Java code.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, Dict[str, JCallable]]: Dictionary of dictionaries of all methods in the Java code with qualified class name as key and dictionary of methods in that class.
        """
        return self.backend.get_all_methods_in_application()

    def get_classes(self) -> Dict[str, JType]:
        """Should return  all classes in the Java code.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, JType]: A dictionary of all classes in the Java code, with qualified class names as keys.
        """
        return self.backend.get_all_classes()

    def get_classes_by_criteria(self, inclusions=None, exclusions=None) -> Dict[str, JType]:
        """Should return  a dictionary of all classes with the given criteria, in the Java code.

        Args:
            inclusions (List, optional): inlusion criteria for the classes. Defaults to None.
            exclusions (List, optional): exclusion criteria for the classes. Defaults to None.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, JType]: A dict of all classes in the Java code, with qualified class names as keys
        """
        if exclusions is None:
            exclusions = []
        if inclusions is None:
            inclusions = []
        class_dict: Dict[str, JType] = {}
        all_classes = self.backend.get_all_classes()
        for application_class in all_classes:
            is_selected = False
            for inclusion in inclusions:
                if inclusion in application_class:
                    is_selected = True

            for exclusion in exclusions:
                if exclusion in application_class:
                    is_selected = False
            if is_selected:
                class_dict[application_class] = all_classes[application_class]
        return class_dict

    def get_class(self, qualified_class_name: str) -> JType:
        """Should return  a class object given qualified class name.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            JType: Class object for the given qualified class name.
        """

        return self.backend.get_class(qualified_class_name)

    def get_method(self, qualified_class_name: str, qualified_method_name: str) -> JCallable:
        """Should return  a method object given qualified class and method names.

        Args:
            qualified_class_name (str): The qualified name of the class.
            qualified_method_name (str): The qualified name of the method.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            JCallable: A method for the given qualified method name.
        """
        return self.backend.get_method(qualified_class_name, qualified_method_name)

    def get_method_parameters(self, qualified_class_name: str, qualified_method_name: str) -> List[str]:
        """Should return  a list of method parameters given qualified class and method names.

        Args:
            qualified_class_name (str): The qualified name of the class.
            qualified_method_name (str): The qualified name of the method.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            JCallable: A method for the given qualified method name.
        """
        return self.backend.get_method_parameters(qualified_class_name, qualified_method_name)

    def get_java_file(self, qualified_class_name: str) -> str:
        """Should return  a class given qualified class name.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            str: Java file name containing the given qualified class.
        """
        return self.backend.get_java_file(qualified_class_name)

    def get_java_compilation_unit(self, file_path: str) -> JCompilationUnit:
        """Given the path of a Java source file, returns the compilation unit object from the symbol table.

        Args:
            file_path (str): Absolute path to Java source file

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            JCompilationUnit: Compilation unit object for Java source file
        """
        return self.backend.get_java_compilation_unit(file_path)

    def get_methods_in_class(self, qualified_class_name) -> Dict[str, JCallable]:
        """Should return  a dictionary of all methods of the given class.

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, JCallable]: A dictionary of all constructors of the given class.
        """
        return self.backend.get_all_methods_in_class(qualified_class_name)

    def get_constructors(self, qualified_class_name) -> Dict[str, JCallable]:
        """Should return  a dictionary of all constructors of the given class.

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, JCallable]: A dictionary of all constructors of the given class.
        """
        return self.backend.get_all_constructors(qualified_class_name)

    def get_fields(self, qualified_class_name) -> List[JField]:
        """Should return  a dictionary of all fields of the given class

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[JField]: A list of all fields of the given class.
        """
        return self.backend.get_all_fields(qualified_class_name)

    def get_nested_classes(self, qualified_class_name) -> List[JType]:
        """Should return  a dictionary of all nested classes of the given class

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[JType]: A list of nested classes for the given class.
        """
        return self.backend.get_all_nested_classes(qualified_class_name)

    def get_sub_classes(self, qualified_class_name) -> Dict[str, JType]:
        """Should return  a dictionary of all sub-classes of the given class

        Args:
            qualified_class_name (str): qualified class name

        Returns:
            Dict[str, JType]: A dictionary of all sub-classes of the given class, and class details
        """
        return self.backend.get_all_sub_classes(qualified_class_name=qualified_class_name)

    def get_extended_classes(self, qualified_class_name) -> List[str]:
        """Should return  a list of all extended classes for the given class.
        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[str]: A list of extended classes for the given class.
        """
        return self.backend.get_extended_classes(qualified_class_name)

    def get_implemented_interfaces(self, qualified_class_name: str) -> List[str]:
        """Should return  a list of all implemented interfaces for the given class.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[str]: A list of implemented interfaces for the given class.
        """
        return self.backend.get_implemented_interfaces(qualified_class_name)

    def __get_class_call_graph_using_symbol_table(self, qualified_class_name: str, method_signature: str | None = None) -> (List)[Tuple[JMethodDetail, JMethodDetail]]:
        """A call graph using symbol table for a given class and a given method.

        Args:
            qualified_class_name (str): The qualified name of the class.
            method_signature (str | None, optional): The signature of the method in the class.. Defaults to None.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[Tuple[JMethodDetail, JMethodDetail]]: An edge list of the call graph for the given class and method.
        """
        return self.backend.get_class_call_graph_using_symbol_table(qualified_class_name, method_signature)

    def get_class_call_graph(self, qualified_class_name: str, method_signature: str | None = None, using_symbol_table: bool = False) -> List[Tuple[JMethodDetail, JMethodDetail]]:
        """A call graph for a given class and (optionally) a given method.

        Args:
            qualified_class_name (str): The qualified name of the class.
            method_signature (str | None, optional): The signature of the method in the class.. Defaults to None.
            using_symbol_table (bool, optional): Generate call graph using symbol table. Defaults to False.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[Tuple[JMethodDetail, JMethodDetail]]: An edge list of the call graph for the given class and method.
        """
        if using_symbol_table:
            return self.__get_class_call_graph_using_symbol_table(qualified_class_name=qualified_class_name, method_signature=method_signature)
        return self.backend.get_class_call_graph(qualified_class_name, method_signature)

    def get_entry_point_classes(self) -> Dict[str, JType]:
        """Should return  a dictionary of all entry point classes in the Java code.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, JType]: A dict of all entry point classes in the Java code, with qualified class names as keys
        """
        return self.backend.get_all_entry_point_classes()

    def get_entry_point_methods(self) -> Dict[str, Dict[str, JCallable]]:
        """Should return  a dictionary of all entry point methods in the Java code with qualified class name as key and dictionary of methods in that class as value

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, Dict[str, JCallable]]: A dictionary of dictionaries of entry point methods in the Java code.
        """
        return self.backend.get_all_entry_point_methods()

    def remove_all_comments(self) -> str:
        """Remove all comments from the source code.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            str: The source code with all comments removed.
        """
        return self.backend.remove_all_comments(self.source_code)

    def get_methods_with_annotations(self, annotations: List[str]) -> Dict[str, List[Dict]]:
        """Should return  a dictionary of method names and method bodies.

        Args:
            annotations (List[str]): List of annotation strings.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, List[Dict]]: Dictionary with annotations as keys and a list of dictionaries containing method names and bodies, as values.
        """
        # TODO: This call is missing some implementation. The logic currently resides in java_sitter but tree_sitter will no longer be option, rather it will be default and common. Need to implement this differently. Somthing like, self.commons.treesitter.get_methods_with_annotations(annotations)
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_test_methods(self) -> Dict[str, str]:
        """Should return  a dictionary of method names and method bodies

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            Dict[str, str]: Dictionary of method names and method bodies.
        """

        return self.treesitter_java.get_test_methods(source_class_code=self.source_code)

    def get_calling_lines(self, target_method_name: str) -> List[int]:
        """Should return  a list of line numbers in source method block where target method is called.

        Args:
            target_method_name (str): target method  name.

        Raises:
            NotImplementedError: Raised when we do not support this function.

        Returns:
            List[int]: List of line numbers within in source method code block.
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_call_targets(self, declared_methods: dict) -> Set[str]:
        """Uses simple name resolution for finding the call targets. Nothing sophiscticed here. Just a simple search over the AST.

        Args:
            declared_methods (dict): A dictionary of all declared methods in the class.

        Returns:
            Set[str]: A list of call targets (methods).
        """
        raise NotImplementedError("Support for this functionality has not been implemented yet.")

    def get_all_crud_operations(self) -> List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]:
        """Should return  a dictionary of all CRUD operations in the source code.

        Returns:
            List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]: A list of all CRUD operations in the source code.
        """
        return self.backend.get_all_crud_operations()

    def get_all_create_operations(self) -> List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]:
        """Should return  a list of all create operations in the source code.

        Returns:
            List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]: A list of all create operations in the source code.
        """
        return self.backend.get_all_create_operations()

    def get_all_read_operations(self) -> List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]:
        """Should return  a list of all read operations in the source code.

        Returns:
            List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]: A list of all read operations in the source code.
        """
        return self.backend.get_all_read_operations()

    def get_all_update_operations(self) -> List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]:
        """Should return  a list of all update operations in the source code.

        Returns:
            List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]: A list of all update operations in the source code.
        """
        return self.backend.get_all_update_operations()

    def get_all_delete_operations(self) -> List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]:
        """Should return  a list of all delete operations in the source code.

        Returns:
            List[Dict[str, Union[JType, JCallable, List[JCRUDOperation]]]]: A list of all delete operations in the source code.
        """
        return self.backend.get_all_delete_operations()

    # Some APIs to process comments
    def get_comments_in_a_method(self, qualified_class_name: str, method_signature: str) -> List[JComment]:
        """Get all comments in a method.

        Args:
            qualified_class_name (str): Qualified name of the class.
            method_signature (str): Signature of the method.

        Returns:
            List[str]: List of comments in the method.
        """
        return self.backend.get_comments_in_a_method(qualified_class_name, method_signature)

    def get_comments_in_a_class(self, qualified_class_name: str) -> List[JComment]:
        """Get all comments in a class.

        Args:
            qualified_class_name (str): Qualified name of the class.

        Returns:
            List[str]: List of comments in the class.
        """
        return self.backend.get_comments_in_a_class(qualified_class_name)

    def get_comment_in_file(self, file_path: str) -> List[JComment]:
        """Get all comments in a file.

        Args:
            file_path (str): Path to the file.

        Returns:
            List[str]: List of comments in the file.
        """
        return self.backend.get_comment_in_file(file_path)

    def get_all_comments(self) -> Dict[str, List[JComment]]:
        """Get all comments in the Java application.

        Returns:
            Dict[str, List[str]]: Dictionary of file paths and their corresponding comments.
        """
        return self.backend.get_all_comments()

    def get_all_docstrings(self) -> Dict[str, List[JComment]]:
        """Get all docstrings in the Java application.

        Returns:
            Dict[str, List[str]]: Dictionary of file paths and their corresponding docstrings.
        """
        return self.backend.get_all_docstrings()
