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
C/C++ Tests
"""
from ipdb import set_trace
from cldk.analysis.c.c_analysis import CAnalysis


def test_get_c_application(test_fixture_binutils):
    """Should return a CAnalysis object"""
    analysis = CAnalysis(test_fixture_binutils)
    set_trace()
