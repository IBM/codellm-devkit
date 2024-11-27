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
Backend module
"""

import subprocess
import tempfile
from pathlib import Path
import shlex
from typing import List
import pandas as pd
from pandas import DataFrame

from cldk.utils.exceptions import CodeQLQueryExecutionException


class CodeQLQueryRunner:
    """
    A class for executing CodeQL queries against a CodeQL database.

    Parameters
    ----------
    database_path : str
        The path to the CodeQL database.

    Attributes
    ----------
    database_path : Path
        The path to the CodeQL database.
    temp_file_path : Path
        The path to the temporary query file.
    csv_output_file : Path
        The path to the CSV output file.
    temp_bqrs_file_path : Path
        The path to the temporary bqrs file.
    temp_qlpack_file : Path
        The path to the temporary qlpack file.

    Methods
    -------
    __enter__()
        Context entry that creates temporary files to execute a CodeQL query.
    execute(query_string, column_names)
        Writes the query to the temporary file and executes it against the specified CodeQL database.
    __exit__(exc_type, exc_val, exc_tb)
        Clean up resources used by the CodeQL analysis.

    Raises
    ------
    CodeQLQueryExecutionException
        If there is an error executing the query.
    """

    def __init__(self, database_path: str):
        self.database_path: Path = Path(database_path)
        self.temp_file_path: Path = None

    def __enter__(self):
        """
        Context entry that creates temporary files to execute a CodeQL query.

        Returns
        -------
        instance : object
            The instance of the class.

        Notes
        -----
        This method creates temporary files to hold the query and store their paths.
        """

        # Create a temporary file to hold the query and store its path
        temp_file = tempfile.NamedTemporaryFile("w", delete=False, suffix=".ql")
        csv_file = tempfile.NamedTemporaryFile("w", delete=False, suffix=".csv")
        bqrs_file = tempfile.NamedTemporaryFile("w", delete=False, suffix=".bqrs")
        self.temp_file_path = Path(temp_file.name)
        self.csv_output_file = Path(csv_file.name)
        self.temp_bqrs_file_path = Path(bqrs_file.name)

        # Let's close the files, we'll reopen them by path when needed.
        temp_file.close()
        bqrs_file.close()
        csv_file.close()

        # Create a temporary qlpack.yml file
        self.temp_qlpack_file = self.temp_file_path.parent / "qlpack.yml"
        with self.temp_qlpack_file.open("w") as f:
            f.write("name: temp\n")
            f.write("version: 1.0.0\n")
            f.write("libraryPathDependencies: codeql/java-all\n")

        return self

    def execute(self, query_string: str, column_names: List[str]) -> DataFrame:
        """Writes the query to the temporary file and executes it against the specified CodeQL database.

        Args:
            query_string (str): The CodeQL query string to be executed.
            column_names (List[str]): The list of column names for the CSV the CodeQL produces when we execute the query.

        Returns:
            dict: A dictionary containing the resulting DataFrame.

        Raises:
            RuntimeError: If the context manager is not entered using the 'with' statement.
            CodeQLQueryExecutionException: If there is an error executing the query.
        """
        if not self.temp_file_path:
            raise RuntimeError("Context manager not entered. Use 'with' statement.")

        # Write the query to the temp file so we can execute it.
        self.temp_file_path.write_text(query_string)

        # Construct and execute the CodeQL CLI command asking for a JSON output.
        codeql_query_cmd = shlex.split(f"codeql query run {self.temp_file_path} --database={self.database_path} --output={self.temp_bqrs_file_path}", posix=False)

        call = subprocess.Popen(codeql_query_cmd, stdout=None, stderr=None)
        _, err = call.communicate()
        if call.returncode != 0:
            raise CodeQLQueryExecutionException(f"Error executing query: {err.stderr}")

        # Convert the bqrs file to a CSV file
        bqrs2csv_command = shlex.split(f"codeql bqrs decode --format=csv --output={self.csv_output_file} {self.temp_bqrs_file_path}", posix=False)

        # Read the CSV file content and cast it to a DataFrame

        call = subprocess.Popen(bqrs2csv_command, stdout=None, stderr=None)
        _, err = call.communicate()
        if call.returncode != 0:
            raise CodeQLQueryExecutionException(f"Error executing query: {err.stderr}")
        else:
            return pd.read_csv(
                self.csv_output_file,
                header=None,
                names=column_names,
                skiprows=[0],
            )

    def __exit__(self, exc_type, exc_val, exc_tb):
        """
        Clean up resources used by the CodeQL analysis.

        Deletes the temporary files created during the analysis, including the temporary file path,
        the CSV output file, and the temporary QL pack file.
        """
        if self.temp_file_path and self.temp_file_path.exists():
            self.temp_file_path.unlink()

        if self.csv_output_file and self.csv_output_file.exists():
            self.csv_output_file.unlink()

        if self.temp_qlpack_file and self.temp_qlpack_file.exists():
            self.temp_qlpack_file.unlink()
