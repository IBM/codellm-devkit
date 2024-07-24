import re
from copy import deepcopy
from typing import Dict, List, Any, LiteralString

from cldk.analysis.java.treesitter import JavaSitter
from cldk.models.treesitter import Captures

java_sitter = JavaSitter()


def _replace_in_source(
    source_class_code: str,
    original_test_method_dict: dict,
    modified_test_method_dict: dict,
):
    """
    Returns a modified source using original test methods and modified ones.

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.

    original_test_method_dict: dict
        Dictionary of original test methods in the java class.

    modified_test_method_dict: dict
        Dictionary of modified test methods

    Returns:
    --------
    str
        modified source after removing duplicate test methods and merging decomposed ones.


    Comments
    --------
    # It's modifying the source code produced by an LLM.
    """
    modified_source = deepcopy(source_class_code)
    for _, body in original_test_method_dict.items():
        modified_source = modified_source.replace(body, "")
    modified_source = modified_source[: modified_source.rfind("}")]
    for _, body in modified_test_method_dict.items():
        modified_source = modified_source + "\n" + body
    modified_source = modified_source + "\n}"
    return modified_source


def separate_assertions(source_method_code: str) -> tuple[str, str]:
    """
    Separate assertions and non assertions parts

    Args:
        source_method_code: test method body

    Returns:
        tuple[str, str]:
        assertions and non assertions parts
    """
    code_split = source_method_code.splitlines()
    assert_block = ""
    code_block_without_assertions = ""
    assertion_part = []
    query = """
                            (method_invocation 
                                name: (identifier) @method_name
                            )
                    """
    captures: Captures = java_sitter.frame_query_and_capture_output(query, source_method_code)
    for capture in captures:
        if "method_name" in capture.name:
            call_site_method_name = capture.node.text.decode()
            if "assert" in call_site_method_name:
                method_node = java_sitter.safe_ascend(capture.node, 1)
                assertion_start_line = method_node.start_point[0]
                assertion_end_line = method_node.end_point[0]
                for i in range(assertion_start_line, assertion_end_line + 1):
                    assertion_part.append(i)

    for i in range(len(code_split)):
        if i not in assertion_part:
            code_block_without_assertions += code_split[i].strip() + "\n"
        else:
            assert_block += code_split[i].strip() + "\n"
    return assert_block, code_block_without_assertions


def is_empty_test_class(source_class_code: str) -> bool:
    """
    Checks if a test class has no test methods by looking for methods with @Test annotations

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.

    Returns:
    --------
    bool
        True if no test methods False if there are test methods
    """
    test_methods_dict = java_sitter.get_test_methods(source_class_code)
    print(test_methods_dict)
    return not bool(test_methods_dict)


def get_all_field_access(source_class_code: str) -> Dict[str, list[list[int]]]:
    """_summary_

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.

    Returns:
    --------
    Dict[str, [[int, int], [int, int]]]
        Dictionary with field names as keys and a list of starting and ending line, and starting and ending column.
    """
    query = """
                (field_access 
                    field:(identifier) @field_name
                    )
            """
    captures: Captures = java_sitter.frame_query_and_capture_output(query, source_class_code)
    field_dict = {}
    for capture in captures:
        if capture.name == "field_name":
            field_name = capture.node.text.decode()
            field_node = java_sitter.safe_ascend(capture.node, 2)
            start_line = field_node.start_point[0]
            start_column = field_node.start_point[1]
            end_line = field_node.end_point[0]
            end_column = field_node.end_point[1]
            start_list = [start_line, start_column]
            end_list = [end_line, end_column]
            position = [start_list, end_list]
            field_dict[field_name] = position
    return field_dict


def get_all_fields_with_annotations(source_class_code: str) -> Dict[str, Dict]:
    """
    Returns a dictionary of field names and field bodies.

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.

    Returns:
    --------
    Dict[str,Dict]
        Dictionary with field names as keys and a dictionary of annotation and body as values.
    """
    query = """
                (field_declaration 
                    (variable_declarator
                        name: (identifier) @field_name
                    )
                )
            """
    captures: Captures = java_sitter.frame_query_and_capture_output(query, source_class_code)
    field_dict = {}
    for capture in captures:
        if capture.name == "field_name":
            field_name = capture.node.text.decode()
            inner_dict = {}
            annotation = None
            field_node = java_sitter.safe_ascend(capture.node, 2)
            body = field_node.text.decode()
            for fc in field_node.children:
                if fc.type == "modifiers":
                    for mc in fc.children:
                        if mc.type == "marker_annotation":
                            annotation = mc.text.decode()
            inner_dict["annotation"] = annotation
            inner_dict["body"] = body
            field_dict[field_name] = inner_dict
    return field_dict


def get_all_methods_with_test_with_lines(source_class_code: str) -> Dict[str, List[int]]:
    """
    Returns a dictionary of method names and method bodies.

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.
    Returns:
    --------
    Dict[str,List[int]]
        Dictionary with test name as keys and
        starting and ending lines as values.
    """
    query = """
                (method_declaration
                    (modifiers
                        (marker_annotation
                        name: (identifier) @annotation)
                    )
                )
            """
    captures: Captures = java_sitter.frame_query_and_capture_output(query, source_class_code)
    annotation_method_dict = {}
    for capture in captures:
        if capture.name == "annotation":
            annotation = capture.node.text.decode()
            if "Test" in annotation:
                method_node = java_sitter.safe_ascend(capture.node, 3)
                method_start_line = method_node.start_point[0]
                method_end_line = method_node.end_point[0]
                method_name = method_node.children[2].text.decode()
                annotation_method_dict[method_name] = [method_start_line, method_end_line]
    return annotation_method_dict


def _remove_duplicates_empties(test_method_dict: dict) -> tuple[dict[Any, Any], int | Any, int | Any]:
    """
    removes all the duplicates in the test methods.

    Parameters:
    -----------
    code_block : str
        Code block of a test method.

    Returns:
    --------
    tuple[dict, int]
        Dictionary of remaining methods after dedup and number of methods removed because of dedup.
    """
    methods_kept = []
    num_dup_methods_removed = 0
    num_empty_methods_removed = 0
    block_set = set()
    for test in test_method_dict.keys():
        block = test_method_dict[test]
        # capture everything between opening and closing braces
        block = block[block.find("{") + 1 : block.rfind("}")]
        # remove white spaces,tabs and new lines
        block = re.sub(r"[\n\t\s]*", "", block)
        if not block:  # empty method
            num_empty_methods_removed = num_empty_methods_removed + 1
        if block in block_set:
            num_dup_methods_removed = num_dup_methods_removed + 1
        else:
            block_set.add(block)
            methods_kept.append(test)
    deduped_method_dict = {key: test_method_dict[key] for key in test_method_dict if key in methods_kept}
    return (
        deduped_method_dict,
        num_dup_methods_removed,
        num_empty_methods_removed,
    )


def _compose_decomposed(self, test_method_dict: dict) -> tuple[dict, int]:
    """
    merges all the test methods that only have assertions as different and rest of the code same.

    Parameters:
    -----------
    code_block : str
        Code block of a test method.

    Returns:
    --------
    tuple[dict, int]
        Dictionary of merged methods and number of methods removed because of merging.
    """
    composed_test_method_dict = {}
    num_merged_methods = 0
    block_minus_assert_dict = {}
    for test in test_method_dict.keys():
        block = test_method_dict[test]
        # capture everything between opening and closing braces
        block = block[block.find("{") + 1 : block.rfind("}")]
        # remove assertions and keep them aside
        assert_block, block_without_assertions = self._separate_assertions(block)
        if block_without_assertions in block_minus_assert_dict:
            existing_test = block_minus_assert_dict[block_without_assertions]
            composed_test_method_dict[existing_test] = composed_test_method_dict[existing_test] + "\n" + assert_block
            num_merged_methods = num_merged_methods + 1
        else:
            block_minus_assert_dict[block_without_assertions] = test
            composed_test_method_dict[test] = block
    # now add back opening and closing braces
    composed_test_method_dict = {k: "@Test\npublic void " + k + "()\n{" + v + "\n}" for k, v in composed_test_method_dict.items()}
    return composed_test_method_dict, num_merged_methods


def _separate_assertions(code_block: str) -> tuple[str, str]:
    """
    separate assertions from the code block of a test method.

    Parameters:
    -----------
    code_block : str
        Code block of a test method.

    Returns:
    --------
    tuple[str,str]
        assertions and code block without assertions.
    """
    code_block_lines = code_block.split(";")
    # remove new lines and tabs, but not spaces within lines (need to keep them for assertions)
    code_block_lines = [re.sub(r"[\n\t]*", "", x) for x in code_block_lines]
    # strip starting and trailing spaces
    code_block_lines[:] = [x.strip() for x in code_block_lines]
    # remove any empty lines
    code_block_lines[:] = [x for x in code_block_lines if x]
    code_block_lines_without_assertions = []
    code_block_without_assertions = ""
    assert_lines = []
    for line in code_block_lines:
        if line.startswith("assert"):
            assert_lines.append(line)
        else:
            # now we can remove spaces within lines
            line = re.sub(r"[\s]*", "", line)
            code_block_lines_without_assertions.append(line)
    # put back the assertion block like it should be
    assert_block = ";\n".join(assert_lines) + ";"
    if len(code_block_lines_without_assertions) > 0:
        code_block_without_assertions = ";".join(code_block_lines_without_assertions) + ";"
    return assert_block, code_block_without_assertions


def dedup_and_merge(self, source_class_code: str) -> tuple[LiteralString | Any, Any, Any, int]:
    """
    Returns a modified source after removing duplicate test methods and merging decomposed ones.

    Parameters:
    -----------
    source_class_code : str
        String containing code for a java class.

    Returns:
    --------
    str
        modified source after removing duplicate test methods and merging decomposed ones.
    """

    test_method_dict = java_sitter.get_test_methods(source_class_code)
    (
        deduped_method_dict,
        num_dup_methods_removed,
        num_empty_methods_removed,
    ) = _remove_duplicates_empties(test_method_dict)
    merged_method_dict, num_methods_merged = _compose_decomposed(deduped_method_dict)
    modified_source = _replace_in_source(test_class, test_method_dict, merged_method_dict)
    return (
        modified_source,
        num_dup_methods_removed,
        num_empty_methods_removed,
        num_methods_merged,
    )


# TODO: This has to be moved to the test file!
if __name__ == "__main__":
    assert_code_check = """
        {
        StringBuffer result = helpFormatter.renderOptions(sb, width, options, leftPad, descPad);
    assertEquals(result.toString(), "StringBuffer sb = new StringBuffer();\n" +
            "int width = 100;\n" +
            "int leftPad = 1;\n" +
            "int descPad = 1;\n" +
            "StringBuffer result = helpFormatter.renderOptions(sb, width, options, leftPad, descPad);\n" +
            "assertEquals(result.toString(), \"StringBuffer sb = new StringBuffer();\");\n");
    }
    """
    java_code = """
    public class HelloWorld {
        public static void main(String[] args) {
            System.out.println("Hello, World!");
        }
    }
    """
    # tokens = ts1.get_lexical_tokens(code=java_code, filter_by_node_type=['identifier'])
    # print(tokens)
    generated_test_code = """
        @Test
        public void feedDog_mergecandidate(){
            myDog.setWeight(myDog.name);
            myDog.eat();
            assertEquals("Error2 when eating", 10, myDog.getWeight());
        }
                            """
    source_method = """ public void method1() {
                            System.out.println("Start of method1");
                            // Call Method2 at random places
                            method2();
                            System.out.println("Called method2 first time");
                            method2();
                            System.out.println("Called method2 second time");
                            method2();
                            System.out.println("Called method2 last time");
                        }
                    """
    test_class = """   package acd;
                       import a;
                        public class DogTest {
                            private ModResortsCustomerInformation modResortsCustomerInformation;
                            @mock
                            private DataSource dataSource;
                            @mock
                            private Connection connection;
                            private PreparedStatement preparedStatement;
                            private ResultSet resultSet;
                            Dog myDog;
                            @Before
                            public void setUp(){
                                System.out.println("This is run before");
                                myDog = new Dog("Jimmy", "Beagle");
                            }
                            @After
                            public void tearDown(){
                                System.out.println("This is run after");
                            }
                            @Test
                            public void createNewDog(){
                                assertEquals("Error in creating a dog", "Jimmy", myDog.getName());
                            }
                            @Test
                            public void feedDog(){
                                myDog.setWeight(5);
                                myDog.eat();
                                assertEquals("Error when eating", 10, myDog.getWeight());
                            }
                             @Test
                            public void feedDog_mergecandidate(){
                                myDog.setWeight(5);
                                myDog.eat();
                                assertEquals("Error2 when eating", 10, myDog.getWeight());
                            }
                            @Test
                            public void createNewDog_mergecandidate(){
                                assertEquals("another error in creating a dog", "Jimmy", myDog.getName());
                            }
                            @Test
                            public void feedDog2(){
                                myDog.setWeight(5);

                                myDog.eat();
                                assertEquals("Error when eating", 10, myDog.getWeight());
                            }
                            @Test
                            public void feedDog3(){
                                myDog.setWeight(5);       
                                myDog.eat();
                                assertEquals("Error when eating", 10, myDog.getWeight());
                            }
                            @Test
                            public void createNewDog2(){

                                assertEquals("Error in creating a dog", "Jimmy", myDog.getName());
                            }
                            @Test
                            public void emptyTest(){


                            }
                    }
                 """
    empty_test_class = """
                            public class DogTest {
                                private ModResortsCustomerInformation modResortsCustomerInformation;
                                @mock
                                private DataSource dataSource;
                                @mock
                                private Connection connection;
                                private PreparedStatement preparedStatement;
                                private ResultSet resultSet;
                                Dog myDog;
                                @Before
                                public void setUp(){
                                    System.out.println("This is run before");
                                    myDog = new Dog("Jimmy", "Beagle");
                                }
                                @After
                                public void tearDown(){
                                    System.out.println("This is run after");
                            }
                       """
    # target_method_name = "public void method2()"
    print(separate_assertions(source_method_code=assert_code_check))
    # print(ts.get_calling_lines(source_method,target_method_name))
    # print(ts.get_all_methods_with_test_with_lines(test_class))
    # print(ts.get_test_methods(test_class))
    # print(ts.get_all_field_access(generated_test_code))
    # test_method_dict = ts.get_test_methods(test_class)
    # deduped_method_dict, num_methods_removed = ts._remove_duplicates(test_method_dict)
    # print("num_methods_removed ",num_methods_removed)
    # merged_method_dict, num_merged_methods = ts._compose_decomposed(deduped_method_dict)
    # print("merged_methods ",num_merged_methods)
    # print(test_method_dict)
    # print(ts._replace_in_source(test_class,test_method_dict,merged_method_dict))
    # print(ts.get_all_methods_with_annotations(test_class, ["Test", "Before"]))
    # print(ts.get_all_fields_with_annotations(test_class))
    # print(ts.dedup_and_merge(test_class))
    # print(ts.is_empty_test_class(empty_test_class))
