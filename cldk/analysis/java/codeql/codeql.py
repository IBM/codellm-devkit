from pathlib import Path
import shlex
import subprocess
from networkx import DiGraph
from pandas import DataFrame
from cldk.models.java import JApplication
from cldk.analysis.java.codeql.backend import CodeQLQueryRunner
from tempfile import TemporaryDirectory
import atexit
import signal

from cldk.utils.exceptions import CodeQLDatabaseBuildException
import networkx as nx
from typing import Union


class JCodeQL:
    """A class for building the application view of a Java application using CodeQL.

    Parameters
    ----------
    project_dir : str or Path
        The path to the root of the Java project.
    codeql_db : str or Path or None
        The path to the CodeQL database. If None, a temporary directory is created to store the database.

    Attributes
    ----------
    db_path : Path
        The path to the CodeQL database.

    Methods
    -------
    _init_codeql_db(project_dir, codeql_db)
        Initializes the CodeQL database.
    _build_application_view()
        Builds the application view of the java application.
    _build_call_graph()
        Builds the call graph of the application.
    get_application_view()
        Returns the application view of the java application.
    get_class_hierarchy()
        Returns the class hierarchy of the java application.
    get_call_graph()
        Returns the call graph of the java application.
    get_all_methods()
        Returns all the methods of the java application.
    get_all_classes()
        Returns all the classes of the java application.
    """

    def __init__(self, project_dir: Union[str, Path], codeql_db: Union[str, Path, None]) -> None:
        self.db_path = self._init_codeql_db(project_dir, codeql_db)

    @staticmethod
    def _init_codeql_db(project_dir: Union[str, Path], codeql_db: Union[str, Path, None]) -> Path:
        """Initializes the CodeQL database.

        Parameters
        ----------
        project_dir : str or Path
            The path to the root of the Java project.
        codeql_db : str or Path or None
            The path to the CodeQL database. If None, a temporary directory is created to store the database.

        Returns
        -------
        Path
            The path to the CodeQL database.

        Raises
        ------
        CodeQLDatabaseBuildException
            If there is an error building the CodeQL database.
        """

        # Cast to Path if the project_dir is a string.
        project_dir = Path(project_dir) if isinstance(project_dir, str) else project_dir

        # Create a codeql database. Use a temporary directory if the user doesn't specify
        if codeql_db is None:
            db_path: TemporaryDirectory = TemporaryDirectory(delete=False, ignore_cleanup_errors=True)
            codeql_db = db_path.name
            # Since the user is not providing the codeql database path, we'll destroy the database at exit.
            # TODO: this may be a potential gotcha. Is there a better solution here?
            # TODO (BACKWARD COMPATIBILITY ISSUE): Only works on 3.12.
            # If necessary, use shutil to handle this differently in 3.11 and below.
            atexit.register(lambda: db_path.cleanup())
            # Also register the cleanup function for SIGINT and SIGTERM
            signal.signal(signal.SIGINT, lambda *args, **kwargs: db_path.cleanup())
            signal.signal(signal.SIGTERM, lambda *args, **kwargs: db_path.cleanup())

        codeql_db_create_cmd = shlex.split(f"codeql database create {codeql_db} --source-root={project_dir} --language=java",posix=False)
        call = subprocess.Popen(
            codeql_db_create_cmd,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
        )
        _, error = call.communicate()
        if call.returncode != 0:
            raise CodeQLDatabaseBuildException(f"Error building CodeQL database: {error.decode()}")
        return Path(codeql_db)

    def _build_application_view(self) -> JApplication:
        """
        Builds the application view of the java application.

        Returns
        -------
        JApplication
            The JApplication object representing the application view.
        """
        application: JApplication = JApplication()

        # Lets build the class hierarchy tree first and store that information in the application object.
        query = []

        # Add import
        query += ["import java"]

        # List classes and their superclasses (ignoring non-application classes and anonymous classes)
        query += [
            "from Class cls",
            "where cls.fromSource() and not cls.isAnonymous()",
            "select cls, cls.getASupertype().getQualifiedName()",
        ]

        # Execute the query using the CodeQLQueryRunner context manager
        with CodeQLQueryRunner(self.db_path) as codeql_query:
            class_superclass_pairs: DataFrame = codeql_query.execute(
                query_string="\n".join(query),
                column_names=["class", "superclass"],
            )

        application.cha = self.__process_class_hierarchy_pairs_to_tree(class_superclass_pairs)
        return application

    @staticmethod
    def __process_class_hierarchy_pairs_to_tree(
        query_result: DataFrame,
    ) -> DiGraph:
        """
        Processes the query result into a directed graph representing the class hierarchy of the application.

        Parameters
        ----------
        query_result : DataFrame
            The result of the class hierarchy query.

        Returns
        -------
        DiGraph
            A directed graph representing the class hierarchy of the application.
        """
        return nx.from_pandas_edgelist(query_result, "class", "superclass", create_using=nx.DiGraph())

    def _build_call_graph(self) -> DiGraph:
        """Builds the call graph of the application.

        Returns
        -------
        DiGraph
            A directed graph representing the call graph of the application.
        """
        query = []

        # Add import
        query += ["import java"]

        # Add Call edges between caller and callee and filter to only capture application methods.
        query += [
            "from Method caller, Method callee",
            "where",
            "caller.fromSource() and",
            "callee.fromSource() and",
            "caller.calls(callee)",
            "select",
        ]

        # Caller metadata
        query += [
            "caller.getFile().getAbsolutePath(),",
            '"[" + caller.getBody().getLocation().getStartLine() + ", " + caller.getBody().getLocation().getEndLine() + "]", //Caller body slice indices',
            "caller.getQualifiedName(), // Caller's fullsignature",
            "caller.getAModifier(), // caller's method modifier",
            "caller.paramsString(), // caller's method parameter types",
            "caller.getReturnType().toString(),  // Caller's return type",
            "caller.getDeclaringType().getQualifiedName(), // Caller's class",
            "caller.getDeclaringType().getAModifier(), // Caller's class modifier",
        ]

        # Callee metadata
        query += [
            "callee.getFile().getAbsolutePath(),",
            '"[" + callee.getBody().getLocation().getStartLine() + ", " + callee.getBody().getLocation().getEndLine() + "]", //Caller body slice indices',
            "callee.getQualifiedName(), // Caller's fullsignature",
            "callee.getAModifier(), // callee's method modifier",
            "callee.paramsString(), // callee's method parameter types",
            "callee.getReturnType().toString(),  // Caller's return type",
            "callee.getDeclaringType().getQualifiedName(), // Caller's class",
            "callee.getDeclaringType().getAModifier() // Caller's class modifier",
        ]

        query_string = "\n".join(query)

        # Execute the query using the CodeQLQueryRunner context manager
        with CodeQLQueryRunner(self.db_path) as query:
            query_result: DataFrame = query.execute(
                query_string,
                column_names=[
                    # Caller Columns
                    "caller_file",
                    "caller_body_slice_index",
                    "caller_signature",
                    "caller_modifier",
                    "caller_params",
                    "caller_return_type",
                    "caller_class_signature",
                    "caller_class_modifier",
                    # Callee Columns
                    "callee_file",
                    "callee_body_slice_index",
                    "callee_signature",
                    "callee_modifier",
                    "callee_params",
                    "callee_return_type",
                    "callee_class_signature",
                    "callee_class_modifier",
                ],
            )

        # Process the query results into JMethod instances
        callgraph: DiGraph = self.__process_call_edges_to_callgraph(query_result)
        return callable

    @staticmethod
    def __process_call_edges_to_callgraph(query_result: DataFrame) -> DiGraph:
        pass
