from unittest import TestCase

from cldk.analysis.c.treesitter import CSitter


class TestCTreeSitter(TestCase):
    """
    Tests for C TreeSitter nodule
    """

    def setUp(self):
        """Runs before each test case"""
        self.c_ts = CSitter()

    def tearDown(self):
        """Runs after each test case"""

    def test_get_imports(self):
        code = """
        #include <stdlib.h>
        #include "zmalloc.h"
        #include zz_a
        #include zz_b(a, b)

        int main() {
            return 0;
        }
        """

        imports = self.c_ts.get_imports(code=code)
        self.assertEqual(len(imports), 4)
        self.assertEqual(imports[0].value, "stdlib.h")
        self.assertTrue(imports[0].is_system)
        self.assertEqual(imports[1].value, "zmalloc.h")
        self.assertFalse(imports[1].is_system)
        self.assertEqual(imports[2].value, "zz_a")
        self.assertFalse(imports[2].is_system)
        self.assertEqual(imports[3].value, "zz_b(a, b)")
        self.assertFalse(imports[3].is_system)

    def test_get_translation_unit_details(self):
        code = """
        #include <stdlib.h>
        #include "zmalloc.h"
        #include zz_a
        #include zz_b(a, b)

        int main() {
            return 0;
        }
        """

        details = self.c_ts.get_translation_unit_details(code=code)
        self.assertEqual(len(details.imports), 4)
        self.assertEqual(len(details.functions), 1)

    def test_get_all_methods(self):
        code = """
        #include <stdlib.h>
        #include "zmalloc.h"

        typedef void my_callback(void *, size_t);

        static foo bar();
        static baz quux(...);

        pno client_t_AddClient(client_t *self) { /* code */ }

        void * do_stuff(int arg1) {
          return 5;
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 2)
        self.assertEqual(all_functions[0].signature, "pno client_t_AddClient(client_t *self)")
        self.assertEqual(all_functions[0].name, "client_t_AddClient")
        self.assertEqual(all_functions[0].comment, "")
        self.assertEqual(all_functions[0].output.type, "pno")
        self.assertFalse(all_functions[0].output.is_reference)
        self.assertEqual(len(all_functions[0].parameters), 1)
        self.assertEqual(all_functions[0].parameters[0].name, "self")
        self.assertEqual(all_functions[0].parameters[0].type, "client_t*")
        self.assertTrue(all_functions[0].parameters[0].is_reference)

        self.assertEqual(all_functions[1].output.type, "void*")
        self.assertTrue(all_functions[1].output.is_reference)

    def test_return_function_pointer(self):
        code = """
        int (*functionFactory(int n))(int, int) {
            printf("Got parameter %d", n);
            int (*functionPtr)(int,int) = &addInt;
            return functionPtr;
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.signature, "int (*functionFactory(int n))(int, int)")
        self.assertEqual(func.name, "functionFactory")
        self.assertEqual(func.comment, "")
        self.assertEqual(func.output.type, "function")
        self.assertTrue(func.output.is_reference)
        self.assertEqual(len(func.parameters), 1)
        self.assertEqual(func.parameters[0].name, "n")
        self.assertEqual(func.parameters[0].type, "int")
        self.assertFalse(func.parameters[0].is_reference)

    def test_function_array_parameter(self):
        code = """
        void testing(int a[3][5]){
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.signature, "void testing(int a[3][5])")
        self.assertEqual(func.name, "testing")
        self.assertEqual(func.output.type, "void")
        self.assertEqual(len(func.parameters), 1)
        self.assertEqual(func.parameters[0].name, "a")
        self.assertEqual(func.parameters[0].type, "int[3][5]")
        self.assertFalse(func.parameters[0].is_reference)

    def test_old_style_function_definition(self):
        code = """
// K&R style
// test
int foo(bar, baz, qux)
unsigned int bar, baz;
char *qux;
{
    return 0;
}
"""

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.signature, "int foo(bar, baz, qux)\nunsigned int bar, baz;\nchar *qux;")
        self.assertEqual(func.name, "foo")
        self.assertEqual(func.comment, "// K&R style\n// test")
        self.assertEqual(func.output.type, "int")
        self.assertEqual(len(func.parameters), 3)
        self.assertEqual(func.parameters[0].name, "bar")
        self.assertEqual(func.parameters[0].type, "unsigned int")
        self.assertFalse(func.parameters[0].is_reference)
        self.assertEqual(func.parameters[1].name, "baz")
        self.assertEqual(func.parameters[1].type, "unsigned int")
        self.assertFalse(func.parameters[1].is_reference)
        self.assertEqual(func.parameters[2].name, "qux")
        self.assertEqual(func.parameters[2].type, "char*")
        self.assertTrue(func.parameters[2].is_reference)

    def test_function_pointer_type_return(self):
        code = """
        string* getFullName(string *name)
        {
            cout << "First: ";
            cin >> name[0];
            cout << "Middle: ";
            cin >> name[1];
            cout << "Last: ";
            cin >> name[2];
            return name;
        }

        void** func3(char *the_string)
        {
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 2)
        self.assertEqual(all_functions[0].name, "getFullName")
        self.assertEqual(all_functions[0].output.type, "string*")
        self.assertTrue(all_functions[0].output.is_reference)
        self.assertEqual(all_functions[1].name, "func3")
        self.assertEqual(all_functions[1].output.type, "void**")
        self.assertTrue(all_functions[1].output.is_reference)

    def test_function_pointer_parameter(self):
        code = """
        void test(int (* my_int_f)(int)) {
            int a, b, d;
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.name, "test")
        self.assertEqual(len(func.parameters), 1)
        self.assertEqual(func.parameters[0].name, "my_int_f")
        self.assertEqual(func.parameters[0].type, "function")
        self.assertTrue(func.parameters[0].is_reference)

    def test_function_specifiers(self):
        code = """
        unsigned int static inline *do_stuff() {
            return 5;
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.name, "do_stuff")
        self.assertEqual(func.output.type, "unsigned int*")
        self.assertTrue(func.output.is_reference)
        self.assertListEqual(func.specifiers, ["static", "inline"])

    def test_parameter_type_qualifiers(self):
        code = """
        void func(const unsigned int x) {
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.name, "func")
        self.assertEqual(len(func.parameters), 1)
        self.assertEqual(func.parameters[0].type, "unsigned int")
        self.assertEqual(func.parameters[0].name, "x")
        self.assertListEqual(func.parameters[0].qualifiers, ["const"])

    def test_variadic_parameter(self):
        code = """
        void func(int x, ...) {
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.name, "func")
        self.assertEqual(len(func.parameters), 2)
        self.assertEqual(func.parameters[0].type, "int")
        self.assertEqual(func.parameters[0].name, "x")
        self.assertEqual(func.parameters[1].type, "variadic")
        self.assertEqual(func.parameters[1].name, "...")

    def test_complex_pointer_expression_parameter(self):
        code = """
        int *do_stuff(int *((*arg1)[3][4])) {
            return 5;
        }
        """

        all_functions = self.c_ts.get_all_functions(code=code)
        self.assertEqual(len(all_functions), 1)
        func = all_functions[0]
        self.assertEqual(func.name, "do_stuff")
        self.assertEqual(len(func.parameters), 1)
        # TODO: not sure about this type
        self.assertEqual(func.parameters[0].type, "int*((*)[3][4])")
        self.assertEqual(func.parameters[0].name, "arg1")
