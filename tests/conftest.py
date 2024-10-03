import toml
import shutil
import pytest
import zipfile
from pathlib import Path
from urllib.request import urlretrieve


@pytest.fixture(scope="session", autouse=True)
def analysis_json_fixture():
    # Path to your pyproject.toml
    pyproject_path = Path(__file__).parent.parent / "pyproject.toml"

    # Load the configuration
    config = toml.load(pyproject_path)

    return config["tool"]["cldk"]["testing"]["sample-application-analysis-json"]


@pytest.fixture(scope="session", autouse=True)
def test_fixture():
    """
    Returns the path to the test data directory.

    Yields:
        Path : The path to the test data directory.
    """
    # ----------------------------------[ SETUP ]----------------------------------
    # Path to your pyproject.toml
    pyproject_path = Path(__file__).parent.parent / "pyproject.toml"

    # Load the configuration
    config = toml.load(pyproject_path)

    # Access the test data path
    test_data_path = config["tool"]["cldk"]["testing"]["sample-application"]

    if not Path(test_data_path).exists():
        Path(test_data_path).mkdir(parents=True)
    url = "https://github.com/OpenLiberty/sample.daytrader8/archive/refs/tags/v1.2.zip"
    filename = Path(test_data_path).absolute() / "v1.2.zip"
    urlretrieve(url, filename)

    # Extract the zip file to the test data path
    with zipfile.ZipFile(filename, "r") as zip_ref:
        zip_ref.extractall(test_data_path)

    # Remove the zip file
    filename.unlink()
    # --------------------------------------------------------------------------------
    # Daytrader8 sample application path
    yield Path(test_data_path) / "sample.daytrader8-1.2"

    # -----------------------------------[ TEARDOWN ]----------------------------------
    # Remove the daytrader8 sample application that was downloaded for testing
    for directory in Path(test_data_path).iterdir():
        if directory.exists() and directory.is_dir():
            shutil.rmtree(directory)
    # ---------------------------------------------------------------------------------
