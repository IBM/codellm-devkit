from unittest import TestCase

from cldk.treesitter.go import GoSitter


class TestGoTreeSitter(TestCase):
    """
    Tests for Go TreeSitter module
    """

    def setUp(self):
        """Runs before each test case"""
        self.go_ts = GoSitter()

    def tearDown(self):
        """Runs after each test case"""

    def test_get_imports(self):
        code = """
        package main

        import "fmt"
        import "lib/math"
        import _ "lib/some"

        import()
        import ("strings")
        import (
          "net/http"
          . "some/dsl"
          _ "os"
          alias "some/package"
        )
        """

        imports = self.go_ts.get_imports(code=code)
        self.assertEqual(len(imports), 8)
        self.assertEqual(imports[0].path, "fmt")
        self.assertIsNone(imports[0].name)
        self.assertEqual(imports[2].path, "lib/some")
        self.assertEqual(imports[2].name, "_")
        self.assertEqual(imports[4].path, "net/http")
        self.assertIsNone(imports[4].name)
        self.assertEqual(imports[5].path, "some/dsl")
        self.assertEqual(imports[5].name, ".")
        self.assertEqual(imports[7].path, "some/package")
        self.assertEqual(imports[7].name, "alias")

    def test_get_source_file_details(self):
        code = """
        package main

        import (
	        "fmt"
	        "github.com/test/go-simple/internal"
            m "lib/math"
            . "lib/math"
        )

        type rect struct {
            width, height int
        }

        func (r *rect) area() int {
            return r.width * r.height
        }
        """

        details = self.go_ts.get_source_file_details(source_file=code)
        self.assertEqual(len(details.imports), 4)
        self.assertEqual(len(details.callables), 1)

    def test_get_all_callables(self):
        code = """
        package main

        type rect struct {
            width, height int
        }

        func (r *rect) area() int {
            return r.width * r.height
        }

        func main() {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 2)
        self.assertEqual(callables[0].name, "area")
        self.assertEqual(callables[1].name, "main")

    def test_get_method(self):
        code = """
        package main

        type rect struct {
            width, height int
        }

        func (r *rect) area() int {
            return r.width * r.height
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        method = callables[0]
        self.assertEqual(method.name, "area")
        self.assertEqual(method.signature, "(r *rect) area() int")
        self.assertListEqual(method.modifiers, ["private"])
        self.assertEqual(method.comment, "")
        self.assertEqual(method.code, "func (r *rect) area() int {\n            return r.width * r.height\n        }")
        self.assertEqual(method.start_line, 7)
        self.assertEqual(method.end_line, 9)
        self.assertListEqual(method.return_types, ["int"])
        self.assertEqual(len(method.parameters), 0)
        self.assertIsNotNone(method.receiver)
        self.assertEqual(method.receiver.name, "r")
        self.assertEqual(method.receiver.type, "*rect")
        self.assertTrue(method.receiver.is_reference)
        self.assertFalse(method.receiver.is_variadic)

    def test_get_callable_with_comment(self):
        code = """
        package main

        type rect struct {
            width, height int
        }

        // area docs
        func (r *rect) area() int {
            return r.width * r.height
        }

        // main docs
        // test
        func main() {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 2)
        self.assertEqual(callables[0].name, "area")
        self.assertEqual(callables[0].comment, "// area docs")
        self.assertEqual(callables[1].name, "main")
        self.assertEqual(callables[1].comment, "// main docs\n// test")

    def test_get_callable_with_multiple_returns(self):
        code = """
        package main

        func test(a int, b int, c float64) (float64, *[]int) {
        }

        func test2() (a, b float64, func(p *T)) {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 2)
        self.assertEqual(callables[0].name, "test")
        self.assertListEqual(callables[0].return_types, ["float64", "*[]int"])
        self.assertEqual(callables[1].name, "test2")
        self.assertListEqual(callables[1].return_types, ["float64", "float64", "func(p *T)"])

    def test_get_generic_function(self):
        code = """
        package main

        // SumIntsOrFloats sums the values of map m. It supports both floats and integers
        // as map values.
        func SumIntsOrFloats[K comparable, V int64 | float64](m map[K]V) V {
            var s V
            for _, v := range m {
                s += v
            }
            return s
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "SumIntsOrFloats")
        self.assertEqual(function.signature, "SumIntsOrFloats[K comparable, V int64 | float64](m map[K]V) V")
        self.assertListEqual(function.modifiers, ["public"])
        self.assertListEqual(function.return_types, ["V"])
        self.assertEqual(function.comment, "// SumIntsOrFloats sums the values of map m. It supports both floats and integers\n// as map values.")
        self.assertEqual(len(function.parameters), 1)
        self.assertEqual(function.parameters[0].name, "m")
        self.assertEqual(function.parameters[0].type, "map[K]V")

    def test_get_callable_with_variadic(self):
        code = """
        package main

        func test(a, b int, z float64, opt ...interface{}) (success bool) {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, ["bool"])
        self.assertEqual(len(function.parameters), 4)
        self.assertEqual(function.parameters[0].name, "a")
        self.assertEqual(function.parameters[0].type, "int")
        self.assertEqual(function.parameters[1].name, "b")
        self.assertEqual(function.parameters[1].type, "int")
        self.assertEqual(function.parameters[2].name, "z")
        self.assertEqual(function.parameters[2].type, "float64")
        self.assertEqual(function.parameters[3].name, "opt")
        self.assertEqual(function.parameters[3].type, "...interface{}")
        self.assertTrue(function.parameters[3].is_variadic)

    def test_get_callable_with_qualified_type(self):
        code = """
        package main
        
        import alias "some/package"

        func test(a alias.SomeType) (bool) {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, ["bool"])
        self.assertEqual(len(function.parameters), 1)
        self.assertEqual(function.parameters[0].name, "a")
        self.assertEqual(function.parameters[0].type, "alias.SomeType")

    def test_get_callable_with_parms_without_name(self):
        code = """
        package main
        
        func test(int, ...int) (...int) {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, ["...int"])
        self.assertEqual(len(function.parameters), 2)
        self.assertIsNone(function.parameters[0].name)
        self.assertEqual(function.parameters[0].type, "int")
        self.assertIsNone(function.parameters[1].name)
        self.assertEqual(function.parameters[1].type, "...int")
        self.assertTrue(function.parameters[1].is_variadic)

    def test_get_callable_without_parms(self):
        code = """
        package main
        
        func test() (...int) {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, ["...int"])
        self.assertEqual(len(function.parameters), 0)

    def test_get_callable_without_return(self):
        code = """
        package main
        
        func test() {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, [])

    def test_get_callable_with_typed_return(self):
        code = """
        package main
        
        func test() t[K, V] {
        }
        """

        callables = self.go_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 1)
        function = callables[0]
        self.assertEqual(function.name, "test")
        self.assertListEqual(function.modifiers, ["private"])
        self.assertListEqual(function.return_types, ["t[K, V]"])
