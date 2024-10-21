from cldk import CLDK


def test_build_analysis_object_for_java(test_fixture):
    """Build a CLDK Analysis object for a java application."""
    analysis = CLDK("java").analysis(project_path=test_fixture, eager=True, analysis_level="call-graph")
    assert analysis is not None
