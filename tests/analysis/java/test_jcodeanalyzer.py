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
from typing import Dict, List, Tuple
from unittest.mock import patch, MagicMock
import networkx as nx
from networkx import DiGraph

from cldk.analysis import AnalysisLevel
from cldk.analysis.java.codeanalyzer import JCodeanalyzer
from cldk.models.java.models import JApplication, JType, JCallable, JCompilationUnit, JMethodDetail
from cldk.models.java import JGraphEdges


def test_init_japplication(test_fixture, codeanalyzer_jar_path, analysis_json):
    """It should return the initialized JApplication"""

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


def test_init_codeanalyzer_no_json_path(test_fixture, analysis_json):
    """It should initialize the codeanalyzer without a json path"""

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


def test_init_codeanalyzer_with_json_path(test_fixture, analysis_json, analysis_json_fixture):
    """It should initialize the codeanalyzer with a json path"""

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


def test_get_codeanalyzer_exec(test_fixture, codeanalyzer_jar_path, analysis_json):
    """It should return the correct codeanalyzer location"""

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


def test_generate_call_graph(test_fixture, analysis_json):
    """It should generate a graph"""

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


def test_codeanalyzer_single_file(test_fixture, analysis_json):
    """It should process a single file"""

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


def test_get_application(test_fixture, analysis_json):
    """It should return the application"""

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


def test_get_symbol_table(test_fixture, analysis_json):
    """It should return the symbol table"""

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
        symbol_table = code_analyzer.get_symbol_table()
        assert symbol_table is not None
        assert isinstance(symbol_table, Dict)
        for _, comp_unit in symbol_table.items():
            assert isinstance(comp_unit, JCompilationUnit)


def test_get_application_view(test_fixture, analysis_json):
    """It should return an application view"""

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
        code_analyzer.source_code = "./tests/resources/java/application/sample.daytrader8-1.2/src/main/java/com/ibm/websphere/samples/daytrader/web/websocket/ActionMessage.java"
        app = code_analyzer.get_application_view()
        assert app is not None
        assert isinstance(app, JApplication)


def test_get_system_dependency_graph(test_fixture, analysis_json):
    """It should return an system dependency graph"""

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
        assert len(graph) > 0
        assert isinstance(graph[0], JGraphEdges)


def test_get_call_graph(test_fixture, analysis_json):
    """It should return a call graph"""

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
        graph = code_analyzer.get_call_graph()
        assert graph is not None
        assert isinstance(graph, DiGraph)

        # test for symbol table
        code_analyzer.analysis_level = AnalysisLevel.symbol_table
        graph = code_analyzer.get_call_graph()
        assert graph is not None
        assert isinstance(graph, DiGraph)


def test_get_call_graph_json(test_fixture, analysis_json):
    """It should return the call graph as json"""

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


def test_get_all_callers(test_fixture, analysis_json):
    """It should return all of the callers"""

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
        all_callers = code_analyzer.get_all_callers("com.ibm.websphere.samples.daytrader.util.Log", "log(String)", False)
        assert all_callers is not None
        assert isinstance(all_callers, Dict)
        assert len(all_callers) > 0
        assert "caller_details" in all_callers
        assert len(all_callers["caller_details"]) == 18
        for method in all_callers["caller_details"]:
            assert isinstance(method["caller_method"], JMethodDetail)

        # Call using symbol table

        # TODO: This currently doesn't work. Code has bad call as seen in this error message:
        # TypeError: JavaSitter.get_calling_lines() missing 1 required positional argument: 'is_target_method_a_constructor'
        all_callers = code_analyzer.get_all_callers("com.ibm.websphere.samples.daytrader.util.Log", "log(String)", True)
        assert all_callers is not None
        assert isinstance(all_callers, Dict)
        assert "caller_details" in all_callers


def test_get_all_callees(test_fixture, analysis_json):
    """It should return all of the callees"""

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
        all_callees = code_analyzer.get_all_callees("com.ibm.websphere.samples.daytrader.util.Log", "printCollection(String, Collection)", False)
        assert all_callees is not None
        assert isinstance(all_callees, Dict)
        assert "callee_details" in all_callees
        assert len(all_callees["callee_details"]) == 2

        # Call using the symbol table

        # TODO: Throws the following exception
        # TypeError: JavaSitter.get_calling_lines() missing 1 required positional argument: 'is_target_method_a_constructor'
        all_callees = code_analyzer.get_all_callees("com.ibm.websphere.samples.daytrader.util.Log", "printCollection(String, Collection)", True)
        assert all_callees is not None
        assert isinstance(all_callees, Dict)
        assert "callee_details" in all_callees
        assert len(all_callees["callee_details"]) == 2


def test_get_all_methods_in_application(test_fixture, analysis_json):
    """It should return all of the methods in an application"""

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
        assert len(all_methods) > 0
        # Validate structure
        for _, method in all_methods.items():
            assert method is not None
            assert isinstance(method, Dict)
            for _, callable in method.items():
                assert callable is not None
                assert isinstance(callable, JCallable)


def test_get_all_classes(test_fixture, analysis_json):
    """It should return all of the classes in an application"""

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

        all_classes = code_analyzer.get_all_classes()
        assert all_classes is not None
        assert isinstance(all_classes, Dict)
        assert len(all_classes) > 0
        # Validate structure
        for _, a_class in all_classes.items():
            assert a_class is not None
            assert isinstance(a_class, JType)


def test_get_class(test_fixture, analysis_json):
    """It should return a class given the qualified name"""

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

        class_info = code_analyzer.get_class("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect")
        assert class_info is not None
        assert isinstance(class_info, JType)


def test_get_method(test_fixture, analysis_json):
    """It should return the method"""

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

        method = code_analyzer.get_method("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect", "publishQuotePriceChange(QuoteDataBean, BigDecimal, BigDecimal, double)")
        assert method is not None
        assert isinstance(method, JCallable)


def test_get_java_file(test_fixture, analysis_json):
    """It should return the java file for a class"""

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

        java_file = code_analyzer.get_java_file("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect")
        assert java_file is not None
        assert isinstance(java_file, str)
        relative_file = java_file.split("/src/")[1]
        assert relative_file == "main/java/com/ibm/websphere/samples/daytrader/impl/direct/TradeDirect.java"

        # Test compilation unit for this file
        comp_unit = code_analyzer.get_java_compilation_unit(java_file)
        assert comp_unit is not None
        assert isinstance(comp_unit, JCompilationUnit)


def test_get_all_methods_in_class(test_fixture, analysis_json):
    """It should return all of the methods for a class"""

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

        all_methods = code_analyzer.get_all_methods_in_class("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect")
        assert all_methods is not None
        assert isinstance(all_methods, Dict)
        assert len(all_methods) > 0
        # Validate structure
        for _, method in all_methods.items():
            assert method is not None
            assert isinstance(method, JCallable)


def test_get_all_constructors(test_fixture, analysis_json):
    """It should return all of the constructors for a class"""

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

        # Test if it finds the 3 constructors in AccountDataBean
        all_constructors = code_analyzer.get_all_constructors("com.ibm.websphere.samples.daytrader.entities.AccountDataBean")
        assert all_constructors is not None
        assert isinstance(all_constructors, Dict)
        assert len(all_constructors) == 3
        # Validate structure
        for _, constructor in all_constructors.items():
            assert constructor is not None
            assert isinstance(constructor, JCallable)

        # Test class with no constructors
        all_constructors = code_analyzer.get_all_constructors("com.ibm.websphere.samples.daytrader.util.FinancialUtils")
        assert all_constructors is not None
        assert isinstance(all_constructors, Dict)
        assert len(all_constructors) == 0


def test_get_all_sub_classes(test_fixture, analysis_json):
    """It should return all of the subclasses for a class"""

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

        all_subclasses = code_analyzer.get_all_sub_classes("javax.ws.rs.core.Application")
        assert all_subclasses is not None
        assert isinstance(all_subclasses, Dict)
        assert len(all_subclasses) == 1
        assert "com.ibm.websphere.samples.daytrader.jaxrs.JAXRSApplication" in all_subclasses


def test_get_all_fields(test_fixture, analysis_json):
    """It should return all of the fields for a class"""

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

        all_fields = code_analyzer.get_all_fields("com.ibm.websphere.samples.daytrader.entities.AccountDataBean")
        assert all_fields is not None
        assert isinstance(all_fields, List)
        assert len(all_fields) == 12

        # Handle get fields for class not found
        all_fields = code_analyzer.get_all_fields("com.not.Found")
        assert all_fields is not None
        assert isinstance(all_fields, List)
        assert len(all_fields) == 0


def test_get_all_nested_classes(test_fixture, analysis_json):
    """It should return all of the nested classes for a class"""

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

        # TODO: This should return 1 but return 0

        # Test with a KeyBlock that has nested KeyBlockIterator
        all_nested_classes = code_analyzer.get_all_nested_classes("com.ibm.websphere.samples.daytrader.util.KeyBlock")
        assert all_nested_classes is not None
        assert isinstance(all_nested_classes, List)
        assert len(all_nested_classes) == 1

        # Handle class not found
        all_nested_classes = code_analyzer.get_all_nested_classes("com.not.Found")
        assert all_nested_classes is not None
        assert isinstance(all_nested_classes, List)
        assert len(all_nested_classes) == 0


def test_get_extended_classes(test_fixture, analysis_json):
    """It should return all of the extended classes for a class"""

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

        all_extended_classes = code_analyzer.get_extended_classes("com.ibm.websphere.samples.daytrader.util.TradeRunTimeModeLiteral")
        assert all_extended_classes is not None
        assert isinstance(all_extended_classes, List)
        assert len(all_extended_classes) == 1
        assert "javax.enterprise.util.AnnotationLiteral<com.ibm.websphere.samples.daytrader.interfaces.RuntimeMode>" in all_extended_classes

        # Test with class that is not extended
        all_extended_classes = code_analyzer.get_extended_classes("com.ibm.websphere.samples.daytrader.entities.HoldingDataBean")
        assert all_extended_classes is not None
        assert isinstance(all_extended_classes, List)
        assert len(all_extended_classes) == 0


def test_get_implemented_interfaces(test_fixture, analysis_json):
    """It should return all of the implemented interfaces for a class"""

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

        # Call class that implements 2 interfaces
        all_interfaces = code_analyzer.get_implemented_interfaces("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect")
        assert all_interfaces is not None
        assert isinstance(all_interfaces, List)
        assert len(all_interfaces) == 2
        assert "com.ibm.websphere.samples.daytrader.interfaces.TradeServices" in all_interfaces
        assert "java.io.Serializable" in all_interfaces

        # Call class that implements no interfaces
        all_interfaces = code_analyzer.get_implemented_interfaces("com.ibm.websphere.samples.daytrader.util.TradeConfig")
        assert all_interfaces is not None
        assert isinstance(all_interfaces, List)
        assert len(all_interfaces) == 0


def test_get_class_call_graph_using_symbol_table(test_fixture, analysis_json):
    """It should return the call graph using the symbol table"""

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

        # TODO: I could be wrong but I think these should not be zero?

        # Call with method signature
        all_call_graph = code_analyzer.get_class_call_graph_using_symbol_table("com.ibm.websphere.samples.daytrader.impl.direct.AsyncOrder", "run()")
        assert all_call_graph is not None
        assert isinstance(all_call_graph, List)
        assert len(all_call_graph) > 0

        # Call without method signature
        all_call_graph = code_analyzer.get_class_call_graph_using_symbol_table("com.ibm.websphere.samples.daytrader.impl.direct.AsyncOrder", None)
        assert all_call_graph is not None
        assert isinstance(all_call_graph, List)
        assert len(all_call_graph) > 0


def test_get_class_call_graph(test_fixture, analysis_json):
    """It should return the call graph"""

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
        class_call_graph = code_analyzer.get_class_call_graph(
            "com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect", "createHolding(Connection, int, String, double, BigDecimal)"
        )
        assert class_call_graph is not None
        assert isinstance(class_call_graph, List)
        assert len(class_call_graph) == 4
        for method in class_call_graph:
            assert isinstance(method, Tuple)
            assert isinstance(method[0], JMethodDetail)
            assert isinstance(method[1], JMethodDetail)

        # Call without method signature
        class_call_graph = code_analyzer.get_class_call_graph("com.ibm.websphere.samples.daytrader.impl.direct.TradeDirect", None)
        assert class_call_graph is not None
        assert isinstance(class_call_graph, List)
        assert len(class_call_graph) > 0


def test_get_all_entry_point_methods(test_fixture, analysis_json):
    """It should return the all of the entry point methods"""

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
        # Validate structure
        for _, entry_point in entry_point_methods.items():
            assert isinstance(entry_point, Dict)
            for _, method in entry_point.items():
                assert isinstance(method, JCallable)


def test_get_all_entry_point_classes(test_fixture, analysis_json):
    """It should return the all of the entry point classes"""

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
        # Validate structure
        for _, entry_point in entry_point_classes.items():
            assert isinstance(entry_point, JType)
