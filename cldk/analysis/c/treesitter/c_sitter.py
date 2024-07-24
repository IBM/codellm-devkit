from typing import List
from tree_sitter import Language, Parser, Query, Node
import tree_sitter_c as tsc

from cldk.models.c.models import CFunction, CImport, CParameter, CTranslationUnit, COutput
from cldk.models.treesitter import Captures


class CSitter:
    """
    Tree sitter for C use cases.
    """

    def __init__(self) -> None:
        self.language: Language = Language(tsc.language())
        self.parser: Parser = Parser(self.language)

    def get_all_functions(self, code: str) -> List[CFunction]:
        """
        Get all the functions in the provided code.

        Parameters
        ----------
        code: the code you want to analyse.

        Returns
        -------
        List[CFunction]
            returns all the function details within the provided code.
        """

        return [self.__get_function_details(code, capture.node) for capture in self.__get_function_nodes(code)]

    def get_imports(self, code: str) -> List[CImport]:
        """
        Get all the imports in the provided code.

        Parameters
        ----------
        code: the code you want to analyse.

        Returns
        -------
        List[CImport]
            returns all the imports within the provided code.
        """

        query = """(preproc_include) @import"""
        captures: Captures = self.__frame_query_and_capture_output(query, code)
        imports: List[CImport] = []
        for capture in captures:
            path_node: Node = capture.node.child_by_field_name("path")
            text: str = path_node.text.decode()
            if path_node.type == "system_lib_string":
                imports.append(CImport(value=text[1 : len(text) - 1], is_system=True))
            elif path_node.type == "string_literal":
                imports.append(CImport(value=text[1 : len(text) - 1], is_system=False))
            else:
                imports.append(CImport(value=text, is_system=False))

        return imports

    def get_translation_unit_details(self, code: str) -> CTranslationUnit:
        """
        Given the code of a C translation unit, return the details.

        Parameters
        ----------
        code : str
            The source code of the translation unit.

        Returns
        -------
        CTranslationUnit
            The details of the given translation unit.
        """

        return CTranslationUnit(
            functions=self.get_all_functions(code),
            imports=self.get_imports(code),
        )

    def __get_function_details(self, original_code: str, node: Node) -> CFunction:
        """
        Extract the details of a function from a tree-sitter node.

        Parameters
        ----------
        original_code : str
            The original code, used to extract the tree-sitter node.
        node : Node
            The function tree-sitter node we want to evaluate.

        Returns
        -------
        CFunction
            The extracted details of the function.
        """

        nb_pointers = self.__count_pointers(node.child_by_field_name("declarator"))
        return_type: str = self.__get_function_return_type(node)
        if return_type != "function":
            return_type = return_type + nb_pointers * "*"

        output: COutput = COutput(
            type=return_type,
            is_reference=return_type == "function" or nb_pointers > 0,
            qualifiers=self.__get_type_qualifiers(node),
        )

        return CFunction(
            name=self.__get_function_name(node),
            code=node.text.decode(),
            start_line=node.start_point[0],
            end_line=node.end_point[0],
            signature=self.__get_function_signature(original_code, node),
            parameters=self.__get_function_parameters(node),
            output=output,
            comment=self.__get_comment(node),
            specifiers=self.__get_storage_class_specifiers(node),
        )

    def __get_function_parameters(self, function_node: Node) -> List[CParameter]:
        """
        Extract the parameters of a tree-sitter function node.

        Parameters
        ----------
        function_node : Node
            The function node whose parameters we want to extract.

        Returns
        -------
        List[CParameter]
            The parameters of the given function node.
        """

        query = """(function_declarator ((parameter_list) @function.parameters))"""
        parameters_list: Captures = self.__query_node_and_capture_output(query, function_node)

        if not parameters_list:
            return []

        params: dict[str, CParameter] = self.__get_parameter_details(parameters_list)

        # for old-style function definition:
        # https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Old_002dStyle-Function-Definitions.html

        for child in function_node.children:
            if child.type == "declaration":
                for tup in self.__extract_parameter_declarations(child):
                    name, parameter = tup
                    params[name] = parameter

        # filter out params without type
        return [param[1] for param in params.items() if param[1].type]

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

    def __get_function_nodes(self, code: str) -> Captures:
        """Parse the given code and extract tree-sitter function nodes.

        Parameters
        ----------
        code : str
            The input code.

        Returns
        -------
        Captures
            The list of tree-sitter captures.
        """

        query = """((function_definition) @function)"""
        return self.__frame_query_and_capture_output(query, code)

    def __get_function_name(self, function_node: Node) -> str:
        """
        Extract the function name from a tree-sitter function node.

        Parameters
        ----------
        function_node : Node
            The function node whose name we want to extract.

        Returns
        -------
        str
            The name of the function.
        """

        query = """(function_declarator ((identifier) @function.name))"""
        function_name_node: Node = self.__query_node_and_capture_output(query, function_node)[0].node
        return function_name_node.text.decode()

    def __get_function_return_type(self, function_node: Node) -> str:
        """
        Extracts the return type of a tree-sitter function node.

        Parameters
        ----------
        function_node : Node
            The function node whose return type we want to extract.

        Returns
        -------
        str
            The return type of a function or function, if the return is a function pointer.
        """

        # TODO: not sure if this is correct
        # if there's more that 1 function declaration type, we consider it a function pointer
        if self.__count_function_declarations(function_node.child_by_field_name("declarator")) > 1:
            return "function"

        type_node = function_node.child_by_field_name("type")

        return type_node.text.decode() if type_node.type != "struct_specifier" else type_node.child_by_field_name("name").text.decode()

    def __get_function_signature(self, code: str, function_node: Node) -> str:
        """
        Extracts the function signature from a tree-sitter function node.

        Parameters
        ----------
        code : str
            The original code that was used to extract the function node.
        function_node : Node
            The function node whose signature we want to extract.

        Returns
        -------
        str
            The signature of the function.
        """

        body_node: Node = function_node.child_by_field_name("body")
        start_byte = function_node.start_byte
        end_byte = body_node.start_byte
        code_bytes = bytes(code, "utf-8")
        signature = code_bytes[start_byte:end_byte]

        return signature.decode().strip()

    def __get_type_qualifiers(self, node: Node) -> List[str]:
        """
        Extract the type qualifiers from a given tree-sitter node.

        Paramaters
        ----------
        node : Node
            The node whose type qulifiers we want to extract.

        Returns
        -------
        List[str]
            The list of type qualifiers.
        """

        if not node or not node.children:
            return []

        return [child.text.decode() for child in node.children if child.type == "type_qualifier"]

    def __get_storage_class_specifiers(self, node: Node) -> List[str]:
        """
        Extract the storage class specifiers from a given tree-sitter node.

        Paramaters
        ----------
        node : Node
            The node whose storage class speciers we want to extract.

        Returns
        -------
        List[str]
            The list of storage class specifiers.
        """

        if not node or not node.children:
            return []

        return [child.text.decode() for child in node.children if child.type == "storage_class_specifier"]

    def __count_pointers(self, node: Node) -> int:
        """
        Count the number of consecutive pointers for a tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-siter node we want to evaluate.

        Returns
        -------
        int
            The number of consecutive pointers present in the given tree-sitter node.
        """

        count = 0
        curr_node = node
        while curr_node and curr_node.type == "pointer_declarator":
            count += 1
            curr_node = curr_node.child_by_field_name("declarator")

        return count

    def __count_function_declarations(self, node: Node) -> int:
        """
        Counts the number of function declaration nodes for a tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter node we want to evaluate.

        Returns
        -------
        int
            The number of function delacration nodes present in the given tree-sitter node.
        """

        if not node or not node.children:
            return 0

        sum = 1 if node.type == "function_declarator" else 0
        for child in node.children:
            sum += self.__count_function_declarations(child)

        return sum

    def __get_parameter_details(self, parameters_list: Captures) -> dict[str, CParameter]:
        """
        Extract parameter details from a list of tree-sitter parameters.

        Parameters
        ----------
        parameters_list : Captures
            The parameter list node captures.

        Returns
        -------
        Dict[str, CParameter]
            A dictionary of parameter details.
        """

        params: dict[str, CParameter] = {}

        for parameters in parameters_list:
            if not parameters or not parameters.node.children:
                continue
            for param in parameters.node.children:
                # old c style
                if param.type == "identifier":
                    name, parameter = self.__extract_simple_parameter(param, "")
                    params[name] = parameter
                elif param.type == "variadic_parameter":
                    name, parameter = self.__extract_simple_parameter(param, "variadic")
                    params[name] = parameter
                elif param.type == "parameter_declaration":
                    for tup in self.__extract_parameter_declarations(param):
                        name, parameter = tup
                        params[name] = parameter

        return params

    def __extract_simple_parameter(self, node: Node, parameter_type: str) -> tuple[str, CParameter]:
        name: str = node.text.decode()
        parameter: CParameter = CParameter(
            type=parameter_type,
            qualifiers=[],
            specifiers=[],
            is_reference=False,
            name=name,
        )

        return (name, parameter)

    def __extract_parameter_declarations(self, node: Node) -> List[tuple[str, CParameter]]:
        query = """((identifier) @name)"""
        captures: Captures = self.__query_node_and_capture_output(query, node)

        # no name found, skip this node
        if len(captures) == 0:
            return []

        parameters: List[tuple[str, CParameter]] = []
        for capture in captures:
            parameters.append(self.__extract_parameter_declaration(node, capture.node))

        return parameters

    def __extract_parameter_declaration(self, parent_node: Node, identifier_node: Node) -> tuple[str, CParameter]:
        name = identifier_node.text.decode()

        nb_function_declarations = self.__count_function_declarations(parent_node)
        # we have a function pointer
        if nb_function_declarations > 0:
            parameter: CParameter = CParameter(
                type="function",
                qualifiers=[],  # TODO: not sure if this is correct
                specifiers=[],  # TODO: not sure if this is correct
                is_reference=True,
                name=name,
            )
            return (name, parameter)

        type_node = parent_node.child_by_field_name("type")

        param_type: str = type_node.text.decode() if type_node.type != "struct_specifier" else type_node.child_by_field_name("name").text.decode()
        type_augmentor = self.__augment_type(identifier_node, parent_node.type)

        parameter = CParameter(
            type=param_type + type_augmentor,
            qualifiers=self.__get_type_qualifiers(parent_node),
            specifiers=self.__get_storage_class_specifiers(parent_node),
            is_reference=type_augmentor.startswith("*"),
            name=name,
        )

        return (name, parameter)

    def __augment_type(self, identifier_node: Node, stop_node_type: str) -> str:
        """
        Augment types with pointer and array details.
        """

        # not sure about this one
        type_augmentor = ""
        pointer_augmentor = ""
        array_augmentor = ""
        curr_node = identifier_node.parent
        while curr_node and curr_node.type != stop_node_type:
            if curr_node.type == "pointer_declarator":
                pointer_augmentor = f"*{pointer_augmentor}"
            elif curr_node.type == "array_declarator":
                size_node = curr_node.child_by_field_name("size")
                size: str = ""
                if size_node:
                    size = size_node.text.decode()
                array_augmentor = f"{array_augmentor}[{size}]"
            elif curr_node.type == "parenthesized_declarator":
                type_augmentor = f"({pointer_augmentor}{type_augmentor}{array_augmentor})"
                pointer_augmentor = ""
                array_augmentor = ""

            curr_node = curr_node.parent

        return f"{pointer_augmentor}{type_augmentor}{array_augmentor}"

    def __get_comment(self, node: Node) -> str:
        """
        Extract the comment associated with a tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter node whose

        Returns
        -------
        str
            The comment associeted with the given node.
        """

        docs = []
        curr_node = node
        while curr_node.prev_named_sibling and curr_node.prev_named_sibling.type == "comment":
            curr_node = curr_node.prev_named_sibling
            text = curr_node.text.decode()
            docs.append(text)

        return "\n".join(reversed(docs))
