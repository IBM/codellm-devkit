################################################################################
# Copyright IBM Corporation 2024, 2025
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
Test Cases for JCodeanalyzer
"""

import os
import json
from typing import Dict, List
from unittest.mock import patch, MagicMock
import networkx as nx
from networkx import DiGraph

from cldk.analysis import AnalysisLevel
from cldk.analysis.java.codeanalyzer import JCodeanalyzer
from cldk.models.java.models import JApplication, JType, JCallable, JCompilationUnit
from cldk.models.java import JGraphEdges


def get_analysis_json(base_path: str) -> str:
    """Opens the analysis.json file and returns the contents as a json string"""
    # check if the folder exists
    if not os.path.exists(base_path):
        raise ValueError("Error: Folder '%s' does not exist", base_path)

    # Read the json file and return it a a json string
    analysis_json = {}
    with open(os.path.join(base_path, "analysis.json"), "r", encoding="utf-8") as json_data:
        analysis_json = json.dumps(json.load(json_data))
    return analysis_json


def test_init_japplication(test_fixture, codeanalyzer_jar_path, analysis_json_fixture):
    """It should return the initialized JApplication"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=codeanalyzer_jar_path,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        app = code_analyzer.init_japplication(analysis_json)
        assert app is not None
        assert isinstance(app, JApplication)


def test_init_codeanalyzer_no_json_path(test_fixture, analysis_json_fixture):
    """It should initialize the codeanalyzer without a json path"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files="a.java b.java",
        )
        app = code_analyzer.application
        assert app is not None
        assert isinstance(app, JApplication)


def test_init_codeanalyzer_with_json_path(test_fixture, analysis_json_fixture):
    """It should initialize the codeanalyzer with a json path"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=analysis_json_fixture,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        app = code_analyzer.application
        assert app is not None
        assert isinstance(app, JApplication)

        # test for eager_analysis:
        code_analyzer.eager_analysis = True
        app = code_analyzer._init_codeanalyzer(1)
        assert app is not None
        assert isinstance(app, JApplication)

        # Test with target files
        code_analyzer.target_files = "a.java b.java"
        app = code_analyzer._init_codeanalyzer(1)
        assert app is not None
        assert isinstance(app, JApplication)


def test_get_codeanalyzer_exec(test_fixture, codeanalyzer_jar_path, analysis_json_fixture):
    """It should return the correct codeanalyzer location"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)

        # Test with GaalVM as the location
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=codeanalyzer_jar_path,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=True,
            eager_analysis=False,
            target_files=None,
        )
        exec_path = code_analyzer._get_codeanalyzer_exec()[0]
        relative_path = exec_path.split("/cldk")[1]
        assert relative_path == "/analysis/java/codeanalyzer/bin/codeanalyzer"

        # Test with analysis_backend_path as the location
        code_analyzer.use_graalvm_binary = False
        jar_file = code_analyzer._get_codeanalyzer_exec()[-1]
        exec_path = os.path.dirname(jar_file)
        assert exec_path == str(codeanalyzer_jar_path)

        # Test with internal codeanalyzer jar file
        code_analyzer.analysis_backend_path = None
        jar_file = code_analyzer._get_codeanalyzer_exec()[-1]
        exec_path = os.path.dirname(jar_file)
        relative_path = exec_path.split("/cldk")[1]
        assert relative_path == "/analysis/java/codeanalyzer/jar"


def test_generate_call_graph(test_fixture, analysis_json_fixture):
    """It should generate a graph"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # generate with symbol table
        cg = code_analyzer._generate_call_graph(True)
        assert isinstance(cg, nx.DiGraph)

        # generate without symbol table
        cg = code_analyzer._generate_call_graph(False)
        assert isinstance(cg, nx.DiGraph)
        edge = list(cg.edges(data=True))[0]
        assert edge[2]["type"] == "CALL_DEP" or edge[2]["type"] == "CONTROL_DEP"
        assert isinstance(int(edge[2]["weight"]), int)
        assert isinstance(edge[2]["calling_lines"], list)
        # assert all(isinstance(line, str) for line in edge[2]["calling_lines"])


def test_codeanalyzer_single_file(test_fixture, analysis_json_fixture):
    """It should process a single file"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code="dummy.java",
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        app = code_analyzer._codeanalyzer_single_file()
        assert app is not None
        assert isinstance(app, JApplication)


def test_get_application(test_fixture, analysis_json_fixture):
    """It should return the application"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        code_analyzer.application = None
        app = code_analyzer._get_application()
        assert app is not None
        assert isinstance(app, JApplication)


def test_get_symbol_table(test_fixture, analysis_json_fixture):
    """It should return the symbol table"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        code_analyzer.application = None
        comp_unit = code_analyzer.get_symbol_table()
        assert comp_unit is not None
        assert isinstance(comp_unit, Dict)


def test_get_application_view(test_fixture, analysis_json_fixture):
    """It should return an application view"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        code_analyzer.application = None
        app = code_analyzer.get_application_view()
        assert app is not None
        assert isinstance(app, JApplication)

        # Test with source file
        code_analyzer.source_code = "dummy.java"
        app = code_analyzer.get_application_view()
        assert app is not None
        assert isinstance(app, JApplication)


def test_get_system_dependency_graph(test_fixture, analysis_json_fixture):
    """It should return an system dependency graph"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        code_analyzer.application.system_dependency_graph = None
        graph = code_analyzer.get_system_dependency_graph()
        assert graph is not None
        assert isinstance(graph, list)
        assert isinstance(graph[0], JGraphEdges)


def test_get_call_graph(test_fixture, analysis_json_fixture):
    """It should return a call graph"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        graph = code_analyzer.get_call_graph()
        assert graph is not None
        assert isinstance(graph, DiGraph)

        # test for symbol table
        code_analyzer.analysis_level = "symbol_table"
        graph = code_analyzer.get_call_graph()
        assert graph is not None
        assert isinstance(graph, DiGraph)


def test_get_call_graph_json(test_fixture, analysis_json_fixture):
    """It should return the call graph as json"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        graph_json = code_analyzer.get_call_graph_json()
        assert graph_json is not None
        assert isinstance(graph_json, str)
        graph = json.loads(graph_json)
        assert graph is not None
        assert isinstance(graph, list)


def test_get_all_callers(test_fixture, analysis_json_fixture):
    """It should return all of the callers"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # TODO: This currently doesn't work. Code has bad call
        # # Call using symbol table
        # all_callers = code_analyzer.get_all_callers("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)", True)
        # assert all_callers is not None
        # assert isinstance(all_callers, Dict)
        # assert "caller_details" in all_callers

        # Call without using symbol table
        all_callers = code_analyzer.get_all_callers("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)", False)
        assert all_callers is not None
        assert isinstance(all_callers, Dict)
        assert "caller_details" in all_callers


def test_get_all_callees(test_fixture, analysis_json_fixture):
    """It should return all of the callees"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # TODO: This currently doesn't work. Code has bad call
        # # Call using symbol table
        # all_callers = code_analyzer.get_all_callees("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)", True)
        # assert all_callers is not None
        # assert isinstance(all_callers, Dict)
        # assert "caller_details" in all_callers

        # Call without using symbol table
        all_callers = code_analyzer.get_all_callees("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)", False)
        assert all_callers is not None
        assert isinstance(all_callers, Dict)
        assert "callee_details" in all_callers


def test_get_all_methods_in_application(test_fixture, analysis_json_fixture):
    """It should return all of the methods in an application"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )
        all_methods = code_analyzer.get_all_methods_in_application()
        assert all_methods is not None
        assert isinstance(all_methods, Dict)


def test_get_all_classes(test_fixture, analysis_json_fixture):
    """It should return all of the classes in an application"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_methods = code_analyzer.get_all_classes()
        assert all_methods is not None
        assert isinstance(all_methods, Dict)


def test_get_class(test_fixture, analysis_json_fixture):
    """It should return a class given the qualified name"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        class_info = code_analyzer.get_class("com.ibm.websphere.samples.daytrader.TradeAction")
        assert class_info is not None
        assert isinstance(class_info, JType)


def test_get_method(test_fixture, analysis_json_fixture):
    """It should return all of the callees"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        method = code_analyzer.get_method("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)")
        assert method is not None
        assert isinstance(method, JCallable)


def test_get_java_file(test_fixture, analysis_json_fixture):
    """It should return the java file for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        java_file = code_analyzer.get_java_file("com.ibm.websphere.samples.daytrader.TradeAction")
        assert java_file is not None
        assert isinstance(java_file, str)
        relative_filename = java_file.split("/src")[1]
        assert relative_filename == "/main/java/com/ibm/websphere/samples/daytrader/TradeAction.java"

        # Test compilation unit for this file
        comp_unit = code_analyzer.get_java_compilation_unit(java_file)
        assert comp_unit is not None
        assert isinstance(comp_unit, JCompilationUnit)


def test_get_all_methods_in_class(test_fixture, analysis_json_fixture):
    """It should return all of the methods for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_methods = code_analyzer.get_all_methods_in_class("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_methods is not None
        assert isinstance(all_methods, Dict)
        assert len(all_methods) > 0


def test_get_all_constructors(test_fixture, analysis_json_fixture):
    """It should return all of the constructors for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_constructors = code_analyzer.get_all_constructors("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_constructors is not None
        assert isinstance(all_constructors, Dict)
        assert len(all_constructors) > 0


def test_get_all_sub_classes(test_fixture, analysis_json_fixture):
    """It should return all of the subclasses for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_subclasses = code_analyzer.get_all_sub_classes("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_subclasses is not None
        assert isinstance(all_subclasses, Dict)


def test_get_all_fields(test_fixture, analysis_json_fixture):
    """It should return all of the fields for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_fields = code_analyzer.get_all_fields("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_fields is not None
        assert isinstance(all_fields, List)
        assert len(all_fields) > 0

        # Handle get fields for class not found
        all_fields = code_analyzer.get_all_fields("com.not.Found")
        assert all_fields is not None
        assert isinstance(all_fields, List)
        assert len(all_fields) == 0


def test_get_all_nested_classes(test_fixture, analysis_json_fixture):
    """It should return all of the nested classes for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_nested_classes = code_analyzer.get_all_nested_classes("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_nested_classes is not None
        assert isinstance(all_nested_classes, List)

        # Handle class not found
        all_nested_classes = code_analyzer.get_all_nested_classes("com.not.Found")
        assert all_nested_classes is not None
        assert isinstance(all_nested_classes, List)
        assert len(all_nested_classes) == 0


def test_get_extended_classes(test_fixture, analysis_json_fixture):
    """It should return all of the extended classes for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        all_extended_classes = code_analyzer.get_extended_classes("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_extended_classes is not None
        assert isinstance(all_extended_classes, List)

        # Handle class not found
        all_extended_classes = code_analyzer.get_extended_classes("com.not.Found")
        assert all_extended_classes is not None
        assert isinstance(all_extended_classes, List)
        assert len(all_extended_classes) == 0


def test_get_implemented_interfaces(test_fixture, analysis_json_fixture):
    """It should return all of the implemented interfaces for a class"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # Call without using symbol table
        all_interfaces = code_analyzer.get_implemented_interfaces("com.ibm.websphere.samples.daytrader.TradeAction")
        assert all_interfaces is not None
        assert isinstance(all_interfaces, List)

        # Handle class not found
        all_interfaces = code_analyzer.get_implemented_interfaces("com.not.Found")
        assert all_interfaces is not None
        assert isinstance(all_interfaces, List)
        assert len(all_interfaces) == 0


def test_get_class_call_graph_using_symbol_table(test_fixture, analysis_json_fixture):
    """It should return the call graph using the symbol table"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # TODO: The code seems to be broken for this case
        # # Call with method signature
        # all_call_graph = code_analyzer.get_class_call_graph_using_symbol_table("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)")
        # assert all_call_graph is not None
        # assert isinstance(all_call_graph, List)

        # Call without method signature
        all_call_graph = code_analyzer.get_class_call_graph_using_symbol_table("com.ibm.websphere.samples.daytrader.TradeAction", None)
        assert all_call_graph is not None
        assert isinstance(all_call_graph, List)


def test_get_class_call_graph(test_fixture, analysis_json_fixture):
    """It should return the call graph"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # Call with method signature
        class_call_graph = code_analyzer.get_class_call_graph("com.ibm.websphere.samples.daytrader.TradeAction", "getQuote(String)")
        assert class_call_graph is not None
        assert isinstance(class_call_graph, List)

        # Call without method signature
        class_call_graph = code_analyzer.get_class_call_graph("com.ibm.websphere.samples.daytrader.TradeAction", None)
        assert class_call_graph is not None
        assert isinstance(class_call_graph, List)


def test_get_all_entry_point_methods(test_fixture, analysis_json_fixture):
    """It should return the all of the entry point methods"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # Call with method signature
        entry_point_methods = code_analyzer.get_all_entry_point_methods()
        assert entry_point_methods is not None
        assert isinstance(entry_point_methods, Dict)
        assert len(entry_point_methods) > 0


def test_get_all_entry_point_classes(test_fixture, analysis_json_fixture):
    """It should return the all of the entry point classes"""
    # Get a known good analysis file
    analysis_json = get_analysis_json(analysis_json_fixture)

    # Patch subprocess so that it does not run codeanalyzer
    with patch("cldk.analysis.java.codeanalyzer.codeanalyzer.subprocess.run") as run_mock:
        run_mock.return_value = MagicMock(stdout=analysis_json, returncode=0)
        code_analyzer = JCodeanalyzer(
            project_dir=test_fixture,
            source_code=None,
            analysis_backend_path=None,
            analysis_json_path=None,
            analysis_level=AnalysisLevel.call_graph,
            use_graalvm_binary=False,
            eager_analysis=False,
            target_files=None,
        )

        # Call with method signature
        entry_point_classes = code_analyzer.get_all_entry_point_classes()
        assert entry_point_classes is not None
        assert isinstance(entry_point_classes, Dict)
        assert len(entry_point_classes) > 0
