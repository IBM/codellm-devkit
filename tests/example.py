"""Example: Use CLDK to build a code summarization model
"""

from cldk import CLDK
from cldk.analysis.java import JavaAnalysis

# Initialize the Codellm-DevKit object with the project directory, language, and analysis_backend.
ns = CLDK(
    project_dir="/Users/rajupavuluri/development/sample.daytrader8/",
    language="java",
    analysis_json_path="/Users/rkrsn/Downloads/sample.daytrader8/",
)


# Get the java application view for the project.
java_analysis: JavaAnalysis = ns.get_analysis()

classes_dict = ns.preprocessing.get_classes()
# print(classes_dict)
entry_point_classes_dict = ns.preprocessing.get_entry_point_classes()
print(entry_point_classes_dict)

entry_point_methods_dict = ns.preprocessing.get_entry_point_methods()
print(entry_point_methods_dict)


# ##get the first class in this dictionary for testing purposes
test_class_name = next(iter(classes_dict))
print(test_class_name)
test_class = classes_dict[test_class_name]
# print(test_class)
# print(test_class.is_entry_point)

# constructors = ns.preprocessing.get_all_constructors(test_class_name)
# print(constructors)

# fields = ns.preprocessing.get_all_fields(test_class_name)
# print("fields :", fields)

# methods = ns.preprocessing.get_all_methods_in_class(test_class_name)
# # print("number of methods in class ",test_class_name, ": ",len(methods))
# nested_classes = ns.preprocessing.get_all_nested_classes(test_class_name)
# # print("nested_classes: ",nested_classes)
# extended_classes = ns.preprocessing.get_extended_classes(test_class_name)
# # print("extended_classes: ",extended_classes)
# implemented_interfaces = ns.preprocessing.get_implemented_interfaces(
#     test_class_name
# )
# # print("implemented_interfaces: ",implemented_interfaces)
# class_result = ns.preprocessing.get_class(test_class_name)
# print("class_result: ", class_result)
# java_file_name = ns.preprocessing.get_java_file(test_class_name)
# # print("java_file_name ",java_file_name)
# all_methods = ns.preprocessing.get_all_methods_in_application()
# # print(all_methods)
# method = ns.preprocessing.get_method(
#     "com.ibm.websphere.samples.daytrader.util.Log",
#     "public static void trace(String message)",
# )
# print(method)
# # Get the call graph.

# cg = ns.preprocessing.get_call_graph()
# print(cg)
# # print(ns.preprocessing.get_call_graph_json())

# # print(cg.edges)
# # d = ns.preprocessing.get_all_callers("com.ibm.websphere.samples.daytrader.util.Log","public static void trace(String message)")
# # print("caller details::")
# # print(d)
# # v = ns.preprocessing.get_all_callees("com.ibm.websphere.samples.daytrader.impl.ejb3.MarketSummarySingleton","private void updateMarketSummary()")
# # print("callee details::")
# # print(v)

# """
# # Get the user specified method.
# method: JCallable = app.get_method("com.example.foo.Bar.baz")  # <- User specified method.

# # Get the slices that contain the method.
# slices: nx.Generator = ns.preprocessing.get_slices_containing_method(method, sdg=app.sdg)

# # Optional: Get samples for RAG from (say) elasticsearch
# few_shot_samples: List[str] = ns.prompting.rag(
#     database={"hostname": "https://localhost:9200", "index": "summarization"}
# ).retrive_few_shot_samples(method=method, slices=slices)

# # Natively we'll support PDL as the prompting engine to get summaries from the LLM.

# summaries: List[str] = ns.prompting(engine="pdl").summarize(method, context=slices, few_shot_samples=few_shot_samples)

# # Optionally, we will also support other open-source engines such as LMQL, Guidance, user defined Jinja, etc.
# summaries: List[str] = ns.prompting(engine="lmql").summarize(slices=slices, few_shot_samples=few_shot_samples)
# summaries: List[str] = ns.prompting(engine="guidance").summarize(slices=slices, few_shot_samples=few_shot_samples)
# summaries: List[str] = ns.prompting(engine="jinja", template="<template>").summarize(
#     slices=slices, few_shot_samples=few_shot_samples
# )
# """
classes_dict = java_analysis.get_classes()
