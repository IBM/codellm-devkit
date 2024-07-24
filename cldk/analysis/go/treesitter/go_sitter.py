from typing import List
from tree_sitter import Language, Parser, Query, Node
import tree_sitter_go as tsgo

from cldk.models.go.models import GoCallable, GoImport, GoParameter, GoSourceFile
from cldk.models.treesitter import Captures


class GoSitter:
    """
    Tree sitter for Go use cases.
    """

    def __init__(self) -> None:
        self.language: Language = Language(tsgo.language())
        self.parser: Parser = Parser(self.language)

    def get_all_functions(self, code: str) -> List[GoCallable]:
        """
        Get all the functions and methods in the provided code.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[GoCallable]
            All the function and method details within the provided code.
        """

        query = """
        ((function_declaration) @function)
        ((method_declaration) @method)
        """

        callables: List[GoCallable] = []
        captures: Captures = self.__frame_query_and_capture_output(query, code)
        for capture in captures:
            if capture.name == "function":
                callables.append(self.__get_function_details(capture.node))
            elif capture.name == "method":
                callables.append(self.__get_method_details(capture.node))

        return callables

    def get_imports(self, code: str) -> List[GoImport]:
        """
        Get all the imports in the provided code.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[GoImport]
            All the imports within the provided code.
        """

        query = """
        (import_declaration
          (import_spec) @import
        )

        (import_declaration
          (import_spec_list
            (import_spec) @import
          )
        )
        """

        return [self.__extract_import_details(capture.node) for capture in self.__frame_query_and_capture_output(query, code)]

    def get_source_file_details(self, source_file: str) -> GoSourceFile:
        """
        Get the details of the provided source file.

        Parameters
        ----------
        source_file : str
            The source file code you want to analyse.

        Returns
        -------
        GoSourceFile
            The details of the provided source file code.
        """

        return GoSourceFile(
            imports=self.get_imports(source_file),
            callables=self.get_all_functions(source_file),
        )

    def __frame_query_and_capture_output(self, query: str, code_to_process: str) -> Captures:
        """Frame a query for the tree-sitter parser.

        Parameters
        ----------
        query : str
            The query to frame.
        code_to_process : str
            The code to process.

        Returns
        -------
        Captures
            The list of tree-sitter captures.
        """

        framed_query: Query = self.language.query(query)
        tree = self.parser.parse(bytes(code_to_process, "utf-8"))
        return Captures(framed_query.captures(tree.root_node))

    def __query_node_and_capture_output(self, query: str, node: Node) -> Captures:
        """Frame a query for the tree-sitter parser and query the given tree-sitter node.

        Parameters
        ----------
        query : str
            The query to frame.
        node : Node
            The root node used for querying.

        Returns
        -------
        Captures
            The list of tree-sitter captures.
        """

        framed_query: Query = self.language.query(query)
        return Captures(framed_query.captures(node))

    def __get_function_details(self, node: Node) -> GoCallable:
        """
        Extract the function details from a tree-sitter function node.

        Parameters
        ----------
        node : Node
            The tree-sitter function node whose details we want.

        Returns
        -------
        GoCallable
            The function details.
        """

        name: str = self.__get_name(node)
        return GoCallable(
            name=name,
            signature=self.__get_signature(node),
            code=node.text.decode(),
            start_line=node.start_point[0],
            end_line=node.end_point[0],
            modifiers=["public"] if name[0].isupper() else ["private"],
            comment=self.__get_comment(node),
            parameters=self.__get_parameters(node),
            return_types=self.__get_return_types(node),
        )

    def __get_method_details(self, node: Node) -> GoCallable:
        """
        Extract the method details from a tree-sitter method node.

        Parameters
        ----------
        node : Node
            The tree-sitter method node whose details we want.

        Returns
        -------
        GoCallable
            The method details.
        """

        name: str = self.__get_name(node)
        return GoCallable(
            name=name,
            signature=self.__get_signature(node),
            code=node.text.decode(),
            start_line=node.start_point[0],
            end_line=node.end_point[0],
            modifiers=["public"] if name[0].isupper() else ["private"],
            comment=self.__get_comment(node),
            parameters=self.__get_parameters(node),
            return_types=self.__get_return_types(node),
            receiver=self.__get_receiver(node),
        )

    def __get_name(self, node: Node) -> str:
        """
        Extract the name of a tree-sitter function or method node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose name we want.

        Returns
        -------
        str
            The method/function name.
        """

        return node.child_by_field_name("name").text.decode()

    def __get_signature(self, node: Node) -> str:
        """
        Extract the signature of a tree-sitter function or method node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose signature we want.

        Returns
        -------
        str
            The method/function signature.
        """

        signature = ""
        # only methods have a receiver
        receiver_node: Node = node.child_by_field_name("receiver")
        if receiver_node:
            signature += receiver_node.text.decode()

        if signature:
            signature += " "

        name = self.__get_name(node)
        signature += name

        # generics type, optional field available only for functions
        type_params_node: Node = node.child_by_field_name("type_parameters")
        if type_params_node:
            signature += type_params_node.text.decode()

        signature += node.child_by_field_name("parameters").text.decode()

        result_node: Node = node.child_by_field_name("result")
        if result_node:
            signature += " " + result_node.text.decode()

        return signature

    def __get_comment(self, node: Node) -> str:
        """
        Extract the comment associated with a tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose docs we want.

        Returns
        -------
        str
            The comment associated with the given node.
        """

        docs = []
        curr_node = node
        while curr_node.prev_named_sibling and curr_node.prev_named_sibling.type == "comment":
            curr_node = curr_node.prev_named_sibling
            text = curr_node.text.decode()
            docs.append(text)

        return "\n".join(reversed(docs))

    def __get_parameters(self, node: Node) -> List[GoParameter]:
        """
        Extract the parameters from a tree-sitter function/method node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose parameters we want.

        Returns
        -------
        List[GoParameter]
            The list of parameter details.
        """

        parameters_node: Node = node.child_by_field_name("parameters")
        if not parameters_node or not parameters_node.children:
            return []

        parameters: List[GoParameter] = []
        for child in parameters_node.children:
            if child.type == "parameter_declaration":
                parameters.extend(self.__extract_parameters(child))
            elif child.type == "variadic_parameter_declaration":
                parameters.append(self.__extract_variadic_parameter(child))

        return parameters

    def __get_receiver(self, node: Node) -> GoParameter:
        """
        Extract the receiver from a tree-sitter method node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose receiver we want.

        Returns
        -------
        GoParameter
            The receiver details.
        """

        receiver_node: Node = node.child_by_field_name("receiver")

        # it must have 1 non-variadic child
        for child in receiver_node.children:
            if child.type == "parameter_declaration":
                return self.__extract_parameters(child)[0]

    def __get_return_types(self, node: Node) -> List[str]:
        """
        Extract the return types from a callable tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose types we want.

        Returns
        -------
        List[str]
            The list of types returned by the callable. Empty list, if it does not return.
        """

        result_node: Node = node.child_by_field_name("result")
        if not result_node:
            return []

        if result_node.type == "parameter_list":
            return_types: List[str] = []
            for child in result_node.children:
                if child.type == "parameter_declaration":
                    return_types.extend([param.type for param in self.__extract_parameters(child)])
                elif child.type == "variadic_parameter_declaration":
                    return_types.append(self.__extract_variadic_parameter(child).type)

            return return_types
        else:
            # TODO: might need to be more specific
            return [result_node.text.decode()]

    def __extract_parameters(self, parameter_declaration_node: Node) -> List[GoParameter]:
        """
        Extract parameter details from a tree-sitter parameter declaration node.

        Parameters
        ----------
        parameter_declaration_node : Node
            The tree-sitter node whose details we want.

        Returns
        -------
        List[GoParameter]
            The list of parameter details.
        """

        type_node: Node = parameter_declaration_node.child_by_field_name("type")
        param_type = type_node.text.decode()
        is_reference_type = param_type.startswith("*")

        query = """((identifier) @name)"""
        captures: Captures = self.__query_node_and_capture_output(query, parameter_declaration_node)

        # name is optional
        if len(captures) == 0:
            return [
                GoParameter(
                    type=param_type,
                    is_reference=is_reference_type,
                    is_variadic=False,
                )
            ]

        return [
            GoParameter(
                name=capture.node.text.decode(),
                type=param_type,
                is_reference=is_reference_type,
                is_variadic=False,
            )
            for capture in captures
        ]

    def __extract_variadic_parameter(self, variadic_parameter_node: Node) -> GoParameter:
        """
        Extract parameter details from a tree-sitter variadic declaration node.

        Parameters
        ----------
        variadic_parameter_node : Node
            The tree-sitter node whose details we want.

        Returns
        -------
        GoParameter
            The details of the variadic parameter.
        """

        name: str = None
        # name is not mandatory
        name_node: Node = variadic_parameter_node.child_by_field_name("name")
        if name_node:
            name = name_node.text.decode()

        type_node: Node = variadic_parameter_node.child_by_field_name("type")
        param_type = type_node.text.decode()

        return GoParameter(
            name=name,
            type="..." + param_type,
            is_reference=param_type.startswith("*"),
            is_variadic=True,
        )

    def __extract_import_details(self, node: Node) -> GoImport:
        """
        Extract the import details from a tree-sitter import spec node.

        Parameters
        ----------
        node : Node
            The import spec node tree-sitter node whose details we want.

        Returns
        -------
        GoImport
            The import details.
        """

        name_node: Node = node.child_by_field_name("name")
        path_node: Node = node.child_by_field_name("path")
        path = path_node.text.decode()

        return GoImport(
            name=name_node.text.decode() if name_node else None,
            path=path[1 : len(path) - 1],
        )
