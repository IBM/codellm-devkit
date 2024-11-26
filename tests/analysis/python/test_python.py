from cldk import CLDK
from cldk.analysis.python.python import PythonAnalysis


def _get_analysis_object(path_to_test_project):
    cldk = CLDK(language="python")
    analysis = cldk.analysis(project_path=path_to_test_project)
    return analysis


def test_python_analysis(test_fixture_python):
    analysis: PythonAnalysis = _get_analysis_object(test_fixture_python)
    assert analysis is not None, "Analysis object is None"


def test_python_get_all_modules(test_fixture_python):
    analysis: PythonAnalysis = _get_analysis_object(test_fixture_python)
    assert len(analysis.get_modules()) > 0, "No modules found"
