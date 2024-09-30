from pathlib import Path

from typing import Dict, List, Tuple, Set
from networkx import DiGraph

from cldk.analysis import SymbolTable, CallGraph, AnalysisLevel
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
            target_files: List[str] | None,
            use_graalvm_binary: bool,
            eager_analysis: bool,
    ) -> None:

        """ Initialization method for Java Analysis backend.

        Args:
            project_dir (str | Path | None): The directory path of the project.
            source_code (str | None): Java file for single source file analysis.
            analysis_backend (str): The analysis_backend used for analysis. Currently 'codeql' and 'codeanalyzer' are supported.
            analysis_backend_path (str | None): The path to the analysis_backend, defaults to None and in the case of codeql, it is assumed that the cli is installed and available in the PATH. In the case of codeanalyzer the codeanalyzer.jar is downloaded from the lastest release.
            analysis_json_path (str | Path | None): The path save the to the analysis database (analysis.json), defaults to None. If None, the analysis database is not persisted.
            analysis_level (str): Analysis level (symbol-table, call-graph)
            use_graalvm_binary (bool): A flag indicating whether to use the GraalVM binary for SDG analysis, defaults to False. If False, the default Java binary is used and one needs to have Java 17 or higher installed.
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
        self.use_graalvm_binary = use_graalvm_binary
        self.analysis_backend = analysis_backend
        self.target_files = target_files
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
                target_files=self.target_files
            )
        else:
            raise NotImplementedError(f"Support for {analysis_backend} has not been implemented yet.")

    def get_imports(self) -> List[str]:
        """ Returns all the imports in the source code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            List[str]: List of all the imports.
        """        
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_variables(self, **kwargs):
        """ _Returns all the variables.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """        
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_service_entry_point_classes(self, **kwargs):
        """ Returns all service entry point classes.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """        
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_service_entry_point_methods(self, **kwargs):
        """ Returns all the service entry point methods.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.
        """        
        raise NotImplementedError(f"Support for this functionality has not been implemented yet.")

    def get_application_view(self) -> JApplication:
        """ Returns application view of the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            JApplication: Application view of the java code.
        """        
        if self.source_code:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_application_view()

    def get_symbol_table(self) -> Dict[str, JCompilationUnit]:
        """ Returns symbol table.

        Returns:
            Dict[str, JCompilationUnit]: Symbol table
        """        
        return self.backend.get_symbol_table()

    def get_compilation_units(self) -> List[JCompilationUnit]:
        """ Returns a list of all compilation units in the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            List[JCompilationUnit]: Compilation units of the Java code.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_compilation_units()

    def get_class_hierarchy(self) -> DiGraph:
        """ Returns class hierarchy of the java code.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            DiGraph: The class hierarchy of the Java code.
        """        

        if self.backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        raise NotImplementedError("Class hierarchy is not implemented yet.")

    def get_call_graph(self) -> DiGraph:
        """ Returns the call graph of the Java code.

        Returns:
            DiGraph: The call graph of the Java code.
        """        
        return self.backend.get_call_graph()

    def get_call_graph_json(self) -> str:
        """ Returns a serialized call graph in json.

        Raises:
            NotImplementedError: Raised when this functionality is not suported.

        Returns:
            str: Call graph in json.
        """        
        if self.source_code:
            raise NotImplementedError("Producing a call graph over a single file is not implemented yet.")
        return self.backend.get_call_graph_json()

    def get_callers(self, target_class_name: str, target_method_declaration: str,
                    using_symbol_table: bool = False) -> Dict:
        """ Returns a dictionary of callers of the target method.

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


    def get_callees(self, source_class_name: str, source_method_declaration: str, using_symbol_table: bool = False) ->Dict:
        """ Returns a dictionary of callees by the given method in the given class.

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
        """ Returns all methods in the Java code.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, Dict[str, JCallable]]: Dictionary of dictionaries of all methods in the Java code with qualified class name as key and dictionary of methods in that class.
        """               
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_methods_in_application()

    def get_classes(self) -> Dict[str, JType]:
        """ Returns all classes in the Java code.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, JType]: A dictionary of all classes in the Java code, with qualified class names as keys.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_classes()

    def get_classes_by_criteria(self, inclusions=None, exclusions=None) -> Dict[str, JType]:
        """ Returns a dictionary of all classes with the given criteria, in the Java code.

        Args:
            inclusions (List, optional): inlusion criteria for the classes. Defaults to None.
            exclusions (List, optional): exclusion criteria for the classes. Defaults to None.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, JType]: A dict of all classes in the Java code, with qualified class names as keys
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

    def get_class(self, qualified_class_name:str) -> JType:
        """ Returns a class object given qualified class name.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            JType: Class object for the given qualified class name.
        """        

        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_class(qualified_class_name)

    def get_method(self, qualified_class_name:str, qualified_method_name:str) -> JCallable:
        """ Returns a method object given qualified class and method names.

        Args:
            qualified_class_name (str): The qualified name of the class.
            qualified_method_name (str): The qualified name of the method.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            JCallable: A method for the given qualified method name.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_method(qualified_class_name, qualified_method_name)

    def get_java_file(self, qualified_class_name: str) -> str:
        """ Returns a class given qualified class name.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            str: Java file name containing the given qualified class.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_java_file(qualified_class_name)

    def get_java_compilation_unit(self, file_path: str) -> JCompilationUnit:
        """ Given the path of a Java source file, returns the compilation unit object from the symbol table.

        Args:
            file_path (str): Absolute path to Java source file

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            JCompilationUnit: Compilation unit object for Java source file
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_java_compilation_unit(file_path)

    def get_methods_in_class(self, qualified_class_name) -> Dict[str, JCallable]:
        """ Returns a dictionary of all methods of the given class.

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, JCallable]: A dictionary of all constructors of the given class.
        """ 
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_methods_in_class(qualified_class_name)

    def get_constructors(self, qualified_class_name) -> Dict[str, JCallable]:
        """ Returns a dictionary of all constructors of the given class.

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, JCallable]: A dictionary of all constructors of the given class.
        """ 
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_constructors(qualified_class_name)

    def get_fields(self, qualified_class_name) -> List[JField]:
        """ Returns a dictionary of all fields of the given class

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[JField]: A list of all fields of the given class.
        """ 
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_fields(qualified_class_name)

    def get_nested_classes(self, qualified_class_name) -> List[JType]:
        """ Returns a dictionary of all nested classes of the given class

        Args:
            qualified_class_name (str): qualified class name

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[JType]: A list of nested classes for the given class.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_nested_classes(qualified_class_name)

    def get_sub_classes(self, qualified_class_name) -> Dict[str, JType]:
        """ Returns a dictionary of all sub-classes of the given class

        Args:
            qualified_class_name (str): qualified class name

        Returns:
            Dict[str, JType]: A dictionary of all sub-classes of the given class, and class details
        """  
        return self.backend.get_all_sub_classes(qualified_class_name=qualified_class_name)

    def get_extended_classes(self, qualified_class_name) -> List[str]:
        """ Returns a list of all extended classes for the given class.
        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[str]: A list of extended classes for the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_extended_classes(qualified_class_name)

    def get_implemented_interfaces(self, qualified_class_name: str) -> List[str]:
        """Returns a list of all implemented interfaces for the given class.

        Args:
            qualified_class_name (str): The qualified name of the class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[str]: A list of implemented interfaces for the given class.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_implemented_interfaces(qualified_class_name)

    def __get_class_call_graph_using_symbol_table(self, qualified_class_name: str, method_signature: str | None = None) -> (List)[Tuple[JMethodDetail, JMethodDetail]]:
        """A call graph using symbol table for a given class and a given method.

        Args:
            qualified_class_name (str): The qualified name of the class.
            method_signature (str | None, optional): The signature of the method in the class.. Defaults to None.
        
        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[Tuple[JMethodDetail, JMethodDetail]]: An edge list of the call graph for the given class and method.
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_class_call_graph_using_symbol_table(qualified_class_name, method_signature)

    def get_class_call_graph(self, qualified_class_name: str, method_signature: str | None = None,
                             using_symbol_table: bool = False) -> List[Tuple[JMethodDetail, JMethodDetail]]:
        """A call graph for a given class and (optionally) a given method.

        Args:
            qualified_class_name (str): The qualified name of the class.
            method_signature (str | None, optional): The signature of the method in the class.. Defaults to None.
            using_symbol_table (bool, optional): Generate call graph using symbol table. Defaults to False.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[Tuple[JMethodDetail, JMethodDetail]]: An edge list of the call graph for the given class and method.
        """        
        if using_symbol_table:
            return self.__get_class_call_graph_using_symbol_table(qualified_class_name=qualified_class_name,
                                                                  method_signature=method_signature)
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_class_call_graph(qualified_class_name, method_signature)

    def get_entry_point_classes(self) -> Dict[str, JType]:
        """Returns a dictionary of all entry point classes in the Java code.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, JType]: A dict of all entry point classes in the Java code, with qualified class names as keys
        """
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_entry_point_classes()

    def get_entry_point_methods(self) -> Dict[str, Dict[str, JCallable]]:
        """Returns a dictionary of all entry point methods in the Java code with qualified class name as key and dictionary of methods in that class as value

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, Dict[str, JCallable]]: A dictionary of dictionaries of entry point methods in the Java code.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_all_entry_point_methods()

    def remove_all_comments(self) -> str:
        """Remove all comments from the source code.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            str: The source code with all comments removed.
        """        
        # Remove any prefix comments/content before the package declaration
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.remove_all_comments(self.source_code)

    def get_methods_with_annotations(self, annotations: List[str]) -> Dict[str, List[Dict]]:
        """Returns a dictionary of method names and method bodies.

        Args:
            annotations (List[str]): List of annotation strings.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, List[Dict]]: Dictionary with annotations as keys and a list of dictionaries containing method names and bodies, as values.
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_methods_with_annotations(self.source_code, annotations)

    def get_test_methods(self) -> Dict[str, str]:
        """Returns a dictionary of method names and method bodies.

        Args:
            source_class_code (str): String containing code for a java class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Dict[str, str]: Dictionary of method names and method bodies.
        """        
       
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.CODEANALYZER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_test_methods(self.source_code)

    def get_calling_lines(self, target_method_name: str) -> List[int]:
        """Returns a list of line numbers in source method block where target method is called.

        Args:
            target_method_name (str): target method  name.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            List[int]: List of line numbers within in source method code block.
        """        
        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_calling_lines(self.source_code, target_method_name)

    def get_call_targets(self, declared_methods: dict) -> Set[str]:
        """Uses simple name resolution for finding the call targets. Nothing sophiscticed here. Just a simple search over the AST.

        Args:
            declared_methods (dict): A dictionary of all declared methods in the class.

        Raises:
            NotImplementedError: Raised when current AnalysisEngine does not support this function.

        Returns:
            Set[str]: A list of call targets (methods).
        """        
        if self.analysis_backend in [AnalysisEngine.CODEQL, AnalysisEngine.TREESITTER]:
            raise NotImplementedError(f"Support for this functionality has not been implemented yet.")
        return self.backend.get_call_targets(self.source_code, declared_methods)
    