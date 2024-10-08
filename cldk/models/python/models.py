class PyImport(BaseModel):
    from_statement: str
    imports: List[str]


class PyCallSite(BaseModel):
    method_name: str
    declaring_object: str
    arguments: List[str]
    start_line: int
    start_column: int
    end_line: int
    end_column: int


class PyClass(BaseModel):
    code_body: str
    full_signature: str
    super_classes: List[str]
    is_test_class: bool
    class_name: str = None
    methods: List[PyMethod]


class PyMethod(BaseModel):
    code_body: str
    method_name: str
    full_signature: str
    is_constructor: bool
    is_static: bool
    formal_params: List[PyArg]
    call_sites: List[PyCallSite]
    return_type: str
    class_signature: str
    start_line: int
    end_line: int


class PyModule(BaseModel):
    qualified_name: str
    functions: List[PyMethod]
    classes: List[PyClass]
    imports: List[PyImport]


class PyArg(BaseModel):
    arg_name: str
    arg_type: str


class PyBuildAttributes(BaseModel):
    """Handles all the project build tool (requirements.txt/poetry/setup.py) attributes"""

    # Name of the build file (e.g., 'setup.py', 'requirements.txt', 'pyproject.toml')
    build_file_type: str
    code_body: str
    # Build tool used (e.g., 'setuptools', 'poetry', 'pip')
    build_tool: str
    package_name: str = None
    version: str = None
    dependencies: List[str] = []
    dev_dependencies: List[str] = []
    scripts: List[str] = []


class PyConfig(BaseModel):
    """Application configuration information"""

    config_file_names: Dict[str, str]  # Name of the configuration file
    code_body: str  # The content of the configuration file
    config_type: str  # Type of config file (e.g., 'json', 'yaml', 'ini', 'python')
    settings: Dict[str, Any] = {}  # Parsed settings from the config file
    start_line: int = None
    end_line: int = None
