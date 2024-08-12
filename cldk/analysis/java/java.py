from pathlib import Path

from typing import Dict, List, Tuple, Set
from networkx import DiGraph

from cldk.analysis import SymbolTable, CallGraph
from cldk.models.java import JCallable
from cldk.models.java import JApplication
from cldk.models.java.models import JCompilationUnit, JMethodDetail, JType, JField
from cldk.analysis.java.codeanalyzer import JCodeanalyzer
from cldk.analysis.java.codeql import JCodeQL
from cldk.utils.analysis_engine import AnalysisEngine


class JavaAnalysis(SymbolTable, CallGraph):

    def __init__(
        self,
        project_dir: str | Path | None,
        source_code: str | None,
        analysis_backend: str,
        analysis_backend_path: str | None,
        analysis_json_path: str | Path | None,
        analysis_level: str,
        use_graalvm_binary: bool,
        eager_analysis: bool,
    ) -> None:
        """
        Parameters
        ----------
        project_dir : str
            The directory path of the project.
        analysis_backend : str, optional
            The analysis_backend used for analysis, defaults to "codeql".
        analysis_backend_path : str, optional
            The path to the analysis_backend, defaults to None and in the case of codeql, it is assumed that the cli is installed and
            available in the PATH. In the case of codeanalyzer the codeanalyzer.jar is downloaded from the lastest release.
        analysis_json_path : str or Path, optional
            The path save the to the analysis database (analysis.json), defaults to None. If None, the analysis database is
            not persisted.
        use_graalvm_binary : bool, optional
            A flag indicating whether to use the GraalVM binary for SDG analysis, defaults to False. If False, the default
            Java binary is used and one needs to have Java 17 or higher installed.
        eager_analysis : bool, optional
            A flag indicating whether to perform eager analysis, defaults to False. If True, the analysis is performed
            eagerly. That is, the analysis.json file is created during analysis every time even if it already exists.

        Attributes
        ----------
        analysis_backend : JCodeQL | JApplication
            The analysis_backend used for analysis.
        application : JApplication
            The application view of the Java code.
        """
        self.project_dir = project_dir
        self.source_code = source_code
        self.analysis_level = analysis_level
        self.analysis_json_path = analysis_json_path
        self.analysis_backend_path = analysis_backend_path
        self.eager_analysis = eager_analysis
        self.use_graalvm_binary = use_graalvm_binary
        self.analysis_backend =  analysis_backend
        # Initialize the analysis analysis_backend
        if analysis_backend.lower() == "codeql":
            self.analysis_backend: JCodeQL = JCodeQL(self.project_dir, self.analysis_json_path)
        elif analysis_backend.lower() == "codeanalyzer":
            self.backend: JCodeanalyzer = JCodeanalyzer(
                project_dir=self.project_dir,
                source_code=self.source_code,
                eager_analysis=self.eager_analysis,
                analysis_level=self.analysis_level,
                analysis_json_path=self.analysis_json_path,
                use_graalvm_binary=self.use_graalvm_binary,
                analysis_backend_path=self.analysis_backend_path,
            )
        else:
            raise NotImplementedError(f"Support for {analysis_backend} has not been implemented yet.")

    def get_imports(self) -> List[str]:
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_variables(self, **kwargs):
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_service_entry_point_classes(self, **kwargs):
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_service_entry_point_methods(self, **kwargs):
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_application_view(self) -> JApplication:
        """
        Returns the application view of the Java code.

        Returns:
        --------
        JApplication
            The application view of the Java code.
        """
        if self.source_code:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_application_view()

    def get_symbol_table(self) -> Dict[str, JCompilationUnit]:
        """
        Returns the symbol table of the Java code.

        Returns:
        --------
        Dict[str, JCompilationUnit]
            The application view of the Java code.
        """
        return self.backend.get_symbol_table()

    def get_compilation_units(self) -> List[JCompilationUnit]:
        """
        Returns the compilation units of the Java code.

        Returns
        -------
        Dict[str, JCompilationUnit]
            The compilation units of the Java code.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_compilation_units()

    def get_class_hierarchy(self) -> DiGraph:
        """
        Returns the class hierarchy of the Java code.

        Returns:
        --------
        DiGraph
            The class hierarchy of the Java code.
        """
        if self.backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        raise NotImplementedError("Class hierarchy is not implemented yet.")

    def get_call_graph(self) -> DiGraph:
        """
        Returns the call graph of the Java code.

        Returns:
        --------
        DiGraph
            The call graph of the Java code.
        """
        return self.backend.get_call_graph()

    def get_call_graph_json(self) -> str:
        """
        serialize callgraph to json
        """
        if self.source_code:
            raise NotImplementedError("Producing a call graph over a single file is not implemented yet.")
        return self.backend.get_call_graph_json()

    def get_callers(self, target_class_name: str, target_method_declaration: str):
        """
        Get all the caller details for a given java method.

        Returns:
        --------
        Dict
            Caller details in a dictionary.
        """
        if self.source_code:
            raise NotImplementedError("Generating all callers over a single file is not implemented yet.")
        return self.backend.get_all_callers(target_class_name, target_method_declaration)

    def get_callees(self, source_class_name: str, source_method_declaration: str):
        """
        Get all the callee details for a given java method.

        Returns:
        --------
        Dict
            Callee details in a dictionary.
        """
        if self.source_code:
            raise NotImplementedError("Generating all callees over a single file is not implemented yet.")
        return self.backend.get_all_callees(source_class_name, source_method_declaration)

    def get_methods(self) -> Dict[str, Dict[str, JCallable]]:
        """
        Returns a dictionary of all methods in the Java code with
          qualified class name as key and dictionary of methods in that class
          as value

        Returns:
        --------
        Dict[str, Dict[str, JCallable]]:
            A dictionary of dictionaries of all methods in the Java code.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_methods_in_application()

    def get_classes(self) -> Dict[str, JType]:
        """
        Returns a dictionary of all classes in the Java code.

        Returns:
        --------
        Dict[str, JType]
            A dict of all classes in the Java code, with qualified class names as keys
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_classes()

    def get_classes_by_criteria(self, inclusions=None, exclusions=None) -> Dict[str, JType]:
        """
        Returns a dictionary of all classes in the Java code.

        #TODO: Document the parameters inclusions and exclusions.

        Returns:
        --------
        Dict[str, JType]
            A dict of all classes in the Java code, with qualified class names as keys
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

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

    def get_class(self, qualified_class_name) -> JType:
        """
        Returns a class given qualified class name.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        JType
            A class for the given qualified class name.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_class(qualified_class_name)

    def get_method(self, qualified_class_name, qualified_method_name) -> JCallable:
        """
        Returns a method given qualified method name.

        Parameters:
        -----------
        qualified_method_name : str
            The qualified name of the method.

        Returns:
        --------
        JCallable
            A method for the given qualified method name.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_method(qualified_class_name, qualified_method_name)

    def get_java_file(self, qualified_class_name) -> str:
        """
        Returns a class given qualified class name.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        str
            Java file name containing the given qualified class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_java_file(qualified_class_name)

    def get_java_compilation_unit(self, file_path: str) -> JCompilationUnit:
        """
        Given the path of a Java source file, returns the compilation unit object from the symbol table.

        Parameters
        ----------
        file_path : str
            Absolute path to Java source file

        Returns
        -------
        JCompilationUnit
            Compilation unit object for Java source file
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_java_compilation_unit(file_path)

    def get_methods_in_class(self, qualified_class_name) -> Dict[str, JCallable]:
        """
        Returns a dictionary of all methods in the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        Dict[str, JCallable]
            A dictionary of all methods in the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_methods_in_class(qualified_class_name)

    def get_constructors(self, qualified_class_name) -> Dict[str, JCallable]:
        """
        Returns a dictionary of all constructors of the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        Dict[str, JCallable]
            A dictionary of all constructors of the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_constructors(qualified_class_name)

    def get_fields(self, qualified_class_name) -> List[JField]:
        """
        Returns a list of all fields of the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        List[JField]
            A list of all fields of the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_fields(qualified_class_name)

    def get_nested_classes(self, qualified_class_name) -> List[JType]:
        """
        Returns a list of all nested classes for the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        List[JType]
            A list of nested classes for the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_nested_classes(qualified_class_name)

    def get_sub_classes(self, qualified_class_name) -> Dict[str, JType]:
        """
        Returns a dictionary of all sub-classes of the given class
        Parameters
        ----------
        qualified_class_name

        Returns
        -------
            Dict[str, JType]: A dictionary of all sub-classes of the given class, and class details
        """
        return self.backend.get_all_sub_classes(qualified_class_name=qualified_class_name)

    def get_extended_classes(self, qualified_class_name) -> List[str]:
        """
        Returns a list of all extended classes for the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        List[JType]
            A list of extended classes for the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_extended_classes(qualified_class_name)

    def get_implemented_interfaces(self, qualified_class_name) -> List[str]:
        """
        Returns a list of all implemented interfaces for the given class.

        Parameters:
        -----------
        qualified_class_name : str
            The qualified name of the class.

        Returns:
        --------
        List[JType]
            A list of implemented interfaces for the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_implemented_interfaces(qualified_class_name)

    def get_class_call_graph(self, qualified_class_name: str, method_name: str | None = None) -> (List)[Tuple[JMethodDetail, JMethodDetail]]:
        """
        A call graph for a given class and (optionally) a given method.

        Parameters
        ----------
        qualified_class_name : str
            The qualified name of the class.
        method_name : str, optional
            The name of the method in the class.

        Returns
        -------
        List[Tuple[JMethodDetail, JMethodDetail]]
            An edge list of the call graph for the given class and method.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_class_call_graph(qualified_class_name, method_name)

    def get_entry_point_classes(self) -> Dict[str, JType]:
        """
        Returns a dictionary of all entry point classes in the Java code.

        Returns:
        --------
        Dict[str, JType]
            A dict of all entry point classes in the Java code, with qualified class names as keys
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_entry_point_classes()

    def get_entry_point_methods(self) -> Dict[str, Dict[str, JCallable]]:
        """
        Returns a dictionary of all entry point methods in the Java code with
          qualified class name as key and dictionary of methods in that class
          as value

        Returns:
        --------
        Dict[str, Dict[str, JCallable]]:
            A dictionary of dictionaries of entry point methods in the Java code.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_entry_point_methods()

    def remove_all_comments(self) -> str:
        """
        Remove all comments from the source code.

        Parameters
        ----------
        source_code : str
            The source code to process.

        Returns
        -------
        str
            The source code with all comments removed.
        """

        # Remove any prefix comments/content before the package declaration
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.remove_all_comments(self.source_code)

    def get_methods_with_annotations(self, annotations: List[str]) -> Dict[str, List[Dict]]:
        """
        Returns a dictionary of method names and method bodies.

        Parameters:
        -----------
        source_class_code : str
            String containing code for a java class.
        annotations : List[str]
            List of annotation strings.
        Returns:
        --------
        Dict[str,List[Dict]]
            Dictionary with annotations as keys and
            a list of dictionaries containing method names and bodies, as values.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_methods_with_annotations(self.source_code, annotations)

    def get_test_methods(self, source_class_code: str) -> Dict[str, str]:
        """
        Returns a dictionary of method names and method bodies.

        Parameters:
        -----------
        source_class_code : str
            String containing code for a java class.

        Returns:
        --------
        Dict[str,str]
            Dictionary of method names and method bodies.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_test_methods(self.source_code)

    def get_calling_lines(self, target_method_name: str) -> List[int]:
        """
        Returns a list of line numbers in source method where target method is called.

        Parameters:
        -----------
        source_method_code : str
            source method code

        target_method_code : str
            target method code

        Returns:
        --------
        List[int]
            List of line numbers within in source method code block.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_calling_lines(self.source_code, target_method_name)

    def get_call_targets(self, declared_methods: dict) -> Set[str]:
        """Generate a list of call targets from the method body.

        Uses simple name resolution for finding the call targets. Nothing sophiscticed here. Just a simple search
        over the AST.

        Parameters
        ----------
        method_body : Node
            The method body.
        declared_methods : dict
            A dictionary of all declared methods in the class.

        Returns
        -------
        List[str]
            A list of call targets (methods).
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_call_targets(self.source_code, declared_methods)
