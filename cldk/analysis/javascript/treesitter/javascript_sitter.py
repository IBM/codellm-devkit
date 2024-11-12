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
JavascriptSitter module
"""

from typing import List, Optional
from tree_sitter import Language, Parser, Query, Node
import tree_sitter_javascript as tsjavascript

from cldk.models.javascript.models import JsCallable, JsClass, JsParameter, JsProgram
from cldk.models.treesitter import Captures


class JavascriptSitter:
    """
    Tree sitter for Javascript use cases.
    """

    def __init__(self) -> None:
        self.language: Language = Language(tsjavascript.language())
        self.parser: Parser = Parser(self.language)

    def get_all_functions(self, code: str) -> List[JsCallable]:
        """
        Get all the functions and methods in the provided code.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            All the function and method details within the provided code.
        """

        callables: List[JsCallable] = []
        classes: List[JsClass] = self.get_classes(code)
        for clazz in classes:
            callables.extend(clazz.methods)

        callables.extend(self.__get_top_level_functions(code))
        callables.extend(self.__get_top_level_generators(code))
        callables.extend(self.__get_top_level_arrow_functions(code))
        callables.extend(self.__get_top_level_function_expressions(code))
        callables.extend(self.__get_top_level_generator_expressions(code))

        return callables

    def get_classes(self, code: str) -> List[JsClass]:
        """
        Get the classes in the provided code.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsClass]
            All class details within the provided code.
        """

        query = """(class_declaration) @class"""

        return [self.__get_class_details(capture.node) for capture in self.__frame_query_and_capture_output(query, code)]

    def get_program_details(self, source_file: str) -> JsProgram:
        """
        Get the details of the provided code file.

        Parameters
        ----------
        source_file : str
            The code we want to analyse.

        Returns
        -------
            The details of the provided file.
        """

        return JsProgram(
            classes=self.get_classes(source_file),
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

    def __get_class_details(self, node: Node) -> JsClass:
        """
        Get the classe details for a provided tree-sitter class node.

        Parameters
        ----------
        node : Node
            The tree-sitter class node whose details we want.

        Returns
        -------
        JsClass
            The class details of the provided node.
        """

        parent_node: Node = self.__get_class_parent_node(node)

        return JsClass(
            name=node.child_by_field_name("name").text.decode(),
            methods=self.__get_methods(node),
            start_line=node.start_point[0],
            end_line=node.end_point[0],
            # TODO: needs more refinement since you can have more than an identifier
            parent=parent_node.named_children[0].text.decode() if parent_node else None,
        )

    def __get_methods(self, class_node: Node) -> List[JsCallable]:
        """
        Get the methods for a provided tree-sitter class node.

        Parameters
        ----------
        class_node : Node
            The tree-sitter class node whose methods we want.

        Returns
        -------
        List[JsCallable]
            The method details of the provided class node.
        """

        class_body_node = class_node.child_by_field_name("body")

        return [self.__extract_function_details(child) for child in class_body_node.children if child.type == "method_definition"]

    def __get_top_level_functions(self, code: str) -> List[JsCallable]:
        """
        Get the exportable functions from the provided code.
        There is no guarantee that the functions are exported.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            The function details within the provided code.
        """

        query = """
        (program [
            (function_declaration) @function
            (export_statement (function_declaration) @function.export)
        ])
        """
        captures: Captures = self.__frame_query_and_capture_output(query, code)

        return [self.__extract_function_details(capture.node) for capture in captures]

    def __get_top_level_generators(self, code: str) -> List[JsCallable]:
        """
        Get the exportable generator functions from the provided code.
        There is no guarantee that the functions are exported.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            The generator function details within the provided code.
        """

        query = """
        (program [
            (generator_function_declaration) @generator
            (export_statement (generator_function_declaration) @generator.export)
        ])
        """
        captures: Captures = self.__frame_query_and_capture_output(query, code)

        return [self.__extract_function_details(capture.node) for capture in captures]

    def __get_top_level_arrow_functions(self, code: str) -> List[JsCallable]:
        """
        Get the exportable arrow functions from the provided code.
        There is no guarantee that the functions are exported.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            The arrow function details within the provided code.
        """

        # get arrow functions that can be called from an external file.
        query = """
        (program [
            (expression_statement (assignment_expression (arrow_function) @arrow.assignment))
            (expression_statement (sequence_expression (assignment_expression (arrow_function) @arrow.assignment)))
            (lexical_declaration (variable_declarator (arrow_function) @arrow.variable))
            (export_statement (arrow_function) @arrow.export)
            (export_statement (lexical_declaration (variable_declarator (arrow_function) @arrow.export.variable)))
        ])
        """

        captures: Captures = self.__frame_query_and_capture_output(query, code)
        callables: List[JsCallable] = [self.__extract_arrow_function_details(capture.node, capture.name) for capture in captures]

        return [callable for callable in callables if callable.name]

    def __get_top_level_function_expressions(self, code: str) -> List[JsCallable]:
        """
        Get the exportable function expressions from the provided code.
        There is no guarantee that the functions are exported.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            The function expression details within the provided code.
        """

        # get function expressions that can be called from an external file.
        # TODO: function node changed to function_expression in newer tree-sitter versions
        query = """
        (program [
            (expression_statement (assignment_expression (function) @function.assignment))
            (expression_statement (sequence_expression (assignment_expression (function) @function.assignment)))
            (lexical_declaration (variable_declarator (function) @function.variable))
            (export_statement (function) @function.export)
            (export_statement (lexical_declaration (variable_declarator (function) @function.export.variable)))
        ])
        """

        captures: Captures = self.__frame_query_and_capture_output(query, code)

        return [self.__extract_function_expression_details(capture.node, capture.name) for capture in captures]

    def __get_top_level_generator_expressions(self, code: str) -> List[JsCallable]:
        """
        Get the exportable generator expressions from the provided code.
        There is no guarantee that the functions are exported.

        Parameters
        ----------
        code : str
            The code you want to analyse.

        Returns
        -------
        List[JsCallable]
            The generator expression details within the provided code.
        """

        # get generator expressions that can be called from an external file.
        query = """
        (program [
            (expression_statement (assignment_expression (generator_function) @function.assignment))
            (expression_statement (sequence_expression (assignment_expression (generator_function) @function.assignment)))
            (lexical_declaration (variable_declarator (generator_function) @function.variable))
            (export_statement (generator_function) @function.export)
            (export_statement (lexical_declaration (variable_declarator (generator_function) @function.export.variable)))
        ])
        """

        captures: Captures = self.__frame_query_and_capture_output(query, code)

        return [self.__extract_function_expression_details(capture.node, capture.name) for capture in captures]

    def __extract_function_details(self, function_node: Node) -> JsCallable:
        """
        Extract the details from a function/method tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter function node whose details we want.
        capture_name : str
            The identifier used to extract the node.

        Returns
        -------
        JsCallable
            The function details.
        """

        name: str = function_node.child_by_field_name("name").text.decode()
        parameters_node: Node = function_node.child_by_field_name("parameters")

        return JsCallable(
            name=name,
            code=function_node.text.decode(),
            paremeters=self.__extract_parameters_details(parameters_node),
            signature=name + parameters_node.text.decode(),
            start_line=function_node.start_point[0],
            end_line=function_node.end_point[0],
            is_constructor=function_node.type == "method_definition" and name == "constructor",
        )

    def __extract_arrow_function_details(self, node: Node, capture_name: str) -> JsCallable:
        """
        Extract the details from an arrow function tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter arrow function node whose details we want.
        capture_name : str
            The identifier used to extract the node.

        Returns
        -------
        JsCallable
            The function details.
        """

        name: str = None
        if capture_name == "arrow.assignment":
            left_node = node.parent.child_by_field_name("left")
            if left_node.type == "identifier":
                name = left_node.text.decode()
        elif capture_name == "arrow.export":
            name = "default"
        else:
            name_node = node.parent.child_by_field_name("name")
            name = name_node.text.decode()

        parameter_node: Node = node.child_by_field_name("parameter")
        parameters_node: Node = node.child_by_field_name("parameters")

        # TODO: not sure about this
        parameters_text = f"({parameter_node.text.decode()})" if parameter_node else parameters_node.text.decode()
        signature: str = (name if name else "") + parameters_text

        return JsCallable(
            name=name,
            code=node.text.decode(),
            paremeters=[JsParameter(name=parameter_node.text.decode())] if parameter_node else self.__extract_parameters_details(parameters_node),
            signature=signature,
            start_line=node.start_point[0],
            end_line=node.end_point[0],
        )

    def __extract_function_expression_details(self, node: Node, capture_name: str) -> JsCallable:
        """
        Extract the details from a function expression tree-sitter node.

        Parameters
        ----------
        node : Node
            The tree-sitter function node whose details we want.
        capture_name : str
            The identifier used to extract the node.

        Returns
        -------
        JsCallable
            The function details.
        """

        name: str = None
        if capture_name == "function.assignment":
            left_node = node.parent.child_by_field_name("left")
            if left_node.type == "identifier":
                name = left_node.text.decode()
        elif capture_name == "function.export":
            name = "default"
        else:
            name_node = node.parent.child_by_field_name("name")
            name = name_node.text.decode()

        parameters_node: Node = node.child_by_field_name("parameters")

        # TODO: not sure about this
        signature: str = (name if name else "") + parameters_node.text.decode()

        return JsCallable(
            name=name,
            code=node.text.decode(),
            paremeters=self.__extract_parameters_details(parameters_node),
            signature=signature,
            start_line=node.start_point[0],
            end_line=node.end_point[0],
        )

    def __extract_parameters_details(self, parameters_node: Node) -> List[JsParameter]:
        """
        Extract the parameter details from a given tree-sitter parameters node.

        Parameters
        ----------
        parameters_node : Node
            The tree-sitter parameters node whose details we want.

        Returns
        -------
        List[JsParameter]
            The list of parameter details.
        """

        if not parameters_node or not parameters_node.children:
            return []

        parameters: List[JsParameter] = []
        for child in parameters_node.children:
            # TODO incomplete, needs a recursive way of finding the parameters
            if child.type in ["identifier", "undefined"]:
                parameters.append(JsParameter(name=child.text.decode()))

        return parameters

    def __get_class_parent_node(self, class_node: Node) -> Optional[Node]:
        """
        Extracts the tree-sitter heritage node, if it exists from a class node.

        Parameters
        ----------
        class_node : Node
            The tree-sitter class node we want to process.

        Returns
        -------
        Optional[Node]
            The tree-sitter node that has the heritage data for the provided class node.
        """

        for child in class_node.children:
            if child.type == "class_heritage":
                return child

        return None
