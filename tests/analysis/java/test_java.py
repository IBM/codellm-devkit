from typing import List, Tuple
from cldk import CLDK
from cldk.models.java.models import JMethodDetail


def test_get_class_call_graph(test_fixture):
    # Initialize the CLDK object with the project directory, language, and analysis_backend.
    cldk = CLDK(language="java")

    analysis = cldk.analysis(
        project_path=test_fixture,
        analysis_backend="codeanalyzer",
        analysis_json_path="/tmp",
        eager=True,
        analysis_level='call-graph'
    )
    class_call_graph: List[Tuple[JMethodDetail, JMethodDetail]] = analysis.get_class_call_graph(
        qualified_class_name="com.ibm.websphere.samples.daytrader.impl.direct.TradeDirectDBUtils"
    )

    assert class_call_graph is not None
