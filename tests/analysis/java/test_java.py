from typing import List, Tuple

import pytest

from cldk import CLDK
from cldk.analysis import AnalysisLevel
from cldk.models.java.models import JMethodDetail
import toml
import shutil
import pytest
import zipfile
from pathlib import Path
from urllib.request import urlretrieve


# @pytest.mark.parametrize("test_fixture", ["daytrader"], indirect=["test_fixture"])
def test_get_class_call_graph(test_fixture, codeanalyzer_jar_path):
    # Initialize the CLDK object with the project directory, language, and analysis_backend.
    cldk = CLDK(language="java")

    analysis = cldk.analysis(
        project_path=test_fixture,
        analysis_backend="codeanalyzer",
        analysis_backend_path=codeanalyzer_jar_path,
        eager=True,
        analysis_level=AnalysisLevel.call_graph,
    )
    class_call_graph: List[Tuple[JMethodDetail, JMethodDetail]] = analysis.get_class_call_graph(
        qualified_class_name="com.ibm.websphere.samples.daytrader.impl.direct.TradeDirectDBUtils"
    )
    assert class_call_graph is not None


# @pytest.mark.parametrize("test_fixture", ["CLI"], indirect=["test_fixture"])
# def test_get_class_call_graph_using_symbol_table(test_fixture):
#     # Initialize the CLDK object with the project directory, language, and analysis_backend.
#     cldk = CLDK(language="java")

#     analysis = cldk.analysis(
#         project_path=test_fixture,
#         analysis_backend="codeanalyzer",
#         analysis_json_path="../../../tests/resources/java/analysis_db",
#         eager=False,
#         analysis_level=AnalysisLevel.symbol_table,
#     )
#     class_call_graph: List[Tuple[JMethodDetail, JMethodDetail]] = analysis.get_class_call_graph(
#         qualified_class_name="org.apache.commons.cli.DefaultParser", method_signature="handleConcatenatedOptions(String)", using_symbol_table=True
#     )

#     assert class_call_graph is not None
