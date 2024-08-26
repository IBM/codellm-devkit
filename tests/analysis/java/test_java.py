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


@pytest.fixture(scope="session", autouse=True)
def test_fixture(application: str = ''):
    """
    Returns the path to the test data directory.

    Yields:
        Path : The path to the test data directory.
    """
    # ----------------------------------[ SETUP ]----------------------------------
    # Path to your pyproject.toml
    pyproject_path = Path(__file__).parent.parent.parent.parent / "pyproject.toml"

    # Load the configuration
    config = toml.load(pyproject_path)

    # Access the test data path
    test_data_path = config["tool"]["cldk"]["testing"]["sample-application"]

    if not Path(__file__).parent.parent.parent.parent.joinpath(test_data_path).exists():
        Path(test_data_path).mkdir(parents=True)
    if application == "daytrader":
        url = "https://github.com/OpenLiberty/sample.daytrader8/archive/refs/tags/v1.2.zip"
        filename = Path(test_data_path).absolute() / "v1.2.zip"
    elif application == "CLI" or application == "":
        url = "https://github.com/apache/commons-cli/archive/refs/tags/commons-cli-1.8.0-RC2.zip"
        filename = Path(__file__).parent.parent.parent.parent.joinpath(test_data_path).joinpath("commons-cli-1.8.0-RC2.zip")
    urlretrieve(url, filename)

    # Extract the zip file to the test data path
    with zipfile.ZipFile(filename, "r") as zip_ref:
        zip_ref.extractall(Path(__file__).parent.parent.parent.parent.joinpath(test_data_path))

    # Remove the zip file
    filename.unlink()
    # --------------------------------------------------------------------------------
    if application == "daytrader":
        # Daytrader8 sample application path
        yield Path(Path(__file__).parent.parent.parent.parent.joinpath(test_data_path)) / "sample.daytrader8-1.2"
    else:
        yield Path(Path(__file__).parent.parent.parent.parent.joinpath(test_data_path)) / "commons-cli-commons-cli-1.8.0-RC2"

    # -----------------------------------[ TEARDOWN ]----------------------------------
    # Remove the daytrader8 sample application that was downloaded for testing
    for directory in Path(test_data_path).iterdir():
        if directory.exists() and directory.is_dir():
            shutil.rmtree(directory)
    # ---------------------------------------------------------------------------------


@pytest.mark.parametrize('test_fixture', ['daytrader'], indirect=['test_fixture'])
def test_get_class_call_graph(test_fixture):
    # Initialize the CLDK object with the project directory, language, and analysis_backend.
    cldk = CLDK(language="java")

    analysis = cldk.analysis(
        project_path=test_fixture,
        analysis_backend="codeanalyzer",
        analysis_json_path="../../../tests/resources/java/analysis_db",
        eager=True,
        analysis_level=AnalysisLevel.call_graph
    )
    class_call_graph: List[Tuple[JMethodDetail, JMethodDetail]] = analysis.get_class_call_graph(
        qualified_class_name="com.ibm.websphere.samples.daytrader.impl.direct.TradeDirectDBUtils"
    )

    assert class_call_graph is not None


@pytest.mark.parametrize('test_fixture', ['CLI'], indirect=['test_fixture'])
def test_get_class_call_graph_using_symbol_table(test_fixture):
    # Initialize the CLDK object with the project directory, language, and analysis_backend.
    cldk = CLDK(language="java")

    analysis = cldk.analysis(
        project_path=test_fixture,
        analysis_backend="codeanalyzer",
        analysis_json_path="../../../tests/resources/java/analysis_db",
        eager=False,
        analysis_level=AnalysisLevel.symbol_table
    )
    class_call_graph: List[Tuple[JMethodDetail, JMethodDetail]] = analysis.get_class_call_graph(
        qualified_class_name="org.apache.commons.cli.DefaultParser",
        method_name="handleConcatenatedOptions(String)",
        using_symbol_table=True
    )

    assert class_call_graph is not None
