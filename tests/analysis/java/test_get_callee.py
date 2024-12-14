from unittest import TestCase

from cldk import CLDK
from cldk.analysis import AnalysisLevel


class ServletTest(TestCase):
    def setUp(self):
        self.cldk = CLDK(language="java").analysis(
            project_path='path',
            analysis_backend="codeanalyzer",
            analysis_backend_path=None,
            analysis_level=AnalysisLevel.symbol_table,
            analysis_json_path='./output',
        )

    def test_get_callee(self):
        source_class_name = 'com.acme.modres.AvailabilityCheckerServlet'
        source_method_declaration = 'doGet(HttpServletRequest, HttpServletResponse)'
        callee = self.cldk.get_callers(source_class_name, source_method_declaration, True)
        print(callee)
        self.assertIsNotNone(callee)
