from unittest import TestCase

from cldk.analysis.python.treesitter import PythonSitter


class TestPythonTreeSitter(TestCase):
    """
    Tests for Python TreeSitter nodule
    """

    def setUp(self):
        """Runs before each test case"""
        self.python_tree_sitter = PythonSitter()

    def tearDown(self):
        """Runs after each test case"""

    def test_get_all_methods(self):
        module_str = """
        @staticmethod
        def foo() -> None:
            pass
        class Person:
              def __init__(self, name: str, age: int):
                self.name = name
                self.age = age
              @staticmethod
              def __str__(self):
                return f"{self.name}({self.age})"
            """
        all_methods = self.python_tree_sitter.get_all_methods(module=module_str)
        all_functions = self.python_tree_sitter.get_all_functions(module=module_str)
        self.assertEquals(len(all_methods), 2)
        self.assertEquals(len(all_functions), 1)
        self.assertEquals(all_methods[0].full_signature, "__init__(self, name: str, age: int)")
        self.assertTrue(all_methods[0].is_constructor)
        self.assertFalse(all_methods[0].is_static)
        self.assertEquals(all_methods[0].class_signature, "Person")
        self.assertEquals(all_functions[0].class_signature, "")
        self.assertTrue(all_functions[0].is_static)

    def test_get_all_imports(self):
        module_str = """
        from typing import List

        from sphinx.domains.python import PyField
        from tree_sitter import Language, Parser, Query, Node
        import a
        @staticmethod
        def foo() -> None:
            pass
        class Person:
              def __init__(self, name: str, age: int):
                self.name = name
                self.age = age
              @staticmethod
              def __str__(self):
                return f"{self.name}({self.age})"
            """
        all_imports = self.python_tree_sitter.get_all_imports(module=module_str)
        self.assertEquals(len(all_imports), 4)

    def test_get_all_imports_details(self):
        module_str = """
        from typing import List

        from sphinx.domains.python import PyField
        from tree_sitter import Language, Parser, Query, Node
        import a
        @staticmethod
        def foo() -> None:
            pass
        class Person:
              def __init__(self, name: str, age: int):
                self.name = name
                self.age = age
              @staticmethod
              def __str__(self):
                return f"{self.name}({self.age})"
            """
        all_imports = self.python_tree_sitter.get_all_imports_details(module=module_str)
        self.assertEquals(len(all_imports), 4)

    def test_get_module(self):
        module_str = """
                from typing import List

                from sphinx.domains.python import PyField
                from tree_sitter import Language, Parser, Query, Node
                import a
                @staticmethod
                def foo() -> None:
                    pass
                class Person:
                      def __init__(self, name: str, age: int):
                        self.name = name
                        self.age = age
                      @staticmethod
                      def __str__(self):
                        return f"{self.name}({self.age})"
                    """
        module_details = self.python_tree_sitter.get_module_details(module=module_str)
        self.assertIsNotNone(module_details.functions)
        self.assertIsNotNone(module_details.classes)
        self.assertIsNotNone(module_details.imports)

    def test_get_all_classes(self):
        module_str = """
        def foo() -> None:
            pass
        class Person:
              def __init__(self, name: str, age: int):
                self.name = name
                self.age = age

              def __str__(self):
                return f"{self.name}({self.age})"
            """
        all_classes = self.python_tree_sitter.get_all_classes(module=module_str)
        self.assertEquals(len(all_classes), 1)
        self.assertEquals(len(all_classes[0].methods), 2)
        self.assertEquals(all_classes[0].methods[0].full_signature, "__init__(self, name: str, age: int)")
        self.assertEquals(all_classes[0].methods[1].class_signature, "Person")

    def test_get_all_test_classes(self):
        module_str = """
        import unittest

        class TestStringMethods(unittest.TestCase, ABC):
        
            def test_upper(self):
                self.assertEqual('foo'.upper(), 'FOO')
        
            def test_isupper(self):
                self.assertTrue('FOO'.isupper())
                self.assertFalse('Foo'.isupper())
        
            def test_split(self):
                s = 'hello world'
                self.assertEqual(s.split(), ['hello', 'world'])
                # check that s.split fails when the separator is not a string
                with self.assertRaises(TypeError):
                    s.split(2)
        
        if __name__ == '__main__':
            unittest.main()
            """
        all_classes = self.python_tree_sitter.get_all_classes(module=module_str)
        self.assertTrue(all_classes[0].is_test_class)

    def test_call_site(self):
        module_str = """
        import unittest

        class TestStringMethods(unittest.TestCase, ABC):

                def test_get_all_test_classes(self):
                    module_str = "a"
                    all_classes = self.python_tree_sitter.get_all_classes(module=module_str)
                    self.assertTrue(all_classes[0].is_test_class)
        """
        all_classes = self.python_tree_sitter.get_all_classes(module=module_str)
        self.assertTrue(all_classes[0].is_test_class)
