################################################################################
# Copyright IBM Corporation 2024
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
Exceptions module
"""


class CldkInitializationException(Exception):
    """Custom exception for errors during CLDK initialization."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeanalyzerExecutionException(Exception):
    """Exception raised for errors that occur during the execution of Codeanalyzer."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeQLDatabaseBuildException(Exception):
    """Exception raised for errors that occur during the building of a CodeQL database."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeQLQueryExecutionException(Exception):
    """Exception raised for errors that occur during the execution of a CodeQL query."""

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)


class CodeanalyzerUsageException(Exception):
    """
    Exception is raised when the usage of codeanalyzer is incorrect.
    """

    def __init__(self, message):
        self.message = message
        super().__init__(self.message)
