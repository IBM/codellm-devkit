from typing import List

from unittest import TestCase

from cldk.analysis.javascript.treesitter import JavascriptSitter
from cldk.models.javascript.models import JsCallable


class TestJavascriptTreeSitter(TestCase):
    """
    Tests for Javascript TreeSitter module
    """

    def setUp(self):
        """Runs before each test case"""
        self.js_ts = JavascriptSitter()

    def tearDown(self):
        """Runs after each test case"""

    def test_get_source_file_details(self):
        code = """
        import "module-name";

        const toCamelCase = str => {
            return str.toLowerCase().replace(/[-_\s]([a-z\d])(\w*)/g,
                function replacer(m, p1, p2) {
                return p1.toUpperCase() + p2;
                }
            );
        };

        class Foo {
            async bar() {}
        }

        class Cat { 
            constructor(name) {
                this.name = name;
            }
  
            speak() {
                console.log(this.name + ' makes a noise.');
            }
        }

        class Lion extends Cat {
            speak() {
                super.speak();
                console.log(this.name + ' roars.');
            }
        }
        """

        details = self.js_ts.get_program_details(source_file=code)
        self.assertEqual(len(details.classes), 3)
        self.assertEqual(len(details.callables), 5)

    def test_get_classes(self):
        code = """
        class Foo {
            async bar() {}
        }

        class Cat { 
            constructor(name) {
                this.name = name;
            }
  
            speak() {
                console.log(this.name + ' makes a noise.');
            }
        }

        class Lion extends Cat {
            speak() {
                super.speak();
                console.log(this.name + ' roars.');
            }
        }
        """

        classes = self.js_ts.get_classes(code)
        self.assertEqual(len(classes), 3)

        self.assertEqual(classes[0].name, "Foo")
        self.assertEqual(len(classes[0].methods), 1)
        self.assertEqual(classes[0].methods[0].name, "bar")

        self.assertEqual(classes[1].name, "Cat")
        self.assertEqual(len(classes[1].methods), 2)
        self.assertEqual(classes[1].methods[0].name, "constructor")
        self.assertTrue(classes[1].methods[0].is_constructor)
        self.assertEqual(classes[1].methods[1].name, "speak")
        self.assertFalse(classes[1].methods[1].is_constructor)

        self.assertEqual(classes[2].name, "Lion")
        self.assertEqual(len(classes[2].methods), 1)
        self.assertEqual(classes[2].methods[0].name, "speak")
        self.assertEqual(classes[2].parent, "Cat")

    def test_get_all_callables(self):
        code = """
        function test({b}, c = d, e = f, async = true) {
        }

        export function *generateStuff(arg1, arg2) {
            yield arg1;
        }

        async (a) => { return foo; }

        export const 
            a = function() { console.log("say hello")},
            b = function() {},
            yourFunctionName = () => console.log("say hello");
        
        c = str => 1, d = g => "a"
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 7)

        callable: JsCallable = get_callable(callables, "test")
        self.assertEqual(callable.signature, "test({b}, c = d, e = f, async = true)")

        callable: JsCallable = get_callable(callables, "generateStuff")
        self.assertEqual(callable.signature, "generateStuff(arg1, arg2)")

        callable: JsCallable = get_callable(callables, "a")
        self.assertEqual(callable.signature, "a()")

        callable: JsCallable = get_callable(callables, "b")
        self.assertEqual(callable.signature, "b()")

        callable: JsCallable = get_callable(callables, "yourFunctionName")
        self.assertEqual(callable.signature, "yourFunctionName()")

        callable: JsCallable = get_callable(callables, "c")
        self.assertEqual(callable.signature, "c(str)")

        callable: JsCallable = get_callable(callables, "d")
        self.assertEqual(callable.signature, "d(g)")

    def test_get_top_level_functions(self):
        code = """
        function test({b}, c = d, e = f, async = true) {
        }

        export function exported_func(arg1, arg2) {
        }
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 2)

        callable: JsCallable = get_callable(callables, "test")
        self.assertEqual(callable.signature, "test({b}, c = d, e = f, async = true)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "exported_func")
        self.assertEqual(callable.signature, "exported_func(arg1, arg2)")
        self.assertFalse(callable.is_constructor)

    def test_get_top_level_generators(self):
        code = """
        function *test({b}, c = d, e = f, async = true) {
            yield c;
        }

        export function *exported_func(arg1, arg2) {
        }
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 2)

        callable: JsCallable = get_callable(callables, "test")
        self.assertEqual(callable.signature, "test({b}, c = d, e = f, async = true)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "exported_func")
        self.assertEqual(callable.signature, "exported_func(arg1, arg2)")
        self.assertFalse(callable.is_constructor)

    def test_get_top_level_arrow_functions(self):
        code = """
        const toCamelCase = str => {
            return str.toLowerCase().replace(/[-_\s]([a-z\d])(\w*)/g,
                function replacer(m, p1, p2) {
                    return p1.toUpperCase() + p2;
                }
            );
        };

        const matchAll = (regExp, str) => {
            let matches;
            const arr = [];

            while ((matches = regExp.exec(str)) !== null) {
                arr.push(matches);
            }

            return arr;
        }

        export const a = () => "", b = () => 12

        c = str => 1, d = g => "a"
        e = a => 12

        export default () => console.log("say hello");
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 8)

        callable: JsCallable = get_callable(callables, "toCamelCase")
        self.assertEqual(callable.signature, "toCamelCase(str)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "matchAll")
        self.assertEqual(callable.signature, "matchAll(regExp, str)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "a")
        self.assertEqual(callable.signature, "a()")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "b")
        self.assertEqual(callable.signature, "b()")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "c")
        self.assertEqual(callable.signature, "c(str)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "d")
        self.assertEqual(callable.signature, "d(g)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "e")
        self.assertEqual(callable.signature, "e(a)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "default")
        self.assertEqual(callable.signature, "default()")
        self.assertFalse(callable.is_constructor)

    def test_get_top_level_function_expressions(self):
        code = """
        abc = function() {};

        const func = function() {};

        export const bb = function(x, y) {};

        a = function(x) {}, b = function(y, z) {};

        export default function(w) { console.log("say hello")};
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 6)

        callable: JsCallable = get_callable(callables, "abc")
        self.assertEqual(callable.signature, "abc()")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "func")
        self.assertEqual(callable.signature, "func()")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "bb")
        self.assertEqual(callable.signature, "bb(x, y)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "b")
        self.assertEqual(callable.signature, "b(y, z)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "a")
        self.assertEqual(callable.signature, "a(x)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "default")
        self.assertEqual(callable.signature, "default(w)")
        self.assertFalse(callable.is_constructor)

    def test_get_top_level_generator_expressions(self):
        code = """
        abc = function *() {};

        const func = function *(x, y) {};

        export const bb = function*(x, y) {};

        a = function*(x) {}, b = function*(y, z) {};

        export default function *() {};
        """

        callables = self.js_ts.get_all_functions(code=code)
        self.assertEqual(len(callables), 6)

        callable: JsCallable = get_callable(callables, "abc")
        self.assertEqual(callable.signature, "abc()")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "func")
        self.assertEqual(callable.signature, "func(x, y)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "bb")
        self.assertEqual(callable.signature, "bb(x, y)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "b")
        self.assertEqual(callable.signature, "b(y, z)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "a")
        self.assertEqual(callable.signature, "a(x)")
        self.assertFalse(callable.is_constructor)

        callable: JsCallable = get_callable(callables, "default")
        self.assertEqual(callable.signature, "default()")
        self.assertFalse(callable.is_constructor)


def get_callable(callables: List[JsCallable], name: str) -> JsCallable:
    for callable in callables:
        if callable.name == name:
            return callable

    raise ValueError
