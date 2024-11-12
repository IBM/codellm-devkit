# def test_specifiy_codeanalyzer_backend_manually(test_fixture):
#     # Initialize the CLDK object with the project directory, language, and backend.
#     ns = CLDK(
#         project_dir=test_fixture[0],
#         language="java",
#         backend="codeanalyzer",
#         backend_path=test_fixture[1],
#         analysis_db="/tmp",
#         sdg=True,
#         use_graalvm_binary=False,
#         eager=True,
#     )
#     classes_dict = ns.preprocessing.get_all_classes()
#     assert len(classes_dict) > 0


# def test_get_all_calsses(test_fixture):
#     # Initialize the CLDK object with the project directory, language, and analysis_backend.
#     ns = CLDK(
#         project_dir=test_fixture[0],
#         language="java",
#         analysis_backend="codeanalyzer",
#         analysis_json_path="/tmp",
#         sdg=True,
#         use_graalvm_binary=False,
#     )
#     classes_dict = ns.analysis.get_all_classes()
#     assert len(classes_dict) == 148
